# |-------------------------------------------------------------------------|
# | Environment variables                                                   |
# |-------------------------------------------------------------------------|
# 'VULKAN_SDK'
# 'MVK_CONFIG_USE_METAL_ARGUMENT_BUFFERS'
# |-------------------------------------------------------------------------|
# | Directories                                                             |
# |-------------------------------------------------------------------------|
GLFW_DIR			:= /opt/homebrew/Cellar/glfw/3.4
DEPENDENCY_DIR		:= ./Dependency
IMGUI_DIR			:= $(DEPENDENCY_DIR)/imgui
IMGUI_MISC_DIR		:= $(IMGUI_DIR)/misc
IMGUI_BACKEND_DIR	:= $(IMGUI_DIR)/backends
IMPLOT_DIR			:= $(DEPENDENCY_DIR)/implot
GLM_DIR				:= /opt/homebrew/Cellar/glm/1.0.1
SHADER_DIR			:= ./SandBox/Shader
BUILD_DIR			:= ./Build
BIN_DIR				:= $(BUILD_DIR)/Bin
OBJ_DIR				:= $(BUILD_DIR)/Obj
LOG_DIR				:= $(BUILD_DIR)/Log
# |-------------------------------------------------------------------------|
# | Sources                                                                 |
# |-------------------------------------------------------------------------|
APP_SRCS			:= $(IMGUI_DIR)/imgui.cpp 								\
					   $(IMGUI_DIR)/imgui_draw.cpp 							\
					   $(IMGUI_DIR)/imgui_tables.cpp						\
					   $(IMGUI_DIR)/imgui_widgets.cpp						\
					   $(IMGUI_DIR)/imgui_demo.cpp							\
					   $(IMGUI_MISC_DIR)/cpp/imgui_stdlib.cpp				\
					   $(IMGUI_BACKEND_DIR)/imgui_impl_glfw.cpp				\
					   $(IMGUI_BACKEND_DIR)/imgui_impl_vulkan.cpp			\
					   $(IMPLOT_DIR)/implot.cpp								\
					   $(IMPLOT_DIR)/implot_items.cpp						\
					   $(IMPLOT_DIR)/implot_demo.cpp						\
					   ./main.cpp
VERT_SHADER_SRCS	:= $(wildcard $(SHADER_DIR)/*.vert)
FRAG_SHADER_SRCS	:= $(wildcard $(SHADER_DIR)/*.frag)
# |-------------------------------------------------------------------------|
# | Objects                                                                 |
# |-------------------------------------------------------------------------|
OBJS				:= $(addprefix $(OBJ_DIR)/,								\
					   $(addsuffix .o, $(basename $(notdir $(APP_SRCS)))))
DEPS				:= $(OBJS:.o=.d)
# |-------------------------------------------------------------------------|
# | Naming                                                                  |
# |-------------------------------------------------------------------------|
APP_TARGET			:= EngineV2_EXE
VERT_SHADER_TARGET  := $(foreach file,$(notdir $(VERT_SHADER_SRCS)),		\
					   $(patsubst %.vert,%[VERT].spv,$(file)))
FRAG_SHADER_TARGET	:= $(foreach file,$(notdir $(FRAG_SHADER_SRCS)),		\
					   $(patsubst %.frag,%[FRAG].spv,$(file)))
# |-------------------------------------------------------------------------|
# | Flags                                                                   |
# |-------------------------------------------------------------------------|
GLSLC				:= $(VULKAN_SDK)/bin/glslc
CXX					:= clang++
CXXFLAGS			:= -std=c++23 -Wall -Wextra -O3
LD					:= clang++ -o
LDFLAGS				:= -Wall -pedantic `pkg-config --static --libs glfw3`	\
					   -lvulkan -Wl,-rpath,$(VULKAN_SDK)/lib
RMDIR				:= rm -f
RMDIR				:= rm -r -f
INCLUDES			:= -I$(GLFW_DIR)/include								\
					   -I$(VULKAN_SDK)/include								\
					   -I$(DEPENDENCY_DIR)									\
					   -I$(IMGUI_DIR)										\
					   -I$(IMGUI_MISC_DIR)									\
					   -I$(IMGUI_BACKEND_DIR)								\
					   -I$(IMPLOT_DIR)										\
					   -I$(GLM_DIR)/include
# |-------------------------------------------------------------------------|
# | Rules                                                                   |
# |-------------------------------------------------------------------------|
$(OBJ_DIR)/%.o: $(IMGUI_DIR)/%.cpp
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -MMD -o $@
	@echo "[OK] compile" $<

$(OBJ_DIR)/%.o: $(IMGUI_MISC_DIR)/cpp/%.cpp
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -MMD -o $@
	@echo "[OK] compile" $<

$(OBJ_DIR)/%.o: $(IMGUI_BACKEND_DIR)/%.cpp
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -MMD -o $@
	@echo "[OK] compile" $<

$(OBJ_DIR)/%.o: $(IMPLOT_DIR)/%.cpp
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -MMD -o $@
	@echo "[OK] compile" $<

$(OBJ_DIR)/%.o: %.cpp
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -MMD -o $@
	@echo "[OK] compile" $<

$(APP_TARGET): $(OBJS)
	@$(LD) $(BIN_DIR)/$@ $(LDFLAGS) $^
	@echo "[OK] link"

-include $(DEPS)

%[VERT].spv: $(SHADER_DIR)/%.vert
	@$(GLSLC) $< -o $(BIN_DIR)/$@
	@echo "[OK] compile" $<

%[FRAG].spv: $(SHADER_DIR)/%.frag
	@$(GLSLC) $< -o $(BIN_DIR)/$@
	@echo "[OK] compile" $<
# |-------------------------------------------------------------------------|
# | Targets                                                                 |
# |-------------------------------------------------------------------------|
.PHONY: all directories shaders app clean_logs clean run

all: directories shaders app

directories:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(LOG_DIR)/Scene
	@mkdir -p $(LOG_DIR)/Collection
	@mkdir -p $(LOG_DIR)/Renderer
	@mkdir -p $(LOG_DIR)/SandBox
	@echo "[OK] directories"

shaders: $(VERT_SHADER_TARGET) $(FRAG_SHADER_TARGET)

app: $(APP_TARGET)

clean_logs:
	@$(RM) $(LOG_DIR)/Scene/*
	@$(RM) $(LOG_DIR)/Collection/*
	@$(RM) $(LOG_DIR)/Renderer/*
	@$(RM) $(LOG_DIR)/SandBox/*
	@echo "[OK] clean logs"

clean:
	@$(RMDIR) $(BUILD_DIR)/*
	@echo "[OK] clean"

run:
	$(BIN_DIR)/$(APP_TARGET)