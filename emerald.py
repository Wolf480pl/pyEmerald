
import cairo
from collections import namedtuple
import math
import os
import sys
import gi

gi.require_version("Gdk", "2.0")
#from gi.repository import Gdk;

DecorT = namedtuple("DecorT", "event_windows button_windows button_states tobj_pos tobj_size tobj_item_pos tobj_item_state tobj_item_width" \
                                  " pixmap buffer_pixmap gc width height client_width client_height decorated active layout name icon" \
                                  " icon_pixmap icon_pixbuf state actions prop_xid force_quit_dialog fs button_region min_drawn_buttons_region" \
                                  " draw_only_buttons_region button_last_drawn_state button_fade_info p_active p_active_buffer p_inactive" \
                                  " p_inactive_buffer button_region_inact only_change_active")

WindowSettings = namedtuple("WindowSettings", "engine_ws button_offset button_hoffset tobj_layout double_click_action button_hover_cursor" \
                                             " round_top_left round_top_right round_bottom_left round_bottom_right fs_act fs_inact" \
                                             " min_titlebar_height use_pixmap_buttons corner_radius title_text_align ButtonPix ButtonArray" \
                                             " use_button_glow use_button_inactive_glow use_decoration_cropping use_button_fade" \
                                             " ButtonGlowPix ButtonGlowArray ButtonInactiveGlowArray ButtonInactiveGlowPix" \
                                             " button_fade_num_steps button_fade_step_duration button_fade_pulse_len_steps button_fade_pulse_wait_steps"\
                                             " shadow_radius shadow_opacity shadow_color shadow_offset_x shadow_offset_y shadow_extents win_extents" \
                                             " pos left_space right_space top_space bottom_space" \
                                             " left_corner_space right_corner_space top_corner_space bottom_corner_space titlebar_height" \
                                             " normal_top_corner_space shadow_left_space shadow_right_space shadow_top_space shadow_bottom_space" \
                                             " shadow_left_corner_space shadow_right_corner_space shadow_top_corner_space shadow_bottom_corner_space" \
                                             " shadow_pixmap large_shadow_pixmap decor_normal_pixmap decor_active_pixmap shadow_pattern text_height" \
                                             " font_desc pango_context switcher_extents switcher_pixmap switcher_buffer_pixmap" \
                                             " switcher_width switcher_height switcher_top_corner_space switcher_bottom_corner_space"\
                                             " c_icon_size c_glow_size stretch_sides blur_type")

def _wrap_windowsettings(**kw):
    #print("ws:", len(kw))
    ws = WindowSettings(**kw)
    #print(ws)
    return ws

FrameSettings = namedtuple("FrameSettings", "engine_fs ws button button_halo text text_halo")

ButtonRegion = namedtuple("ButtonRegion", "base_x1 base_y1 base_x2 base_y2" \
                                         " glow_x1 glow_y1 glow_x2 glow_y2" \
                                         " overlap_buttons bg_pixmap")

ButtonFadeInfo = namedtuple("ButtonFadeInfo", "cr y1 counters pulsating timer first_draw")

Rectangle = namedtuple("Rectangle", "x1 y1 x2 y2")

def _wrap_rectangle(x1, y1, x2, y2):
    return Rectangle._make((x1, y1, x2, y2))

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
#        print("draw")
#        print(decor.event_windows)

    if (script == None):
        raise Exception("No script")

    script.draw(ctx, size, space, extents, titlebar_height)



