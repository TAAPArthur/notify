CFLAGS += -Wall -Wextra -pedantic
LDFLAGS += -ldtext -lxcb
PREFIX ?= /usr
BIN := notify

ifdef NO_XRANDR
	CFLAGS += -DNO_XRANDR
else
	LDFLAGS += -lxcb-randr
endif

ifdef NO_MSD_ID
	CFLAGS += -DNO_MSD_ID
endif

ifdef NO_PARSE_ENV
	CFLAGS += -DNO_PARSE_ENV
endif

ifdef NO_PARSE_ARGS
	CFLAGS += -DNO_PARSE_ARGS
endif

ifdef NO_OVERLAP_DETECTION
	CFLAGS += -DNO_OVERLAP_DETECTION
endif

ifdef DEBUG
	CFLAGS += -g -DDEBUG=1
endif

all: $(BIN)

config.h:
	cp config.def.h config.h

$(BIN): $(BIN).c $(wildcard *.h) config.h
	$(CC) $< $(CFLAGS) -o $@ $(LDFLAGS)

install: $(BIN)
	install -m755 -Dt ${DESTDIR}${PREFIX}/bin $(BIN)
	install -m755 -D notify-send.sh ${DESTDIR}${PREFIX}/bin/notify-send

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/$(BIN)

clean:
	rm -f $(BIN)

.PHONY: all install uninstall clean
