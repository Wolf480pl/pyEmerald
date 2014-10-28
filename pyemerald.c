/*
 * Python theme engine
 *
 * python.c
 *
 * Copyright (C) 2006 Quinn Storm <livinglatexkali@gmail.com> (original line theme engine)
 * Copyright (C) 2007 Patrick Niklaus <patrick.niklaus@googlemail.com> (original line theme engine)
 * Copyright (C) 2014 Wolf480pl <wolf480@interia.pl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <Python.h> // Python.h should be always before any system headers

#include <emerald.h>
#include <engine.h>

#include <py3cairo.h>
#include <pygobject.h>

#include "pixmap_icon.h"

#include <config.h>

#define SECT "pyemerald_settings"

#define LOCAL_SCRIPT_DIR g_get_home_dir(),".emerald/engines",SCRIPT_SUBDIR
#define SCRIPT_NAME "emerald"
#define SCRIPT_FILE SCRIPT_NAME".py"

//#define USE_PY_STRUCTSEQ

/*
 * color privates
 */
typedef struct _private_fs
{
    alpha_color border;
    alpha_color title_bar;
    PyObject* python_fs;
} private_fs;

/*
 * python wrapping privates
 */
typedef struct _wrappers
{
#ifdef USE_PY_STRUCTSEQ
    PyTypeObject* decor;
#else
    PyObject* decor;
    PyObject* frame_settings;
    PyObject* button_region;
    PyObject* rectangle;
    PyObject* fade_info;
    PyObject* window_settings;
#endif // USE_PY_STRUCTSEQ
} wrappers;

typedef struct _private_py
{
    PyObject* module;
    PyObject* draw;
    PyObject* init;
    wrappers* wrs;
} private_py;


/*
 * settings privates
 */
typedef struct _private_ws
{
    private_py* py;
    wrappers* wrs;
    PyObject* python_ws;
} private_ws;

void get_meta_info (EngineMetaInfo * emi)
{
    emi->version = g_strdup("0.1");
    emi->description = g_strdup(_("Wrapper for python scripts"));
    emi->last_compat = g_strdup("0.0"); // old themes still compatible
    emi->icon = gdk_pixbuf_new_from_inline(-1, my_pixbuf, TRUE, NULL);
}

static void print_python_error(const char* desc) {
    fprintf(stderr, "%s\n", desc);
    if (PyErr_Occurred()) {
        PyErr_Print();
    }
}

static PyObject* get_python_func(PyObject* module, char* name) {

    PyObject* pFunc = PyObject_GetAttrString(module, name);

    if (!pFunc) {
        print_python_error("Missing module function");
        return NULL;
    }
    if (!PyCallable_Check(pFunc)) {
        fprintf(stderr, "Module has non-callable attribute where function was expected\n");
        return NULL;
    }

    return pFunc;
}

#ifdef USE_PY_STRUCTSEQ
static PyStructSequence_Field decor_fields[] = {
    {"event_windows", ""},
    {"button_windows", ""},
    {"button_states", ""},
    {"tobj_pos",""},
    {"tobj_size",""},
    {"tobj_item_pos",""},
    {"tobj_item_state",""},
    {"tobj_item_width",""},
    NULL
};

static PyStructSequence_Desc decor_desc = {
    "Decor_T",    /* name */
    "",           /* doc */
    decor_fields, /* fields*/
    8             /* n_in_sequence */
};
#endif // USE_PY_STRUCTSEQ

static wrappers* init_wrappers(private_ws* pws) {
    wrappers* wrs;
    wrs = malloc(sizeof(wrappers));
    bzero(wrs, sizeof(wrs));

#ifdef USE_PY_STRUCTSEQ
    wrs->decor = PyStructSequence_NewType(&decor_desc);
    if (!wrs->decor) {
        print_python_error("Couldn't create structseq type for decor");
    }
#else
    private_py* py = pws->py;

    wrs->decor = get_python_func(py->module, "DecorT");
    if (!wrs->decor) {
        print_python_error("No DecorT in the python module");
        free(wrs);
        return NULL;
    }
    wrs->frame_settings = get_python_func(py->module, "FrameSettings");
    if (!wrs->frame_settings) {
        print_python_error("No FrameSettings in the python module");
        Py_DECREF(wrs->decor);
        free(wrs);
        return NULL;
    }
    wrs->button_region = get_python_func(py->module, "ButtonRegion");
    if (!wrs->button_region) {
        print_python_error("No ButtonRegion in the python module");
        Py_DECREF(wrs->frame_settings);
        Py_DECREF(wrs->decor);
        free(wrs);
        return NULL;
    }
    wrs->rectangle = get_python_func(py->module, "_wrap_rectangle");
    if (!wrs->rectangle) {
        print_python_error("No _wrap_rectangle in the python module");
        Py_DECREF(wrs->button_region);
        Py_DECREF(wrs->frame_settings);
        Py_DECREF(wrs->decor);
        free(wrs);
        return NULL;
    }
    wrs->fade_info = get_python_func(py->module, "ButtonFadeInfo");
    if (!wrs->fade_info) {
        print_python_error("No ButtonFadeInfo in the python module");
        Py_DECREF(wrs->rectangle);
        Py_DECREF(wrs->button_region);
        Py_DECREF(wrs->frame_settings);
        Py_DECREF(wrs->decor);
        free(wrs);
        return NULL;
    }
    wrs->window_settings = get_python_func(py->module, "_wrap_windowsettings");
    if (!wrs->window_settings) {
        print_python_error("No _wrap_windowsettings in the python module");
        Py_DECREF(wrs->fade_info);
        Py_DECREF(wrs->rectangle);
        Py_DECREF(wrs->button_region);
        Py_DECREF(wrs->frame_settings);
        Py_DECREF(wrs->decor);
        free(wrs);
        return NULL;
    }
#endif // USE_PY_STRUCTSEQ

    return wrs;
}

