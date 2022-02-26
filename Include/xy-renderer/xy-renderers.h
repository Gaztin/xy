/*
 * Copyright (c) 2022 Sebastian Kylander https://gaztin.com/
 *
 * This software is provided 'as-is', without any express or implied warranty. In no event will
 * the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not claim that you wrote the
 *    original software. If you use this software in a product, an acknowledgment in the product
 *    documentation would be appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be misrepresented as
 *    being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#pragma once

#include "../xy.h"

#if __has_include( <vulkan/vulkan.h> )
#define XY_HAS_VULKAN 1
#pragma comment( lib, "vulkan-1.lib" )
#endif // __has_include( <vulkan/vulkan.h> )

#if __has_include( <d3d11.h> )
#define XY_HAS_D3D11 1
#pragma comment( lib, "d3d11.lib" )
#endif // __has_include( <d3d11.h> )
