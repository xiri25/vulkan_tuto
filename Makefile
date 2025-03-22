CC=gcc
CFLAGS=-Wall -Wextra
LDFLAGS=-g -lglfw -lvulkan -ldl -lpthread #-lX11 -lXxf86vm -lXrandr -lXi
SRC_FILES=main.c

.PHONY: all clean

all: vulkan

vulkan: $(SRC_FILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f vulkan
