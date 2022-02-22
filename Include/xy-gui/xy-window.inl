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

#include "xy-window.h"

#include <utility>

#ifdef XY_OS_WINDOWS
#include <Windows.h>
#endif // XY_OS_WINDOWS

//////////////////////////////////////////////////////////////////////////


xyWindow::xyWindow( xyWindow&& rrOther )
	: m_pPlatformHandle( std::exchange( rrOther.m_pPlatformHandle, nullptr ) )
{

} // xyWindow

//////////////////////////////////////////////////////////////////////////

xyWindow::xyWindow( xySize DesiredSize )
{
	// Desktop is the only UI mode that supports multiple windows
	static int NumWindowsCreated = 0;
	if( ++NumWindowsCreated > 1 )
		xyGetContext().CompatibleUIModes = XY_UI_MODE_DESKTOP;

#ifdef XY_OS_WINDOWS

	static const WNDCLASSEXW ClassDesc{ sizeof( WNDCLASSEXW ), CS_VREDRAW | CS_HREDRAW, DefWindowProcW, 0, 0, NULL, NULL, NULL, CreateSolidBrush( RGB( 255, 0, 255 ) ), NULL, L"xyWindow" };
	static auto              Class    = RegisterClassExW( &ClassDesc );

	m_pPlatformHandle = CreateWindowExW( WS_EX_OVERLAPPEDWINDOW, L"xyWindow", L"xyWindow", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, ( int )DesiredSize.Width, ( int )DesiredSize.Height, NULL, NULL, NULL, NULL );

#endif // XY_OS_WINDOWS

} // xyWindow

//////////////////////////////////////////////////////////////////////////

xyWindow::~xyWindow()
{

#ifdef XY_OS_WINDOWS
	DestroyWindow( ( HWND )m_pPlatformHandle );
#endif // XY_OS_WINDOWS

} // ~xyWindow

//////////////////////////////////////////////////////////////////////////

void xyWindow::Show()
{

#ifdef XY_OS_WINDOWS
	ShowWindow( ( HWND )m_pPlatformHandle, SW_SHOW );
#endif // XY_OS_WINDOWS

} // Show

//////////////////////////////////////////////////////////////////////////

void xyWindow::PollEvents()
{

#ifdef XY_OS_WINDOWS

	MSG Message;
	while( PeekMessageW( &Message, ( HWND )m_pPlatformHandle, 0, 0, PM_REMOVE ) )
	{
		TranslateMessage( &Message );
		DispatchMessageW( &Message );
	}

#endif // XY_OS_WINDOWS

} // PollEvents

//////////////////////////////////////////////////////////////////////////

bool xyWindow::IsOpen() const
{

#ifdef XY_OS_WINDOWS
	return IsWindow( ( HWND )m_pPlatformHandle );
#endif // XY_OS_WINDOWS

} // IsOpen

