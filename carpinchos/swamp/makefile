TARGET = ./build/swamp
LIBS = -lmate -lncurses -lpthread -lcommons
CC = gcc
CFLAGS = -g -Wall

default: 
	mkdir -p build && $(MAKE) --no-print-directory $(TARGET)
all: default

run: 
	LD_LIBRARY_PATH="../matelib/build/" $(TARGET)
valgrind: 
	LD_LIBRARY_PATH="../matelib/build/" valgrind --track-origins=yes --leak-check=full $(TARGET)
helgrind: 
	LD_LIBRARY_PATH="../matelib/build/" valgrind --tool=helgrind $(TARGET)

OBJECTS = $(patsubst ./src/%.c, ./build/%.o, $(wildcard ./src/*.c))
HEADERS = $(wildcard /*.h)

.PHONY: default all clean
.PRECIOUS: $(TARGET) $(OBJECTS)

build/%.o: ./src/%.c $(HEADERS)
	$(CC) -I"../matelib/src" $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) -L"../matelib/build" $(OBJECTS) $(CFLAGS) $(LIBS) -o $@

clean: 
	-rm -f ./build/*.o
	-rm -f $(TARGET)