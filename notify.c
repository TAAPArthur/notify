#define DTEXT_XCB_IMPLEMENTATION
#include "dtext_xcb.h"
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <xcb/xcb.h>

#include "config.h"
#include "debug.h"
#include "msg_id.h"
#include "no_overlap.h"
#include "parse_args.h"
#include "parse_env.h"
#include "primary_monitor.h"
#include "util.h"

static char lines[LINE_BUFFER_SIZE];

void signalHandler(int sig) {
    exit(sig == SIGALRM ? EXIT_TIMEOUT : EXIT_DISMISS);
}

static xcb_rectangle_t monitor_dims;
xcb_screen_t* convertRelativeDims(xcb_connection_t* dis) {
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator (xcb_get_setup (dis));
    xcb_screen_t* screen = iter.data;
    monitor_dims = (xcb_rectangle_t){0, 0, screen->width_in_pixels, screen->height_in_pixels};
#ifndef NO_XRANDR
    set_rect_to_primary_dimensions(dis, screen->root, &monitor_dims );
#endif
    for(int i = 0; i < 2; i++) {
        (&X)[i] += (&monitor_dims.x)[i];
        if((&WIDTH)[i] < 0)
            (&X)[i] += (&monitor_dims.width)[i] - ((&WIDTH)[i]*=-1) - BORDER_SIZE*2;
        if((&WIDTH)[i] == 0 && (FIXED_HEIGHT || i == 0))
            (&WIDTH)[i] = (&monitor_dims.width)[i];
    }
    VERBOSE("Monitor dims %d %d %d %d\n", monitor_dims.x, monitor_dims.y, monitor_dims.width, monitor_dims.height);
    return screen;
}

xcb_window_t createWindow(xcb_connection_t* dis, xcb_screen_t* screen) {
    xcb_window_t win = xcb_generate_id(dis);
    uint32_t values [] = {BG_COLOR, BORDER_COLOR , 1, EVENT_MASKS};
    xcb_create_window(dis, XCB_COPY_FROM_PARENT, win, screen->root, X, Y, WIDTH, HEIGHT ? HEIGHT : 1, BORDER_SIZE, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, XCB_CW_BACK_PIXEL| XCB_CW_BORDER_PIXEL  | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK, &values);
    return win;
}

static inline void redraw(xcb_connection_t* dis, xcb_window_t win, dt_context *ctx, dt_font *fnt, char* lines, int num_lines) {
    VERBOSE("DIMS: %d %d %d %d\n", X, Y, WIDTH, HEIGHT);
    xcb_clear_area(dis, 0, win, 0 , 0, WIDTH, HEIGHT);
    int y_offset = 0;
    dt_draw_all_lines(ctx, fnt, FG_COLOR, PADDING_X, y_offset, PADDING_Y, lines, num_lines);
}

void resize(xcb_connection_t* dis, xcb_window_t win, dt_font *fnt, int totalLines) {
    if(FIXED_HEIGHT)
        return;
    HEIGHT = (dt_get_font_height(fnt) + PADDING_Y) * totalLines;
    xcb_configure_window(dis, win, XCB_CONFIG_WINDOW_HEIGHT, &HEIGHT);
}

int addLines(char* dest, char* const* src) {
    while(*src) {
        if(dest + strlen(*src) + 1 >= lines + LINE_BUFFER_SIZE) {
            return 0;
        }
        dest = stpcpy(dest, *src++);
        if(*src) {
            *dest = '\n';
            dest++;
        }
    }
    return 1;
}

int main(int argc, char *argv[]) {
#ifndef NO_PARSE_ENV
    parseEnv();
#endif
    addLines(lines, parseArgs(argv+1));

    xcb_connection_t* dis = xcb_connect(NULL, NULL);
    if(!dis)
        exit(EXIT_ERR);
    xcb_screen_t* screen = convertRelativeDims(dis);
    xcb_window_t win = createWindow(dis, screen);

#ifndef NO_MSD_ID
    if(MSG_ID) {
        int ret = maybeSyncWithExistingClientWithId(dis, win, MSG_ID, lines);
        if(ret)
            exit(-ret);
    }
#endif

    dt_context *ctx = dt_create_context(dis, win);
    dt_font *fnt = dt_load_font(dis, FONT_NAME, FONT_SIZE);
    if (!fnt)
        exit(EXIT_FONT);

    int totalLines = dt_word_wrap_line(dis, fnt, lines, WIDTH - PADDING_X * 2);

    VERBOSE("Detected %d initial lines\n", totalLines);
    if(HEIGHT == 0) {
        resize(dis, win, fnt, totalLines);
    }
    VERBOSE("Initial Size %d %d %d %d\n", X,Y,WIDTH,HEIGHT);

#ifndef NO_OVERLAP_DETECTION
    if(adjust_position(dis, win, &monitor_dims)) {
        xcb_configure_window(dis, win, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, (int[2]) {X,Y });
    }
#endif

    redraw(dis, win, ctx, fnt, lines, totalLines);
    VERBOSE("Mapping window\n");
    xcb_map_window(dis, win);

    struct sigaction action  = {.sa_handler = signalHandler};
    sigaction(SIGALRM, &action, NULL);
    alarm(TIMEOUT);
    int xRef, press = 0;
    xcb_generic_event_t* event;
    xcb_flush(dis);
    while((event = xcb_wait_for_event(dis))) {
        VERBOSE("Received event of type %d \n", event->response_type);
        switch(event->response_type & 127) {
            case XCB_EXPOSE:
                redraw(dis, win, ctx, fnt, lines, totalLines);
                break;
            case XCB_MOTION_NOTIFY:
                if(press)
                    xcb_configure_window(dis, win, XCB_CONFIG_WINDOW_X, (int[1]) {X + ((xcb_motion_notify_event_t*)event)->root_x - xRef});
                break;
            case XCB_BUTTON_PRESS:
                press = 1;
                xRef = ((xcb_button_press_event_t*)event)->root_x;
                alarm(0);
                break;
            case XCB_BUTTON_RELEASE:
                if(abs(((xcb_button_release_event_t*)event)->root_x - xRef) > THRESHOLD)
                    exit(EXIT_DISMISS);
                else if (((xcb_button_release_event_t*)event)->detail == DISMISS_BUTTON)
                    exit(EXIT_DISMISS);
                else if (((xcb_button_release_event_t*)event)->detail == ACTION_BUTTON)
                    exit(EXIT_ACTION);
                alarm(TIMEOUT);
                break;
            case XCB_DESTROY_NOTIFY:
                if(((xcb_destroy_notify_event_t*)event)->event == win)
                    exit(EXIT_DISMISS);
                break;
#ifndef NO_MSD_ID
            case XCB_CLIENT_MESSAGE:
                if(handleClientMessage(dis, (xcb_client_message_event_t*)event, lines, totalLines)) {
                    alarm(TIMEOUT);
                    totalLines = dt_word_wrap_line(dis, fnt, lines, WIDTH - PADDING_X * 2);
                    resize(dis, win, fnt, totalLines);
                    redraw(dis, win, ctx, fnt, lines, totalLines);
                }
                break;
#endif
        }
        free(event);
        xcb_flush(dis);
    }
    exit(EXIT_UNKNOWN);
}
