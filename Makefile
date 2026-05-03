# =============================================================================
# Makefile for disk-monitor
# =============================================================================

CC          := gcc
CFLAGS      := -Wall -Wextra -Wpedantic -std=c11 -O2
TARGET      := disk-monitor
SRC         := main.c
OBJ         := $(SRC:.c=.o)

PREFIX      ?= /usr/local
BINDIR      := $(PREFIX)/bin
MANDIR      := $(PREFIX)/share/man/man1

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

install: $(TARGET) disk-monitor.1
	install -d $(DESTDIR)$(BINDIR)
	install -d $(DESTDIR)$(MANDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/
	install -m 644 disk-monitor.1 $(DESTDIR)$(MANDIR)/
	gzip -f $(DESTDIR)$(MANDIR)/disk-monitor.1
	@echo "Installed binary to $(DESTDIR)$(BINDIR)/$(TARGET)"
	@echo "Installed man page to $(DESTDIR)$(MANDIR)/disk-monitor.1.gz"

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -f $(DESTDIR)$(MANDIR)/disk-monitor.1.gz
	@echo "Uninstalled disk-monitor"

run: $(TARGET)
	./$(TARGET)

test: run
