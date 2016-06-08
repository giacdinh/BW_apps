/**
 *  SECTION:capture_manager
 *  @title: CaptureManager
 *  @short_description: Manage the camera and audio capture streams, request snapshots
 *  and be notified of successful captures and new output files.
 *
 *  The #CaptureManager controls audio/video captures and provides the ability
 *  to capture snapshots and insert tracepoints into the capture.
 */
#include "capture_manager.h"

#include <gst/gst.h>
#include <stdlib.h>

struct _CaptureManager {
    GObject parent_instance;

    gchar          *name;
    guint           segment_size;
    gchar          *trace_file;
    gchar          *photo_file;

    gchar          *video_device;
    gchar          *video_file;

    CaptureSize     video_size;
    CaptureMotion   video_motion;
    guint           video_framerate;
    guint           video_bitrate;

    gboolean        audio_mute;
    gfloat          audio_volume;

    /* Allow certain elements to be overridden
     * by client software.
     */
    gchar          *video_source_element;
    gchar          *video_enc_element;

    gchar          *audio_source_element;
    gchar          *audio_enc_element;

    gchar          *av_mux_element;
    gchar          *photo_enc_element;

    /*
     * GStreamer Pipeline Elements
     */
    GstElement     *pipeline;
    guint           bus_id;

    GOptionGroup   *options;
};

static void capture_manager_free_pipeline(CaptureManager*);

/**
 *  CaptureManager:
 *
 *  The #CaptureManager controls audio/video captures and provides the ability
 *  to capture snapshots and insert tracepoints into the capture.
 */
G_DEFINE_TYPE(CaptureManager, capture_manager, G_TYPE_OBJECT);
#define CAPTURE_TYPE_SIZE (capture_size_get_type())

static GType
capture_size_get_type(void)
{
    static GType capture_size_type = 0;

    if (!capture_size_type) {
        static GEnumValue capture_sizes[] = {
            { CaptureSize_QVGA,   "320x240",      "QVGA"  },
            { CaptureSize_VGA,    "640x480",      "VGA"   },
            { CaptureSize_NTSC,   "720x480",      "NTSC"  },
            { CaptureSize_PAL,    "720x576",      "PAL"   },
            { CaptureSize_720P,   "1280x720",     "720P"  },
            { CaptureSize_1080P,  "1920x1080",    "1080P" },
        };

        capture_size_type = g_enum_register_static("CaptureSize", capture_sizes);
    }

    return capture_size_type;
}

#define CAPTURE_TYPE_MOTION (capture_motion_get_type())

static GType
capture_motion_get_type(void)
{
    static GType capture_motion_type = 0;

    if (!capture_motion_type) {
        static GEnumValue capture_motions[] = {
            { CaptureMotion_Low,    "Low",      "Low"       },
            { CaptureMotion_Medium, "Medium",   "Medium"    },
            { CaptureMotion_High,   "High",     "High"      },
        };

        capture_motion_type = g_enum_register_static("CaptureMotion", capture_motions);
    }

    return capture_motion_type;
}

GQuark
capture_manager_error_quark(void)
{
    return g_quark_from_static_string("capture-manager-error-quark");
}

enum {
    PROP_NAME = 1,
    PROP_SEGMENT_SIZE,
    PROP_TRACE_FILE,
    PROP_PHOTO_FILE,
    PROP_VIDEO_DEVICE,
    PROP_VIDEO_FILE,
    PROP_VIDEO_SIZE,
    PROP_VIDEO_MOTION,
    PROP_VIDEO_FRAMERATE,
    PROP_VIDEO_BITRATE,
    PROP_AUDIO_MUTE,
    PROP_AUDIO_VOLUME,
    N_PROPERTIES
};

static void
set_property(GObject       *object,
             guint          property,
             const GValue  *value,
             GParamSpec    *spec)
{
    CaptureManager *self = CAPTURE_MANAGER(object);

    switch (property) {
    case PROP_NAME:
        g_free(self->name);
        self->name = g_value_dup_string(value);
        break;
    case PROP_SEGMENT_SIZE:
        self->segment_size = g_value_get_uint(value);
        break;
    case PROP_TRACE_FILE:
        g_free(self->trace_file);
        self->trace_file = g_value_dup_string(value);
        break;
    case PROP_PHOTO_FILE:
        g_free(self->photo_file);
        self->photo_file = g_value_dup_string(value);
        break;
    case PROP_VIDEO_DEVICE:
        g_free(self->video_device);
        self->video_device = g_value_dup_string(value);
        break;
    case PROP_VIDEO_FILE:
        g_free(self->video_file);
        self->video_file = g_value_dup_string(value);
        break;
    case PROP_VIDEO_SIZE:
        self->video_size = (CaptureSize) g_value_get_enum(value);
        break;
    case PROP_VIDEO_MOTION:
        self->video_motion = (CaptureMotion) g_value_get_enum(value);
        break;
    case PROP_VIDEO_FRAMERATE:
        self->video_framerate = g_value_get_uint(value);
        break;
    case PROP_VIDEO_BITRATE:
        self->video_bitrate = g_value_get_uint(value);
        break;
    case PROP_AUDIO_MUTE:
        self->audio_mute = g_value_get_boolean(value);
        if (self->pipeline) {
            GstElement *volume = gst_bin_get_by_name(GST_BIN(self->pipeline), "volume");
            if (volume) {
                g_object_set(volume, "mute", self->audio_mute, NULL);
                gst_object_unref(volume);
            }
        }
        break;
    case PROP_AUDIO_VOLUME:
        self->audio_volume = g_value_get_float(value);
        if (self->pipeline) {
            GstElement *volume = gst_bin_get_by_name(GST_BIN(self->pipeline), "volume");
            if (volume) {
                g_object_set(volume, "volume", self->audio_volume, NULL);
                gst_object_unref(volume);
            }
        }
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property, spec);
        break;
    }
}

