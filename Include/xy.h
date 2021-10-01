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

struct xyDisplay
{
	uint32_t Width  = 0;
	uint32_t Height = 0;

}; // xyDisplay

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
 * Obtains the display of this device.
 * In desktop environments, this is the virtual screen. The virtual screen is the collection of all monitors.
 * To obtain individual desktop monitors, refer to xyGetPrimaryDesktopMonitor or xyGetAllDesktopMonitors.
 *
 * @return The display data.
 */
extern xyDisplay xyGetDisplay( void );

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


#if defined( XY_ENV_DESKTOP )

//////////////////////////////////////////////////////////////////////////
/// Desktop-specific includes

#include <vector>


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

#if defined( XY_OS_WINDOWS )

//////////////////////////////////////////////////////////////////////////
/// Windows-specific includes

#include <windows.h>


//////////////////////////////////////////////////////////////////////////
/// Windows-specific data structures

struct xyPlatformImpl
{
	HINSTANCE ApplicationInstanceHandle = NULL;

}; // xyPlatformImpl


#elif defined( XY_OS_MACOS ) // XY_OS_WINDOWS

//////////////////////////////////////////////////////////////////////////
/// macOS-specific data structures

struct xyPlatformImpl
{
}; // xyPlatformImpl


#elif defined( XY_OS_ANDROID ) // XY_OS_MACOS

//////////////////////////////////////////////////////////////////////////
/// Android-specific includes

#include <thread>
#include <tuple>

#include <android/configuration.h>
#include <android/native_activity.h>


//////////////////////////////////////////////////////////////////////////
/// Android-specific data structures

struct xyPlatformImpl
{
	ANativeActivity* pNativeActivity     = nullptr;
	AConfiguration*  pConfiguration      = nullptr;
	int              JavaThreadPipe[ 2 ] = { };

}; // xyPlatformImpl

struct xyRunnable
{
	virtual ~xyRunnable( void ) = default;

	virtual void Execute( void ) = 0;

}; // xyRunnable


//////////////////////////////////////////////////////////////////////////
/// Android-specific template functions

/**
 * Invokes a callable object with arguments from a packed tuple.
 *
 * @param rrFunction The object that gets called.
 * @param rTuple The tuple that gets unpacked into arguments.
 */
template< typename Function, typename Tuple, size_t... Is >
auto xyInvokeWithTuple( Function&& rrFunction, const Tuple& rTuple, std::integer_sequence< size_t, Is... > )
{
	return std::invoke( std::forward< Function >( rrFunction ), ( std::get< Is >( rTuple ) )... );

} // xyInvokeWithTuple

/**
 * Runs a callable object on the java thread and waits for it to finish.
 *
 * @param rrFunction The object that gets called.
 * @param rrArgs Optional arguments that gets passed to the function.
 */
template< typename Function, typename... Args
        , typename = typename std::enable_if_t< std::is_void_v< std::invoke_result_t< Function, Args... > > > >
void xyRunOnJavaThread( Function&& rrFunction, Args&&... rrArgs )
{
	struct JavaRunnable : xyRunnable
	{
		virtual void Execute( void ) override
		{
			xyInvokeWithTuple( Callback, Arguments, std::index_sequence_for< Args... >{ } );
			Finished = true;

		} // Execute

		std::decay_t< Function > Callback;
		std::tuple< Args... >    Arguments;
		bool                     Finished = false;

	}; // JavaRunnable

	xyContext& rContext  = xyGetContext();
	auto*      pRunnable = new JavaRunnable();
	pRunnable->Callback  = std::forward< Function >( rrFunction );
	pRunnable->Arguments = std::forward_as_tuple( std::forward< Args >( rrArgs )... );

	if( write( rContext.pPlatformImpl->JavaThreadPipe[ 1 ], &pRunnable, sizeof( pRunnable ) ) == sizeof( pRunnable ) )
	{
		while( !pRunnable->Finished )
			std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
	}

	delete pRunnable;

} // xyRunOnJavaThread

/**
 * Runs a callable object on the java thread and waits for it to finish.
 *
 * @param rrFunction The object that gets called.
 * @param rrArgs Optional arguments that gets passed to the function.
 * @return The return value of the call.
 */
template< typename Function, typename... Args
        , typename = typename std::enable_if_t< !std::is_void_v< std::invoke_result_t< Function, Args... > > > >
