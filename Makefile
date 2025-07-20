CC=gcc
CFLAGS=-Wall -Wextra -O2
LDFLAGS=-g -Lglfw_lib/build/src -lglfw3 -lvulkan -ldl -lpthread -lm
SRC_FILES=main.c

.PHONY: all clean

all: vulkan

vulkan: $(SRC_FILES)
	$(CC) $(CFLAGS) -Icglmlib/include -Istb_lib -Iglfw_lib/include -o $@ $^ $(LDFLAGS)

clean:
	rm -f vulkan