static void
get_property(GObject       *object,
             guint          property,
             GValue        *value,
             GParamSpec    *spec)
{
    CaptureManager *self = CAPTURE_MANAGER(object);

    switch (property) {
    case PROP_NAME:
        g_value_set_string(value, self->name);
        break;
    case PROP_SEGMENT_SIZE:
        g_value_set_uint(value, self->segment_size);
        break;
    case PROP_TRACE_FILE:
        g_value_set_string(value, self->trace_file);
        break;
    case PROP_PHOTO_FILE:
        g_value_set_string(value, self->photo_file);
        break;
    case PROP_VIDEO_DEVICE:
        g_value_set_string(value, self->video_device);
        break;
    case PROP_VIDEO_FILE:
        g_value_set_string(value, self->video_file);
        break;
    case PROP_VIDEO_SIZE:
        g_value_set_enum(value, self->video_size);
        break;
    case PROP_VIDEO_MOTION:
        g_value_set_enum(value, self->video_motion);
        break;
    case PROP_VIDEO_FRAMERATE:
        g_value_set_uint(value, self->video_framerate);
        break;
    case PROP_VIDEO_BITRATE:
        self->video_bitrate = capture_manager_calculate_bitrate(self);
        g_value_set_uint(value, self->video_bitrate);
        break;
    case PROP_AUDIO_MUTE:
        if (self->pipeline) {
            GstElement *volume = gst_bin_get_by_name(GST_BIN(self->pipeline), "volume");
            if (volume) {
                g_object_get(volume, "mute", &self->audio_mute, NULL);
                gst_object_unref(volume);
            }
        }
        g_value_set_boolean(value, self->audio_mute);
        break;
    case PROP_AUDIO_VOLUME:
        if (self->pipeline) {
            GstElement *volume = gst_bin_get_by_name(GST_BIN(self->pipeline), "volume");
            if (volume) {
                g_object_get(volume, "volume", &self->audio_volume, NULL);
                gst_object_unref(volume);
            }
        }
        g_value_set_float(value, self->audio_volume);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property, spec);
        break;
    }
}

