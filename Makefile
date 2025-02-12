# |-------------------------------------------------------------------------|
# | Environment variables                                                   |
# |-------------------------------------------------------------------------|
# 'VULKAN_SDK'
# 'MVK_CONFIG_USE_METAL_ARGUMENT_BUFFERS'
# |-------------------------------------------------------------------------|
# | Directories                                                             |
# |-------------------------------------------------------------------------|
GLFW_DIR            := /opt/homebrew/Cellar/glfw/3.4
GLM_DIR             := /opt/homebrew/Cellar/glm/1.0.1
BUILD_DIR           := ./Build
BIN_DIR             := $(BUILD_DIR)/Bin
OBJ_DIR             := $(BUILD_DIR)/Obj
LOG_DIR             := $(BUILD_DIR)/Log
# |-------------------------------------------------------------------------|
# | Sources                                                                 |
# |-------------------------------------------------------------------------|
APP_SRCS            := ./main.cpp
# |-------------------------------------------------------------------------|
# | Objects                                                                 |
# |-------------------------------------------------------------------------|
OBJS                := $(addprefix $(OBJ_DIR)/,                             \
                       $(addsuffix .o, $(basename $(notdir $(APP_SRCS)))))
DEPS                := $(OBJS:.o=.d)
# |-------------------------------------------------------------------------|
# | Naming                                                                  |
# |-------------------------------------------------------------------------|
APP_TARGET          := EngineV2_EXE
# |-------------------------------------------------------------------------|
# | Flags                                                                   |
# |-------------------------------------------------------------------------|
CXX                 := clang++
CXXFLAGS            := -std=c++23 -Wall -Wextra -O3
LD                  := clang++ -o
LDFLAGS             := -Wall -pedantic `pkg-config --static --libs glfw3`   \
                       -lvulkan -Wl,-rpath,$(VULKAN_SDK)/lib
RM                  := rm -f
RMDIR               := rm -r -f
INCLUDES            := -I$(VULKAN_SDK)/include                              \
                       -I$(GLFW_DIR)/include                                \
                       -I$(GLM_DIR)/include
# |-------------------------------------------------------------------------|
# | Rules                                                                   |
# |-------------------------------------------------------------------------|
$(OBJ_DIR)/%.o: %.cpp
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -MMD -o $@
	@echo "[OK] compile" $<

$(APP_TARGET): $(OBJS)
	@$(LD) $(BIN_DIR)/$@ $(LDFLAGS) $^
	@echo "[OK] link"

-include $(DEPS)
# |-------------------------------------------------------------------------|
# | Targets                                                                 |
# |-------------------------------------------------------------------------|
.PHONY: all directories app clean_logs clean run

all: directories app

directories:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(LOG_DIR)/Scene
	@mkdir -p $(LOG_DIR)/Collection
	@mkdir -p $(LOG_DIR)/Renderer
	@echo "[OK] directories"

app: $(APP_TARGET)

clean_logs:
	@$(RM) $(LOG_DIR)/Scene/*
	@$(RM) $(LOG_DIR)/Collection/*
	@$(RM) $(LOG_DIR)/Renderer/*
	@echo "[OK] clean logs"

clean:
	@$(RMDIR) $(BUILD_DIR)/*
	@echo "[OK] clean"

run:
	$(BIN_DIR)/$(APP_TARGET)