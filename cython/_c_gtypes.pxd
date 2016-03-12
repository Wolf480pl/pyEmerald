cdef extern from "glib.h" nogil:
    ctypedef char   gchar
    ctypedef short  gshort
    ctypedef long   glong
    ctypedef int    gint
    ctypedef gint   gboolean
    ctypedef unsigned char   guchar
    ctypedef unsigned short  gushort
    ctypedef unsigned long   gulong
    ctypedef unsigned int    guint
    ctypedef float   gfloat
    ctypedef double  gdouble
    ctypedef void* gpointer
    ctypedef const void *gconstpointer