static void
capture_manager_class_init(CaptureManagerClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->set_property = set_property;
    object_class->get_property = get_property;

    /**
     *  CaptureManager:name:
     *
     *  Configures the object name for this instance of #CaptureManager.  This
     *  is used as the Pipeline name in capture_manager_graph() but otherwise
     *  has no affect on the object.
     */
    g_object_class_install_property(
        object_class,
        PROP_NAME,
        g_param_spec_string("name",
                            "Name",
                            "CaptureManager instance name",
                            "CaptureManager",
                            G_PARAM_READWRITE));

    /**
     *  CaptureManager:segment-size:
     *
     *  The segment size property enables splitting of the MKV file when size is
     *  greater than 0.  If non-zero segmentation will happen at or before the
     *  requested size, on a key-frame.  The size is in bytes.
     */
    g_object_class_install_property(
        object_class,
        PROP_SEGMENT_SIZE,
        g_param_spec_uint("segment-size",
                          "Segment Size",
                          "MKV file segmentation size in bytes (0 to disable segmentation)",
                          0,
                          UINT_MAX,
                          0,
                          G_PARAM_READWRITE));

    /**
     *  CaptureManager:photo-file:
     *
     *  The filename of the file used for snapshot captures.  To allow for multiple
     *  snapshots the %d sequence can be added to the filename which will cause the
     *  filename to replace %d with 0..n where n is the latest snapshot count.
     */
    g_object_class_install_property(
        object_class,
        PROP_PHOTO_FILE,
        g_param_spec_string("photo-file",
                            "Photo File",
                            "Filename of photo storage when snapshot is taken",
                            "/tmp/capture_manager_%d.jpg",
                            G_PARAM_READWRITE));

    /**
     *  CaptureManager:trace-file:
     *
     *  The location of the trace file or empty to disable trace points.
     */
    g_object_class_install_property(
        object_class,
        PROP_TRACE_FILE,
        g_param_spec_string("trace-file",
                            "Trace File",
                            "Trace point file (empty to disable)",
                            "",
                            G_PARAM_READWRITE));

    /**
     *  CaptureManager:video-device:
     *
     *  Configure the Video4Linux camera device to use for captureing.  This
     *  property needs to be set before calling capture_manager_start() otherwise
     *  it is ignored.
     */
    g_object_class_install_property(
        object_class,
        PROP_VIDEO_DEVICE,
        g_param_spec_string("video-device",
                            "Video Device",
                            "Video4Linux camera device",
                            "/dev/video0",
                            G_PARAM_READWRITE));

    /**
     *  CaptureManager:video-file:
     *
     *  The current video filename for the active stream.  When a new segment is
     *  created this property will be updated and the CaptureManager::new-segment
     *  signal will be emitted with the new filename.
     *
     *  Snapshots will be stored to a file of the same name but with _X appended
     *  to the filename where X is the current snapshot count and with the file
     *  extension changed to jpg.
     *
     *  <emphasis>
     *      Snapshot Ex: /tmp/capture.mkv -> /tmp/capture_1.jpg
     *  </emphasis>
     */
    g_object_class_install_property(
        object_class,
        PROP_VIDEO_FILE,
        g_param_spec_string("video-file",
                            "Video File",
                            "Current video file for the active stream",
                            "/tmp/capture.mkv",
                            G_PARAM_READWRITE));

    /**
     *  CaptureManager:video-size:
     *
     *  Configure the resolution of the video capture.  This property
     *  needs to be set before calling capture_manager_start() otherwise
     *  it is ignored.
     */
    g_object_class_install_property(
        object_class,
        PROP_VIDEO_SIZE,
        g_param_spec_enum("video-size",
                          "Video Size",
                          "The video capture resolution",
                          CAPTURE_TYPE_SIZE,
                          CaptureSize_NTSC,
                          G_PARAM_READWRITE));

    /**
     *  CaptureManager:video-motion:
     *
     *  Configure the expected level of motion in the video scene.  This affects
     *  the codec bitrate calculations.  Refer to CaptureManager:video-bitrate for
     *  details on how the calculation is done.  When CaptureManager:video-bitrate
     *  is non-zero this value will be ignored in favour of the manual bitrate control.
     */
    g_object_class_install_property(
        object_class,
        PROP_VIDEO_MOTION,
        g_param_spec_enum("video-motion",
                          "Video Motion",
                          "The level of motion in the video",
                          CAPTURE_TYPE_MOTION,
                          CaptureMotion_Medium,
                          G_PARAM_READWRITE));

    /**
     *  CaptureManager:video-framerate:
     *
     *  Configure the framerate of the video capture in frames per second.  This is
     *  limited by the camera sensor and it's driver's supported framerates.  The
     *  pipeline will use the GStreamer videorate plugin to adjust for sensor and
     *  sensor driver limitation.
     */
    g_object_class_install_property(
        object_class,
        PROP_VIDEO_FRAMERATE,
        g_param_spec_uint("video-framerate",
                          "Video Framerate",
                          "The number of frames per second for the video",
                          1,
                          240,
                          30,
                          G_PARAM_READWRITE));

    /**
     *  CaptureManager:video-bitrate:
     *  @see_also: #CaptureManager:video-motion
     *  @see_also: #CaptureManager:video-framerate
     *
     *  Configure the h.264 bitrate used for encoding the video in bits per second.  A 0 value
     *  will cause the #CaptureManager to automatically calculate an ideal bitrate using the
     *  Kush Gauge.  Refer to CaptureManager:video-motion to adjust the motion level used in
     *  the Kush calculations.
     *
     *  <emphasis>Note: When bitrate is 0 (kush gauge) reading this property will return the calculated
     *  bitrate and not the 0 value.</emphasis>
     *
     *  The Kush Gauge uses three motion levels low=1, medium=2, high=4 as well as the framerate
     *  and resolution to calculate a theoretical ideal bitrate.
     *
     *  <equation>
     *      <title>Kush Equation</title>
     *      <mathphrase>Width * Height * FPS * Motion * 0.07</mathphrase>
     *  </equation>
     *
     *  <table>
     *      <title>Table of Bitrates at 30FPS with Medium Motion</title>
     *      <tgroup cols='2'>
     *          <colspec colname='Size' />
     *          <colspec colname='Bitrate' />
     *          <thead>
     *              <row><entry>Size</entry><entry>Bitrate</entry></row>
     *          </thead>
     *          <tbody>
     *              <row><entry>QVGA</entry><entry>322560</entry></row>
     *              <row><entry>VGA</entry><entry>1290240</entry></row>
     *              <row><entry>NTSC</entry><entry>1451520</entry></row>
     *              <row><entry>PAL</entry><entry>1741824</entry></row>
     *              <row><entry>720P</entry><entry>3870720</entry></row>
     *              <row><entry>1080P</entry><entry>8709120</entry></row>
     *          </tbody>
     *      </tgroup>
     *  </table>
     *
     *  <emphasis>
     *  For more details refer to [Adobe h.264 Primer](http://www.adobe.com/devnet/adobe-media-server/articles/h264_primer.html).
     *  </emphasis>
     */
    g_object_class_install_property(
        object_class,
        PROP_VIDEO_BITRATE,
        g_param_spec_uint("video-bitrate",
                          "Video Bitrate",
                          "The h.264 encoding bitrate in bits per second (0 to disable compression).",
                          0,
                          UINT_MAX,
                          0,
                          G_PARAM_READWRITE));

    /**
     *  CaptureManager:audio-mute:
     *
     *  Toggle muting of the audio stream.
     */
    g_object_class_install_property(
        object_class,
        PROP_AUDIO_MUTE,
        g_param_spec_boolean("audio-mute",
                             "Audio Mute",
                             "Mute the audio recording",
                             FALSE,
                             G_PARAM_READWRITE));

    /**
     *  CaptureManager:audio-volume:
     *
     *  Control the audio volume level.  Volume represented as a normalized level where 0.0 is the minimum
     *  and 1.0 is the maximum.  A value of 0.25 would represent 25% volume or 1/4 turn on a dial.
     */
    g_object_class_install_property(
        object_class,
        PROP_AUDIO_VOLUME,
        g_param_spec_float("audio-volume",
                           "Audio Volume",
                           "Volume of the audio recording",
                           0.0f,
                           1.0f,
                           1.0f,
                           G_PARAM_READWRITE));

    /**
     *  CaptureManager::error:
     *
     *  The ::error signal is emitted whenever an asynchronous error is observed in the GStreamer
     *  pipeline.  Because much of GStreamer is run within a number of threads the functions offered
     *  by #CaptureManager are not always able to report success or failure immediately so this
     *  signal takes care of notifying the client of asynchronous errors.
     */

    /**
     *  CaptureManager::new-segment:
     *
     *  The ::new-segment signal is emitted whenever a new segment is created for the output video
     *  stream.
     */

    /**
     *  CaptureManager::new-snapshot:
     *
     *  The ::new-snapshot signal is emitted whenever a snapshot is successfully saved to disk.
     */
}

