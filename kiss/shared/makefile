CC = gcc
CFLAGS = -fPIC -g -Wall -Wextra
LDFLAGS = -shared
LIBS = -lcommons -pthread

SOURCE_DIR = src
INCLUDE_DIR = include
TARGET_DIR = build

APPLICATIONS = shared

OBJECTS = $(patsubst $(SOURCE_DIR)/%.c, $(TARGET_DIR)/%.o, $(wildcard $(SOURCE_DIR)/*.c))
HEADERS = $(wildcard $(INCLUDE_DIR)/*.h)

.PHONY : buildDirectoryCreation all clean

all : buildDirectoryCreation $(APPLICATIONS)

buildDirectoryCreation:
	@echo "About to create $(TARGET_DIR)/ (if it doesn't exist) ..."
	mkdir -p $(TARGET_DIR)

$(TARGET_DIR)/%.o: $(SOURCE_DIR)/%.c $(HEADERS)
	@echo "About to compile $< ..."
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(APPLICATIONS): $(OBJECTS)
	@echo "About to link $(OBJECTS) with $(LIBS) and create $(APPLICATIONS) ..."
	$(CC) $(LDFLAGS) $(CFLAGS) $(OBJECTS) $(LIBS) -o $(TARGET_DIR)/lib$@.so

clean:
	@echo "About to delete objects and applications in $(TARGET_DIR)/ ..."
	rm -f $(TARGET_DIR)/*.o
	rm -f $(TARGET_DIR)/$(APPLICATIONS)
