CC           = gcc
CFLAGS      ?= -O2 -pipe -Wall -Wextra -pedantic
STRIP        = strip
INSTALL      = install
UNAME        = uname

PREFIX       = /usr/local

OS           = $(shell $(UNAME))
LUA_INCDIR   = $(PREFIX)/include
LUA_SHAREDIR = $(PREFIX)/share/lua/5.1
LUA_LIBDIR   = $(PREFIX)/lib/lua/5.1

ifeq ($(OS),Darwin)
SHARED       = -dynamiclib -Wl,-undefined,dynamic_lookup
STRIP_ARGS   = -u -r
else
SHARED       = -shared
endif

ifdef NDEBUG
CFLAGS      += -DNDEBUG
endif

clibs = ircparse

.PHONY: all strip install clean
.PRECIOUS: %.pic.o

all: $(clibs:%=%.so)

%.pic.o: %.c
	@echo '  CC $@'
	@$(CC) $(CFLAGS) -fPIC -nostartfiles -I'$(LUA_INCDIR)' -c $< -o $@

%.so: %.pic.o
	@echo '  LD $@'
	@$(CC) $(SHARED) $(LDFLAGS) $^ -o $@

libdir-install:
	@echo "  INSTALL -d $(LUA_LIBDIR)"
	@$(INSTALL) -d $(DESTDIR)$(LUA_LIBDIR)

%.so-install: %.so libdir-install
	@echo "  INSTALL $<"
	@$(INSTALL) $< $(DESTDIR)$(LUA_LIBDIR)/$<

install: $(clibs:%=%.so-install)

%-strip: %
	@echo '  STRIP $<'
	@$(STRIP) $(STRIP_ARGS) $<

strip: $(clibs:%=%.so-strip)

clean:
	rm -f $(clibs:%=%.so) *.o *.c~ *.h~
