TARGET = ./build/libmate.so
CC = gcc

default: 
	mkdir -p build && $(MAKE) --no-print-directory $(TARGET)
all: default

OBJECTS = $(patsubst ./src/%.c, ./build/%.o, $(wildcard ./src/*.c))

.PHONY: default all clean
.PRECIOUS: $(TARGET) $(OBJECTS)

build/%.o: ./src/%.c
	$(CC) -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

$(TARGET): $(OBJECTS)
	$(CC) -shared -o "build/libmate.so" $(OBJECTS)

clean: 
	-rm -rf build