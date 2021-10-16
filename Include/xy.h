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
/// Includes

#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <thread>


//////////////////////////////////////////////////////////////////////////
/// Pre-processor defines

#if defined( _WIN32 )
/// Windows

#define XY_OS_WINDOWS
#define XY_ENV_DESKTOP

#elif defined( __APPLE__ ) // _WIN32
/// Apple

#include <TargetConditionals.h>

#if TARGET_OS_OSX
	#define XY_OS_MACOS
	#define XY_ENV_DESKTOP
#elif TARGET_OS_IOS // TARGET_OS_OSX
	#define XY_OS_IOS
	#define XY_ENV_PHONE
#elif TARGET_OS_WATCH // TARGET_OS_IOS
	#define XY_OS_WATCHOS
	#define XY_ENV_WATCH
#elif TARGET_OS_TV // TARGET_OS_WATCH
	#define XY_OS_TVOS
	#define XY_ENV_TV
#endif // TARGET_OS_TV

#elif defined( __ANDROID__ ) // __APPLE__
/// Android

#define XY_OS_ANDROID
#define XY_ENV_PHONE

#elif defined( __linux__ ) // __ANDROID__
/// Linux

#define XY_OS_LINUX
#define XY_ENV_DESKTOP

#endif // __linux__


//////////////////////////////////////////////////////////////////////////
/// Enumerators

enum class xyTheme
{
	Light,
	Dark,

}; // xyTheme


//////////////////////////////////////////////////////////////////////////
/// Data structures

struct xyPlatformImpl;

struct xyContext
{
	std::span< char* >                CommandLineArgs;
	std::unique_ptr< xyPlatformImpl > pPlatformImpl;

}; // xyContext

struct xyRect
{
	int32_t Left   = 0;
	int32_t Top    = 0;
	int32_t Right  = 0;
	int32_t Bottom = 0;

}; // xyRect

struct xyDevice
{
	std::string Name;

}; // xyDevice

struct xyLanguage
{
	std::string LocaleName;

}; // xyLanguage


//////////////////////////////////////////////////////////////////////////
/// Functions

/**
 * Obtains the internal xy context where all the global data is stored
 *
 * @return A reference to the context data.
 */
extern xyContext& xyGetContext( void );

/**
 * Prompts a system message box containing a user-defined message and an 'OK' button.
 * The current thread is blocked until the message box is closed.
 *
 * @param Title The title of the message box window.
 * @param Message The content of the message text box.
 */
extern void xyMessageBox( std::string_view Title, std::string_view Message );

/**
 * Obtains information about the current device.
 *
 * @return The device data.
 */
extern xyDevice xyGetDevice( void );

/**
 * Obtains the preferred theme of this device.
 *
 * @return The theme enumerator.
 */
extern xyTheme xyGetPreferredTheme( void );

/**
 * Obtains the system language.
 *
 * @return The language code.
 */
extern xyLanguage xyGetLanguage( void );