static void
capture_manager_init(CaptureManager *self)
{
    g_assert(self);

    self->name = g_strdup("CaptureManager");

    self->trace_file = g_strdup("");
    self->photo_file = g_strdup("/tmp/capture_manager_%d.jpg");

    self->video_device = g_strdup("/dev/video0");
    self->video_file = g_strdup("/tmp/capture.mkv");

    self->video_size = CaptureSize_NTSC;
    self->video_motion = CaptureMotion_Medium;
    self->video_framerate = 30;
    self->video_bitrate = capture_manager_calculate_bitrate(self);

    self->audio_mute = FALSE;
    self->audio_volume = 1.0f;

    self->video_source_element = g_strdup("imxv4l2videosrc");
    self->video_enc_element = g_strdup("imxvpuenc_h264");
    self->audio_source_element = g_strdup("alsasrc");
    self->audio_enc_element = g_strdup("imxmp3audioenc");
    self->av_mux_element = g_strdup("matroskamux");
    self->photo_enc_element = g_strdup("jpegenc");
}

static void
capture_manager_finalize(GObject *gobject)
{
    g_assert(gobject);

    CaptureManager *self = CAPTURE_MANAGER(gobject);

    capture_manager_free_pipeline(self);

    g_free(self->video_device);

    g_free(self->video_source_element);
    g_free(self->video_enc_element);
    g_free(self->audio_source_element);
    g_free(self->audio_enc_element);
    g_free(self->av_mux_element);
    g_free(self->photo_enc_element);

    G_OBJECT_CLASS(capture_manager_parent_class)->finalize(gobject);
}

static void
capture_manager_free_pipeline(CaptureManager *self)
{
    g_assert(self);

    if (self->pipeline) {
        g_source_remove(self->bus_id);
        gst_element_set_state(self->pipeline, GST_STATE_NULL);
        gst_object_unref(self->pipeline);
    }
}

static gboolean
capture_manager_bus_message(GstBus     *bus,
                            GstMessage *message,
                            gpointer    user_data)
{
    g_assert(user_data);
    CaptureManager *self = CAPTURE_MANAGER(user_data);

    switch (GST_MESSAGE_TYPE(message)) {
    case GST_MESSAGE_ERROR: {
        GError *error = NULL;
        gchar *debug = NULL;
        gchar *name = GST_MESSAGE_SRC_NAME(message);

        gst_message_parse_error(message, &error, &debug);

        g_warning("gst_bus_error (%s): %s\n", name, error->message);
        g_warning("gst_bus_debug (%s): %s\n", name, debug);

        g_free(debug);
        g_error_free(error);

        break;
    }
    case GST_MESSAGE_EOS:
        g_info("gst_bus_message: end of stream.\n");

        break;
    default:
        break;
    }

    return TRUE;
}

/**
 *  capture_manager_probe_snapshot:
 *
 *  This pad probe captures buffers coming into the photo_enc element and triggers
 *  the photo_valve to be disabled.  This is to provide the snapshot function, where
 *  we have the valve drop frames until a snapshot is requested at which point the
 *  valve is enabled for a single frame.
 *
 *  <emphasis>
 *      The valve must start enabled for GStreamer to negotiate the link.  The side
 *      effect is a photo will always be captured at startup.  This should be corrected.
 *  </emphasis>
 */
static GstPadProbeReturn
capture_manager_probe_snapshot(GstPad          *pad,
                               GstPadProbeInfo *info,
                               gpointer         user_data)
{
    g_assert(user_data);
    CaptureManager *self = CAPTURE_MANAGER(user_data);

    GstElement *photo_valve = gst_bin_get_by_name(GST_BIN(self->pipeline), "photo_valve");
    if (photo_valve) {
        g_object_set(photo_valve, "drop", TRUE, NULL);
    }

    return GST_PAD_PROBE_OK;
}

/**
 *  capture_manager_setup:
 *  @self: (in): The #CaptureManager handle.
 *
 *  Setup the internal GStreamer pipeline according to the user preferences.  If
 *  a pipeline was previously configured it will be destroyed and a new one will
 *  be built in it's place.
 *
 *  Because of the asynchronous nature of the underlying GStreamer pipeline this
 *  function is unable to report the cause of an error directly only that one has
 *  or has not occurred.  The retrieve the actual error the client application
 *  must watch the CaptureManager::error signal.
 *
 *  <emphasis>
 *      Note: it is highly improbable to recover from a failure of the setup function.
 *      This would generally point to a program error and is more of a sanity check.
 *  </emphasis>
 *
 *  Returns: TRUE on success, FALSE if an error occurred
 */
