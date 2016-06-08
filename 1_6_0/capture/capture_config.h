#ifndef __CAPTURE_CONFIG_H__
#define __CAPTURE_CONFIG_H__

#include <glib-object.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(CaptureConfig, capture_config, CAPTURE, CONFIG, GObject);

extern void capture_config_load(CaptureConfig *capture_config, const gchar *filename);
extern void capture_config_save(CaptureConfig *capture_config, const gchar *filename);

G_END_DECLS

#endif /* __CAPTURE_CONFIG_H__ */
