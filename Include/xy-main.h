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

#include "xy.h"


//////////////////////////////////////////////////////////////////////////
/// Global functions

/*
 * Main entry function
 * Define this in your application's main source file!
 */
extern int xyMain( void );


//////////////////////////////////////////////////////////////////////////
/// Platform-specific implementations

#if defined( XY_OS_WINDOWS )

#include <windows.h>

INT WINAPI WinMain( _In_ HINSTANCE Instance, _In_opt_ HINSTANCE /*PrevInstance*/, _In_ LPSTR /*CmdLine*/, _In_ int /*ShowCmd*/ )
{
	xyContext& rContext      = xyGetContext();
	rContext.CommandLineArgs = std::span< char* >( __argv, __argc );
	rContext.pPlatformImpl   = std::make_unique< xyPlatformImpl >();

	// Store the handle to the application instance
	rContext.pPlatformImpl->ApplicationInstanceHandle = Instance;

	return xyMain();

} // WinMain

#elif defined( XY_OS_ANDROID ) // XY_OS_WINDOWS

#include <android/native_activity.h>
#include <fcntl.h>
#include <unistd.h>

[[maybe_unused]] JNIEXPORT void ANativeActivity_onCreate( ANativeActivity* pActivity, void* /*pSavedState*/, size_t /*SavedStateSize*/ )
{
	xyContext& rContext    = xyGetContext();
	rContext.pPlatformImpl = std::make_unique< xyPlatformImpl >();

	// Store the activity data
	rContext.pPlatformImpl->pNativeActivity = pActivity;

	// Obtain the configuration
	rContext.pPlatformImpl->pConfiguration = AConfiguration_new();
	AConfiguration_fromAssetManager( rContext.pPlatformImpl->pConfiguration, rContext.pPlatformImpl->pNativeActivity->assetManager );

	// Obtain the looper for the main thread
	ALooper* pMainLooper = ALooper_forThread();
	ALooper_acquire( pMainLooper );

	// Listen for data on the main thread
	pipe2( rContext.pPlatformImpl->JavaThreadPipe, O_NONBLOCK | O_CLOEXEC );
	ALooper_addFd( pMainLooper, rContext.pPlatformImpl->JavaThreadPipe[ 0 ], 0, ALOOPER_EVENT_INPUT, []( int Read, int /*Events*/, void* /*pData*/ ) -> int
	{
		xyRunnable* pRunnable;
		if( read( Read, &pRunnable, sizeof( pRunnable ) ) == sizeof( pRunnable ) )
		{
			pRunnable->Execute();
		}

		// Keep listening
		return 1;

	}, nullptr );

	std::thread AppThread( &xyMain );
	AppThread.detach();

} // ANativeActivity_onCreate

#elif defined( XY_OS_IOS ) // XY_OS_ANDROID

#include <thread>

#include <UIKit/UIKit.h>

@interface xyAppDelegate : NSObject< UIApplicationDelegate >
@end // xyAppDelegate

@implementation xyAppDelegate

-( BOOL )application:( UIApplication* )application didFinishLaunchingWithOptions:( NSDictionary* )launchOptions
{
	std::thread Thread( &xyMain );
	Thread.detach();

} // didFinishLaunchingWithOptions

@end // xyAppDelegate

int main( int ArgC, char** ppArgV )
{
	xyContext& rContext = xyGetContext();
	rContext.CommandLineArgs = std::span< char* >( ppArgV, ArgC );

	@autoreleasepool
	{
		return UIApplicationMain( ArgC, ppArgV, nil, NSStringFromClass( [ xyAppDelegate class ] ) );
	}

} // main

#else // XY_OS_IOS

int main( int ArgC, char** ppArgV )
{
	xyContext& rContext = xyGetContext();
	rContext.CommandLineArgs = std::span< char* >( ppArgV, ArgC );

	return xyMain();

} // main

#endif // !XY_OS_WINDOWS && !XY_OS_ANDROID && !XY_OS_IOS
