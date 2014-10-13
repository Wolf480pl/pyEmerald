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
#endif // USE_PY_STRUCTSEQ
} wrappers;

/*typedef struct _private_py
{
    PyObject* module;
    PyObject* draw;
    PyObject* init;
    wrappers* wrs;
} private_py
*/

/*
 * settings privates
 */
typedef struct _private_ws
{
    PyObject* module;
    PyObject* draw;
    PyObject* init;
    wrappers* wrs;
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
    wrs->decor = get_python_func(pws->module, "wrap_decor");
    if (!wrs->decor) {
        print_python_error("No wrap_decor in the python module");
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

#define ARRAY2TUPLE_BEGIN(tuple, n) \
        PyObject* tuple = PyTuple_New(n); \
        {\
            int i; \
            for (i = 0; i < n; ++i) {

#define ARRAY2TUPLE_END(tuple, element) \
                PyTuple_SET_ITEM(tuple, i, element); \
            }\
        }
#define ARRAY2TUPLE_FLAT(tuple, buildstr, array, n, desc) \
        ARRAY2TUPLE_BEGIN(tuple, n) \
            PyObject* element = Py_BuildValue(buildstr, array[i]); \
            if (!element) { \
                print_python_error(desc); \
                return NULL; \
            } \
        ARRAY2TUPLE_END(tuple, element)

//For use in Py_BuildValue
//#define DICT_ELEM(name) #name , name

#define DICT_ADD_ELEM(dict, elem) \
        if (PyDict_SetItemString(dict, #elem, elem) < 0) { \
            return NULL; \
        }

#define NEW_NONE (Py_INCREF(Py_None), Py_None)
#define NULL2NONE(var, wrap) var ? wrap : NEW_NONE

static PyObject* wrap_decor(decor_t* d) {
    wrappers* wrs = ((private_ws*) d->fs->ws->engine_ws)->wrs;

    if (!wrs->decor) {
        return NULL;
    }

    ARRAY2TUPLE_BEGIN(event_windows, 3)
        PyObject* element = Py_BuildValue("(kkk)", d->event_windows[i][0], d->event_windows[i][1], d->event_windows[i][2]);
        if (!element) {
            print_python_error("Couldn't create tuple");
            return NULL;
        }
    ARRAY2TUPLE_END(event_windows, element)

    ARRAY2TUPLE_FLAT(button_windows, "k", d->button_windows, B_T_COUNT, "Couldn't wrap long")
    ARRAY2TUPLE_FLAT(button_states, "I", d->button_states, B_T_COUNT, "Couldn't wrap int")

    ARRAY2TUPLE_FLAT(tobj_pos, "i", d->tobj_pos, 3, "Couldn't wrap int")
    ARRAY2TUPLE_FLAT(tobj_size, "i", d->tobj_size, 3, "Couldn't wrap int")
    ARRAY2TUPLE_FLAT(tobj_item_pos, "i", d->tobj_item_pos, 11, "Couldn't wrap int")
    ARRAY2TUPLE_FLAT(tobj_item_state, "i", d->tobj_item_state, 11, "Couldn't wrap int")
    ARRAY2TUPLE_FLAT(tobj_item_width, "i", d->tobj_item_width, 11, "Couldn't wrap int")
    PyObject* pixmap = pygobject_new((GObject*) d->pixmap);
    PyObject* buffer_pixmap = pygobject_new((GObject*) d->buffer_pixmap);
    PyObject* gc = pygobject_new((GObject*) d->gc);
    PyObject* width = Py_BuildValue("i", d->width);
    PyObject* height = Py_BuildValue("i", d->height);
    PyObject* client_width = Py_BuildValue("i", d->client_width);
    PyObject* client_height = Py_BuildValue("i", d->client_height);
    PyObject* decorated = PyBool_FromLong(d->decorated);
    PyObject* active = PyBool_FromLong(d->active);
    PyObject* layout = pygobject_new((GObject*) d->layout);
    PyObject* name = Py_BuildValue("s", d->name);
    PyObject* icon = NULL2NONE(d->icon, PycairoPattern_FromPattern(d->icon, NULL));
    PyObject* icon_pixmap = pygobject_new((GObject*) d->icon_pixmap);
    PyObject* icon_pixbuf = pygobject_new((GObject*) d->icon_pixbuf);
    guint* i_state = &(d->state); // GCC will warn us if this enum has different size than uint
    PyObject* state = Py_BuildValue("I", i_state);
    guint* i_actions = &(d->actions); // GCC will warn us if this enum has different size than uint
    PyObject* actions = Py_BuildValue("I", i_actions);
    PyObject* prop_xid = Py_BuildValue("k", d->prop_xid);
    PyObject* force_quit_dialog = pygobject_new((GObject*) d->force_quit_dialog);
    PyObject* fs = NEW_NONE; //TODO
    //PyObject* draw = sorry, not this time
    PyObject* button_region = NEW_NONE; //TODO
    PyObject* min_drawn_buttons_region = NEW_NONE; //TODO
    PyObject* draw_only_buttons_region = PyBool_FromLong(d->draw_only_buttons_region);
    ARRAY2TUPLE_FLAT(button_last_drawn_state, "i", d->button_last_drawn_state, B_T_COUNT, "Couldn't wrap int")
    PyObject* button_fade_info = NEW_NONE; //TODO
    PyObject* p_active = pygobject_new((GObject*) d->p_active);
    PyObject* p_active_buffer = pygobject_new((GObject*) d->p_active_buffer);
    PyObject* p_inactive = pygobject_new((GObject*) d->p_inactive);
    PyObject* p_inactive_buffer = pygobject_new((GObject*) d->p_inactive_buffer);
    PyObject* button_region_inact = NEW_NONE; //TODO
    PyObject* only_change_active = PyBool_FromLong(d->only_change_active);

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
#else
    PyObject* args = Py_BuildValue("()");
    PyObject* kw = PyDict_New();
    DICT_ADD_ELEM(kw, event_windows)
    DICT_ADD_ELEM(kw, button_windows)
    DICT_ADD_ELEM(kw, button_states)
    DICT_ADD_ELEM(kw, tobj_pos)
    DICT_ADD_ELEM(kw, tobj_size)
    DICT_ADD_ELEM(kw, tobj_item_pos)
    DICT_ADD_ELEM(kw, tobj_item_state)
    DICT_ADD_ELEM(kw, tobj_item_width)
    DICT_ADD_ELEM(kw, pixmap)
    DICT_ADD_ELEM(kw, buffer_pixmap)
    DICT_ADD_ELEM(kw, gc)
    DICT_ADD_ELEM(kw, width)
    DICT_ADD_ELEM(kw, height)
    DICT_ADD_ELEM(kw, client_width)
    DICT_ADD_ELEM(kw, client_height)
    DICT_ADD_ELEM(kw, decorated)
    DICT_ADD_ELEM(kw, active)
    DICT_ADD_ELEM(kw, layout)
    DICT_ADD_ELEM(kw, name)
    DICT_ADD_ELEM(kw, icon)
    DICT_ADD_ELEM(kw, icon_pixmap)
    DICT_ADD_ELEM(kw, icon_pixbuf)
    DICT_ADD_ELEM(kw, state)
    DICT_ADD_ELEM(kw, actions)
    DICT_ADD_ELEM(kw, prop_xid)
    DICT_ADD_ELEM(kw, force_quit_dialog)
    DICT_ADD_ELEM(kw, fs)
    DICT_ADD_ELEM(kw, button_region)
    DICT_ADD_ELEM(kw, min_drawn_buttons_region)
    DICT_ADD_ELEM(kw, draw_only_buttons_region)
    DICT_ADD_ELEM(kw, button_last_drawn_state)
    DICT_ADD_ELEM(kw, button_fade_info)
    DICT_ADD_ELEM(kw, p_active)
    DICT_ADD_ELEM(kw, p_active_buffer)
    DICT_ADD_ELEM(kw, p_inactive)
    DICT_ADD_ELEM(kw, p_inactive_buffer)
    DICT_ADD_ELEM(kw, button_region_inact)
    DICT_ADD_ELEM(kw, only_change_active)

    PyObject* decor = PyObject_Call(wrs->decor, args, kw);
#endif // USE_PY_STRUCTSEQ
    return decor;
}

static gboolean python_draw_frame (decor_t* d, cairo_t* cr) {
    frame_settings *fs = d->fs;
    private_fs *pfs = fs->engine_fs;
    window_settings *ws = fs->ws;
    private_ws* pws = ws->engine_ws;

    PyObject* pFunc = pws->draw;
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

    PyObject* pyCtx = PycairoContext_FromContext(cr, &PycairoContext_Type, NULL);
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
        ((private_ws*) d->fs->ws->engine_ws)->draw = NULL; // Don't try python again
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

    pws->module = pModule;
    pws->draw = pDrawFunc;
    pws->init = pInitFunc;

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

    //If something goes wrong, pws->func will be null.

    if (init_python(pws)) {
        pws->wrs = init_wrappers(pws);
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

