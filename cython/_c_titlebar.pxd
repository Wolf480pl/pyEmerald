from _c_gtypes cimport *

cdef extern from "titlebar.h" nogil:
    enum: EMERALD_TITLEBAR_H
    enum: PRESSED_EVENT_WINDOW
    enum: BX_COUNT

    enum buttons:
        B_CLOSE
        B_MAXIMIZE
        B_RESTORE
        B_MINIMIZE
        B_HELP
        B_MENU
        B_SHADE
        B_UNSHADE
        B_ABOVE
        B_UNABOVE
        B_STICK
        B_UNSTICK
        B_COUNT
    enum states:
        S_ACTIVE
        S_ACTIVE_HOVER
        S_ACTIVE_PRESS
        S_INACTIVE
        S_INACTIVE_HOVER
        S_INACTIVE_PRESS
        S_COUNT
    enum btypes:
        B_T_CLOSE
        B_T_MAXIMIZE
        B_T_MINIMIZE
        B_T_HELP
        B_T_MENU
        B_T_SHADE
        B_T_ABOVE
        B_T_STICKY
        B_T_COUNT
    gboolean btbistate[B_T_COUNT*1]
    int btstateflag[B_T_COUNT*1]
    enum tbtypes:
        TBT_CLOSE=B_T_CLOSE
        TBT_MAXIMIZE=B_T_MAXIMIZE
        TBT_MINIMIZE=B_T_MINIMIZE
        TBT_HELP=B_T_HELP
        TBT_MENU=B_T_MENU
        TBT_SHADE=B_T_SHADE
        TBT_ONTOP=B_T_ABOVE
        TBT_STICKY=B_T_STICKY
        TBT_TITLE=B_T_COUNT
        TBT_ICON
        TBT_ONBOTTOM
        TBT_COUNT

    guint button_actions[B_T_COUNT*1]
    gchar * b_types[]
    gchar * b_names[]
    enum _enum_:
        DOUBLE_CLICK_SHADE=0
        DOUBLE_CLICK_MAXIMIZE
        DOUBLE_CLICK_MINIMIZE
        TITLEBAR_ACTION_COUNT
    gchar * titlebar_action_name[TITLEBAR_ACTION_COUNT*1]
