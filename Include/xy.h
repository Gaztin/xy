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
/// Enumerators

enum class xyTheme
{
	Light,
	Dark,

}; // xyTheme


//////////////////////////////////////////////////////////////////////////
/// Data structures

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

struct xyDisplay
{
	uint32_t Width  = 0;
	uint32_t Height = 0;

}; // xyDisplay

struct xyUser
{
	std::string Name;

}; // xyUser


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

/*
 * Obtains information about the current device.
 *
 * @return The device data.
 */
extern xyDevice xyGetDevice( void );

/*
 * Obtains the display of this device.
 * In desktop environments, this is the virtual screen. The virtual screen is the collection of all monitors.
 * To obtain individual desktop monitors, refer to xyGetPrimaryDesktopMonitor or xyGetAllDesktopMonitors.
 *
 * @return The display data.
 */
extern xyDisplay xyGetDisplay( void );

/*
 * Obtains information about the current user on this device.
 *
 * @return The user data.
 */
extern xyUser xyGetUser( void );

/*
 * Obtains the preferred theme of this device.
 *
 * @return The theme enumerator.
 */
extern xyTheme xyGetPreferredTheme( void );


#if defined( XY_ENV_DESKTOP )

//////////////////////////////////////////////////////////////////////////
/// Desktop-specific data structures

struct xyMonitor
{
	std::string Name;
	xyRect      FullRect;
	xyRect      WorkRect;

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
 * Obtain the desktop mouse pointer.
 *
 * @return The mouse data.
 */
extern xyMouse xyGetMouse( void );

/*
 * Obtain the primary monitor for this desktop device.
 *
 * @return The monitor data.
 */
extern xyMonitor xyGetPrimaryDesktopMonitor( void );

/*
 * Gather a list of all desktop monitors for this desktop device.
 *
 * @return A vector of monitor data.
 */
extern std::vector< xyMonitor > xyGetAllDesktopMonitors( void );


#endif // XY_ENV_DESKTOP


#if defined( XY_IMPLEMENT )

//////////////////////////////////////////////////////////////////////////
/// Includes

#if defined( XY_OS_WINDOWS )
#include <windows.h>
#include <lmcons.h>
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

//////////////////////////////////////////////////////////////////////////

xyDevice xyGetDevice( void )
{

#if defined( XY_OS_WINDOWS )

	CHAR  Buffer[ CNLEN + 1 ];
	DWORD Size = static_cast< DWORD >( std::size( Buffer ) );
	if( GetComputerNameA( Buffer, &Size ) )
	{
		return { .Name={ Buffer, Size } };
	}

	return { };

#endif // XY_OS_WINDOWS

} // xyGetDevice

//////////////////////////////////////////////////////////////////////////

xyDisplay xyGetDisplay( void )
{
	xyDisplay Display;

#if defined( XY_OS_WINDOWS )

	Display.Width  = static_cast< uint32_t >( GetSystemMetrics( SM_CXVIRTUALSCREEN ) );
	Display.Height = static_cast< uint32_t >( GetSystemMetrics( SM_CYVIRTUALSCREEN ) );

#endif // XY_OS_WINDOWS

	return Display;

} // xyGetDisplay

//////////////////////////////////////////////////////////////////////////

xyUser xyGetUser( void )
{

#if defined( XY_OS_WINDOWS )

	CHAR  Buffer[ UNLEN + 1 ];
	DWORD Size = static_cast< DWORD >( std::size( Buffer ) );
	if( GetUserNameA( Buffer, &Size ) )
	{
		return { .Name={ Buffer, Size } };
	}

	return { };

#endif // XY_OS_WINDOWS

} // xyGetUser

//////////////////////////////////////////////////////////////////////////

xyTheme xyGetPreferredTheme( void )
{

#if defined( XY_OS_WINDOWS )

	// Courtesy of https://stackoverflow.com/a/51336913
	DWORD AppsUseLightTheme;
	DWORD DataSize = sizeof( AppsUseLightTheme );
	if( RegGetValueA( HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", "AppsUseLightTheme", RRF_RT_REG_DWORD, NULL, &AppsUseLightTheme, &DataSize ) == ERROR_SUCCESS )
		return AppsUseLightTheme ? xyTheme::Light : xyTheme::Dark;

	return xyTheme::Light;

#endif // XY_OS_WINDOWS

} // xyGetPreferredTheme


#if defined( XY_ENV_DESKTOP )

//////////////////////////////////////////////////////////////////////////
/// Desktop-specific functions

xyMouse xyGetMouse( void )
{

#if defined( XY_OS_WINDOWS )

	CURSORINFO Info = { .cbSize=sizeof( CURSORINFO ) };
	if( GetCursorInfo( &Info ) )
	{
		return { .X=Info.ptScreenPos.x, .Y=Info.ptScreenPos.y, .Active=Info.flags==CURSOR_SHOWING };
	}

#endif // XY_OS_WINDOWS

	return { .Active=false };

} // xyGetMouse

//////////////////////////////////////////////////////////////////////////

xyMonitor xyGetPrimaryDesktopMonitor( void )
{

#if defined( XY_OS_WINDOWS )

	HMONITOR       MonitorHandle = MonitorFromWindow( NULL, MONITOR_DEFAULTTOPRIMARY );
	MONITORINFOEXA Info          = { sizeof( MONITORINFOEXA ) };
	if( GetMonitorInfoA( MonitorHandle, &Info ) )
	{
		return { .Name=Info.szDevice,
		         .FullRect { .Left=Info.rcMonitor.left, .Top=Info.rcMonitor.top, .Right=Info.rcMonitor.right, .Bottom=Info.rcMonitor.bottom },
		         .WorkRect { .Left=Info.rcWork   .left, .Top=Info.rcWork   .top, .Right=Info.rcWork   .right, .Bottom=Info.rcWork   .bottom } };
	}

#endif // XY_OS_WINDOWS

	return { };

} // xyGetPrimaryDesktopMonitor

//////////////////////////////////////////////////////////////////////////

std::vector< xyMonitor > xyGetAllDesktopMonitors( void )
{
	std::vector< xyMonitor > Monitors;

#if defined( XY_OS_WINDOWS )

	auto EnumProc = []( HMONITOR MonitorHandle, HDC /*DeviceContextHandle*/, LPRECT /*pRect*/, LPARAM UserData ) -> BOOL
	{
		auto& rMonitors = *reinterpret_cast< std::vector< xyMonitor >* >( UserData );

		MONITORINFOEXA Info = { sizeof( MONITORINFOEXA ) };
		if( GetMonitorInfoA( MonitorHandle, &Info ) )
		{
			xyMonitor Monitor = { .Name=Info.szDevice,
			                      .FullRect { .Left=Info.rcMonitor.left, .Top=Info.rcMonitor.top, .Right=Info.rcMonitor.right, .Bottom=Info.rcMonitor.bottom },
			                      .WorkRect { .Left=Info.rcWork   .left, .Top=Info.rcWork   .top, .Right=Info.rcWork   .right, .Bottom=Info.rcWork   .bottom } };

			rMonitors.emplace_back( std::move( Monitor ) );
		}

		// Always continue
		return TRUE;
	};

	EnumDisplayMonitors( NULL, NULL, EnumProc, reinterpret_cast< LPARAM >( &Monitors ) );

#endif // XY_OS_WINDOWS

	return Monitors;

} // xyGetAllDesktopMonitors

#endif // XY_ENV_DESKTOP


#endif // XY_IMPLEMENT
