CC  := gcc
SRC := src
OBJ := obj
TST := test
CFLAGS := -c -Wall
DEBUG:= -g
SOURCES := $(wildcard $(SRC)/*.c)
TST_SOURCES := $(wildcard $(TST)/*.c)
OBJECTS := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SOURCES))
TEST_OBJECTS := $(patsubst $(TST)/%.c, $(TST)/%.o, $(TST_SOURCES))
TST_LIBS := -lcheck -lm -lpthread
EXE_NAME := ocache
MULTI_EXE_NAME := ocache_multi

UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
	TST_LIBS += -DNORT
else
	TST_LIBS += -lrt
endif

setup:
	mkdir -p obj

all : clean setup generate_test test

debug: clean setup
debug: CFLAGS += -g -D__DEBUG
debug: $(OBJECTS) $(TEST_OBJECTS)
#debug: generate_test

multi_test: $(OBJECTS) 
	$(CC) $^ $(TST_LIBS) -o $(MULTI_EXE_NAME)

generate_test: $(OBJECTS) $(TEST_OBJECTS)
	$(CC) $^ $(TST_LIBS) -o $(EXE_NAME)

test: $(EXE_NAME)
	./$(EXE_NAME)

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) $< -o $@

$(ST)/%.o: $(TST)/%.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: setup clean all test

clean:
	rm -rf $(OBJ)