auto xyRunOnJavaThread( Function&& rrFunction, Args&&... rrArgs )
{
	using ReturnType = std::invoke_result_t< Function, Args... >;

	struct JavaRunnable : xyRunnable
	{
		virtual void Execute( void ) override
		{
			ReturnValue = xyInvokeWithTuple( Callback, Arguments, std::index_sequence_for< Args... >{ } );
			Finished    = true;

		} // Execute

		std::decay_t< Function > Callback;
		std::tuple< Args... >    Arguments;
		ReturnType               ReturnValue;
		bool                     Finished = false;

	}; // JavaRunnable

	xyContext& rContext  = xyGetContext();
	auto* pRunnable      = new JavaRunnable();
	pRunnable->Callback  = std::forward< Function >( rrFunction );
	pRunnable->Arguments = std::forward_as_tuple( std::forward< Args >( rrArgs )... );

	if( write( rContext.pPlatformImpl->JavaThreadPipe[ 1 ], &pRunnable, sizeof( pRunnable ) ) == sizeof( pRunnable ) )
	{
		while( !pRunnable->Finished )
			std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );

		ReturnType ReturnValue = std::move( pRunnable->ReturnValue );
		delete pRunnable;
		return ReturnValue;
	}

	delete pRunnable;
	return ReturnType{};

} // xyRunOnJavaThread


#endif // XY_OS_ANDROID


#if defined( XY_IMPLEMENT )

//////////////////////////////////////////////////////////////////////////
/// Includes

#if defined( XY_OS_WINDOWS )
#include <lmcons.h>
#elif defined( XY_OS_MACOS ) // XY_OS_WINDOWS
#include <Cocoa/Cocoa.h>
#elif defined( XY_OS_ANDROID ) // XY_OS_MACOS
#include <android/configuration.h>
#include <android/native_activity.h>
#include <unistd.h>
#endif // XY_OS_ANDROID


//////////////////////////////////////////////////////////////////////////
/// Functions

xyContext& xyGetContext( void )
{
	static xyContext Context;

	return Context;

} // xyGetContext

//////////////////////////////////////////////////////////////////////////

void xyMessageBox( std::string_view Title, std::string_view Message )
{

#if defined( XY_OS_WINDOWS )

	MessageBoxA( NULL, Message.data(), Title.data(), MB_OK | MB_ICONINFORMATION | MB_TASKMODAL );

#elif defined( XY_OS_MACOS ) // XY_OS_WINDOWS

	NSAlert*  pAlert           = [ [ NSAlert alloc ] init ];
	NSString* pMessageText     = [ NSString stringWithUTF8String:Title.data() ];
	NSString* pInformativeText = [ NSString stringWithUTF8String:Message.data() ];

	[ pAlert addButtonWithTitle:@"OK" ];
	[ pAlert setMessageText:pMessageText ];
	[ pAlert setInformativeText:pInformativeText ];
	[ pAlert setAlertStyle:NSAlertStyleInformational ];
	[ pAlert runModal ];

	[ pInformativeText release ];
	[ pMessageText release ];
	[ pAlert release ];

#elif defined( XY_OS_ANDROID ) // XY_OS_MACOS

	jobject Alert = xyRunOnJavaThread( []( std::string Title, std::string Message )
	{
		xyContext& rContext = xyGetContext();
		JNIEnv*    pJNI     = rContext.pPlatformImpl->pNativeActivity->env;

		// Easy way to tidy up all our local references when we're done
		pJNI->PushLocalFrame( 16 );

		// Obtain all necessary classes and method IDs
		jclass    ClassBuilder           = pJNI->FindClass( "android/app/AlertDialog$Builder" );
		jmethodID CtorBuilder            = pJNI->GetMethodID( ClassBuilder, "<init>", "(Landroid/content/Context;)V" );
		jmethodID MethodSetTitle         = pJNI->GetMethodID( ClassBuilder, "setTitle", "(Ljava/lang/CharSequence;)Landroid/app/AlertDialog$Builder;" );
		jmethodID MethodSetMessage       = pJNI->GetMethodID( ClassBuilder, "setMessage", "(Ljava/lang/CharSequence;)Landroid/app/AlertDialog$Builder;" );
		jmethodID MethodSetNeutralButton = pJNI->GetMethodID( ClassBuilder, "setNeutralButton", "(Ljava/lang/CharSequence;Landroid/content/DialogInterface$OnClickListener;)Landroid/app/AlertDialog$Builder;" );
		jmethodID MethodSetCancelable    = pJNI->GetMethodID( ClassBuilder, "setCancelable", "(Z)Landroid/app/AlertDialog$Builder;" );
		jmethodID MethodShow             = pJNI->GetMethodID( ClassBuilder, "show","()Landroid/app/AlertDialog;" );

		// Create the alert dialog
		jobject Builder = pJNI->NewObject( ClassBuilder, CtorBuilder, rContext.pPlatformImpl->pNativeActivity->clazz );
		pJNI->CallObjectMethod( Builder, MethodSetTitle, pJNI->NewStringUTF( Title.data() ) );
		pJNI->CallObjectMethod( Builder, MethodSetMessage, pJNI->NewStringUTF( Message.data() ) );
		pJNI->CallObjectMethod( Builder, MethodSetNeutralButton, pJNI->NewStringUTF( "OK" ), nullptr );
		pJNI->CallObjectMethod( Builder, MethodSetCancelable, false );
		jobject Alert = pJNI->CallObjectMethod( Builder, MethodShow );
		Alert         = pJNI->NewGlobalRef( Alert );

		// Clean up local references
		pJNI->PopLocalFrame( nullptr );

		return Alert;

	}, std::string( Title ), std::string( Message ) );

	xyContext& rContext = xyGetContext();
	JNIEnv*    pJNI;
	rContext.pPlatformImpl->pNativeActivity->vm->AttachCurrentThread( &pJNI, nullptr );

	jclass    ClassAlertDialog = pJNI->GetObjectClass( Alert );
	jmethodID MethodIsShowing  = pJNI->GetMethodID( ClassAlertDialog, "isShowing", "()Z" );

	// Sleep until dialog is closed
	while( pJNI->CallBooleanMethod( Alert, MethodIsShowing ) )
		std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );

	pJNI->DeleteGlobalRef( Alert );

	rContext.pPlatformImpl->pNativeActivity->vm->DetachCurrentThread();

