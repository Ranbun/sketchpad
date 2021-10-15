#pragma once

// only header files that are external to your project (rarely changed) AND needed
// by most of your source files (often used) should be included in the precompiled
// header, do not blindly include every single external library header file, avoid
// inclusion of headers that are included only in few source files.

// you can also include your own headers if you know they are hardly ever changed.

// as long as you follow this rule, compilation will be significantly faster, at
// the cost of some disk space. The overhead is that some sources will include
// some headers that are not needed (but are inside #include "pch.h" anyway), so
// the compiled binaries of these translation units will be slightly larger in
// size, but this overhead is trivial in most cases (unless your project is huge).

// STL and Boost headers typically belong in the precompiled header file

#include <algorithm>
#include <array>
#include <bitset>
#include <chrono>
#include <cmath>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <initializer_list>
#include <map>
#include <memory>
#include <numeric>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#pragma warning(push)
#pragma warning(disable : 4505)
#include <GL/glew.h>
#include <GL/freeglut.h>
#pragma warning(pop)

#define GLM_ENABLE_EXPERIMENTAL

#pragma warning(push)
#pragma warning(disable : 4201)
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/perpendicular.hpp>
#pragma warning(pop)

#define IMGUI_DISABLE_METRICS_WINDOW
#define IMGUI_DEFINE_MATH_OPERATORS

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_impl_glut.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imstb_rectpack.h>
#include <imgui/imstb_textedit.h>
#include <imgui/imstb_truetype.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>