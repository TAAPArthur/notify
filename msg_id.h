#ifndef NOTIFY_MSG_H
#define NOTIFY_MSG_H

#include <assert.h>
#include <poll.h>
#include <string.h>
#include <xcb/xcb_ewmh.h>

#include "config.h"
#include "util.h"
#define MIN(A,B) (A<B?A:B)

xcb_atom_t notify_id_atom;

static const char* MSG_ID;
static int SEQ_NUM;
static int APPEND_MSG;

static inline void getNotificationProperties(xcb_connection_t* dis, xcb_window_t win, xcb_atom_t atom, char* dest) {
    static char buffer[LINE_BUFFER_SIZE];
    char* ptr[] = {buffer, NULL};
    xcb_get_property_reply_t* reply;
    xcb_get_property_cookie_t cookie = xcb_get_property(dis, 0, win, atom, XCB_ATOM_STRING, 0, -1);
    if((reply = xcb_get_property_reply(dis, cookie, NULL))) {
        int len = xcb_get_property_value_length(reply);
        strncpy(buffer, xcb_get_property_value(reply), MIN(len, LINE_BUFFER_SIZE - 1));
        addLines(dest, ptr);
        free(reply);
    }
}

int send_data_to_selection_owner(xcb_connection_t* dis, xcb_window_t win, xcb_window_t owner, xcb_atom_t atom, int timeStamp ) {
    int mask = XCB_EVENT_MASK_STRUCTURE_NOTIFY;
    VERBOSE("Adding destroy listener for selection owner\n");
    if(hasError(dis, xcb_change_window_attributes_checked(dis, owner, XCB_CW_EVENT_MASK, &mask)))
        return 0;
    xcb_client_message_event_t event = {
        XCB_CLIENT_MESSAGE,
        32,
        .window = owner,
        .type = atom,
        .data.data32 = {win, SEQ_NUM ? SEQ_NUM : timeStamp, timeStamp, APPEND_MSG, 0}
    };

    VERBOSE("Sending client message to selection owner\n");
    if(hasError(dis, xcb_send_event_checked(dis, 0, owner, 0, (char*)&event)))
        return 0;

    int xfd = xcb_get_file_descriptor(dis);
    struct pollfd poll_fd = {xfd, POLLIN};
    xcb_flush(dis);

    while(poll(&poll_fd, 1, 1000)) {
        xcb_generic_event_t* event = xcb_poll_for_event(dis);
        VERBOSE("Received event from selection owner: %d\n", event->response_type);
        if((event->response_type & 127) == XCB_DESTROY_NOTIFY) {
            xcb_window_t event_win = ((xcb_destroy_notify_event_t *)event)->event;
            free(event);
            if(event_win == owner)
                return 0;
            if(event_win == win)
                return 1;
        }
        else
            free(event);
    }
    return 1;
}

int maybeSyncWithExistingClientWithId(xcb_connection_t* dis, xcb_window_t win, const char* id, const char* lines) {
    notify_id_atom = getAtom(dis, id );

    const char* start_of_second_line = APPEND_MSG ? strchr(lines, '\n') : NULL;
    const char * lines_to_send = start_of_second_line ? start_of_second_line : lines;
    xcb_change_property(dis, XCB_PROP_MODE_REPLACE, win, notify_id_atom, XCB_ATOM_STRING, 8, strlen(lines_to_send) + 1, lines_to_send);
    xcb_timestamp_t time = XCB_CURRENT_TIME;
    xcb_flush(dis);
    xcb_generic_event_t* event = xcb_wait_for_event(dis);
    VERBOSE("Waiting to detect property notify event on %d\n", win);
    if(event->response_type == XCB_PROPERTY_NOTIFY) {
        time = ((xcb_property_notify_event_t*) event)->time;
    }
    free(event);

    int alreadySet = 0;
    while(1) {
        VERBOSE("Querying selection; Set %d\n", alreadySet);
        xcb_get_selection_owner_reply_t* ownerReply = xcb_get_selection_owner_reply(dis, xcb_get_selection_owner(dis,
                                    notify_id_atom), NULL);
        xcb_window_t owner = ownerReply ? ownerReply->owner: 0 ;
        free(ownerReply);
        if(alreadySet && owner !=  win)
            return -EXIT_TO_SLOW;
        if(owner) {
            VERBOSE("Selection has owner %d\n", owner);
            if(owner == win) {
                break;
            }
            if(send_data_to_selection_owner(dis, win, owner, notify_id_atom, time)) {
                return -EXIT_COMBINED;
            }
        }
        else if(!hasError(dis, xcb_set_selection_owner_checked(dis, win, notify_id_atom, time)))
            alreadySet = 1;
        else
            return -12;
    }
    return 0;
}

int handleClientMessage(xcb_connection_t* dis, xcb_client_message_event_t* event, char* lines, int num_lines) {
    static uint32_t lastReceivedTimeStamp;
    if(event->type == notify_id_atom) {
        xcb_window_t win = event->data.data32[0];
        int append = event->data.data32[3];
        VERBOSE("Received message from a different process; win: %d; append %d\n", win, append);
        if (win) {
            if(lastReceivedTimeStamp > event->data.data32[1]) {
                VERBOSE("Time stamp is old; ignoring message\n");
            }
            else {
                lastReceivedTimeStamp = event->data.data32[1];
                char * dest = lines;
                if (append) {
                    join_lines(lines, num_lines);
                    dest = lines + strlen(lines);
                }
                getNotificationProperties(dis, win, event->type, dest);
            }
            xcb_destroy_window(dis, win);
            return 1;
        }
    }
    return 0;
}
#endif
