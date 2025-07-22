CC=gcc
CFLAGS=-Wall -Wextra -O2
LDFLAGS=-g -Lglfw_lib/build/src -lglfw3 -lvulkan -ldl -lpthread -lm
SRC_FILES=main.c

.PHONY: all clean

all: vulkan

vulkan: $(SRC_FILES)
	$(CC) $(CFLAGS) -Icglmlib/include -Istb_lib -Iglfw_lib/include -DPROFILE_WITH_TRACY=0 -o $@ $^ $(LDFLAGS)
	
vulkan_tracy:
	# TODO: Is hardcoded
	gcc -Wall -Wextra -O2 -Icglmlib/include -Istb_lib -Iglfw_lib/include -DPROFILE_WITH_TRACY=1 -c main.c -g
	g++ -o $@ main.o -Lglfw_lib/build/src -Ltracy/ -lglfw3 -lvulkan -ldl -lpthread -lm -lTracyClient

clean:
	rm -f vulkan
	rm -f vulkan_tracy
