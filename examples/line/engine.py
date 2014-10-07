import cairo
from emerald import *

def init():
    pass

def draw(ctx, size, space, extents, titlebar_height):
    width, height = size

    x1 = space.left - extents.left
    y1 = space.top - extents.top
    x2 = width - space.right + extents.right
    y2 = height - space.bottom + extents.bottom
    top = extents.top + titlebar_height

    m1 = min(extents.left, extents.right)
    m2 = min(extents.top, extents.bottom)

    border_width = min(m1, m2)
    border_offset = border_width / 2.0

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

