#include "capture_config.h"

struct _CaptureConfig {
    GObject parent_instance;
};

G_DEFINE_TYPE(CaptureConfig, capture_config, G_TYPE_OBJECT);

static void
capture_config_class_init(CaptureConfigClass *klass)
{

}

static void
capture_config_init(CaptureConfig *self)
{

}