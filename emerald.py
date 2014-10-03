
import cairo
from collections import namedtuple
import math

LeftTopRightBottom = namedtuple('LeftTopRightBottom', 'left top right bottom')

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

def convert_ltrb(arg):
    left, top, right, bottom = arg;
    return LeftTopRightBottom(left=left, top=top, right=right, bottom=bottom)

def draw(ctx, size, space, extents, titlebar_height):
    width, height = size
    space = convert_ltrb(space)
    extents = convert_ltrb(extents)
    
    x1 = space.left - extents.left
    y1 = space.top - extents.top
    x2 = width - space.right + extents.right
    y2 = height - space.bottom + extents.bottom
    top = extents.top + titlebar_height
    
    m1 = min(extents.left, extents.right)
    m2 = min(extents.top, extents.bottom)
    
    border_width = min(m1, m2)
    border_offset = border_width / 2.0
    
#    ctx.save()
    
    ctx.set_line_width(border_width)
    ctx.set_operator(cairo.OPERATOR_SOURCE)
    
    rounded_rectangle(ctx, x1 + border_offset, y1 + top - border_offset, x2 - x1 - border_width, y2 - y1 - top, 0, 0)
    ctx.set_source_rgba(0, 1, 0)
    ctx.stroke()
    
    titlebar_alpha = 1
    if (titlebar_alpha != 0):
        rounded_rectangle(ctx, x1, y1, x2 - x1, top, 0, 0)
        ctx.set_source_rgba(1, 0, 0, titlebar_alpha)
        ctx.fill()
    else:
        ctx.save()
        ctx.set_operator(cairo.OPERATOR_CLEAR)
        ctx.rectangle(0, 0, width, top + y1 - border_width)
        ctx.set_source_rgba(1, 1, 1, 1)
        ctx.fill()
        ctx.restore()
    
#    ctx.restore()