gboolean
capture_manager_setup(CaptureManager *self)
{
    g_assert(self);
    capture_manager_free_pipeline(self);

    self->pipeline = gst_pipeline_new(self->name);

    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(self->pipeline));
    self->bus_id = gst_bus_add_watch(bus, capture_manager_bus_message, self);
    gst_object_unref(bus);

    /*
     * NOTE: Failure to create or link any of these bins will cause the library to leak
     * resources as they will no be correctly freed.  This should never happen in a normal
     * workflow and this error handling is more of a sanity check as it is not feasible
     * to ever recover from such errors.
     */

    /* Video stream. */
    GstElement *video_source = gst_element_factory_make(self->video_source_element, "video_source");
    if (!gst_bin_add(GST_BIN(self->pipeline), video_source)) {
        g_warning("failed to add video_source element to pipeline");
        return FALSE;
    }

    GstElement *video_tee = gst_element_factory_make("tee", "video_tee");
    if (!gst_bin_add(GST_BIN(self->pipeline), video_tee)) {
        g_warning("failed to add video_tee element to pipeline");
        return FALSE;
    }

    GstElement *video_queue = gst_element_factory_make("queue", "video_queue");
    if (!gst_bin_add(GST_BIN(self->pipeline), video_queue)) {
        g_warning("failed to add video_queue element to pipeline");
        return FALSE;
    }

    GstElement *video_enc = gst_element_factory_make(self->video_enc_element, "video_enc");
    if (!gst_bin_add(GST_BIN(self->pipeline), video_enc)) {
        g_warning("failed to add video_enc element to pipeline");
        return FALSE;
    }

    GstElement *video_parse = gst_element_factory_make("h264parse", "video_parse");
    if (!gst_bin_add(GST_BIN(self->pipeline), video_parse)) {
        g_warning("failed to add video_parse element to pipeline");
        return FALSE;
    }

    /* Audio stream. */
    GstElement *audio_source = gst_element_factory_make(self->audio_source_element, "audio_source");
    if (!gst_bin_add(GST_BIN(self->pipeline), audio_source)) {
        g_warning("failed to add audio_source element to pipeline");
        return FALSE;
    }

    GstElement *audio_queue = gst_element_factory_make("queue", "audio_queue");
    if (!gst_bin_add(GST_BIN(self->pipeline), audio_queue)) {
        g_warning("failed to add audio_queue element to pipeline");
        return FALSE;
    }

    GstElement *volume = gst_element_factory_make("volume", "volume");
    if (!gst_bin_add(GST_BIN(self->pipeline), volume)) {
        g_warning("failed to add volume element to pipeline");
        return FALSE;
    }

    GstElement *audio_enc = gst_element_factory_make(self->audio_enc_element, "audio_enc");
    if (!gst_bin_add(GST_BIN(self->pipeline), audio_enc)) {
        g_warning("failed to add audio_enc element to pipeline");
        return FALSE;
    }

    /* A/V muxing and storage stream. */
    GstElement *av_mux = gst_element_factory_make(self->av_mux_element, "av_mux");
    if (!gst_bin_add(GST_BIN(self->pipeline), av_mux)) {
        g_warning("failed to add av_mux element to pipeline");
        return FALSE;
    }

    GstElement *av_sink = gst_element_factory_make("filesink", "av_sink");
    if (!gst_bin_add(GST_BIN(self->pipeline), av_sink)) {
        g_warning("failed to add av_sink element to pipeline");
        return FALSE;
    }

    /* Snapshot photo stream. */
    GstElement *photo_queue = gst_element_factory_make("queue", "photo_queue");
    if (!gst_bin_add(GST_BIN(self->pipeline), photo_queue)) {
        g_warning("failed to add photo_queue element to pipeline");
        return FALSE;
    }

    GstElement *photo_valve = gst_element_factory_make("valve", "photo_valve");
    if (!gst_bin_add(GST_BIN(self->pipeline), photo_valve)) {
        g_warning("failed to add photo_valve element to pipeline");
        return FALSE;
    }

    GstElement *photo_enc = gst_element_factory_make(self->photo_enc_element, "photo_enc");
    if (!gst_bin_add(GST_BIN(self->pipeline), photo_enc)) {
        g_warning("failed to add photo_enc element to pipeline");
        return FALSE;
    }

    GstElement *photo_sink = gst_element_factory_make("multifilesink", "photo_sink");
    if (!gst_bin_add(GST_BIN(self->pipeline), photo_sink)) {
        g_warning("failed to add photo_sink element to pipeline");
        return FALSE;
    }

    /*
     *  Configure Elements
     */

    /* FIXME: Assuming bitrate is set in kbps as with the imxvpuenc_h264 element */
    g_object_set(video_enc, "bitrate", self->video_bitrate / 1000, NULL);

    g_message("saving video file to: %s", self->video_file);
    g_object_set(av_sink, "location", self->video_file, NULL);

    g_object_set(photo_sink, "location", self->photo_file, NULL);

    /*
     *  Link Elements
     */
    if (!gst_element_link_many(video_source, video_tee, video_queue, video_enc, video_parse, av_mux, NULL)) {
        return FALSE;
    }

    if (!gst_element_link_many(video_tee, photo_queue, photo_valve, photo_enc, photo_sink, NULL)) {
        return FALSE;
    }

    if (!gst_element_link_many(audio_source, audio_queue, volume, audio_enc, av_mux, NULL)) {
        return FALSE;
    }

    if (!gst_element_link(av_mux, av_sink)) {
        return FALSE;
    }

    GstPad *photo_pad = gst_element_get_static_pad(photo_enc, "src");
    if (photo_pad == NULL) {
        g_warning("Failed to retrieve photo_enc src pad");
        return FALSE;
    }

    gst_pad_add_probe(photo_pad,
                      GST_PAD_PROBE_TYPE_BUFFER,
                      capture_manager_probe_snapshot,
                      self,
                      NULL);

    return TRUE;
}

/**
 *  capture_manager_start:
 *  @self: (in): The #CaptureManager handle
 *
 *  Starts the AV capture.  Once capture is active updating of the capture
 *  properties is ignored.  If the pipeline has not yet been created one
 *  will be created automatically using capture_manager_setup().
 */
void
capture_manager_start(CaptureManager *self)
{
    g_assert(self);

    if (!self->pipeline) {
        capture_manager_setup(self);
    }

    gst_element_set_state(self->pipeline, GST_STATE_PLAYING);
}

