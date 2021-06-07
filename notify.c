#include <dtext/dtext.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <xcb/xcb.h>

#include "config.h"
#include "parse_args.h"

#ifndef NO_XRANDR
#include "primary_monitor.h"
#endif

#ifndef NO_MSD_ID
#include "msg_id.h"
#endif

void signalHandler(int sig) {
	exit(sig == SIGALRM ? EXIT_TIMEOUT : EXIT_DISMISS);
}

xcb_screen_t* convertRelativeDims(xcb_connection_t* dis) {
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator (xcb_get_setup (dis));
    xcb_screen_t* screen = iter.data;
    xcb_rectangle_t ref = {0, 0, screen->width_in_pixels, screen->height_in_pixels};
#ifndef NO_XRANDR
    set_rect_to_primary_dimensions(dis, screen->root, &ref);
#endif
    for(int i = 0; i < 2; i++) {
        (&x)[i] += (&ref.x)[i];
        if((&width)[i] < 0)
            (&x)[i] += (&ref.x)[i] + (&ref.width)[i] - ((&width)[i]*=-1);
    }
    if(width == 0)
        width = ref.width;
    return screen;
}

xcb_window_t createWindow(xcb_connection_t* dis, xcb_screen_t* screen) {
    xcb_window_t win = xcb_generate_id(dis);
    uint32_t values [] = {bg_color, border_color , 1, EVENT_MASKS};
    xcb_create_window(dis, XCB_COPY_FROM_PARENT, win, screen->root, x, y, width, height ? height : 1, border_size, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, XCB_CW_BACK_PIXEL| XCB_CW_BORDER_PIXEL  | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK, &values);
    return win;
}

static inline void redraw(xcb_connection_t* dis, xcb_window_t win, dt_context *ctx, dt_font *fnt, char**lines, int*num_lines) {
    xcb_clear_area(dis, 0, win, 0 , 0, width, height);
    int y_offset = y + PADDING ;
    for(int i = 0; i < MAX_ARGS && lines[i]; i++) {
        y_offset = dt_draw_all_lines(ctx, fnt, &color.color, x + PADDING, y_offset, PADDING, lines[i], num_lines[i]);
    }
}

int main(int argc, char *argv[]) {
    char** lines = parseArgs(argv+1);

    xcb_connection_t* dis = xcb_connect(NULL, NULL);
    xcb_screen_t* screen = convertRelativeDims(dis);
    xcb_window_t win = createWindow(dis, screen);

#ifndef NO_MSD_ID
    if(notify_id) {
        int ret = maybeSyncWithExistingClientWithId(dis, win, notify_id, combine_all_args(lines));
        if(ret)
            exit(-ret);
    }
#endif

    dt_context *ctx;
    dt_font *fnt;
    dt_init(&ctx, dis, win);
    dt_load(ctx, &fnt, FONT);

    int num_lines[MAX_ARGS];
    num_lines[0] = word_wrap(lines[0]);
    int totalLines = num_lines[0];
    for(int i = 1; i < MAX_ARGS && lines[i]; i++)
        totalLines += num_lines[i] = word_wrap(lines[i]);
    if(height == 0) {
        height = (get_font_height(fnt) + PADDING) * totalLines;
        xcb_configure_window(dis, win, XCB_CONFIG_WINDOW_HEIGHT, &height);
    }

    redraw(dis, win, ctx, fnt, lines, num_lines);
    xcb_map_window(dis, win);

    struct sigaction action  = {signalHandler};
    sigaction(SIGALRM, &action, NULL);
    alarm(timeout);
    int xRef, press = 0;
    xcb_generic_event_t* event;
    xcb_flush(dis);
    while((event = xcb_wait_for_event(dis))) {
        switch(event->response_type &127) {
            case XCB_EXPOSE:
                redraw(dis, win, ctx, fnt, lines, num_lines);
                break;
            case XCB_MOTION_NOTIFY:
                alarm(timeout);
                if(press)
                    xcb_configure_window(dis, win, XCB_CONFIG_WINDOW_X, (int[1]) {x + ((xcb_motion_notify_event_t*)event)->root_x - xRef});
                break;
            case XCB_BUTTON_PRESS:
                press = 1;
                xRef = ((xcb_button_press_event_t*)event)->root_x;
                break;
            case XCB_BUTTON_RELEASE:
                if(abs(((xcb_button_release_event_t*)event)->root_x - xRef) > THRESHOLD)
                    exit(EXIT_DISMISS);
                else if (((xcb_button_release_event_t*)event)->detail == DISMISS_BUTTON)
                    exit(EXIT_DISMISS);
                else if (((xcb_button_release_event_t*)event)->detail == ACTION_BUTTON)
                    exit(EXIT_ACTION);
#ifndef NO_MSD_ID
            case XCB_CLIENT_MESSAGE:
                handleClientMessage(dis, (xcb_client_message_event_t*)event, &lines);
                redraw(dis, win, ctx, fnt, lines, num_lines);
#endif
        }
        free(event);
        xcb_flush(dis);
	}
}
