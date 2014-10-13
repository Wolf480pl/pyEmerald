
import cairo
from collections import namedtuple
import math
import os
import sys
import gi

gi.require_version("Gdk", "2.0")
#from gi.repository import Gdk;

wrap_decor = namedtuple("DecorT", "event_windows button_windows button_states tobj_pos tobj_size tobj_item_pos tobj_item_state tobj_item_width" \
                                  " pixmap buffer_pixmap gc width height client_width client_height decorated active layout name icon" \
                                  " icon_pixmap icon_pixbuf state actions prop_xid force_quit_dialog fs button_region min_drawn_buttons_region" \
                                  " draw_only_buttons_region button_last_drawn_state button_fade_info p_active p_active_buffer p_inactive" \
                                  " p_inactive_buffer button_region_inact only_change_active")

theme_dir = os.path.expanduser("~/.emerald/theme/")

def make_filename(sect, key=None, ext=None):
    filename = theme_dir + sect
    if (key != None):
        filename += "." + key
    if (ext != None):
        filename += "." + ext
    return filename


CORNER_TOPLEFT = 1
CORNER_TOPRIGHT = 2
CORNER_BOTTOMRIGHT = 4
CORNER_BOTTOMLEFT = 8

def rounded_rectangle(ctx, x, y, w, h, corner, radius):
    if (radius == 0):
        corner = 0

    if (corner & CORNER_TOPLEFT):
        ctx.move_to(x + radius, y)
    else:
        ctx.move_to(x, y)

    if (corner & CORNER_TOPRIGHT):
        ctx.arc(x + w - radius, y + radius, radius, math.pi * 1.5, math.pi * 2)
    else:
        ctx.line_to(x + w, y)

    if (corner & CORNER_BOTTOMRIGHT):
        ctx.arc(x + w - radius, y + h - radius, radius, 0.0, math.pi * 0.5)
    else:
        ctx.line_to(x + w, y + h)

    if (corner & CORNER_BOTTOMLEFT):
        crx.arc(x + radius, y + h - radius, radius, math.pi * 0.5, math.pi)
    else:
        ctx.line_to(x, y + h)

    if (corner & CORNER_TOPLEFT):
        ctx.arc(x + radius, y + radius, radius, math.pi, math.pi * 1.5)
    else:
        ctx.line_to(x, y)

LeftTopRightBottom = namedtuple('LeftTopRightBottom', 'left top right bottom')

def convert_ltrb(arg):
    left, top, right, bottom = arg;
    return LeftTopRightBottom(left=left, top=top, right=right, bottom=bottom)

script = None

def init():
    global script
    sys.path.insert(0, make_filename(""))
    import engine
    script = engine
    script.init()


def draw(ctx, size, space, extents, titlebar_height, decor=None):
    width, height = size
    space = convert_ltrb(space)
    extents = convert_ltrb(extents)

    if (decor == None):
        pass
    else:
        print(decor)
#        print(decor.event_windows)

    if (script == None):
        raise Exception("No script")

    script.draw(ctx, size, space, extents, titlebar_height)



