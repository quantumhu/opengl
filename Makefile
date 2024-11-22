DEBUG = 
# DEBUG = -v

INCLUDES = -I./include/glad -I./include
OPT = -Wall -Wextra -g -Wno-deprecated-declarations
LINKFLAGS = -L./lib/glfw-3.4/lib-arm64/ -lglfw.3 -rpath ./lib/glfw-3.4/lib-arm64/

SRC_DIR   = src
BUILD_DIR = build
OBJ_MODEL_DIR = assets
EXE       = $(BUILD_DIR)/main

C_SOURCES   = $(wildcard $(SRC_DIR)/*.c)
CPP_SOURCES = $(wildcard $(SRC_DIR)/*.cpp)

C_OBJECTS   = $(C_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
CPP_OBJECTS = $(CPP_SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

all: $(EXE)

$(EXE): $(C_OBJECTS) $(CPP_OBJECTS)
	clang++ $(DEBUG) $^ -o $@ $(LINKFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	clang++ $(OPT) $(INCLUDES) -c $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	clang $(OPT) $(INCLUDES) -c $^ -o $@

$(BUILD_DIR):
	mkdir -p $@

3dobjs:
	python3 src/convert_to_vertices.py --search-path $(OBJ_MODEL_DIR) --cpp-output-path $(SRC_DIR) --header-output-path include -z

clean:
	rm -rf $(BUILD_DIR)

.PHONY: clean