/**
 *  capture_manager_pause:
 *  @self: (in): The #CaptureManager handle
 *
 *  Pauses the AV capture.
 */
void
capture_manager_pause(CaptureManager *self)
{
    g_assert(self);
    gst_element_set_state(self->pipeline, GST_STATE_PAUSED);
}

/**
 *  capture_manager_stop:
 *  @self: (in): The #CaptureManager handle
 *
 *  Stops the AV capture.  Properties are once again available for update.
 */
void
capture_manager_stop(CaptureManager *self)
{
    g_assert(self);
    gst_element_set_state(self->pipeline, GST_STATE_NULL);
}

/**
 *  capture_manager_snapshot:
 *  @self: (in): The #CaptureManager handle
 *
 *  Captures a snapshot image of the next available frame without interrupting
 *  the video stream.  The snapshot is stored to CaptureManager:photo-file and
 *  if %d was used will automatically increment the snapshot count in the name.
 */
void
capture_manager_snapshot(CaptureManager *self)
{
    g_assert(self);

    GstElement *photo_valve = gst_bin_get_by_name(GST_BIN(self->pipeline), "photo_valve");
    if (photo_valve) {
        g_object_set(photo_valve, "drop", FALSE, NULL);
    }
}

/**
 *  capture_manager_tracepoint:
 *  @self: (in): The #CaptureManager handle.
 *
 *  Records a tracepoint event to the CaptureManager:trace-file.  On ::new-segment the
 *  trace-file is inserted into the video file's MKV metadata.
 */
void
capture_manager_tracepoint(CaptureManager *self)
{
    g_assert(self);
    g_message("Insert trace-point not yet implemented!");
}

/**
 *  capture_manager_graph:
 *  @self: (in): The #CaptureManager handle.
 *  @filename: (in): The filename where we wish to store the generated graph.
 *
 *  Generates a dot graph of the internal GStreamer pipeline used for capture, including
 *  the audio/video stream and the snapshot feature.  This can be called at any time during
 *  the execution of the pipeline to also get feedback of the current state.
 *
 *  Requires the environment variable GST_DEBUG_DUMP_DOT_DIR to be set prior to running the
 *  client application.
 *
 *  Ex: export GST_DEBUG_DUMP_DOT_DIR=/tmp
 *
 *  <emphasis>
 *      Note: If GStreamer was not built with --gst-enable-gst-debug this function will fail quietly.
 *  </emphasis>
 */
void
capture_manager_graph(CaptureManager *self, const gchar *filename)
{
    g_assert(self);
    g_return_if_fail(filename);
    gst_debug_bin_to_dot_file(GST_BIN(self->pipeline), GST_DEBUG_GRAPH_SHOW_ALL, filename);
}

/**
 *  capture_manager_bitrate:
 *  @self: (in): The #CaptureManager handle.
 *  @see_also: CaptureManager:video-size
 *  @see_also: CaptureManager:video-motion
 *  @see_also: CaptureManager:video-framerate
 *
 *  Returns: The calculated bitrate in bits-per-second or 0 if there was an error.
 *
 *  Calculates the theoretically ideal bitrate using the Kush Gauge.  The Kush Gauge uses
 *  three motion levels low=1, medium=2, high=4 as well as the framerate and resolution to
 *  calculate a theoretically ideal bitrate.
 *
 *  The only error conditions would be an invalid CaptureManager:video-size CaptureManager:video-motion
 *  or CaptureManager:video-framerate which should never happen in normal circumstances.
 *
 *  <emphasis>
 *      Ensure that the CaptureManager:video-size CaptureManager:video-motion CaptureManager:video-framerate
 *      properties are set to the desired values before calling capture_manager_calculate_bitrate().
 *      The #CaptureManager class is always initialized with sane defaults, see property documentation
 *      for further details.
 *  </emphasis>
 *
 *  <equation>
 *      <title>Kush Equation</title>
 *      <mathphrase>Width * Height * FPS * Motion * 0.07</mathphrase>
 *  </equation>
 */
guint
capture_manager_calculate_bitrate(CaptureManager *self)
{
    g_assert(self);

    guint width = 0, height = 0;
    guint motion = 0;

    switch (self->video_size) {
    case CaptureSize_QVGA:
        width = 320;
        height = 240;
        break;
    case CaptureSize_VGA:
        width = 640;
        height = 480;
        break;
    case CaptureSize_NTSC:
        width = 720;
        height = 480;
        break;
    case CaptureSize_PAL:
        width = 720;
        height = 576;
        break;
    case CaptureSize_720P:
        width = 1280;
        height = 720;
        break;
    case CaptureSize_1080P:
        width = 1920;
        height = 1080;
        break;
    }

    switch (self->video_motion) {
    case CaptureMotion_Low:
        motion = 1;
        break;
    case CaptureMotion_Medium:
        motion = 2;
        break;
    case CaptureMotion_High:
        motion = 4;
        break;
    }

    return (guint)(width * height * self->video_framerate * motion * 0.07f);
}

/**
 *  capture_manager_option_parser:
 *
 *  Provides a Glib comman line option parser.  This is used by capture_manager_option_group()
 *  to parse the available options.  Users can use either depending on how much control of
 *  parsing they required, capture_manager_option_group() is the simpler and preferred method.
 *
 *  Example Usage
 *
 *  <programlisting>
 *  <![CDATA[
 *  ]]>
 *  </programlisting>
 */
