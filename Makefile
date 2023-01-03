CC = gcc

TARGET = PROG
TARGET_PREF = build/

FLAGS = -Wall -Wextra -Werror

LIBS = -lncurses

SCRS = \
	src/game_of_life.c


all: clean configure rebuild

configure:
	@if ! [ -d ./build/ ]; then \
	mkdir "build"; \
	fi

rebuild:
	$(CC) $(FLAGS) $(SCRS) $(LIBS) -o $(TARGET_PREF)$(TARGET)

clean:
	rm -rf build/*
