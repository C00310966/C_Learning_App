CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
CPPFLAGS += -Iinclude
RAYLIB_CFLAGS ?= $(shell pkg-config --cflags raylib)
RAYLIB_LIBS ?= $(shell pkg-config --libs raylib)
CORE = src/main.c src/ui.c src/progress.c src/tracks.c src/lessons.c src/lesson_details.c src/tutorials.c src/tutorials_extra.c src/quizzes.c src/exercises.c
HEADERS = $(wildcard include/*.h)

.PHONY: all clean
all: bin/CodeQuest

bin:
	mkdir -p bin

bin/CodeQuest: $(CORE) $(HEADERS) | bin
	$(CC) $(CPPFLAGS) $(CFLAGS) $(RAYLIB_CFLAGS) $(CORE) -o $@ $(RAYLIB_LIBS) -lm

clean:
	rm -f bin/CodeQuest
