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

all: $(BIN)

config.h:
	cp config.def.h config.h

$(BIN): $(BIN).c | config.h
	$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)

install: $(BIN)
	install -m755 -Dt ${DESTDIR}${PREFIX}/bin $(BIN)

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/$(BIN)

clean:
	rm -f $(BIN)

.PHONY: all install uninstall clean
