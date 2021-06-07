#ifndef NOTIFY_UTIL_H
#define NOTIFY_UTIL_H

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
    if(e) {
        free(e);
    }
    return e ? 1 : 0;
}
#endif