/*
static PyObject* eval_python(char* expr) {
    PyCodeObject* code = (PyCodeObject*) Py_CompileString(expr, "eval", Py_eval_input);
    PyObject* main_module = PyImport_AddModule("__main__");
    PyObject* global_dict = PyModule_GetDict(main_module);
    return PyEval_EvalCode(code, global_dict, global_dict);
}
*/

static void fallback_draw_frame(decor_t* d, cairo_t* cr) {
    frame_settings *fs = d->fs;
    window_settings *ws = fs->ws;
    gboolean active = d->active;

    double x1, y1, x2, y2;

    x1 = ws->left_space - ws->win_extents.left;
    y1 = ws->top_space - ws->win_extents.top;
    x2 = d->width  - ws->right_space  + ws->win_extents.right;
    y2 = d->height - ws->bottom_space + ws->win_extents.bottom;
    int top;
    top = ws->win_extents.top + ws->titlebar_height;

    double m1 = MIN(ws->win_extents.left, ws->win_extents.right);
    double m2 = MIN(ws->win_extents.top,  ws->win_extents.bottom);

    double border_width = MIN(m1, m2);
    double border_offset = border_width/2.0;

    cairo_set_line_width (cr, border_width);

    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);

    rounded_rectangle (cr,
            x1 + border_offset,
            y1 + top - border_offset,
            x2 - x1 - border_width,
            y2 - y1 - top,
            0, NULL, 0);
    if (active) {
        cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
    } else {
        cairo_set_source_rgba(cr, 0.7, 0.7, 0.0, 1.0);
    }
    cairo_stroke (cr);

    // title bar
    rounded_rectangle (cr,
            x1,
            y1,
            x2 - x1,
            top,
            0, NULL, 0);
    if (active) {
        cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
    } else {
        cairo_set_source_rgba(cr, 0.7, 0.0, 0.0, 1.0);
    }
    cairo_fill(cr);
}

#define NEW_NONE (Py_INCREF(Py_None), Py_None)
#define NULL2NONE(var, wrap) var ? wrap : NEW_NONE

#define ACOLOR2TUPLE(acolor) Py_BuildValue("(dddd)", acolor.color.r, acolor.color.g, acolor.color.b, acolor.alpha)

#define ARRAY2TUPLE_BEGIN(tuple, n) \
        PyObject* tuple = PyTuple_New(n); \
        if (!tuple) { \
            print_python_error("Couldn't create tuple"); \
        } else {\
            int i; \
            gboolean error = FALSE; \
            for (i = 0; i < n; ++i) {

#define ARRAY2TUPLE_END(tuple, element) \
                PyTuple_SET_ITEM(tuple, i, element); \
            }\
            if (error) {\
                Py_DECREF(tuple); \
                tuple = NULL; \
            } \
        }

#define ARRAY2TUPLE(tuple, n, fun, desc) \
        ARRAY2TUPLE_BEGIN(tuple, n) \
            PyObject* element = fun ; \
            if (!element) { \
                print_python_error(desc); \
                element = NEW_NONE; \
                error = TRUE; \
            } \
        ARRAY2TUPLE_END(tuple, element)


#define ARRAY2TUPLE_FLAT(tuple, buildstr, array, n, desc) \
        ARRAY2TUPLE_BEGIN(tuple, n) \
            PyObject* element = Py_BuildValue(buildstr, array[i]); \
            if (!element) { \
                print_python_error(desc); \
            } \
        ARRAY2TUPLE_END(tuple, element)

#define ARRAY2TUPLE_GOBJ(tuple, array, n) \
        ARRAY2TUPLE(tuple, n, pygobject_new((GObject*) array[i]), "Couldn't wrap GObject")

//For use in Py_BuildValue
//#define DICT_ELEM(name) #name , name

