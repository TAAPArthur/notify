#include <xcb/randr.h>
static inline void set_rect_to_primary_dimensions(xcb_connection_t*dis, xcb_window_t root, xcb_rectangle_t* rect) {
    xcb_randr_get_monitors_cookie_t cookie = xcb_randr_get_monitors(dis, root, 1);
    xcb_randr_get_monitors_reply_t* monitors = xcb_randr_get_monitors_reply(dis, cookie, NULL);
    if(!monitors)
        return;
    xcb_randr_monitor_info_iterator_t iter = xcb_randr_get_monitors_monitors_iterator(monitors);
    while(iter.rem) {
        xcb_randr_monitor_info_t* monitorInfo = iter.data;
        if(monitorInfo->primary) {
            memcpy(rect, &monitorInfo->x, sizeof(*rect));
            break;
        }
        xcb_randr_monitor_info_next(&iter);
    }
    free(monitors);
}
