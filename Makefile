VERSION := \"1.1\"
RM      := rm
CC      ?= clang
CFLAGS  := -std=gnu99 -Wall -O2 -pedantic -march=native -DVERSION=$(VERSION)
LDFLAGS :=
SOURCES := $(shell find . -name "*.c" -type f -print)
OBJECTS := $(SOURCES:%.c=%.o)
DEPS    := $(OBJECTS:%.o=%.d)
PROG    := kwc
PREFIX  ?= /usr/local
INSTALL ?= /usr/bin/install

ifeq ($(VERBOSE),1)
  Q =
else
  Q = @
endif

all: $(PROG)

$(PROG): $(OBJECTS)
	@echo "LD		$@"
	$(Q)$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	@echo "CC		$@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

%.d: %.c
	@echo "DEP		$@"
	$(Q)$(CC) $(CFLAGS) -MM -MF $@ -MT $*.o $<

clean:
	@echo "CLEAN"
	$(Q)$(RM) -f *.o *.d $(PROG)

install: $(PROG)
	@echo "Installing		$@"
	$(Q)$(INSTALL) -m 0755 $(PROG) $(PREFIX)/bin

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif

.PHONY: all clean install
