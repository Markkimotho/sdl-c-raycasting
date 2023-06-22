# Default recipe
all	: build

# compiler
CC := gcc

# flags
CFLAGS	:= `sdl2-config --libs --cflags` -Wall -Werror -Wextra -pedantic

# linker flags
LFLAGS	:= -lSDL2 -lm

# source files here 
SRCS	:= ./src/game.c 

# Generate the names of object files
# OBJS	:= $(SRCS:.c=.o)

# Name of executable
EXEC	:= game

# Recipe for building the final executable
build:
	$(CC) $(CFLAGS) $(LFLAGS) $(SRCS) -o $(EXEC)
# Clean the workspace
clean:
	rm -f $(EXEC) $(OBJS)

.PHONY: all clean
