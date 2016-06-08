#ifndef __CAPTURE_SESSION_H__
#define __CAPTURE_SESSION_H__

#include <glib-object.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(CaptureSession, capture_config, CAPTURE, SESSION, GObject);

extern void capture_session_load(CaptureSession *session, const gchar *filename);
extern void capture_session_save(CaptureSession *session, const gchar *filename);

G_END_DECLS

#endif /* __CAPTURE_SESSION_H__ */
