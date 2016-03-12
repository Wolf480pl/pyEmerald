#ifndef WRAPUTILS_H_
#define WRAPUTILS_H_

#include <py3cairo.h>

void print_python_error(const char* desc);

PyObject* wrap_cairo(cairo_t *cr);
PyObject* wrap_cairo_pattern(cairo_pattern_t* pattern);
PyObject* wrap_gobject(GObject *gobj);
PyObject* wrap_fontdesc(PangoFontDescription *fontDesc, gboolean copy);

Pycairo_CAPI_t *PycairoAPI;

#endif /* WRAPUTILS_H_ */
