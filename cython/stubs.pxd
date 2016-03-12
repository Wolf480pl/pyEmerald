cdef extern from "gtk/gtk.h" nogil:
    ctypedef struct GdkPixbuf:
        pass
    ctypedef struct GdkPixmap:
        pass
    ctypedef struct GtkWidget:
        pass
    ctypedef struct GdkGC:
        pass

cdef extern from "pango/pangocairo.h" nogil:
    ctypedef enum PangoAlignment:
        PANGO_ALIGN_LEFT
        PANGO_ALIGN_CENTER
        PANGO_ALIGN_RIGHT
    ctypedef struct PangoContext:
        pass
    ctypedef struct PangoFontDescription:
        pass
    ctypedef struct PangoLayout:
        pass

cdef extern from "cairo.h" nogil:
    ctypedef struct cairo_pattern_t:
        pass
    ctypedef struct cairo_t:
        pass

cdef extern from "X11/Xlib.h" nogil:
    ctypedef unsigned long XID
    ctypedef XID Window

cdef extern from "decoration.h" nogil:
    ctypedef struct decor_extents_t:
        int left
        int right
        int top
        int bottom
