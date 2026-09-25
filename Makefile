CC = cl65
CFLAGS = -t c64 -O
TARGET = raycast.prg

SRCS = src/tables.c src/map.c src/raycast.c src/render.c src/main.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

tables: generate_tables.py
	python generate_tables.py

clean:
	rm -f $(TARGET) src/*.o *.o

.PHONY: all tables clean
