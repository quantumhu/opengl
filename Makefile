main:  DEBUG = 
maind: DEBUG = -v

INCLUDES = -I./glad/include -I./glfw-3.4/include -I./include
OPT = -Wall -Wextra -g
LINKFLAGS = -L./glfw-3.4/lib-arm64/ -lglfw.3 -rpath ./glfw-3.4/lib-arm64/

C_SOURCES = glad.c
CPP_SOURCES = main.cpp stb_image.cpp

OBJECTS = $(C_SOURCES:.c=.o) $(CPP_SOURCES:.cpp=.o)

main maind: $(OBJECTS)
	clang++ $(DEBUG) $^ -o main $(LINKFLAGS)

%.o: %.cpp
	clang++ $(OPT) $(INCLUDES) -c $^ -o $@

%.o: %.c
	clang $(OPT) $(INCLUDES) -c $^ -o $@

clean:
	rm $(OBJECTS) main

.PHONY: clean