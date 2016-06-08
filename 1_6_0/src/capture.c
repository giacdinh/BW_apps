#include <stdio.h>

#include <gst/gst.h>
#include <glib-unix.h>

#include "capture_manager.h"
#include "capture_session.h"
#include "capture_config.h"

#include "odi-config.h"

#define CAPTURE_LOG_DOMAIN "L3Capture"
#define CAPTURE_LOG_LOCATION "/odi/log/gst_capture.log"

/**
 *  log_handler:
 *
 *  Callback used to retrieve all L3 domain log messages and save
 *  them to the disk.
 */
void log_handler(const gchar       *domain,
                 GLogLevelFlags     level,
                 const gchar       *message,
                 gpointer           user_data)
{
    FILE* logfile = (FILE*) user_data;
    if (!logfile) logfile = stderr;
    fputs(message, logfile);
    fputs("\n", logfile);
}

gboolean toggle_pause(gpointer user_data)
{
    static gboolean paused = FALSE;
    g_assert(user_data);
    CaptureManager *manager = CAPTURE_MANAGER(user_data);

    if (paused) {
        paused = FALSE;
        capture_manager_start(manager);
    } else {
        paused = TRUE;
        capture_manager_pause(manager);
    }

    return TRUE;
}

gboolean snapshot(gpointer user_data)
{
    g_assert(user_data);
    CaptureManager *manager = CAPTURE_MANAGER(user_data);
    capture_manager_snapshot(manager);
    return TRUE;
}

int main(int argc, char** argv)
{
    GError *error = NULL;
    GOptionContext *context;

    GMainLoop *main_loop = g_main_loop_new(NULL, FALSE);
    CaptureManager *manager = g_object_new(CAPTURE_TYPE_MANAGER, NULL);

    context = g_option_context_new("- capture audio/video streams and snapshots.");
    g_option_context_set_main_group(context, capture_manager_option_group(manager));
    g_option_context_add_group(context, gst_init_get_option_group());

    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_error("invalid command option: %s", error->message);
        return 1;
    }
    g_option_context_free(context);

    gchar *filename;
    g_object_get(manager, "video-file", &filename, NULL);

    FILE* current = fopen(CURRENT_FILE, "w");
    if (current) {
        fputs(filename, current);
        fclose(current);
    }

    g_unix_signal_add(SIGUSR1, toggle_pause, manager);
    g_unix_signal_add(SIGUSR2, snapshot, manager);

    /**
     * If the environment variable GST_DEBUG_DUMP_DOT_DIR is set to a valid
     * directory then the GStreamer pipeline graph will be generated as
     * capture.dot in the provided directory.  If the variable is not set
     * then no graph will be generated.
     */
    capture_manager_setup(manager);
    capture_manager_graph(manager, "capture");
    capture_manager_start(manager);

    g_main_loop_run(main_loop);
    g_object_unref(manager);

    return 0;
}
