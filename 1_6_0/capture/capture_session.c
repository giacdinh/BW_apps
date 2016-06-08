#include "capture_session.h"

struct _CaptureSession {
    GObject parent_instance;
};

G_DEFINE_TYPE(CaptureSession, capture_session, G_TYPE_OBJECT);

static void
capture_session_class_init(CaptureSessionClass *klass)
{

}

static void
capture_session_init(CaptureSession *self)
{

}