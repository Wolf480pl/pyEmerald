from _c_emerald cimport *
#from _c_py3cairo import *

import sys
import emerald

script = None

cdef public void pyemerald_init():
    global script
    sys.path.insert(0, emerald.make_filename(""))
    import engine
    script = engine
    script.init()

cdef public gboolean pyemerald_draw_frame(decor_t* decor, object ctx):
    width = decor.width
    height = decor.height
    size = (width, height)
    cdef frame_settings *fs = decor.fs
    cdef window_settings *ws = fs.ws
    space = emerald.convert_ltrb((ws.left_space, ws.top_space, ws.right_space, ws.bottom_space))
    extents = emerald.convert_ltrb((ws.win_extents.left, ws.win_extents.top, ws.win_extents.right, ws.win_extents.bottom))

    if script is None:
        return False

    script.draw(ctx, size, space, extents, ws.titlebar_height)
    return True
