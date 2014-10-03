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

#include "pixmap_icon.h"

#define SECT "python_settings"

/*
 * color privates
 */
typedef struct _private_fs
{
    alpha_color border;
    alpha_color title_bar;
} private_fs;

/*
 * settings privates
 */
typedef struct _private_ws
{
    PyObject* func;
} private_ws;

void get_meta_info (EngineMetaInfo * emi)
{
    emi->version = g_strdup("0.1");
    emi->description = g_strdup(_("Wrapper for python scripts"));
    emi->last_compat = g_strdup("0.0"); // old themes still compatible
    emi->icon = gdk_pixbuf_new_from_inline(-1, my_pixbuf, TRUE, NULL);
}
/*
static PyObject* eval_python(char* expr) {
    PyCodeObject* code = (PyCodeObject*) Py_CompileString(expr, "eval", Py_eval_input);
    PyObject* main_module = PyImport_AddModule("__main__");
    PyObject* global_dict = PyModule_GetDict(main_module);
    return PyEval_EvalCode(code, global_dict, global_dict);
}
*/

static void print_python_error(const char* desc) {
    fprintf(stderr, "%s\n", desc);
    if (PyErr_Occurred()) {
        PyErr_Print();
    }
}

void engine_draw_frame (decor_t * d, cairo_t * cr)
{
    frame_settings *fs = d->fs;
    private_fs *pfs = fs->engine_fs;
    window_settings *ws = fs->ws;
    private_ws* pws = ws->engine_ws;

#if 1
    PyObject* pFunc = pws->func;
    if (pFunc == NULL) {
        //The python-related part of initialization must have failed
        //TODO: draw some fallback frame
        return;
    }

    PyObject* pExtents = Py_BuildValue("(iiii)", ws->win_extents.left, ws->win_extents.top, ws->win_extents.right, ws->win_extents.bottom);
    if (!pExtents) {
        print_python_error("Couldn't build extents tuple.");
    }
    PyObject* pSpace = Py_BuildValue("(iiii)", ws->left_space, ws->top_space, ws->right_space, ws->bottom_space);
    if (!pSpace) {
        print_python_error("Couldn't build space tuple.");
    }
    PyObject* pSize = Py_BuildValue("(ii)", d->width, d->height);
    if (!pSize) {
        print_python_error("Couldn't build size tuple.");
    }
    PyObject* pTitleBarHeight = Py_BuildValue("i", ws->titlebar_height);
    if (!pTitleBarHeight) {
        print_python_error("Couldn't build titlebar_height.");
    }

    PyObject* pyCtx = PycairoContext_FromContext(cr, &PycairoContext_Type, NULL);
    if (!pyCtx) {
        // This fool we've just called destroyed our context just because he was unable to wrap it...
        print_python_error("Couldn't wrap cairo context.");
    }

    PyObject* pArgs = PyTuple_Pack(5, pyCtx, pSize, pSpace, pExtents, pTitleBarHeight);
    if (!pArgs) {
        print_python_error("Couldn't build function argument tuple.");
    }
    PyObject* pRet = PyObject_CallObject(pFunc, pArgs);
    Py_DECREF(pArgs);
    Py_XDECREF(pRet);

#else
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
    cairo_set_source_alpha_color(cr, &pfs->border);
    cairo_stroke (cr);

    // title bar
    if (pfs->title_bar.alpha != 0.0) {
        rounded_rectangle (cr,
                x1,
                y1,
                x2 - x1,
                top,
                0, NULL, 0);
        cairo_set_source_alpha_color(cr, &pfs->title_bar);
        cairo_fill(cr);
    } else {
        cairo_save(cr);
        cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
        cairo_rectangle (cr, 0.0, 0.0, d->width, top + y1 - border_width);
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
        cairo_fill(cr);
        cairo_restore(cr);
    }
#endif
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

    Py_Initialize();
    wchar_t* argv[] = { L"/home/wolf480/git/pyEmerald/emerald.py" };
    PySys_SetArgv(1, argv);

    //If something goes wrong, we return leaving pws->func null.

    PyObject* pName = PyUnicode_FromString("emerald");
    if (pName == NULL) {
        print_python_error("Couldn't create python string for module name.");
        return;
    }
    PyObject* pModule = PyImport_Import(pName);
    Py_DECREF(pName);
    if (pModule == NULL) {
        print_python_error("Couldn't import module.");
        return;
    }

    int err = import_cairo();
    if (err < 0) {
        fprintf(stderr, "Couldn't import pycairo C API: %d\n", err);
        return;
    }

    PyObject* pFunc = PyObject_GetAttrString(pModule, "draw");
    Py_DECREF(pModule);
    if (!pFunc) {
        print_python_error("Missing module function");
        return;
    }
    if (!PyCallable_Check(pFunc)) {
        fprintf(stderr, "Module has non-callable attribute where function was expected\n");
        return;
    }

    pws->func = pFunc;
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

