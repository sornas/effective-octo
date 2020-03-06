GAME = effective-octo-disco
SOURCE = src/game.c
FOG_FOLDER = fog

CC = gcc
WARNINGS = -Werror -Wall
FLAGS = $(WARNINGS) -ggdb -std=c11
LIB_FOLDER = lib

ENGINE =
ifeq ($(shell uname -s),Darwin)
	ENGINE = $(LIB_FOLDER)/libfog.dylib
endif
ifeq ($(shell uname -s),Linux)
	ENGINE = $(LIB_FOLDER)/libfog.a
endif
# Would be nice to remove some of these...
LIBS = -lfog -lSDL2 -lSDL2main -ldl -lpthread -lc -lm
ifeq ($(shell uname -s),Darwin)
	LIBS += -lc++
    endif
INCLUDES = -Iinc

ASSET_BUILDER = $(FOG_FOLDER)/out/mist
ASSET_FILE = data.fog
ASSETS = $(shell find res/ -type f -name "*.*")
HEADERS = $(shell find src/ -type f -name "*.h")
SRCS = $(shell find src/ -type f -name "*.c")
OBJS = $(SRCS:src/%.c=%.o)

.PHONY: default run game engine update-engine clean $(ENGINE) all

default: game
all: clean update-engine run
game: $(GAME) $(ASSET_FILE)
engine: $(ENGINE)

run: $(GAME)
	./$<

$(ASSET_FILE): $(ASSETS) $(ASSET_BUILDER)
	$(ASSET_BUILDER) -o $@ $(ASSETS)

# $(GAME): $(ENGINE) $(SRCS) $(HEADERS)
# 	$(CC) $(FLAGS) -o $@ $(SOURCE) -L$(LIB_FOLDER) $(LIBS) $(INCLUDES)

$(GAME): $(ENGINE) $(OBJS)
	$(CC) $(FLAGS) $(OBJS) -o $@ -L$(LIB_FOLDER) $(LIBS)

%.o: src/%.c $(HEADERS)
	$(CC) $(FLAGS) -c $< -o $@ $(INCLUDES)

$(ASSET_BUILDER): $(ENGINE)

$(LIB_FOLDER):
	mkdir -p $@

update-engine:
	@git submodule update --remote

$(ENGINE): | $(LIB_FOLDER)
	make -C $(FOG_FOLDER) engine
	@cp $(FOG_FOLDER)/out/libfog.* $(LIB_FOLDER)/
	@mkdir -p inc
	@cp $(FOG_FOLDER)/out/fog.h inc/

clean:
	make -C $(FOG_FOLDER) clean
	rm -rf $(LIB_FOLDER)
	rm -f $(GAME)
	rm -f $(ASSET_FILE)
	rm -f *.o