gboolean
capture_manager_option_parser(const gchar      *name,
                              const gchar      *value,
                              gpointer          data,
                              GError          **error)
{
    CaptureManager *self = CAPTURE_MANAGER(data);

    if (self == NULL) {
        g_set_error(error,
                    CAPTURE_MANAGER_ERROR,
                    CAPTURE_MANAGER_ERROR_FAILED,
                    "capture_manager_option_parser without a handle to CaptureManager");
        return FALSE;
    }

    if (g_strcmp0(name, "--segment-size") == 0 ||
        g_strcmp0(name, "-S") == 0) {
        guint segment = strtoul(value, NULL, 0);
        if (errno == EINVAL || errno == ERANGE) {
            g_set_error(error,
                        CAPTURE_MANAGER_ERROR,
                        CAPTURE_MANAGER_ERROR_INVALID_OPTION,
                        "Option segment-size %s: %s",
                        errno == EINVAL ? "invalid" : "out of range",
                        value);
            return FALSE;
        } else {
            g_object_set(self, "segment-size", segment, NULL);
        }
    } else if (g_strcmp0(name, "--trace-file") == 0 ||
               g_strcmp0(name, "-S") == 0) {
        g_object_set(self, "trace-file", value, NULL);
    } else if (g_strcmp0(name, "--video-device") == 0 ||
               g_strcmp0(name, "-d") == 0) {
        g_object_set(self, "video-device", value, NULL);
    } else if (g_strcmp0(name, "--photo-file") == 0 ||
               g_strcmp0(name, "-p") == 0) {
        g_object_set(self, "photo-file", value, NULL);
    } else if (g_strcmp0(name, "--video-file") == 0 ||
               g_strcmp0(name, "-f") == 0) {
        g_object_set(self, "video-file", value, NULL);
    } else if (g_strcmp0(name, "--video-size") == 0 ||
               g_strcmp0(name, "-s") == 0) {
        if (g_ascii_strcasecmp(value, "QVGA") == 0) {
            g_object_set(self, "video-size", CaptureSize_QVGA, NULL);
        } else if (g_ascii_strcasecmp(value, "VGA") == 0) {
            g_object_set(self, "video-size", CaptureSize_VGA, NULL);
        } else if (g_ascii_strcasecmp(value, "NTSC") == 0) {
            g_object_set(self, "video-size", CaptureSize_NTSC, NULL);
        } else if (g_ascii_strcasecmp(value, "PAL") == 0) {
            g_object_set(self, "video-size", CaptureSize_PAL, NULL);
        } else if (g_ascii_strcasecmp(value, "720P") == 0) {
            g_object_set(self, "video-size", CaptureSize_720P, NULL);
        } else if (g_ascii_strcasecmp(value, "1080P") == 0) {
            g_object_set(self, "video-size", CaptureSize_1080P, NULL);
        } else {
            g_set_error(error,
                        CAPTURE_MANAGER_ERROR,
                        CAPTURE_MANAGER_ERROR_INVALID_SIZE,
                        "Invalid video size option: %s",
                        value);
            return FALSE;
        }
    } else if (g_strcmp0(name, "--video-motion") == 0 ||
               g_strcmp0(name, "-m") == 0) {
        if (g_ascii_strcasecmp(value, "LOW") == 0) {
            g_object_set(self, "video-motion", CaptureMotion_Low, NULL);
        } else if (g_ascii_strcasecmp(value, "MEDIUM") == 0) {
            g_object_set(self, "video-motion", CaptureMotion_Medium, NULL);
        } else if (g_ascii_strcasecmp(value, "HIGH") == 0) {
            g_object_set(self, "video-motion", CaptureMotion_High, NULL);
        } else {
            g_set_error(error,
                        CAPTURE_MANAGER_ERROR,
                        CAPTURE_MANAGER_ERROR_INVALID_MOTION,
                        "Invalid video motion option: %s",
                        value);
            return FALSE;
        }
    } else if (g_strcmp0(name, "--video-framerate") == 0 ||
               g_strcmp0(name, "-r") == 0) {
        guint framerate = strtoul(value, NULL, 0);
        if (errno == EINVAL || errno == ERANGE) {
            g_set_error(error,
                        CAPTURE_MANAGER_ERROR,
                        CAPTURE_MANAGER_ERROR_INVALID_OPTION,
                        "Option video-framerate %s: %s",
                        errno == EINVAL ? "invalid" : "out of range",
                        value);
            return FALSE;
        } else {
            g_object_set(self, "video-framerate", framerate, NULL);
        }
    } else if (g_strcmp0(name, "--video-bitrate") == 0 ||
               g_strcmp0(name, "-b") == 0) {
        guint bitrate = strtoul(value, NULL, 0);
        if (errno == EINVAL || errno == ERANGE) {
            g_set_error(error,
                        CAPTURE_MANAGER_ERROR,
                        CAPTURE_MANAGER_ERROR_INVALID_OPTION,
                        "Option video-bitrate %s: %s",
                        errno == EINVAL ? "invalid" : "out of range",
                        value);
            return FALSE;
        } else {
            g_object_set(self, "video-bitrate", bitrate, NULL);
        }
    } else if (g_strcmp0(name, "--audio-mute") == 0 ||
               g_strcmp0(name, "-M") == 0) {
        if (g_ascii_strcasecmp(value, "on") == 0 ||
            g_ascii_strcasecmp(value, "true") == 0 ||
            g_ascii_strcasecmp(value, "mute") == 0) {
            g_object_set(self, "audio-mute", TRUE, NULL);
        } else if (g_ascii_strcasecmp(value, "off") == 0 ||
                   g_ascii_strcasecmp(value, "false") == 0 ||
                   g_ascii_strcasecmp(value, "unmute") == 0) {
            g_object_set(self, "audio-mute", FALSE, NULL);
        } else {
            g_set_error(error,
                        CAPTURE_MANAGER_ERROR,
                        CAPTURE_MANAGER_ERROR_INVALID_OPTION,
                        "Option audio-mute has invalid state: %s",
                        value);
            return FALSE;
        }
    } else if (g_strcmp0(name, "--audio-volume") == 0 ||
               g_strcmp0(name, "-v") == 0) {
        float volume = strtof(value, NULL);
        if (errno == EINVAL || errno == ERANGE) {
            g_set_error(error,
                        CAPTURE_MANAGER_ERROR,
                        CAPTURE_MANAGER_ERROR_INVALID_OPTION,
                        "Option audio-volume %s: %s",
                        errno == EINVAL ? "invalid" : "out of range",
                        value);
            return FALSE;
        } else if (volume < 0.0f || volume > 1.0f) {
            g_set_error(error,
                        CAPTURE_MANAGER_ERROR,
                        CAPTURE_MANAGER_ERROR_INVALID_OPTION,
                        "Option audio-volume is out of range 0 - 1: %s",
                        value);
        } else {
            g_object_set(self, "audio-volume", volume, NULL);
        }
    } else {
        g_set_error(error,
                    CAPTURE_MANAGER_ERROR,
                    CAPTURE_MANAGER_ERROR_UNKNOWN_OPTION,
                    "Unknown command line option: %s",
                    name);
        return FALSE;
    }

    return TRUE;
}