#endif // XY_OS_ANDROID

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

#elif defined( XY_OS_ANDROID ) // XY_OS_WINDOWS

	xyContext& rContext = xyGetContext();
	JNIEnv*    pJNI;
	rContext.pPlatformImpl->pNativeActivity->vm->AttachCurrentThread( &pJNI, nullptr );

	jclass      BuildClass           = pJNI->FindClass( "android/os/Build" );
	jfieldID    ManufacturerField    = pJNI->GetStaticFieldID( BuildClass, "MANUFACTURER", "Ljava/lang/String;" );
	jfieldID    ModelField           = pJNI->GetStaticFieldID( BuildClass, "MODEL", "Ljava/lang/String;" );
	jstring     ManufacturerName     = static_cast< jstring >( pJNI->GetStaticObjectField( BuildClass, ManufacturerField ) );
	jstring     ModelName            = static_cast< jstring >( pJNI->GetStaticObjectField( BuildClass, ModelField ) );
	const char* pManufacturerNameUTF = pJNI->GetStringUTFChars( ManufacturerName, nullptr );
	const char* pModelNameUTF        = pJNI->GetStringUTFChars( ModelName, nullptr );
	std::string DeviceName           = std::string( pManufacturerNameUTF ) + ' ' + pModelNameUTF;

	pJNI->ReleaseStringUTFChars( ModelName, pModelNameUTF );
	pJNI->ReleaseStringUTFChars( ManufacturerName, pManufacturerNameUTF );
	rContext.pPlatformImpl->pNativeActivity->vm->DetachCurrentThread();

	return { .Name=std::move( DeviceName ) };

#endif // XY_OS_ANDROID

} // xyGetDevice

//////////////////////////////////////////////////////////////////////////

