from cpython.object cimport *
from stubs cimport *

cdef extern from "py3cairo.h" nogil:
    ctypedef struct PycairoContext:
        PyObject_HEAD cairo_t *ctx
        PyObject *base
    ctypedef struct PycairoFontFace:
        PyObject_HEAD cairo_font_face_t *font_face
    ctypedef struct PycairoFontOptions:
        PyObject_HEAD cairo_font_options_t *font_options
    ctypedef struct PycairoMatrix:
        PyObject_HEAD cairo_matrix_t matrix
    ctypedef struct PycairoPath:
        PyObject_HEAD cairo_path_t *path
    ctypedef struct PycairoPattern:
        PyObject_HEAD cairo_pattern_t *pattern
        PyObject *base
    ctypedef struct PycairoScaledFont:
        PyObject_HEAD cairo_scaled_font_t *scaled_font
    ctypedef struct PycairoSurface:
        PyObject_HEAD cairo_surface_t *surface
        PyObject *base
    ctypedef struct Pycairo_CAPI_t:
        PyTypeObject *Context_Type
        PyObject *(*Context_FromContext)(cairo_t *ctx, PyTypeObject *type, PyObject *base)
        PyTypeObject *FontFace_Type
        PyTypeObject *ToyFontFace_Type
        PyObject *(*FontFace_FromFontFace)(cairo_font_face_t *font_face)
        PyTypeObject *FontOptions_Type
        PyObject *(*FontOptions_FromFontOptions)(cairo_font_options_t *font_options)
        PyTypeObject *Matrix_Type
        PyObject *(*Matrix_FromMatrix)(cairo_matrix_t *matrix)
        PyTypeObject *Path_Type
        PyObject *(*Path_FromPath)(cairo_path_t *path)
        PyTypeObject *Pattern_Type
        PyTypeObject *SolidPattern_Type
        PyTypeObject *SurfacePattern_Type
        PyTypeObject *Gradient_Type
        PyTypeObject *LinearGradient_Type
        PyTypeObject *RadialGradient_Type
        PyObject *(*Pattern_FromPattern)(cairo_pattern_t *pattern, PyObject *base)
        PyTypeObject *ScaledFont_Type
        PyObject *(*ScaledFont_FromScaledFont)(cairo_scaled_font_t *scaled_font)
        PyTypeObject *Surface_Type
        PyTypeObject *ImageSurface_Type
        PyTypeObject *PDFSurface_Type
        PyTypeObject *PSSurface_Type
        PyTypeObject *SVGSurface_Type
        PyTypeObject *Win32Surface_Type
        PyTypeObject *Win32PrintingSurface_Type
        PyTypeObject *XCBSurface_Type
        PyTypeObject *XlibSurface_Type
        PyObject *(*Surface_FromSurface)(cairo_surface_t *surface, PyObject *base)
        int (*Check_Status)(cairo_status_t status)
    Pycairo_CAPI_t *Pycairo_CAPI
    int import_cairo()
