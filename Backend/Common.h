#pragma once
/* GLFW/Vulkan */
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vk_enum_string_helper.h>
/* GLM */
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
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
#include <cstdint>
#include <iomanip>
#include <iterator>
#include <stdexcept>
#include <functional>