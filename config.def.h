#ifndef NOTIFY_CONFIG_H
#define NOTIFY_CONFIG_H

#define FONT "/usr/share/fonts/TTF/LiberationMono-Regular.ttf:32"
#define PADDING 10

#define THRESHOLD 100
#define MAX_ARGS 20

#define DISMISS_BUTTON 3
#define ACTION_BUTTON 1

#define EXIT_ACTION   0
#define EXIT_DISMISS  2
#define EXIT_TO_SLOW  3
#define EXIT_COMBINED 4
#define EXIT_TIMEOUT  255

static uint32_t bg_color = 0x3E3E3E;
static union {
    dt_color color;
    uint32_t value;
} color = {.value = 0xECECEC};

static uint16_t x = 0;
static uint16_t y = 0;
static uint16_t height = 0;
static uint16_t width = 450;

static const uint32_t EVENT_MASKS = XCB_EVENT_MASK_BUTTON_PRESS |XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_PROPERTY_CHANGE;

static int timeout = 5;

#endif
