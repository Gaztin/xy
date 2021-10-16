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

#include "xy-desktop.h"

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

	return { .Active=false };

#endif // XY_OS_WINDOWS

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

	return { };

#endif // XY_OS_WINDOWS

} // xyGetPrimaryDesktopMonitor

//////////////////////////////////////////////////////////////////////////

std::vector< xyMonitor > xyGetAllDesktopMonitors( void )
{

#if defined( XY_OS_WINDOWS )

	std::vector< xyMonitor > Monitors;

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

	return Monitors;

#endif // XY_OS_WINDOWS

} // xyGetAllDesktopMonitors

#endif // XY_ENV_DESKTOP
