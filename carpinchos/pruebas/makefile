LIBS = -lmate -lcommons -lpthread
CC = gcc
CFLAGS = -g3 -Wall
INCLUDES = -L"../matelib/build" -I"../matelib/src"
LIBRARY = LD_LIBRARY_PATH=../matelib/build

%: %.c
	$(CC) $(INCLUDES) $< -o $@.out $(CFLAGS) $(LIBS)
	$(LIBRARY) ./$@.out

clean:
	rm -f *.out