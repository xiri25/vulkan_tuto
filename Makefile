CC=gcc
CFLAGS=-Wall -Wextra -O2
LDFLAGS=-g -lglfw -lvulkan -ldl -lpthread #-lX11 -lXxf86vm -lXrandr -lXi
SRC_FILES=main.c

.PHONY: all clean

all: vulkan

vulkan: $(SRC_FILES)
	$(CC) $(CFLAGS) -Icglmlib/include -o $@ $^ $(LDFLAGS)

clean:
	rm -f vulkan
