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

#include "xy-platforms/xy-android.h"
#include "xy-platforms/xy-ios.h"
#include "xy-platforms/xy-macos.h"
#include "xy-platforms/xy-windows.h"


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
	rContext.UIMode          = XY_UI_MODE_DESKTOP;

	// Store the handle to the application instance
	rContext.pPlatformImpl->ApplicationInstanceHandle = Instance;

	return xyMain();

} // WinMain

#elif defined( XY_OS_MACOS ) // XY_OS_WINDOWS

#include <locale.h>

int main( int ArgC, char** ppArgV )
{
	xyContext& rContext = xyGetContext();
	rContext.CommandLineArgs = std::span< char* >( ppArgV, ArgC );
	rContext.UIMode          = XY_UI_MODE_DESKTOP;

	setlocale( LC_CTYPE, "UTF-8" );

	return xyMain();

} // main

#elif defined( XY_OS_ANDROID ) // XY_OS_MACOS

#include <android/native_activity.h>
#include <fcntl.h>
#include <unistd.h>

[[maybe_unused]] JNIEXPORT void ANativeActivity_onCreate( ANativeActivity* pActivity, void* /*pSavedState*/, size_t /*SavedStateSize*/ )
{
	xyContext& rContext    = xyGetContext();
	rContext.pPlatformImpl = std::make_unique< xyPlatformImpl >();

	// Store the activity data
	rContext.pPlatformImpl->pNativeActivity = pActivity;

	// Set up callbacks
	pActivity->callbacks->onInputQueueCreated = []( ANativeActivity* /*pActivity*/, AInputQueue* pQueue )
	{
		xyContext& rContext = xyGetContext();
		ALooper*   pLooper  = ALooper_prepare( 0 );

		pipe2( rContext.pPlatformImpl->InputQueuePipe, O_NONBLOCK | O_CLOEXEC );
		AInputQueue_attachLooper( pQueue, pLooper, rContext.pPlatformImpl->InputQueuePipe[ 0 ], []( int Read, int Events, void* pData ) -> int
		{
			AInputQueue* pQueue = static_cast< AInputQueue* >( pData );
			if( AInputQueue_hasEvents( pQueue ) )
			{
				AInputEvent* pEvent;
				while( AInputQueue_getEvent( pQueue, &pEvent ) >= 0 )
				{
					int32_t Type = AInputEvent_getType( pEvent );

					switch( Type )
					{
						case AINPUT_EVENT_TYPE_KEY:
						{
							const int32_t Action = AKeyEvent_getAction( pEvent );
							if( Action == AKEY_EVENT_ACTION_DOWN )
							{
								xyContext& rContext       = xyGetContext();
								JNIEnv*    pEnv           = rContext.pPlatformImpl->pNativeActivity->env;
								jclass     KeyEventClass  = pEnv->FindClass( "android/view/KeyEvent" );
								jmethodID  KeyEventCtor   = pEnv->GetMethodID( KeyEventClass, "<init>", "(II)V" );
								jobject    KeyEvent       = pEnv->NewObject( KeyEventClass, KeyEventCtor, Action, AKeyEvent_getKeyCode( pEvent ) );
								jmethodID  GetUnicodeChar = pEnv->GetMethodID( KeyEventClass, "getUnicodeChar", "()I" );
								char32_t   UnicodeChar    = pEnv->CallIntMethod( KeyEvent, GetUnicodeChar );
								printf( "" );

								if( UnicodeChar == '0' )
								{
								}
							}
						}
						break;

						default:
						break;
					}

					AInputQueue_finishEvent( pQueue, pEvent, 1 );
				}
			}

			// Keep listening
			return 1;
		}, pQueue );

		rContext.pPlatformImpl->pInputLooper = pLooper;
	};
	pActivity->callbacks->onInputQueueDestroyed = []( ANativeActivity* /*pActivity*/, AInputQueue* pQueue )
	{
		xyContext& rContext = xyGetContext();
		AInputQueue_detachLooper( pQueue );
		close( rContext.pPlatformImpl->InputQueuePipe[ 1 ] );
		close( rContext.pPlatformImpl->InputQueuePipe[ 0 ] );
	};

	// Obtain the configuration
	rContext.pPlatformImpl->pConfiguration = AConfiguration_new();
	AConfiguration_fromAssetManager( rContext.pPlatformImpl->pConfiguration, rContext.pPlatformImpl->pNativeActivity->assetManager );

	// Obtain the UI mode
	switch( AConfiguration_getUiModeType( rContext.pPlatformImpl->pConfiguration ) )
	{
		case ACONFIGURATION_UI_MODE_TYPE_CAR:        { rContext.UIMode = XY_UI_MODE_CAR;      } break;
		case ACONFIGURATION_UI_MODE_TYPE_TELEVISION: { rContext.UIMode = XY_UI_MODE_TV;       } break;
		case ACONFIGURATION_UI_MODE_TYPE_APPLIANCE:  { rContext.UIMode = XY_UI_MODE_HEADLESS; } break;
		case ACONFIGURATION_UI_MODE_TYPE_WATCH:      { rContext.UIMode = XY_UI_MODE_WATCH;    } break;
		case ACONFIGURATION_UI_MODE_TYPE_VR_HEADSET: { rContext.UIMode = XY_UI_MODE_VR;       } break;
		default:                                     { rContext.UIMode = XY_UI_MODE_PHONE;    } break; // Default to phone UI
	}

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

@interface xyViewController : UIViewController
@end // xyViewController

@interface xyAppDelegate : NSObject< UIApplicationDelegate >
@end // xyAppDelegate

@implementation xyViewController
@end // xyViewController

@implementation xyAppDelegate

-( BOOL )application:( UIApplication* )pApplication didFinishLaunchingWithOptions:( NSDictionary* )pLaunchOptions
{
	UIScene*          pScene          = [ [ [ pApplication connectedScenes ] allObjects ] firstObject ];
	UIWindow*         pWindow         = [ [ UIWindow alloc ] initWithWindowScene:( UIWindowScene* )pScene ];
	xyViewController* pViewController = [ [ xyViewController alloc ] init ];
	pWindow.rootViewController        = pViewController;
	[ pWindow makeKeyAndVisible ];

	std::thread Thread( &xyMain );
	Thread.detach();

} // didFinishLaunchingWithOptions

@end // xyAppDelegate

int main( int ArgC, char** ppArgV )
{
	xyContext& rContext = xyGetContext();
	rContext.CommandLineArgs = std::span< char* >( ppArgV, ArgC );
	rContext.UIMode          = XY_UI_MODE_PHONE;

	setlocale( LC_CTYPE, "UTF-8" );

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
	rContext.UIMode          = XY_UI_MODE_HEADLESS; // We don't know the UI mode. Might as well assume the worst.

	return xyMain();

} // main

#endif // !XY_OS_WINDOWS && !XY_OS_ANDROID && !XY_OS_IOS
