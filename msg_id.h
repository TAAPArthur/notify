#ifndef NOTIFY_MSG_H
#define NOTIFY_MSG_H

#include <xcb/xcb_ewmh.h>
#define MIN(A,B) (A<B?A:B)

xcb_atom_t notify_id_atom;

static const char* notify_id;
static int seq_num;
static char all_msg_buffer[4096];
static const char* _sentienal = NULL;
static int lastReceivedTimeStamp;

static inline char** getNotificationProperties(xcb_connection_t* dis, xcb_window_t win, xcb_atom_t atom) {
    xcb_get_property_reply_t* reply;
    xcb_get_property_cookie_t cookie = xcb_get_property(dis, 0, win, atom, XCB_ATOM_STRING, 0, -1);
    if((reply = xcb_get_property_reply(dis, cookie, NULL))) {
        int len = xcb_get_property_value_length(reply);
        strncpy(all_msg_buffer, xcb_get_property_value(reply), MIN(len, sizeof(all_msg_buffer)));
        free(reply);
        return &all_msg_buffer;
    }
    return NULL;
}

static inline xcb_atom_t getAtom(xcb_connection_t* dis, const char* name) {
    if(!name)return XCB_ATOM_NONE;
    xcb_intern_atom_reply_t* reply;
    reply = xcb_intern_atom_reply(dis, xcb_intern_atom(dis, 0, strlen(name), name), NULL);
    xcb_atom_t atom = reply->atom;
    free(reply);
    return atom;
}

static inline int hasError(xcb_connection_t* dis, xcb_void_cookie_t cookie) {
    xcb_generic_error_t* e = xcb_request_check(dis, cookie);
    if(e) free(e);
    return e ? 1 : 0;
}

int send_data_to_selection_owner(xcb_connection_t* dis, xcb_window_t win, xcb_atom_t atom, xcb_window_t owner, int ts) {
    int mask = XCB_EVENT_MASK_STRUCTURE_NOTIFY;
    if(hasError(dis, xcb_change_window_attributes_checked(dis, owner, XCB_CW_EVENT_MASK, &mask)))
        return 0;
    const char* id = getenv("HERBE_ID");
    xcb_client_message_event_t event = {
        XCB_CLIENT_MESSAGE,
        32,
        .window = owner,
        .type = atom,
        .data.data32 = {win, ts, 0}
    };

    int xfd = xcb_get_file_descriptor(dis);
	struct pollfd poll_fd = {xfd, POLLIN, 0};
    if(hasError(dis, xcb_send_event_checked(dis, 0, owner, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (char*)&event)))
        return 0;
    while(poll(&poll_fd, 1, 1)) {
        xcb_generic_event_t* event = xcb_poll_for_event(dis);
        if(event->response_type & 127 == XCB_DESTROY_NOTIFY) {
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
    return 0;
}

void set_args_as_properties(xcb_connection_t* dis, xcb_window_t win, xcb_atom_t atom, const char* buffer, int len) {
    xcb_change_property(dis, XCB_PROP_MODE_REPLACE, win, atom, XCB_ATOM_STRING, 8, len, buffer);
}

char* combine_all_args(const char *argv[]) {
    int rem_size = sizeof(all_msg_buffer) - 2;
    for(int i=0;argv[i]  && rem_size > 0; i++) {
        int part_size = strlen(argv[i]);
        strncat(all_msg_buffer, argv[i], rem_size );
        if(argv[i+1])
            strcat(all_msg_buffer, "\n");
        rem_size -= part_size + 1;
    }
    return all_msg_buffer;
}

int maybeSyncWithExistingClientWithId(xcb_connection_t* dis, xcb_window_t win, const char* id, const char* combinedArgs) {
    xcb_atom_t notify_id_atom = getAtom(dis, id );
    set_args_as_properties(dis, win, notify_id_atom, combinedArgs, strlen(combinedArgs));

    xcb_timestamp_t time = XCB_CURRENT_TIME;
    xcb_generic_event_t* event = xcb_poll_for_event(dis);
    if(event->response_type == XCB_PROPERTY_NOTIFY) {
        time = ((xcb_property_notify_event_t*) event)->time;
        if(!seq_num)
            seq_num = ((xcb_property_notify_event_t*) event)->sequence;
    }

    int alreadySet = 0;
    while(1) {
        xcb_get_selection_owner_reply_t* ownerReply = xcb_get_selection_owner_reply(dis, xcb_get_selection_owner(dis,
                                    notify_id_atom), NULL);
        if(alreadySet && !(ownerReply && ownerReply->owner ==  win))
            return -EXIT_TO_SLOW;
        else if(ownerReply && ownerReply->owner) {
            if(ownerReply->owner == win)
                break;
            if(send_data_to_selection_owner(dis, win, ownerReply->owner, notify_id_atom, seq_num)) {
                return -EXIT_COMBINED;
            }
        }
        else if(!hasError(dis, xcb_set_selection_owner_checked(dis, win, notify_id_atom, time)))
            alreadySet = 1;
    }
    return 0;
}

char** handleClientMessage(xcb_connection_t* dis, xcb_client_message_event_t* event, char*** lines) {
    if(event->type == notify_id_atom) {
        xcb_window_t win = event->data.data32[0];
        if(lastReceivedTimeStamp > event->data.data32[1])
            return NULL;
        lastReceivedTimeStamp = event->data.data32[1];
        if(win) {
            char ** data = getNotificationProperties(dis, win, event->type);
            if(data)
                *lines = data;
        }
    }
    return NULL;
}
#endif