/**
 *  capture_manager_option_group:
 *  @self: (in): The #CaptureManager handle
 *
 *  Provides a Glib command line option group for parsing the various properties
 *  offered by the class.  This can be used by client applications to have the
 *  library provide all available command line options.
 *
 *  Example Usage
 *
 *  <programlisting>
 *  <![CDATA[
 *  ]]>
 *  </programlisting>
 */
extern GOptionGroup*
capture_manager_option_group(CaptureManager *self)
{
    g_assert(self);

    static GOptionEntry capture_manager_options[] = {
        {
            "segment-size",
            'S',
            G_OPTION_FLAG_OPTIONAL_ARG,
            G_OPTION_ARG_CALLBACK,
            capture_manager_option_parser,
            "Split the video every SIZE bytes or less, on a keyframe.  (0 to disable segmentation)",
            "SIZE",
        },
        {
            "trace-file",
            't',
            G_OPTION_FLAG_OPTIONAL_ARG,
            G_OPTION_ARG_CALLBACK,
            capture_manager_option_parser,
            "Store trace-points to FILE",
            "FILE",
        },
        {
            "photo-file",
            'p',
            G_OPTION_FLAG_OPTIONAL_ARG,
            G_OPTION_ARG_CALLBACK,
            capture_manager_option_parser,
            "Store photo snapshots to FILE",
            "FILE",
        },
        {
            "video-device",
            'd',
            G_OPTION_FLAG_OPTIONAL_ARG | G_OPTION_FLAG_FILENAME,
            G_OPTION_ARG_CALLBACK,
            capture_manager_option_parser,
            "Use DEVICE as the Video4Linux capture source",
            "DEVICE",
        },
        {
            "video-file",
            'f',
            G_OPTION_FLAG_OPTIONAL_ARG | G_OPTION_FLAG_FILENAME,
            G_OPTION_ARG_CALLBACK,
            capture_manager_option_parser,
            "Save captured video to FILENAME",
            "FILENAME",
        },
        {
            "video-size",
            's',
            G_OPTION_FLAG_OPTIONAL_ARG | G_OPTION_FLAG_FILENAME,
            G_OPTION_ARG_CALLBACK,
            capture_manager_option_parser,
            "Capture video at SIZE (QVGA, VGA, NTSC, PAL, 720P, 1080P)",
            "SIZE",
        },
        {
            "video-motion",
            'm',
            G_OPTION_FLAG_OPTIONAL_ARG | G_OPTION_FLAG_FILENAME,
            G_OPTION_ARG_CALLBACK,
            capture_manager_option_parser,
            "Use MOTION level for auto bitrate setup (Low, Medium, High)",
            "MOTION",
        },
        {
            "video-framerate",
            'r',
            G_OPTION_FLAG_OPTIONAL_ARG | G_OPTION_FLAG_FILENAME,
            G_OPTION_ARG_CALLBACK,
            capture_manager_option_parser,
            "Capture video at FPS frames per second",
            "FPS",
        },
        {
            "video-bitrate",
            'b',
            G_OPTION_FLAG_OPTIONAL_ARG | G_OPTION_FLAG_FILENAME,
            G_OPTION_ARG_CALLBACK,
            capture_manager_option_parser,
            "Use manual BITRATE bits per second codec setting",
            "BITRATE",
        },
        {
            "audio-mute",
            'M',
            G_OPTION_FLAG_OPTIONAL_ARG | G_OPTION_FLAG_FILENAME,
            G_OPTION_ARG_CALLBACK,
            capture_manager_option_parser,
            "Toggle MUTE on or off",
            "MUTE",
        },
        {
            "audio-volume",
            'v',
            G_OPTION_FLAG_OPTIONAL_ARG | G_OPTION_FLAG_FILENAME,
            G_OPTION_ARG_CALLBACK,
            capture_manager_option_parser,
            "Set audio recording VOLUME",
            "VOLUME",
        },
        { NULL },
    };

    if (!self->options) {
        self->options = g_option_group_new("capture-manager",
                                           "CaptureManager Options",
                                           "CaptureManager Options",
                                           self,
                                           NULL);
        g_option_group_add_entries(self->options, capture_manager_options);
    }

    return self->options;
}
