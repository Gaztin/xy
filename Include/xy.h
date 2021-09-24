/*
 * Copyright (c) 2021 Sebastian Kylander https://gaztin.com/
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

//////////////////////////////////////////////////////////////////////////
/// Pre-processor defines

#if defined( _WIN32 )
/// Windows

#define XY_OS_WINDOWS


#elif defined( __linux__ ) // _WIN32
/// Linux

#define XY_OS_LINUX


#elif defined( __APPLE__ ) // __linux__
/// Apple

#include <TargetConditionals.h>

#if defined( TARGET_OS_OSX )
	#define XY_OS_MACOS
#elif defined( TARGET_OS_IOS ) // TARGET_OS_OSX
	#define XY_OS_IOS
#elif defined( TARGET_OS_WATCH ) // TARGET_OS_IOS
	#define XY_OS_WATCHOS
#elif defined( TARGET_OS_TV ) // TARGET_OS_WATCH
	#define XY_OS_TVOS
#endif // TARGET_OS_TV


#elif defined( __ANDROID__ ) // __APPLE__
/// Android

#define XY_OS_ANDROID


#endif // __ANDROID__

//////////////////////////////////////////////////////////////////////////
/// Implementations

#if defined( XY_IMPLEMENT )

#endif // XY_IMPLEMENT