xyDisplay xyGetDisplay( void )
{

#if defined( XY_OS_WINDOWS )

	return { .Width  = static_cast< uint32_t >( GetSystemMetrics( SM_CXVIRTUALSCREEN ) ),
	         .Height = static_cast< uint32_t >( GetSystemMetrics( SM_CYVIRTUALSCREEN ) ) };

#elif defined( XY_OS_ANDROID ) // XY_OS_WINDOWS

	xyContext& rContext = xyGetContext();
	JNIEnv*    pJNI;
	rContext.pPlatformImpl->pNativeActivity->vm->AttachCurrentThread( &pJNI, nullptr );

	jobject Activity      = rContext.pPlatformImpl->pNativeActivity->clazz;
	jobject WindowManager = pJNI->CallObjectMethod( Activity, pJNI->GetMethodID( pJNI->GetObjectClass( Activity ), "getWindowManager", "()Landroid/view/WindowManager;" ) );
	jobject WindowMetrics = pJNI->CallObjectMethod( WindowManager, pJNI->GetMethodID( pJNI->GetObjectClass( WindowManager ), "getMaximumWindowMetrics", "()Landroid/view/WindowMetrics;" ) );
	jobject BoundsRect    = pJNI->CallObjectMethod( WindowMetrics, pJNI->GetMethodID( pJNI->GetObjectClass( WindowMetrics ), "getBounds", "()Landroid/graphics/Rect;" ) );
	jclass  RectClass     = pJNI->GetObjectClass( BoundsRect );
	jint    Left          = pJNI->GetIntField( BoundsRect, pJNI->GetFieldID( RectClass, "left", "I" ) );
	jint    Top           = pJNI->GetIntField( BoundsRect, pJNI->GetFieldID( RectClass, "top", "I" ) );
	jint    Right         = pJNI->GetIntField( BoundsRect, pJNI->GetFieldID( RectClass, "right", "I" ) );
	jint    Bottom        = pJNI->GetIntField( BoundsRect, pJNI->GetFieldID( RectClass, "bottom", "I" ) );

	rContext.pPlatformImpl->pNativeActivity->vm->DetachCurrentThread();

	return { .Width=static_cast< uint32_t >( Right - Left ), .Height=static_cast< uint32_t >( Bottom - Top ) };

#endif // XY_OS_ANDROID

} // xyGetDisplay

//////////////////////////////////////////////////////////////////////////

xyTheme xyGetPreferredTheme( void )
{
	// Default to light theme
	xyTheme Theme = xyTheme::Light;

#if defined( XY_OS_WINDOWS )

	// Courtesy of https://stackoverflow.com/a/51336913
	DWORD AppsUseLightTheme;
	DWORD DataSize = sizeof( AppsUseLightTheme );
	if( RegGetValueA( HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", "AppsUseLightTheme", RRF_RT_REG_DWORD, NULL, &AppsUseLightTheme, &DataSize ) == ERROR_SUCCESS )
		Theme = AppsUseLightTheme ? xyTheme::Light : xyTheme::Dark;

#elif defined( XY_OS_ANDROID ) // XY_OS_WINDOWS

	xyContext& rContext = xyGetContext();

	switch( AConfiguration_getUiModeNight( rContext.pPlatformImpl->pConfiguration ) )
	{
		case ACONFIGURATION_UI_MODE_NIGHT_NO:  { Theme = xyTheme::Light; } break;
		case ACONFIGURATION_UI_MODE_NIGHT_YES: { Theme = xyTheme::Dark;  } break;
		default: break;
	}

#endif // XY_OS_ANDROID

	return Theme;

} // xyGetPreferredTheme

//////////////////////////////////////////////////////////////////////////

xyLanguage xyGetLanguage( void )
{

#if defined( XY_OS_WINDOWS )

	xyLanguage Language;
	WCHAR      Buffer[ LOCALE_NAME_MAX_LENGTH ];
	GetUserDefaultLocaleName( Buffer, static_cast< int >( std::size( Buffer ) ) );

	const size_t LocaleNameLength = wcslen( Buffer );
	Language.LocaleName.resize( LocaleNameLength );
	WideCharToMultiByte( CP_UTF8, 0, Buffer, static_cast< int >( LocaleNameLength ), Language.LocaleName.data(), static_cast< int >( Language.LocaleName.size() ), NULL, NULL );

	return Language;

#elif defined( XY_OS_ANDROID ) // XY_OS_WINDOWS

	xyContext& rContext = xyGetContext();
	char       LanguageCode[ 2 ];
	AConfiguration_getLanguage( rContext.pPlatformImpl->pConfiguration, LanguageCode );

	return { .LocaleName=std::string( LanguageCode, 2 ) };

#endif // XY_OS_ANDROID

} // xyGetLanguage


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


#endif // XY_IMPLEMENT
