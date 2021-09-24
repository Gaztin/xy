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

#include <string_view>
#include <vector>


//////////////////////////////////////////////////////////////////////////
/// Pre-processor defines

#if defined( _WIN32 )
/// Windows

#define XY_OS_WINDOWS
#define XY_ENV_DESKTOP

#elif defined( __linux__ ) // _WIN32
/// Linux

#define XY_OS_LINUX
#define XY_ENV_DESKTOP

#elif defined( __APPLE__ ) // __linux__
/// Apple

#include <TargetConditionals.h>

#if defined( TARGET_OS_OSX )
	#define XY_OS_MACOS
	#define XY_ENV_DESKTOP
#elif defined( TARGET_OS_IOS ) // TARGET_OS_OSX
	#define XY_OS_IOS
	#define XY_ENV_PHONE
#elif defined( TARGET_OS_WATCH ) // TARGET_OS_IOS
	#define XY_OS_WATCHOS
	#define XY_ENV_WATCH
#elif defined( TARGET_OS_TV ) // TARGET_OS_WATCH
	#define XY_OS_TVOS
	#define XY_ENV_TV
#endif // TARGET_OS_TV

#elif defined( __ANDROID__ ) // __APPLE__
/// Android

#define XY_OS_ANDROID
#define XY_ENV_PHONE

#endif // __ANDROID__


//////////////////////////////////////////////////////////////////////////
/// Data structures

struct xyRect
{
	int32_t Left   = 0;
	int32_t Top    = 0;
	int32_t Right  = 0;
	int32_t Bottom = 0;

}; // xyRect


//////////////////////////////////////////////////////////////////////////
/// Functions

/*
* Prompts a system message box containing a user-defined message and two buttons: 'Yes' and 'No'.
*
* @param Title The title of the message box window.
* @param Message The content of the message text box.
* @return True if 'Yes' was clicked, false otherwise.
*/
extern bool xyMessageBox( std::string_view Title, std::string_view Message );


#if defined( XY_ENV_DESKTOP )

//////////////////////////////////////////////////////////////////////////
/// Desktop-specific data structures

struct xyMonitor
{
	xyRect FullRect = { };
	xyRect WorkRect = { };

}; // xyMonitor

struct xyMouse
{
	operator bool( void ) const { return Active; } // Allows `if(auto m = xyGetMouse()) {...}`

	int32_t X      = 0;
	int32_t Y      = 0;
	bool    Active = false; // The mouse may be inactive if the user has a controller or pen plugged in, or if there was an error obtaining the cursor data.

}; // xyMouse


//////////////////////////////////////////////////////////////////////////
/// Desktop-specific functions

/*
 * Obtain the mouse pointer in desktop coordinates.
 *
 * @return The mouse data.
 */
extern xyMouse xyGetMouse( void );

/*
 * Gather a list of all desktop monitors for this device.
 *
 * @return A vector of monitor data.
 */
extern std::vector< xyMonitor > xyGetDesktopMonitors( void );


#endif // XY_ENV_DESKTOP


#if defined( XY_IMPLEMENT )

//////////////////////////////////////////////////////////////////////////
/// Includes

#if defined( XY_OS_WINDOWS )
#include <windows.h>
#endif // XY_OS_WINDOWS


//////////////////////////////////////////////////////////////////////////
/// Functions

bool xyMessageBox( std::string_view Title, std::string_view Message )
{

#if defined( XY_OS_WINDOWS )
	return ( MessageBoxA( NULL, Message.data(), Title.data(), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL ) == IDYES );
#endif // XY_OS_WINDOWS

	return false;

} // xyMessageBox


#if defined( XY_ENV_DESKTOP )

//////////////////////////////////////////////////////////////////////////
/// Desktop-specific functions

xyMouse xyGetMouse( void )
{
	xyMouse Mouse;

#if defined( XY_OS_WINDOWS )

	CURSORINFO Info { .cbSize=sizeof( CURSORINFO ) };
	if( GetCursorInfo( &Info ) )
	{
		Mouse = { .X=Info.ptScreenPos.x, .Y=Info.ptScreenPos.y, .Active=Info.flags==CURSOR_SHOWING };
	}

#endif // XY_OS_WINDOWS

	return Mouse;

} // xyGetMouse

std::vector< xyMonitor > xyGetDesktopMonitors( void )
{
	std::vector< xyMonitor > Monitors;

#if defined( XY_OS_WINDOWS )

	auto EnumProc = []( HMONITOR MonitorHandle, HDC /*DeviceContextHandle*/, LPRECT /*pRect*/, LPARAM UserData ) -> BOOL
	{
		auto& rMonitors = *reinterpret_cast< std::vector< xyMonitor >* >( UserData );

		MONITORINFO Info { .cbSize=sizeof( MONITORINFO ) };
		if( GetMonitorInfoA( MonitorHandle, &Info ) )
		{
			xyMonitor Monitor
			{
				.FullRect { .Left=Info.rcMonitor.left, .Top=Info.rcMonitor.top, .Right=Info.rcMonitor.right, .Bottom=Info.rcMonitor.bottom },
				.WorkRect { .Left=Info.rcWork   .left, .Top=Info.rcWork   .top, .Right=Info.rcWork   .right, .Bottom=Info.rcWork   .bottom },
			};

			rMonitors.emplace_back( std::move( Monitor ) );
		}

		// Always continue
		return TRUE;
	};

	EnumDisplayMonitors( NULL, NULL, EnumProc, reinterpret_cast< LPARAM >( &Monitors ) );

#endif // XY_OS_WINDOWS

	return Monitors;

} // xyGetDesktopMonitors

#endif // XY_ENV_DESKTOP


#endif // XY_IMPLEMENT
