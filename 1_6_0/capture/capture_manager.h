#ifndef __CAPTURE_MANAGER_H__
#define __CAPTURE_MANAGER_H__

#include <glib-object.h>

G_BEGIN_DECLS

/**
 *  CaptureSize:
 *  @CaptureSize_QVGA:  QVGA Resolution 320x240
 *  @CaptureSize_VGA:   VGA Resolution 640x380
 *  @CaptureSize_NTSC:  NTSC Resolution 720x480
 *  @CaptureSize_PAL:   PAL Resolution 720x576
 *  @CaptureSize_720P:  720P Resolution 1280x720
 *  @CaptureSize_1080P: 1080P Resolution 1920x1080
 *
 *  The #CaptureSize enum provides standard resolution definitions for capturing
 *  video from the camera.  Not all resolutions are guaranteed to be supported by
 *  the underlying camera.
 *
 *  FIXME:  Currently selecting a size which the camera does not support has undefined behaviour.
 */
typedef enum {
    CaptureSize_QVGA = 0,
    CaptureSize_VGA,
    CaptureSize_NTSC,
    CaptureSize_PAL,
    CaptureSize_720P,
    CaptureSize_1080P,
} CaptureSize;

/**
 *  CaptureMotion:
 *  @CaptureMotion_Low:     Low scene motion
 *  @CaptureMotion_Medium:  Medium scene motion
 *  @CaptureMotion_High:    High scene motion
 *
 *  The approximate level of motion in the video scene is used to automatically adjust
 *  the codec stream bitrate for a good balance between quality and level of compression.
 */
typedef enum {
    CaptureMotion_Low = 1,
    CaptureMotion_Medium = 2,
    CaptureMotion_High = 4,
} CaptureMotion;

/**
 *  CaptureManagerError:
 *  @CAPTURE_MANAGER_ERROR_SUCCESS:         Success
 *  @CAPTURE_MANAGER_ERROR_FAILED:          Generic failure
 *  @CAPTURE_MANAGER_ERROR_UNKNOWN_OPTION:  Command line option is unknown
 *  @CAPTURE_MANAGER_ERROR_INVALID_OPTION:  Command line option is invalid
 *  @CAPTURE_MANAGER_ERROR_INVALID_SIZE:    Invalid capture size
 *  @CAPTURE_MANAGER_ERROR_INVALID_MOTION:  Invalid capture motion
 *
 *  #GError definitions for #CaptureManager.
 */
typedef enum {
    CAPTURE_MANAGER_ERROR_SUCCESS = 0,
    CAPTURE_MANAGER_ERROR_FAILED,
    CAPTURE_MANAGER_ERROR_UNKNOWN_OPTION,
    CAPTURE_MANAGER_ERROR_INVALID_OPTION,
    CAPTURE_MANAGER_ERROR_INVALID_SIZE,
    CAPTURE_MANAGER_ERROR_INVALID_MOTION,
} CaptureManagerError;

#define CAPTURE_TYPE_MANAGER capture_manager_get_type()
G_DECLARE_FINAL_TYPE(CaptureManager, capture_manager, CAPTURE, MANAGER, GObject);

#define CAPTURE_MANAGER_ERROR capture_manager_error_quark()
extern GQuark capture_manager_error_quark(void);

extern gboolean capture_manager_setup(CaptureManager *self);
extern void capture_manager_start(CaptureManager *self);
extern void capture_manager_pause(CaptureManager *self);
extern void capture_manager_stop(CaptureManager *self);
extern void capture_manager_snapshot(CaptureManager *self);
extern void capture_manager_graph(CaptureManager *self, const gchar *filename);
extern guint capture_manager_calculate_bitrate(CaptureManager *self);
extern GOptionGroup* capture_manager_option_group(CaptureManager *self);
extern gboolean capture_manager_option_parser(const gchar      *name,
                                              const gchar      *value,
                                              gpointer          data,
                                              GError          **error);
G_END_DECLS

#endif /* __CAPTURE_MANAGER_H__ */
