cdef extern from "wnck.h" nogil:
    ctypedef enum WnckWindowState:
        WNCK_WINDOW_STATE_MINIMIZED
        WNCK_WINDOW_STATE_MAXIMIZED_HORIZONTALLY
        WNCK_WINDOW_STATE_MAXIMIZED_VERTICALLY
        WNCK_WINDOW_STATE_SHADED
        WNCK_WINDOW_STATE_SKIP_PAGER
        WNCK_WINDOW_STATE_SKIP_TASKLIST
        WNCK_WINDOW_STATE_STICKY
        WNCK_WINDOW_STATE_HIDDEN
        WNCK_WINDOW_STATE_FULLSCREEN
        WNCK_WINDOW_STATE_DEMANDS_ATTENTION
        WNCK_WINDOW_STATE_URGENT
        WNCK_WINDOW_STATE_ABOVE
        WNCK_WINDOW_STATE_BELOW

    ctypedef enum WnckWindowActions:
        WNCK_WINDOW_ACTION_MOVE
        WNCK_WINDOW_ACTION_RESIZE
        WNCK_WINDOW_ACTION_SHADE
        WNCK_WINDOW_ACTION_STICK
        WNCK_WINDOW_ACTION_MAXIMIZE_HORIZONTALLY
        WNCK_WINDOW_ACTION_MAXIMIZE_VERTICALLY
        WNCK_WINDOW_ACTION_CHANGE_WORKSPACE
        WNCK_WINDOW_ACTION_CLOSE
        WNCK_WINDOW_ACTION_UNMAXIMIZE_HORIZONTALLY
        WNCK_WINDOW_ACTION_UNMAXIMIZE_VERTICALLY
        WNCK_WINDOW_ACTION_UNSHADE
        WNCK_WINDOW_ACTION_UNSTICK
        WNCK_WINDOW_ACTION_MINIMIZE
        WNCK_WINDOW_ACTION_UNMINIMIZE
        WNCK_WINDOW_ACTION_MAXIMIZE
        WNCK_WINDOW_ACTION_UNMAXIMIZE
        WNCK_WINDOW_ACTION_FULLSCREEN
        WNCK_WINDOW_ACTION_ABOVE
        WNCK_WINDOW_ACTION_BELOW