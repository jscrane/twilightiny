BOARD := t85db
SKETCH := twilight3.ino
TERM_SPEED := 115200
CPPFLAGS := -DDEBUG -DTERM_SPEED=$(TERM_SPEED)
include arduino-tiny.mk
