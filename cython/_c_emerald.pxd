from _c_gtypes cimport *
from stubs cimport *
from _c_window cimport *
from _c_titlebar cimport *
#from _c_decoration cimport *

cdef extern from "emerald.h" nogil:
    enum: GDK_DISABLE_DEPRECATED
    enum: GTK_DISABLE_DEPRECATED
    enum: DBUS_API_SUBJECT_TO_CHANGE
    enum: WNCK_I_KNOW_THIS_IS_UNSTABLE
    enum: CAIRO_EXTEND_PAD
    enum: FAKE_WINDOW_ACTION_HELP
    enum: WM_MOVERESIZE_SIZE_TOPLEFT
    enum: WM_MOVERESIZE_SIZE_TOP
    enum: WM_MOVERESIZE_SIZE_TOPRIGHT
    enum: WM_MOVERESIZE_SIZE_RIGHT
    enum: WM_MOVERESIZE_SIZE_BOTTOMRIGHT
    enum: WM_MOVERESIZE_SIZE_BOTTOM
    enum: WM_MOVERESIZE_SIZE_BOTTOMLEFT
    enum: WM_MOVERESIZE_SIZE_LEFT
    enum: WM_MOVERESIZE_MOVE
    enum: WM_MOVERESIZE_SIZE_KEYBOARD
    enum: WM_MOVERESIZE_MOVE_KEYBOARD
    enum: SHADOW_RADIUS
    enum: SHADOW_OPACITY
    enum: SHADOW_COLOR_RED
    enum: SHADOW_COLOR_GREEN
    enum: SHADOW_COLOR_BLUE
    enum: SHADOW_OFFSET_X
    enum: SHADOW_OFFSET_Y
    enum: MWM_HINTS_DECORATIONS
    enum: MWM_DECOR_ALL
    enum: MWM_DECOR_BORDER
    enum: MWM_DECOR_HANDLE
    enum: MWM_DECOR_TITLE
    enum: MWM_DECOR_MENU
    enum: MWM_DECOR_MINIMIZE
    enum: MWM_DECOR_MAXIMIZE
    enum: BLUR_TYPE_NONE
    enum: BLUR_TYPE_TITLEBAR
    enum: BLUR_TYPE_ALL
    enum: PROP_MOTIF_WM_HINT_ELEMENTS
    enum: SWITCHER_SPACE
    enum: SWITCHER_TOP_EXTRA
    enum: SHADE_LEFT
    enum: SHADE_RIGHT
    enum: SHADE_TOP
    enum: SHADE_BOTTOM
    enum: CORNER_TOPLEFT
    enum: CORNER_TOPRIGHT
    enum: CORNER_BOTTOMRIGHT
    enum: CORNER_BOTTOMLEFT

    ctypedef struct MwmHints:
        unsigned long flags
        unsigned long functions
        unsigned long decorations

    ctypedef struct decor_color_t:
        double r
        double g
        double b

    #ctypedef void (*event_callback) (WnckWindow *win, XEvent *event)
    cdef struct _alpha_color:
        decor_color_t color
        double alpha
    ctypedef _alpha_color alpha_color
    cdef struct _pos_t:
        int x, y, w, h
        int xw, yh, ww, hh
    ctypedef _pos_t pos_t
    ctypedef _frame_settings frame_settings
    cdef struct _icon_size:
        int w
        int h
    cdef struct _window_settings:
        void * engine_ws
        gint button_offset
        gint button_hoffset
        gchar * tobj_layout
        gint double_click_action
        gint button_hover_cursor
        gboolean round_top_left
        gboolean round_top_right
        gboolean round_bottom_left
        gboolean round_bottom_right
        frame_settings * fs_act
        frame_settings * fs_inact
        gint min_titlebar_height
        gboolean use_pixmap_buttons # = FALSE
        double corner_radius # = 5.0
        PangoAlignment title_text_align # = PANGO_ALIGN_CENTER
        GdkPixbuf * ButtonPix[S_COUNT*B_COUNT]
        GdkPixbuf * ButtonArray[<int>B_COUNT]

        gboolean use_button_glow
        gboolean use_button_inactive_glow
        gboolean use_decoration_cropping
        gboolean use_button_fade
        GdkPixbuf * ButtonGlowPix[<int>B_COUNT]
        GdkPixbuf * ButtonGlowArray
        GdkPixbuf * ButtonInactiveGlowArray
        GdkPixbuf * ButtonInactiveGlowPix[<int>B_COUNT]
        int button_fade_num_steps        # number of steps
        int button_fade_step_duration    # step duration in milliseconds
        int button_fade_pulse_len_steps  # length of pulse (number of steps)
        int button_fade_pulse_wait_steps # how much pulse waits before fade out
        gdouble shadow_radius
        gdouble shadow_opacity
        gint shadow_color[3]
        gint shadow_offset_x
        gint shadow_offset_y
        decor_extents_t shadow_extents # = { 0, 0, 0, 0 }
        decor_extents_t win_extents    # = { 6, 6, 4, 6 }
        pos_t pos[3][3]
        gint left_space   # = 6
        gint right_space  # = 6
        gint top_space    # = 4
        gint bottom_space # = 6

        gint left_corner_space   # = 0
        gint right_corner_space  # = 0
        gint top_corner_space    # = 0
        gint bottom_corner_space # = 0

        gint titlebar_height # = 17 #Titlebar Height

        gint normal_top_corner_space # = 0

        gint shadow_left_space       # = 0
        gint shadow_right_space      # = 0
        gint shadow_top_space        # = 0
        gint shadow_bottom_space     # = 0

        gint shadow_left_corner_space    # = 0
        gint shadow_right_corner_space   # = 0
        gint shadow_top_corner_space     # = 0
        gint shadow_bottom_corner_space  # = 0

        GdkPixmap *shadow_pixmap        # = NULL
        GdkPixmap *large_shadow_pixmap  # = NULL
        GdkPixmap *decor_normal_pixmap  # = NULL
        GdkPixmap *decor_active_pixmap  # = NULL

        cairo_pattern_t *shadow_pattern # = NULL

        gint text_height

        PangoFontDescription *font_desc
        PangoContext * pango_context

        decor_extents_t switcher_extents  # = { 0, 0, 0, 0 }
        GdkPixmap *switcher_pixmap        # = NULL
        GdkPixmap *switcher_buffer_pixmap # = NULL
        gint switcher_width
        gint switcher_height

        gint switcher_top_corner_space    # = 0
        gint switcher_bottom_corner_space # = 0

        _icon_size c_icon_size[<int>B_COUNT]
        _icon_size c_glow_size  # one glow size for all buttons
                                # (buttons will be centered in their glows)
                                # active and inactive glow pixmaps are assumed to be of same size

        gboolean stretch_sides
        gint blur_type # = BLUR_TYPE_NONE
    ctypedef _window_settings window_settings

    cdef struct _frame_settings:
        void * engine_fs
        window_settings *ws
        alpha_color button
        alpha_color button_halo
        alpha_color text
        alpha_color text_halo

    cdef struct _rectangle:
        gint x1, y1, x2, y2
    ctypedef _rectangle rectangle_t

    cdef struct _button_fade_info:
        gpointer * d# needed by the timer function
        cairo_t * cr
        double y1
        int counters[<int>B_COUNT] # 0: not fading, > 0: fading in, < 0: fading out
                                  # max value: ws->button_fade_num_steps+1 (1 is reserved to indicate
                                  #                                         fade-in initiation)
                                  # min value: -ws->button_fade_num_steps
        gboolean pulsating[<int>B_T_COUNT]
        gint timer
        gboolean first_draw
    ctypedef _button_fade_info button_fade_info_t

    cdef struct _button_region_t:
        gint base_x1, base_y1, base_x2, base_y2 # button coords with no glow
        gint glow_x1, glow_y1, glow_x2, glow_y2 # glow coordinates
        
        # holds whether this button's glow overlap with the other button's non-glow (base) area
        gboolean overlap_buttons[<int>B_T_COUNT]
        GdkPixmap * bg_pixmap
    ctypedef _button_region_t button_region_t

    cdef struct _decor:
        Window event_windows[3][3]
        Window button_windows[<int>B_COUNT]
        guint button_states[<int>B_COUNT]
        gint tobj_pos[3]
        gint tobj_size[3]
        gint tobj_item_pos[11]
        gint tobj_item_state[11]
        gint tobj_item_width[11]
        GdkPixmap *pixmap
        GdkPixmap *buffer_pixmap
        GdkGC *gc
        gint width
        gint height
        gint client_width
        gint client_height
        gboolean decorated
        gboolean active
        PangoLayout *layout
        gchar *name
        cairo_pattern_t *icon
        GdkPixmap *icon_pixmap
        GdkPixbuf *icon_pixbuf
        WnckWindowState state
        WnckWindowActions actions
        XID prop_xid
        GtkWidget *force_quit_dialog
        frame_settings * fs
        void (*draw) (_decor *d)
        button_region_t button_region[<int>B_COUNT]
        rectangle_t min_drawn_buttons_region # minimal rectangle enclosing all drawn regions
        gboolean draw_only_buttons_region
        gint button_last_drawn_state[<int>B_COUNT] # last drawn state or fade counter
        button_fade_info_t button_fade_info
        GdkPixmap * p_active
        GdkPixmap * p_active_buffer
        GdkPixmap * p_inactive
        GdkPixmap * p_inactive_buffer
        button_region_t button_region_inact[<int>B_COUNT]
        gboolean only_change_active
    ctypedef _decor decor_t
