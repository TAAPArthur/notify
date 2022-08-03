# notify
## Daemon-less notifications

## Install
```
make
make install
```

## Features
* No daemon/dbus dependency
* Core program is about ~100 lines
* No external dependencies except xcb and [dtext-xcb](https://codeberg.org/TAAPArthur/dtext-xcb) (no Xft)
* Can "swipe" to dismiss notifications
* Auto wraps text to fit within a specified width
* Extra features can be disabled on build
* (optionally) replace existing notifications by id
* (optionally) Display on primary monitor (requires xcb-randr)
* Comes with a notify-send wrapper

## Usage

### Exit status
Return 0 on success and non-zero for various exit conditions like timeouts and explicit dismal (see config.def.h)

### Lines breaking
* By default every argument get printed on it own line.
* Lines are split on `\n`

### Position
Can use `-x` `-y` `-w` `-h` to control the position. If width or height is
negative the x is offset from the right or bottom edge respectively.

### Examples
```
notify -x -20 -y 20 -w -40 LN1 LN2
```
Display notification consisting of 2 lines with width of 40px such that there is a 20px gap from the top and right edge

```
notify -w 0 LN1
```
Display notification consisting of 1 lines with width of the entire screen (or primary monitor)

## notify-send
notify-send is included as bonus. By default it just calls notify with the arguments passed, but it can be configured to do various things depending on your settings. It checks `NOTIFY_SEND_PATH` (defaults to ~/.config/notify) for \*.sh files and will source all of them. You can use this to change the settings for the notification depending on whatever you want. Like changing the color or position or just dropping some notifications all together.

## Similar projects
* [herbe](https://github.com/dudik/herbe)
