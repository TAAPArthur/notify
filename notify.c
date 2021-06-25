#include <dtext/dtext.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <xcb/xcb.h>

#include "debug.h"
#include "config.h"
#include "parse_args.h"
#include "parse_env.h"
#include "util.h"

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
        (&X)[i] += (&ref.x)[i];
        if((&WIDTH)[i] < 0)
            (&X)[i] += (&ref.x)[i] + (&ref.width)[i] - ((&WIDTH)[i]*=-1);
        if((&WIDTH)[i] == 0 && (FIXED_HEIGHT || i == 0))
            (&WIDTH)[i] = (&ref.width)[i];
    }
    VERBOSE("DIMS: %d %d %d %d\n", X, Y, WIDTH, HEIGHT);
    return screen;
}

xcb_window_t createWindow(xcb_connection_t* dis, xcb_screen_t* screen) {
    xcb_window_t win = xcb_generate_id(dis);
    uint32_t values [] = {BG_COLOR, BORDER_COLOR , 1, EVENT_MASKS};
    xcb_create_window(dis, XCB_COPY_FROM_PARENT, win, screen->root, X, Y, WIDTH, HEIGHT ? HEIGHT : 1, BORDER_SIZE, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, XCB_CW_BACK_PIXEL| XCB_CW_BORDER_PIXEL  | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK, &values);
    return win;
}

static inline void redraw(xcb_connection_t* dis, xcb_window_t win, dt_context *ctx, dt_font *fnt, char**lines, int*num_lines) {
    xcb_clear_area(dis, 0, win, 0 , 0, WIDTH, HEIGHT);
    int y_offset = 0;
    for(int i = 0; i < MAX_ARGS && num_lines[i] && lines[i] ; i++) {
        y_offset = dt_draw_all_lines(ctx, fnt, (dt_color*)&FG_COLOR, PADDING_X, y_offset, PADDING_Y, lines[i], num_lines[i]);
    }
}

void resize(xcb_connection_t* dis, xcb_window_t win, dt_font *fnt, int totalLines) {
    if(FIXED_HEIGHT)
        return;
    HEIGHT = (dt_get_font_height(fnt) + PADDING_Y) * totalLines;
    xcb_configure_window(dis, win, XCB_CONFIG_WINDOW_HEIGHT, &HEIGHT);
    VERBOSE("DIMS: %d %d %d %d\n", X, Y, WIDTH, HEIGHT);
}
int main(int argc, char *argv[]) {
#ifndef NO_PARSE_ENV
    parseEnv();
#endif
    char** lines = parseArgs(argv+1);

    xcb_connection_t* dis = xcb_connect(NULL, NULL);
    if(!dis)
        exit(EXIT_ERR);
    xcb_screen_t* screen = convertRelativeDims(dis);
    xcb_window_t win = createWindow(dis, screen);

#ifndef NO_MSD_ID
    if(NOTIFY_ID) {
        int ret = maybeSyncWithExistingClientWithId(dis, win, NOTIFY_ID, combine_all_args(lines));
        if(ret)
            exit(-ret);
    }
#endif

    dt_context *ctx;
    dt_font *fnt;
    dt_init_context(&ctx, dis, win);
    dt_load_font(dis, &fnt, FONT_NAME, FONT_SIZE);

    int num_lines[MAX_ARGS];
    int totalLines = 0;
    for(int i = 0; i < MAX_ARGS && lines[i]; i++)
        totalLines += num_lines[i] = dt_word_wrap_line(dis, fnt, lines[i], WIDTH);
    if(HEIGHT == 0) {
        resize(dis, win, fnt, totalLines);
    }

    redraw(dis, win, ctx, fnt, lines, num_lines);
    xcb_map_window(dis, win);

    struct sigaction action  = {signalHandler};
    sigaction(SIGALRM, &action, NULL);
    alarm(TIMEOUT);
    int xRef, press = 0;
    xcb_generic_event_t* event;
    xcb_flush(dis);
    while((event = xcb_wait_for_event(dis))) {
        VERBOSE("Received event of type %d \n", event->response_type);
        switch(event->response_type &127) {
            case XCB_EXPOSE:
                redraw(dis, win, ctx, fnt, lines, num_lines);
                break;
            case XCB_MOTION_NOTIFY:
                alarm(TIMEOUT);
                if(press)
                    xcb_configure_window(dis, win, XCB_CONFIG_WINDOW_X, (int[1]) {X + ((xcb_motion_notify_event_t*)event)->root_x - xRef});
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
                break;
            case XCB_DESTROY_NOTIFY:
                if(((xcb_destroy_notify_event_t*)event)->event == win)
                    exit(EXIT_DISMISS);
                break;
#ifndef NO_MSD_ID
            case XCB_CLIENT_MESSAGE:
                if(handleClientMessage(dis, (xcb_client_message_event_t*)event, &lines)) {
                    alarm(TIMEOUT);
                    num_lines[0] = dt_word_wrap_line(dis, fnt, lines[0], WIDTH);
                    num_lines[1] = 0;
                    resize(dis, win, fnt, num_lines[0]);
                    redraw(dis, win, ctx, fnt, lines, num_lines);
                }
                break;
#endif
        }
        free(event);
        xcb_flush(dis);
	}
    exit(EXIT_UNKNOWN);
}
