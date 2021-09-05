#ifndef NOTIFY_NO_OVERLAP_H
#define NOTIFY_NO_OVERLAP_H

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>
#include <xcb/xcb_ewmh.h>

#include "config.h"
#include "util.h"

enum Edge {
    TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT
};

static int getPositionFromProperty(xcb_connection_t* dis,  xcb_window_t win, xcb_atom_t atom, int* buffer) {
    xcb_get_property_reply_t* reply;
    xcb_get_property_cookie_t cookie = xcb_get_property(dis, 0, win, atom, XCB_ATOM_INTEGER, 0, -1);
    if((reply = xcb_get_property_reply(dis, cookie, NULL)) && xcb_get_property_value_length(reply) == sizeof(int) * 4) {
        memcpy(buffer, xcb_get_property_value(reply), sizeof(int) * 4);
        free(reply);
        return 1;
    }
    return 0;
}

void setIntProperty(xcb_connection_t* dis, xcb_window_t win, xcb_atom_t atom, int* buffer, int len) {
    xcb_change_property(dis, XCB_PROP_MODE_REPLACE, win, atom, XCB_ATOM_INTEGER, 32, len, buffer);
}

int adjust_position_helper(xcb_connection_t* dis, xcb_window_t win, xcb_rectangle_t* monitor_dims) {
    enum Edge alignment;
    if(X - monitor_dims->x <= PADDING_X && Y - monitor_dims->y <= PADDING_Y)
        alignment = TOP_LEFT;
    else if(X + WIDTH - monitor_dims->x - monitor_dims->width <= PADDING_X && Y - monitor_dims->y <= PADDING_Y)
        alignment = TOP_RIGHT;
    else if(X - monitor_dims->x <= PADDING_X && Y +HEIGHT - monitor_dims->y - monitor_dims->height <= PADDING_Y)
        alignment = BOTTOM_LEFT;
    else if(X + WIDTH - monitor_dims->x - monitor_dims->width <= PADDING_X && Y +HEIGHT - monitor_dims->y - monitor_dims->height <= PADDING_Y)
        alignment = BOTTOM_RIGHT;
    else
        return 0;

    char atom_prefix[] = "NOTIFY_POSITION_X";
    atom_prefix[sizeof(atom_prefix) - 2] = '0' + alignment;
    xcb_atom_t position_atom = getAtom(dis, atom_prefix);

    int changed = 0;
    xcb_get_selection_owner_reply_t* ownerReply = xcb_get_selection_owner_reply(dis, xcb_get_selection_owner(dis,
                                position_atom), NULL);
    int existingWindowDims[4];
    if(ownerReply && getPositionFromProperty(dis, ownerReply->owner, position_atom, existingWindowDims)) {
        int original_x = X;
        changed = 1;
        switch(alignment) {
            case TOP_LEFT:
            case TOP_RIGHT:
                if ( existingWindowDims[1] + existingWindowDims[3] + HEIGHT + PADDING_Y > monitor_dims->height)
                    X += existingWindowDims[0] - monitor_dims->x + (PADDING_X+ WIDTH) * ((alignment == TOP_LEFT) * 2 - 1);
                else {
                    Y += existingWindowDims[1] + existingWindowDims[3] - monitor_dims->y + PADDING_Y;
                    X += existingWindowDims[0] - monitor_dims->x;
                }
                break;
            case BOTTOM_LEFT:
            case BOTTOM_RIGHT:
                if (Y - HEIGHT - PADDING_Y < 0)
                    X += existingWindowDims[0] - monitor_dims->x + (PADDING_X + WIDTH) * ((alignment == BOTTOM_LEFT) * 2 - 1);
                else {
                    Y = existingWindowDims[1] - PADDING_Y -  Y -  HEIGHT + monitor_dims->y + monitor_dims->height;
                    X += existingWindowDims[0] - monitor_dims->x;
                }
                break;
        }
        if ((X < 0 || X > monitor_dims->width)) {
            X = original_x;
        }
    }
    free(ownerReply);
    setIntProperty(dis, win, position_atom, &X, 4);
    xcb_set_selection_owner_checked(dis, win, position_atom, XCB_CURRENT_TIME);
    return changed;
}

int adjust_position(xcb_connection_t* dis, xcb_window_t win, xcb_rectangle_t* monitor_dims) {
    int fd = open(LOCK_FILE, O_WRONLY | O_CREAT, 0666);
    int ret = 0;
    if(fd !=-1 && !flock(fd, LOCK_EX)) {
        ret = adjust_position_helper(dis, win, monitor_dims);
    }
    else {
        perror("Could not lock file");
        exit(EXIT_ERR);
    }
    close(fd);
    return ret;
}
#endif