//FIXME: don't just return NULL! what about print_python_error? decref?
#define DICT_ADD_ELEM(dict, elem) \
        if (PyDict_SetItemString(dict, #elem, elem) < 0) { \
            print_python_error("Couldn't add element to dictionary"); \
            Py_DECREF(elem); \
            Py_DECREF(dict); \
            return NULL; \
        }

#define DICT_ADD_ELEMX(dict, elem) \
        if (!elem) { \
            Py_DECREF(dict); \
            return NULL; \
        } \
        DICT_ADD_ELEM(dict, elem) \
        Py_DECREF(elem); \

#define DICT_ADD_ELEMXE(dict, elem, desc) \
        if (!elem) { \
            print_python_error(desc); \
            Py_DECREF(dict); \
            return NULL; \
        } \
        DICT_ADD_ELEM(dict, elem) \
        Py_DECREF(elem);


#define DICT_ADD_ARRAY(dict, elem, n, fun, desc) \
        ARRAY2TUPLE(elem, n, fun, desc) \
        DICT_ADD_ELEMX(dict, elem)

#define DICT_ADD_ARRAY_FLAT(dict, elem, buildstr, array, n, desc) \
        ARRAY2TUPLE_FLAT(elem, buildstr, array, n, desc) \
        DICT_ADD_ELEMX(dict, elem)

#define DICT_ADD_ARRAY_GOBJ(dict, elem, array, n) \
        ARRAY2TUPLE_GOBJ(elem, array, n) \
        DICT_ADD_ELEMX(dict, elem)

#define DICT_ADD_GOBJECT(dict, elem, obj) \
        PyObject* elem = pygobject_new((GObject*) obj); \
        DICT_ADD_ELEMXE(dict, elem, "Couldn't wrap GObject")

#define DICT_ADD_INT(dict, elem, int) \
        PyObject* elem = Py_BuildValue("i", int); \
        DICT_ADD_ELEMXE(dict, elem, "Couldn't wrap int")

#define DICT_ADD_UINT(dict, elem, int) \
        PyObject* elem = Py_BuildValue("I", int); \
        DICT_ADD_ELEMXE(dict, elem, "Couldn't wrap uint")

#define DICT_ADD_LONG(dict, elem, long) \
        PyObject* elem = Py_BuildValue("k", long); \
        DICT_ADD_ELEMXE(dict, elem, "Couldn't wrap long")

#define DICT_ADD_DOUBLE(dict, elem, double) \
        PyObject* elem = Py_BuildValue("d", double); \
        DICT_ADD_ELEMXE(dict, elem, "Couldn't wrap double")

#define DICT_ADD_BOOL(dict, elem, bool) \
        PyObject* elem = PyBool_FromLong(bool); \
        DICT_ADD_ELEMXE(dict, elem, "Couldn't wrap bool")

#define DICT_ADD_STRING(dict, elem, str) \
        PyObject* elem = Py_BuildValue("s", str); \
        DICT_ADD_ELEMXE(dict, elem, "Couldn't wrap string")

#define DICT_ADD_CAIRO(dict, elem, var, wrap) \
        PyObject* elem = NULL2NONE(var, wrap); \
        DICT_ADD_ELEMXE(dict, elem, "Couldn't wrap cairo structure")

#define DICT_ADD_ENUM(dict, elem, enum) \
        guint* i_##elem = &(enum); /*GCC will warn us if this enum has different size than uint*/ \
        DICT_ADD_UINT(dict, elem, i_##elem)

#define DICT_ADD_ACOLOR(dict, elem, acolor) \
        PyObject* elem = ACOLOR2TUPLE(acolor); \
        DICT_ADD_ELEMXE(dict, elem, "Couldn't wrap alpha_color")

#define SWRAP_ARRAY_FLAT(elem, buildstr, n, desc) DICT_ADD_ARRAY_FLAT(kw, elem, buildstr, stru->elem, n, desc)
#define SWRAP_ARRAY_GOBJ(elem, n) DICT_ADD_ARRAY_GOBJ(kw, elem, stru->elem, n)
#define SWRAP_GOBJECT(elem) DICT_ADD_GOBJECT(kw, elem, stru->elem)
#define SWRAP_INT(elem) DICT_ADD_INT(kw, elem, stru->elem)
#define SWRAP_UINT(elem) DICT_ADD_UINT(kw, elem, stru->elem)
#define SWRAP_BOOL(elem) DICT_ADD_BOOL(kw, elem, stru->elem)
#define SWRAP_LONG(elem) DICT_ADD_LONG(kw, elem, stru->elem)
#define SWRAP_DOUBLE(elem) DICT_ADD_DOUBLE(kw, elem, stru->elem)
#define SWRAP_STRING(elem) DICT_ADD_STRING(kw, elem, stru->elem)
#define SWRAP_CAIRO(elem, wrap) DICT_ADD_CAIRO(kw, elem, stru->elem, wrap)
#define SWRAP_ENUM(elem) DICT_ADD_ENUM(kw, elem, stru->elem)
#define SWRAP_ACOLOR(elem) DICT_ADD_ACOLOR(kw, elem, stru->elem)

static void fs_set_ws(PyObject* fs, PyObject* ws) {
    //TODO: Magic goes here
}

static PyObject* wrap_cairo_pattern(cairo_pattern_t* pattern, gboolean incref) {
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

static PyObject* wrap_framesettings(frame_settings* fs) {
    wrappers* wrs = ((private_ws*) fs->ws->engine_ws)->wrs;

    if (!wrs->frame_settings) {
        return NULL;
    }

    PyObject* kw = PyDict_New();
    if (!kw) {
        print_python_error("Couldn't create dictionary");
        return NULL;
    }
    frame_settings* stru = fs;

    PyObject* engine_fs = ((private_fs*)fs->engine_fs)->python_fs;
    DICT_ADD_ELEMXE(kw, engine_fs, "Missing python_fs dictionary");

    PyObject* ws = NEW_NONE; //TODO
    DICT_ADD_ELEM(kw, ws)

    SWRAP_ACOLOR(button)
    SWRAP_ACOLOR(button_halo)
    SWRAP_ACOLOR(text)
    SWRAP_ACOLOR(text_halo)

    PyObject* args = Py_BuildValue("()");

    if (!args) {
        print_python_error("Couldn't create empty args");
        Py_DECREF(kw);
        return NULL;
    }

    PyObject* frs = PyObject_Call(wrs->frame_settings, args, kw);

    Py_DECREF(kw);
    Py_DECREF(args);
    if (!frs) {
        print_python_error("Couldn't wrap button frame settings");
    }
    return frs;
}

static PyObject* wrap_windowsettings(window_settings* ws) {
    wrappers* wrs = ((private_ws*) ws->engine_ws)->wrs;

    if (!wrs->window_settings) {
        return NULL;
    }

    PyObject* kw = PyDict_New();
#ifdef PYTHON_DICT_GC_HACK
    /* Double reference so that the circular GC doesn't clear our dict
     * while we're still building it */
    Py_INCREF(kw);
#endif // PYTHON_DICT_GC_HACK
    if (!kw) {
        print_python_error("Couldn't create dictionary");
        return NULL;
    }
    window_settings* stru = ws;

    PyObject* engine_ws = ((private_ws*) ws->engine_ws)->python_ws;
    DICT_ADD_ELEMXE(kw, engine_ws, "Missing python_ws dictionary");
    SWRAP_INT(button_offset)
    SWRAP_INT(button_hoffset)
    SWRAP_STRING(tobj_layout)

    SWRAP_INT(double_click_action)
    SWRAP_INT(button_hover_cursor)

    SWRAP_BOOL(round_top_left)
    SWRAP_BOOL(round_top_right)
    SWRAP_BOOL(round_bottom_left)
    SWRAP_BOOL(round_bottom_right)

    PyObject* fs_act = wrap_framesettings(ws->fs_act);
    DICT_ADD_ELEMX(kw, fs_act)
    PyObject* fs_inact = wrap_framesettings(ws->fs_inact);
    DICT_ADD_ELEMX(kw, fs_inact)

    SWRAP_INT(min_titlebar_height)
    SWRAP_BOOL(use_pixmap_buttons)
    SWRAP_DOUBLE(corner_radius)
    SWRAP_ENUM(title_text_align)

    SWRAP_ARRAY_GOBJ(ButtonPix, S_COUNT * B_COUNT)
    SWRAP_ARRAY_GOBJ(ButtonArray, B_COUNT)

    SWRAP_BOOL(use_button_glow)
    SWRAP_BOOL(use_button_inactive_glow)
    SWRAP_BOOL(use_button_fade)
    SWRAP_BOOL(use_decoration_cropping)

    SWRAP_ARRAY_GOBJ(ButtonGlowPix, B_COUNT)

    SWRAP_GOBJECT(ButtonGlowArray)
    SWRAP_GOBJECT(ButtonInactiveGlowArray)

    SWRAP_ARRAY_GOBJ(ButtonInactiveGlowPix, B_COUNT)

    SWRAP_INT(button_fade_num_steps)
    SWRAP_INT(button_fade_step_duration)
    SWRAP_INT(button_fade_pulse_len_steps)
    SWRAP_INT(button_fade_pulse_wait_steps)

    SWRAP_DOUBLE(shadow_radius)
    SWRAP_DOUBLE(shadow_opacity)
    SWRAP_ARRAY_FLAT(shadow_color, "i", 3, "Couldn't wrap int");
    SWRAP_INT(shadow_offset_x)
    SWRAP_INT(shadow_offset_y)

    PyObject* shadow_extents = NEW_NONE; //TODO
    DICT_ADD_ELEMX(kw, shadow_extents)

    PyObject* win_extents = NEW_NONE; //TODO
    DICT_ADD_ELEMX(kw, win_extents)

    PyObject* pos = NEW_NONE; //TODO
    DICT_ADD_ELEMX(kw, pos)

    SWRAP_INT(left_space)
    SWRAP_INT(right_space)
    SWRAP_INT(top_space)
    SWRAP_INT(bottom_space)

    SWRAP_INT(left_corner_space)
    SWRAP_INT(right_corner_space)
    SWRAP_INT(top_corner_space)
    SWRAP_INT(bottom_corner_space)

    SWRAP_INT(titlebar_height)
    SWRAP_INT(normal_top_corner_space)

    SWRAP_INT(shadow_left_space)
    SWRAP_INT(shadow_right_space)
    SWRAP_INT(shadow_top_space)
    SWRAP_INT(shadow_bottom_space)

    SWRAP_INT(shadow_left_corner_space)
    SWRAP_INT(shadow_right_corner_space)
    SWRAP_INT(shadow_top_corner_space)
    SWRAP_INT(shadow_bottom_corner_space)

    SWRAP_GOBJECT(shadow_pixmap)
    SWRAP_GOBJECT(large_shadow_pixmap)
    SWRAP_GOBJECT(decor_normal_pixmap)
    SWRAP_GOBJECT(decor_active_pixmap)

    SWRAP_CAIRO(shadow_pattern, wrap_cairo_pattern(ws->shadow_pattern, TRUE))

    SWRAP_INT(text_height)

    //SWRAP_GOBJECT(font_desc) //TODO: If it's not a GObject then what is it?
    PyObject* font_desc = NEW_NONE;
    DICT_ADD_ELEMX(kw, font_desc);
    SWRAP_GOBJECT(pango_context)

    PyObject* switcher_extents = NEW_NONE; //TODO
    DICT_ADD_ELEMX(kw, switcher_extents)

    SWRAP_GOBJECT(switcher_pixmap)
    SWRAP_GOBJECT(switcher_buffer_pixmap)
    SWRAP_INT(switcher_width)
    SWRAP_INT(switcher_height)

    SWRAP_INT(switcher_top_corner_space)
    SWRAP_INT(switcher_bottom_corner_space)

    PyObject* c_icon_size = NEW_NONE; //TODO
    DICT_ADD_ELEMX(kw, c_icon_size)
    PyObject* c_glow_size = NEW_NONE; //TODO
    DICT_ADD_ELEMX(kw, c_glow_size)

    SWRAP_BOOL(stretch_sides)
    SWRAP_INT(blur_type)

    PyObject* args = Py_BuildValue("()");
    if (!args) {
        print_python_error("Couldn't create empty args");
        Py_DECREF(kw);
        return NULL;
    }

    PyObject* wns = PyObject_Call(wrs->window_settings, args, kw);

    Py_DECREF(kw);
#ifdef PYTHON_DICT_GC_HACK
    // Remove the extra reference
    Py_DECREF(kw);
#endif
    Py_DECREF(args);
    if (!wns) {
        print_python_error("Couldn't wrap window settings");
    }
    return wns;
}

static PyObject* wrap_fade_info(wrappers* wrs, button_fade_info_t* bfi) {
    if (!wrs->fade_info) {
        return NULL;
    }

    PyObject* kw = PyDict_New();
    if (!kw) {
        print_python_error("Couldn't create dictionary");
        return NULL;
    }
    button_fade_info_t* stru = bfi;

    //PyObject* d = Not today...
    SWRAP_CAIRO(cr, PycairoContext_FromContext(cairo_reference(bfi->cr), &PycairoContext_Type, NULL));
    SWRAP_DOUBLE(y1)
    SWRAP_ARRAY_FLAT(counters, "i", B_T_COUNT, "Couldn't wrap int")
    DICT_ADD_ARRAY(kw, pulsating, B_T_COUNT, PyBool_FromLong(bfi->pulsating[i]), "Couldn't wrap boolean")
    SWRAP_INT(timer)
    SWRAP_BOOL(first_draw)

    PyObject* args = Py_BuildValue("()");
    if (!args) {
        print_python_error("Couldn't create empty args");
        Py_DECREF(kw);
        return NULL;
    }

    PyObject* fade_info = PyObject_Call(wrs->fade_info, args, kw);

    Py_DECREF(kw);
    Py_DECREF(args);
    if (!fade_info) {
        print_python_error("Couldn't wrap button fade info");
    }
    return fade_info;
}

static PyObject* wrap_rectangle(wrappers* wrs, rectangle_t* r) {
    if (!wrs->rectangle) {
        return NULL;
    }

    PyObject* rect = PyObject_CallFunction(wrs->rectangle, "iiii", r->x1, r->y1, r->x2, r->y2);
    if (!rect) {
        print_python_error("Couldn't wrap rectangle");
    }
    return rect;
}

static PyObject* wrap_button_region(wrappers* wrs, button_region_t* br) {
    if (!wrs->button_region) {
        return NULL;
    }

    PyObject* kw = PyDict_New();
    if (!kw) {
        print_python_error("Couldn't create dictionary");
        return NULL;
    }
    button_region_t* stru = br;

    SWRAP_INT(base_x1)
    SWRAP_INT(base_y1)
    SWRAP_INT(base_x2)
    SWRAP_INT(base_y2)
    SWRAP_INT(glow_x1)
    SWRAP_INT(glow_y1)
    SWRAP_INT(glow_x2)
    SWRAP_INT(glow_y2)
    DICT_ADD_ARRAY(kw, overlap_buttons, B_T_COUNT, PyBool_FromLong(br->overlap_buttons[i]), "Couldn't wrap boolean")
    SWRAP_GOBJECT(bg_pixmap)

    PyObject* args = Py_BuildValue("()");
    if (!args) {
        print_python_error("Couldn't create empty args");
        Py_DECREF(kw);
        return NULL;
    }

    PyObject* region = PyObject_Call(wrs->button_region, args, kw);

    Py_DECREF(kw);
    Py_DECREF(args);
    if (!region) {
        print_python_error("Couldn't wrap button region");
    }
    return region;
}

static PyObject* wrap_decor(decor_t* d) {
    wrappers* wrs = ((private_ws*) d->fs->ws->engine_ws)->wrs;

    if (!wrs->decor) {
        return NULL;
    }

    PyObject* kw = PyDict_New();
    if (!kw) {
        print_python_error("Couldn't create dictionary");
        return NULL;
    }
    decor_t* stru = d;

    DICT_ADD_ARRAY(kw, event_windows, 3, Py_BuildValue("(kkk)", d->event_windows[i][0], d->event_windows[i][1], d->event_windows[i][2]), "Couldn't create tuple")

    SWRAP_ARRAY_FLAT(button_windows, "k", B_T_COUNT, "Couldn't wrap long")
    SWRAP_ARRAY_FLAT(button_states, "I", B_T_COUNT, "Couldn't wrap int")

    SWRAP_ARRAY_FLAT(tobj_pos, "i", 3, "Couldn't wrap int")
    SWRAP_ARRAY_FLAT(tobj_size, "i", 3, "Couldn't wrap int")
    SWRAP_ARRAY_FLAT(tobj_item_pos, "i", 11, "Couldn't wrap int")
    SWRAP_ARRAY_FLAT(tobj_item_state, "i", 11, "Couldn't wrap int")
    SWRAP_ARRAY_FLAT(tobj_item_width, "i", 11, "Couldn't wrap int")
    SWRAP_GOBJECT(pixmap)
    SWRAP_GOBJECT(buffer_pixmap)
    SWRAP_GOBJECT(gc);
    SWRAP_INT(width)
    SWRAP_INT(height)
    SWRAP_INT(client_width)
    SWRAP_INT(client_height)
    SWRAP_BOOL(decorated)
    SWRAP_BOOL(active)
    SWRAP_GOBJECT(layout)
    SWRAP_STRING(name)
    SWRAP_CAIRO(icon, wrap_cairo_pattern(stru->icon, FALSE))
    SWRAP_GOBJECT(icon_pixmap)
    SWRAP_GOBJECT(icon_pixbuf)
    SWRAP_ENUM(state)
    SWRAP_ENUM(actions)
    SWRAP_LONG(prop_xid)
    SWRAP_GOBJECT(force_quit_dialog);

//#define NO_WS
#ifdef NO_WS
    PyObject* ws = NEW_NONE;
#else
    PyObject* ws = wrap_windowsettings(d->fs->ws);
#endif // NO_WS
    if (!ws) {
        Py_DECREF(kw);
        return NULL;
    }
    PyObject* fs;
#ifndef NO_WS
    if (d->fs == d->fs->ws->fs_act) {
        fs = PyObject_GetAttrString(ws, "fs_act");
    } else if (d->fs == d->fs->ws->fs_inact) {
        fs = PyObject_GetAttrString(ws, "fs_inact");
    } else {
#endif
        fs = wrap_framesettings(d->fs);
        if (fs) {
            fs_set_ws(fs, ws);
        }
#ifndef NO_WS
    }
#endif
    Py_DECREF(ws);
    DICT_ADD_ELEMX(kw, fs)
    //PyObject* draw = sorry, not this time
    DICT_ADD_ARRAY(kw, button_region, B_T_COUNT, wrap_button_region(wrs, &d->button_region[i]), "Couldn't wrap button region")

    PyObject* min_drawn_buttons_region = wrap_rectangle(wrs, &d->min_drawn_buttons_region);
    DICT_ADD_ELEMX(kw, min_drawn_buttons_region)

    SWRAP_BOOL(draw_only_buttons_region)
    SWRAP_ARRAY_FLAT(button_last_drawn_state, "i", B_T_COUNT, "Couldn't wrap int")

    PyObject* button_fade_info = wrap_fade_info(wrs, &d->button_fade_info);
    DICT_ADD_ELEMX(kw, button_fade_info)

    SWRAP_GOBJECT(p_active)
    SWRAP_GOBJECT(p_active_buffer)
    SWRAP_GOBJECT(p_inactive)
    SWRAP_GOBJECT(p_inactive_buffer)

    DICT_ADD_ARRAY(kw, button_region_inact, B_T_COUNT, wrap_button_region(wrs, &d->button_region_inact[i]), "Couldn't wrap button region")

    SWRAP_BOOL(only_change_active)

#ifdef USE_PY_STRUCTSEQ
    PyObject* decor = PyStructSequence_New(wrs->decor);
    if (!decor) {
        print_python_error("Couldn't create decor structsequence.");
        return NULL;
    }
    PyStructSequence_SET_ITEM(decor, 0, event_windows);
    PyStructSequence_SET_ITEM(decor, 1, button_windows);
    PyStructSequence_SET_ITEM(decor, 2, button_states);
    PyStructSequence_SET_ITEM(decor, 3, tobj_pos);
    PyStructSequence_SET_ITEM(decor, 4, tobj_size);
    PyStructSequence_SET_ITEM(decor, 5, tobj_item_pos);
    PyStructSequence_SET_ITEM(decor, 6, tobj_item_state);
    PyStructSequence_SET_ITEM(decor, 7, tobj_item_width);
#endif // USE_PY_STRUCTSEQ

    PyObject* args = Py_BuildValue("()");
    if (!args) {
        print_python_error("Couldn't create empty args");
        Py_DECREF(kw);
        return NULL;
    }

    PyObject* decor = PyObject_Call(wrs->decor, args, kw);

    Py_DECREF(kw);
    Py_DECREF(args);
    if (!decor) {
        print_python_error("Couldn't wrap decor");
    }
    return decor;
}

static gboolean python_draw_frame (decor_t* d, cairo_t* cr) {
    frame_settings *fs = d->fs;
    private_fs *pfs = fs->engine_fs;
    window_settings *ws = fs->ws;
    private_ws* pws = ws->engine_ws;

    PyObject* pFunc = pws->py->draw;
    if (pFunc == NULL) {
        //The python-related part of initialization must have failed
        return FALSE;
    }

    // TODO: These error conditions should decref stuff and call the fallback too!
    PyObject* pExtents = Py_BuildValue("(iiii)", ws->win_extents.left, ws->win_extents.top, ws->win_extents.right, ws->win_extents.bottom);
    if (!pExtents) {
        print_python_error("Couldn't build extents tuple.");
        return FALSE; // Don't need to decref anything, haven't succesfully wrapped anything yet
    }
    PyObject* pSpace = Py_BuildValue("(iiii)", ws->left_space, ws->top_space, ws->right_space, ws->bottom_space);
    if (!pSpace) {
        print_python_error("Couldn't build space tuple.");
        goto fail;
    }
    PyObject* pSize = Py_BuildValue("(ii)", d->width, d->height);
    if (!pSize) {
        print_python_error("Couldn't build size tuple.");
        goto fail;
    }
    PyObject* pTitleBarHeight = Py_BuildValue("i", ws->titlebar_height);
    if (!pTitleBarHeight) {
        print_python_error("Couldn't build titlebar_height.");
        goto fail;
    }

    PyObject* pyCtx = PycairoContext_FromContext(cairo_reference(cr), &PycairoContext_Type, NULL);
    if (!pyCtx) {
        // This fool we've just called destroyed our context just because he was unable to wrap it...
        print_python_error("Couldn't wrap cairo context.");
        goto fail; // FIXME: This may not work (i.e. the fallback may crash)
    }

    PyObject* pyDecor = wrap_decor(d);
    if (!pyDecor) {
        print_python_error("Couldn't wrap decor.");
        goto fail;
    }

    PyObject* pArgs = PyTuple_Pack(6, pyCtx, pSize, pSpace, pExtents, pTitleBarHeight, pyDecor);
    if (!pArgs) {
        print_python_error("Couldn't build function argument tuple.");
        goto fail;
    }
    PyObject* pRet = PyObject_CallObject(pFunc, pArgs);
    Py_DECREF(pArgs);
    if (!pRet) {
        print_python_error("Exception in draw() on the python side.");
        return FALSE;
    }
    Py_DECREF(pRet);

    return TRUE;

fail:
    Py_XDECREF(pExtents);
    Py_XDECREF(pSpace);
    Py_XDECREF(pSize);
    Py_XDECREF(pTitleBarHeight);
    Py_XDECREF(pyCtx);
    Py_XDECREF(pyDecor);

    return FALSE;

}

void engine_draw_frame (decor_t * d, cairo_t * cr)
{
    if (!python_draw_frame(d, cr)) {
        ((private_ws*) d->fs->ws->engine_ws)->py->draw = NULL; // Don't try python again
        fallback_draw_frame(d, cr);
    }
}

void load_engine_settings(GKeyFile * f, window_settings * ws)
{
    PFACS(border);
    PFACS(title_bar);
    //PyObject* pName = PyUnicode_FromString(make_filename("python", "theme", "py"));
    //wchar_t* argv[] = { make_filename("python", "theme", "py") };
    //PySys_SetArgv(1, argv);
    //PyObject* pModule = PyImport_Import(pName);
    //Py_DECREF(pName);
}

static gboolean init_python(private_ws* pws) {
    #ifdef LIBPY_LINK_HACK
    void* dl = dlopen(LIBPY_SONAME, RTLD_LAZY | RTLD_NOLOAD | RTLD_GLOBAL);
    if (dl) {
        dlclose(dl);
    } else {
        fprintf(stderr, "Couldn't promote " LIBPY_SONAME " to global namespace");
    }
    #endif // LIBPY_LINK_HACK

    Py_Initialize();

    gchar* scriptPath = g_strjoin("/", LOCAL_SCRIPT_DIR, SCRIPT_FILE, NULL);
    if (access(scriptPath, F_OK) != 0) {
        scriptPath = g_strjoin("/", SCRIPT_DIR, SCRIPT_FILE, NULL);
    }

    const size_t len = strlen(scriptPath);
    wchar_t scriptPath_w[len + 1];
    mbstowcs(scriptPath_w, scriptPath, len);


    wchar_t* argv[] = { scriptPath_w };
    PySys_SetArgv(1, argv);

    PyObject* pModule = PyImport_ImportModule(SCRIPT_NAME);
    if (!pModule) {
        print_python_error("Couldn't import module.");
        return FALSE;
    }

    int err = import_cairo();
    if (err < 0) {
        fprintf(stderr, "Couldn't import pycairo C API: %d\n", err);
        return FALSE;
    }
    if (!pygobject_init(-1, -1, -1)) {
        print_python_error("Couldn't import PyGObject C API");
        return FALSE;
    }

    PyObject* pDrawFunc = get_python_func(pModule, "draw");
    if (!pDrawFunc) {
        return FALSE;
    }

    PyObject* pInitFunc = get_python_func(pModule, "init");
    if (pInitFunc) {
        PyObject* ret = PyObject_CallObject(pInitFunc, NULL);
        if (!ret) {
            print_python_error("Init failed on python side");
        } else {
            Py_DECREF(ret);
        }
    }

    private_py* py;
    py = malloc(sizeof(private_py));
    bzero(py,sizeof(private_py));

    py->module = pModule;
    py->draw = pDrawFunc;
    py->init = pInitFunc;

    pws->py = py;

    return TRUE;
}

void init_engine(window_settings * ws)
{
    private_fs * pfs;
    private_ws * pws;

    pws = malloc(sizeof(private_ws));
    ws->engine_ws = pws;
    bzero(pws,sizeof(private_ws));

    pfs = malloc(sizeof(private_fs));
    ws->fs_act->engine_fs = pfs;
    bzero(pfs, sizeof(private_fs));
    ACOLOR(border, 0.0, 0.0, 0.0, 1.0);
    ACOLOR(title_bar, 0.0, 0.0, 0.0, 0.3);

    pfs = malloc(sizeof(private_fs));
    ws->fs_inact->engine_fs = pfs;
    bzero(pfs,sizeof(private_fs));
    ACOLOR(border, 0.0, 0.0, 0.0, 1.0);
    ACOLOR(title_bar, 0.0, 0.0, 0.0, 0.0);

    //If something goes wrong, pws->draw will be null.

    if (init_python(pws)) {
        pws->wrs = init_wrappers(pws);

        PyObject* pyfs;
        pyfs = PyDict_New();
        ((private_fs*)ws->fs_act->engine_fs)->python_fs = pyfs;
        pyfs = PyDict_New();
        ((private_fs*)ws->fs_inact->engine_fs)->python_fs = pyfs;
        pyfs = PyDict_New();
        pws->python_ws = pyfs;
    }
}

void fini_engine(window_settings * ws)
{
    Py_Finalize();
    free(ws->fs_act->engine_fs);
    free(ws->fs_inact->engine_fs);
}

void my_engine_settings(GtkWidget * hbox, gboolean active)
{
    GtkWidget * vbox;
    GtkWidget * scroller;
    vbox = gtk_vbox_new(FALSE,2);
    gtk_box_pack_startC(hbox, vbox, TRUE, TRUE, 0);
    gtk_box_pack_startC(vbox, gtk_label_new(active?"Active Window":"Inactive Window"), FALSE, FALSE, 0);
    gtk_box_pack_startC(vbox, gtk_hseparator_new(), FALSE, FALSE, 0);
    scroller = gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_startC(vbox, scroller, TRUE, TRUE, 0);

    table_new(3, FALSE, FALSE);

    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroller), GTK_WIDGET(get_current_table()));

    make_labels(_("Colors"));
    table_append_separator();
    ACAV(_("Outer Frame Blend"), "border", SECT);
    ACAV(_("Title Bar"), "title_bar", SECT);
}

void layout_engine_colors(GtkWidget * vbox)
{
    GtkWidget * hbox;
    hbox = gtk_hbox_new(FALSE, 2);
    gtk_box_pack_startC(vbox, hbox, TRUE, TRUE, 0);
    my_engine_settings(hbox, TRUE);
    gtk_box_pack_startC(hbox, gtk_vseparator_new(), FALSE, FALSE, 0);
    my_engine_settings(hbox, FALSE);
}

void layout_engine_settings(GtkWidget * vbox)
{
    GtkWidget * note;
    note = gtk_notebook_new();
    gtk_box_pack_startC(vbox, note, TRUE, TRUE, 0);
    layout_engine_colors(build_notebook_page(_("Colors"), note));
}

