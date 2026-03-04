#ifndef PCH_H
#define PCH_H

// Windows and Graphics
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>

// Standard Library
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <atomic>
#include <fstream>
#include <unordered_map>
#include <numbers>
#include <mutex>
#include <cmath>
#include <algorithm>

// MinHook
#include "minhook/include/MinHook.h"

// ImGui Headers
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"
#include <imgui_internal.h>

// SDK Headers
#include "ReadyOrNot_SDK/SDK/ReadyOrNot_parameters.hpp"
#include "ReadyOrNot_SDK/SDK/ReadyOrNot_classes.hpp"
#include "ReadyOrNot_SDK/SDK/Engine_classes.hpp"
#include "ReadyOrNot_SDK/SDK/CoreUObject_classes.hpp"
#include "ReadyOrNot_SDK/SDK/CoreUObject_parameters.hpp"
#include "ReadyOrNot_SDK/SDK/Engine_parameters.hpp"
#include "ReadyOrNot_SDK/SDK/Basic.hpp"

using namespace SDK;

// Project Core Headers
#include "Utils/Localization.h"
#include "Utils/Utils.h"
#include "Cheats.h"

#endif //PCH_H
