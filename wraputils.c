
#include <Python.h> // Python.h should be always before any system headers

#include <py3cairo.h>
#include <pygobject.h>
#include <pango/pangocairo.h>

#include "wraputils.h"

void print_python_error(const char* desc) {
    fprintf(stderr, "%s\n", desc);
    if (PyErr_Occurred()) {
        PyErr_Print();
    }
}


PyObject* wrap_cairo(cairo_t *cr) {
    return PycairoAPI->Context_FromContext(cairo_reference(cr), PycairoAPI->Context_Type, NULL);
}

PyObject* wrap_cairo_pattern(cairo_pattern_t* pattern) {
    PyObject* base = NULL;
    if (cairo_pattern_get_type(pattern) == CAIRO_PATTERN_TYPE_SURFACE) {
        cairo_surface_t* surface;
        cairo_pattern_get_surface(pattern, &surface);
        cairo_surface_reference(surface);
        base = PycairoSurface_FromSurface(surface, NULL);
        if (!base) {
            print_python_error("Couldn't wrap cairo surface");
        }
    }
    cairo_pattern_reference(pattern);
    PyObject* ret = PycairoPattern_FromPattern(pattern, base);
    if (!ret) {
            print_python_error("Couldn't wrap cairo pattern");
    }
    Py_XDECREF(base);
    return ret;
}

PyObject* wrap_gobject(GObject *gobj) {
    return pygobject_new(gobj);
}

PyObject* wrap_fontdesc(PangoFontDescription *fontDesc, gboolean copy) {
    return pyg_boxed_new(pango_font_description_get_type(), fontDesc, copy, copy);
}
