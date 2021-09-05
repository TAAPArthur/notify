#ifndef NOTIFY_CONFIG_H
#define NOTIFY_CONFIG_H

#include <stdint.h>
#include <xcb/xcb.h>

#define PADDING_X 10
#define PADDING_Y 0
#define THRESHOLD 100
#define MAX_ARGS 20

#define EXIT_ACTION   0
#define EXIT_DISMISS  2
#define EXIT_TO_SLOW  3
#define EXIT_COMBINED 4
#define EXIT_ERR      5
#define EXIT_UNKNOWN  6
#define EXIT_TIMEOUT  255

uint8_t DISMISS_BUTTON = 3;
uint8_t ACTION_BUTTON = 1;

static uint32_t FG_COLOR = 0xECECEC;
static uint32_t BG_COLOR = 0x3E3E3E;
static uint32_t BORDER_COLOR = 0xFF;
static uint32_t BORDER_SIZE = 1;

static int32_t X = 0;
static int32_t Y = 0;
static int32_t WIDTH = 450;
static int32_t HEIGHT = 0;
static uint8_t FIXED_HEIGHT = 0;

static const uint32_t EVENT_MASKS = XCB_EVENT_MASK_BUTTON_PRESS |XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_STRUCTURE_NOTIFY;

static int TIMEOUT = 5;

const char* FONT_NAME = "/usr/share/fonts/TTF/LiberationMono-Regular.ttf";
int FONT_SIZE = 32;
#endif
