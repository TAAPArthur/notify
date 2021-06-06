#ifndef NOTIFY_CONFIG_H
#define NOTIFY_CONFIG_H

#define FONT "/usr/share/fonts/TTF/LiberationMono-Regular.ttf:32"
#define PADDING 10

#define THRESHOLD 100
#define MAX_ARGS 20

#define DISMISS_BUTTON 3
#define ACTION_BUTTON 1

#define EXIT_ACTION 0
#define EXIT_DISMISS 2
#define EXIT_TIMEOUT 255

static uint32_t bg_color = 0x3E3E3E;
static union {
    dt_color color;
    uint32_t value;
} color = {.value = 0xECECEC};

static int x = 0;
static int y = 0;
static int height = 0;
static int width = 450;

static int timeout = 5;

#endif
