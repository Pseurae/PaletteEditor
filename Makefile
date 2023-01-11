TOOLCHAIN 	?= 

CC			:= $(TOOLCHAIN)gcc
CXX			:= $(TOOLCHAIN)g++

GLEWFLAGS   := $(shell pkg-config --cflags glew)
GLFWFLAGS	:= $(shell pkg-config --cflags glfw3)
FTFLAGS		:= $(shell pkg-config --cflags freetype2)

LIBS		:= $(shell pkg-config --libs glew) $(shell pkg-config --libs glfw3) $(shell pkg-config --libs freetype2)

UNAME	 	:= $(shell uname -s)

ifeq ($(UNAME), Darwin)
LIBS := $(LIBS) -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
endif

FLAGS		:= -I. -Ithird_party/imgui -Ithird_party/tinyfiledialogs -O3 -ffunction-sections -MMD $(GLFWFLAGS) $(GLEWFLAGS) $(FTFLAGS)
CFLAGS		:= $(FLAGS) 
CXXFLAGS	:= $(FLAGS) -std=c++20

HEADERS 	:= $(shell find . -name "*.h") $(shell find . -name "*.hpp")
CSRC		:= $(shell find . -name "*.c")
CPPSRC		:= $(shell find . -name "*.cpp")

OBJ_DIR 	:= build
OBJ_NAMES	:= $(CSRC:.c=.o) $(CPPSRC:.cpp=.o)
OBJ_PATHS	:= $(OBJ_NAMES:./%=$(OBJ_DIR)/%)
DEP_NAMES	:= $(OBJ_PATHS:%.o=%.d)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(shell dirname "$@")
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(shell dirname "$@")
	$(CXX) $(CXXFLAGS) -c $< -o $@

all: $(DEP_PATHS) $(OBJ_PATHS)
	$(CXX) $(CXXFLAGS) -o palette-editor $(OBJ_PATHS) $(LIBS)
	@echo Done.

.PHONY: all

-include $(DEP_NAMES)

clean:
	rm -rf $(OBJ_DIR)
	rm -f palette-editor
