from stubs cimport *

cdef extern from "wraputils.h" nogil:
    cdef object wrap_cairo(cairo_t *cr)
    cdef object wrap_cairo_patter(cairo_pattern_t *pattern)
    cdef object wrap_gobject(GObject* gobj)
    cdef object wrap_fontdesc(PangoFontDescription* fontdesc)
