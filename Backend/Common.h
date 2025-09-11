#pragma once
/* GLFW/Vulkan */
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vk_enum_string_helper.h>
/* Icons */
#include <IconFontCppHeaders/IconsFontAwesome6.h>
/* Imgui */
#include <imgui.h>
#include <cpp/imgui_stdlib.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
/* Implot */
#include <implot.h>
#include <implot_internal.h>
/* GLM */
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
/* Container */
#include <set>
#include <map>
#include <vector>
#include <array>
#include <queue>
#include <string>
#include <bitset>
#include <utility>
#include <optional>
#include <unordered_map>
/* Stream */
#include <fstream>
#include <sstream>
#include <iostream>
/* Math */
#include <cmath>
#include <limits>
#include <algorithm>
/* Utility */
#include <chrono>
#include <cstdio>
#include <cstdint>
#include <cfloat>
#include <iomanip>
#include <iterator>
#include <stdexcept>
#include <functional>