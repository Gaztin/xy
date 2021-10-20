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

//////////////////////////////////////////////////////////////////////////
/// Includes

#include "xy.h"

#if defined( XY_OS_WINDOWS )
#include <windows.h>
#include <lmcons.h>
#elif defined( XY_OS_MACOS ) // XY_OS_WINDOWS
#include <Cocoa/Cocoa.h>
#elif defined( XY_OS_ANDROID ) // XY_OS_MACOS
#include <android/configuration.h>
#include <android/native_activity.h>
#include <unistd.h>
#elif defined( XY_OS_IOS ) // XY_OS_ANDROID
#include <UIKit/UIKit.h>
#endif // XY_OS_IOS


//////////////////////////////////////////////////////////////////////////
/// Functions

xyContext& xyGetContext( void )
{
	static xyContext Context;

	return Context;

} // xyGetContext

//////////////////////////////////////////////////////////////////////////

extern std::string xyUTFString( std::wstring_view String )
{
	// TODO: xyUTFString on all platforms

#if defined( XY_OS_WINDOWS )

	const int   RequiredSize = WideCharToMultiByte( CP_UTF8, 0, String.data(), static_cast< int >( String.size() ), NULL, 0, NULL, NULL );
	std::string UTFString( RequiredSize, '\0' );

	if( WideCharToMultiByte( CP_UTF8, 0, String.data(), static_cast< int >( String.size() ), UTFString.data(), static_cast< int >( UTFString.size() ), NULL, NULL ) > 0 )
		return UTFString;

	return { };

#endif // XY_OS_WINDOWS

} // xyUTFString

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

#elif defined( XY_OS_IOS ) // XY_OS_ANDROID

	__block bool Presented = true;

	dispatch_async_and_wait( dispatch_get_main_queue(), ^
	{
		UIApplication*     pApplication     = [ UIApplication sharedApplication ];
		UIScene*           pScene           = [ [ [ pApplication connectedScenes ] allObjects ] firstObject ];
		UIViewController*  pViewController  = [ [ [ ( UIWindowScene* )pScene windows ] firstObject ] rootViewController ];
		NSString*          pTitle           = [ NSString stringWithUTF8String:Title.data() ];
		NSString*          pMessage         = [ NSString stringWithUTF8String:Message.data() ];
		UIAlertController* pAlertController = [ UIAlertController alertControllerWithTitle:pTitle message:pMessage preferredStyle:UIAlertControllerStyleAlert ];
		UIAlertAction*     pActionOK        = [ UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:^( UIAlertAction* pAction ) { Presented = false; } ];

		[ pAlertController addAction:pActionOK ];
		[ pViewController presentViewController:pAlertController animated:NO completion:^{ } ];
	} );

	// Sleep until dialog is closed
	while( Presented )
		std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );

#endif // XY_OS_IOS

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

#elif defined( XY_OS_MACOS ) // XY_OS_WINDOWS

	NSString* pName = [ [ NSHost currentHost ] name ];

	return { .Name=[ pName UTF8String ] };

#elif defined( XY_OS_ANDROID ) // XY_OS_MACOS

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

#elif defined( XY_OS_IOS ) // XY_OS_ANDROID

	NSString* pDeviceName = [ [ UIDevice currentDevice ] name ];

	return { .Name=[ pDeviceName UTF8String ] };

#endif // XY_OS_IOS

} // xyGetDevice

//////////////////////////////////////////////////////////////////////////

xyTheme xyGetPreferredTheme( void )
{
	// Default to light theme
	xyTheme Theme = xyTheme::Light;

#if defined( XY_OS_WINDOWS )

	DWORD AppsUseLightTheme;
	DWORD DataSize = sizeof( AppsUseLightTheme );
	if( RegGetValueA( HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", "AppsUseLightTheme", RRF_RT_REG_DWORD, NULL, &AppsUseLightTheme, &DataSize ) == ERROR_SUCCESS )
		Theme = AppsUseLightTheme ? xyTheme::Light : xyTheme::Dark;

#elif defined( XY_OS_MACOS ) // XY_OS_WINDOWS

	NSString* pStyle = [ [ NSUserDefaults standardUserDefaults ] stringForKey:@"AppleInterfaceStyle" ];
	if( [ pStyle isEqualToString:@"Dark" ] )
		Theme = xyTheme::Dark;

#elif defined( XY_OS_ANDROID ) // XY_OS_MACOS

	xyContext& rContext = xyGetContext();

	switch( AConfiguration_getUiModeNight( rContext.pPlatformImpl->pConfiguration ) )
	{
		case ACONFIGURATION_UI_MODE_NIGHT_NO:  { Theme = xyTheme::Light; } break;
		case ACONFIGURATION_UI_MODE_NIGHT_YES: { Theme = xyTheme::Dark;  } break;

		default: break;
	}

#elif defined( XY_OS_IOS ) // XY_OS_ANDROID

	UITraitCollection* pTraitCollection = [ UITraitCollection currentTraitCollection ];

	switch( [ pTraitCollection userInterfaceStyle ] )
	{
		case UIUserInterfaceStyleLight: { Theme = xyTheme::Light; } break;
		case UIUserInterfaceStyleDark:  { Theme = xyTheme::Dark;  } break;

		default: break;
	}

#endif // XY_OS_IOS

	return Theme;

} // xyGetPreferredTheme

//////////////////////////////////////////////////////////////////////////

xyLanguage xyGetLanguage( void )
{

#if defined( XY_OS_WINDOWS )

	xyLanguage Language;
	WCHAR      Buffer[ LOCALE_NAME_MAX_LENGTH ];
	GetUserDefaultLocaleName( Buffer, static_cast< int >( std::size( Buffer ) ) );

	return { .LocaleName=xyUTFString( Buffer ) };

#elif defined( XY_OS_MACOS ) // XY_OS_WINDOWS

	NSString* pLanguageCode = [ [ NSLocale currentLocale ] languageCode ];

	return { .LocaleName=[ pLanguageCode UTF8String ] };

#elif defined( XY_OS_ANDROID ) // XY_OS_MACOS

	xyContext& rContext = xyGetContext();
	char       LanguageCode[ 2 ];
	AConfiguration_getLanguage( rContext.pPlatformImpl->pConfiguration, LanguageCode );

	return { .LocaleName=std::string( LanguageCode, 2 ) };

#elif defined( XY_OS_IOS ) // XY_OS_ANDROID

	NSString* pLanguage = [ [ NSLocale preferredLanguages ] firstObject ];

	return { .LocaleName=[ pLanguage UTF8String ] };

#endif // XY_OS_IOS

} // xyGetLanguage

//////////////////////////////////////////////////////////////////////////

std::vector< xyDisplayAdapter > xyGetDisplayAdapters( void )
{
	std::vector< xyDisplayAdapter > DisplayAdapters;

#if defined( XY_OS_WINDOWS )

	auto EnumProc = []( HMONITOR MonitorHandle, HDC /*DeviceContextHandle*/, LPRECT /*pRect*/, LPARAM UserData ) -> BOOL
	{
		auto& rMonitors = *reinterpret_cast< std::vector< xyDisplayAdapter >* >( UserData );

		MONITORINFOEXA Info = { sizeof( MONITORINFOEXA ) };
		if( GetMonitorInfoA( MonitorHandle, &Info ) )
		{
			xyDisplayAdapter Adapter = { .Name     = Info.szDevice,
			                             .FullRect = { .Left=Info.rcMonitor.left, .Top=Info.rcMonitor.top, .Right=Info.rcMonitor.right, .Bottom=Info.rcMonitor.bottom },
			                             .WorkRect = { .Left=Info.rcWork   .left, .Top=Info.rcWork   .top, .Right=Info.rcWork   .right, .Bottom=Info.rcWork   .bottom } };

			DISPLAY_DEVICEA DisplayDevice = { .cb=sizeof( DISPLAY_DEVICEA ) };
			if( EnumDisplayDevicesA( Adapter.Name.c_str(), 0, &DisplayDevice, 0 ) )
				Adapter.Name = DisplayDevice.DeviceString;

			rMonitors.emplace_back( std::move( Adapter ) );
		}

		// Always continue
		return TRUE;
	};

	EnumDisplayMonitors( NULL, NULL, EnumProc, reinterpret_cast< LPARAM >( &DisplayAdapters ) );

#elif defined( XY_OS_MACOS ) // XY_OS_WINDOWS

	for( NSScreen* pScreen in [ NSScreen screens ] )
	{
		xyDisplayAdapter Adapter = { .Name     = [ [ pScreen localizedName ] UTF8String ],
		                             .FullRect = { .Left=NSMinX( pScreen.frame ),        .Top=NSMinY( pScreen.frame ),        .Right=NSMaxX( pScreen.frame ),        .Bottom=NSMaxY( pScreen.frame ) },
		                             .WorkRect = { .Left=NSMinX( pScreen.visibleFrame ), .Top=NSMinY( pScreen.visibleFrame ), .Right=NSMaxX( pScreen.visibleFrame ), .Bottom=NSMaxY( pScreen.visibleFrame ) } };

		DisplayAdapters.emplace_back( std::move( Adapter ) );
	}

#elif defined( XY_OS_ANDROID ) // XY_OS_MACOS

	xyContext& rContext = xyGetContext();
	JNIEnv*    pJNI;
	rContext.pPlatformImpl->pNativeActivity->vm->AttachCurrentThread( &pJNI, nullptr );

	jobject      Activity       = rContext.pPlatformImpl->pNativeActivity->clazz;
	jclass       ActivityClass  = pJNI->GetObjectClass( Activity );
	jstring      DisplayService = ( jstring )pJNI->GetStaticObjectField( ActivityClass, pJNI->GetStaticFieldID( ActivityClass, "DISPLAY_SERVICE", "Ljava/lang/String;" ) );
	jobject      DisplayManager = pJNI->CallObjectMethod( Activity, pJNI->GetMethodID( ActivityClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;" ), DisplayService );
	jobjectArray Displays       = ( jobjectArray )pJNI->CallObjectMethod( DisplayManager, pJNI->GetMethodID( pJNI->GetObjectClass( DisplayManager ), "getDisplays", "()[Landroid/view/Display;" ) );
	jsize        DisplayCount   = pJNI->GetArrayLength( Displays );

	for( jsize i = 0; i < DisplayCount; ++i )
	{
		jobject     Display      = pJNI->GetObjectArrayElement( Displays, i );
		jclass      DisplayClass = pJNI->GetObjectClass( Display );
		jstring     Name         = ( jstring )pJNI->CallObjectMethod( Display, pJNI->GetMethodID( DisplayClass, "getName", "()Ljava/lang/String;" ) );
		const char* pNameUTF     = pJNI->GetStringUTFChars( Name, nullptr );
		jclass      RectClass    = pJNI->FindClass( "android/graphics/Rect" );
		jobject     Bounds       = pJNI->AllocObject( RectClass );

		xyDisplayAdapter Adapter = { .Name=pNameUTF };

		// NOTE: "getRectSize" is deprecated as of SDK v30.
		// The documentation suggests using WindowMetric#getBounds(), but there seems to be no way of obtaining the bounds of a specific Display object.
		pJNI->CallVoidMethod( Display, pJNI->GetMethodID( DisplayClass, "getRectSize", "(Landroid/graphics/Rect;)V" ), Bounds );
		Adapter.FullRect.Left   = pJNI->GetIntField( Bounds, pJNI->GetFieldID( RectClass, "left",   "I" ) );
		Adapter.FullRect.Top    = pJNI->GetIntField( Bounds, pJNI->GetFieldID( RectClass, "top",    "I" ) );
		Adapter.FullRect.Right  = pJNI->GetIntField( Bounds, pJNI->GetFieldID( RectClass, "right",  "I" ) );
		Adapter.FullRect.Bottom = pJNI->GetIntField( Bounds, pJNI->GetFieldID( RectClass, "bottom", "I" ) );

		if( jobject DisplayCutout = pJNI->CallObjectMethod( Display, pJNI->GetMethodID( DisplayClass, "getCutout", "()Landroid/view/DisplayCutout;" ) ) )
		{
			jclass DisplayCutoutClass = pJNI->GetObjectClass( DisplayCutout );

			Adapter.WorkRect.Left   = pJNI->CallIntMethod( DisplayCutout, pJNI->GetMethodID( DisplayCutoutClass, "getSafeInsetLeft",   "()I" ) );
			Adapter.WorkRect.Top    = pJNI->CallIntMethod( DisplayCutout, pJNI->GetMethodID( DisplayCutoutClass, "getSafeInsetTop",    "()I" ) );
			Adapter.WorkRect.Right  = pJNI->CallIntMethod( DisplayCutout, pJNI->GetMethodID( DisplayCutoutClass, "getSafeInsetRight",  "()I" ) );
			Adapter.WorkRect.Bottom = pJNI->CallIntMethod( DisplayCutout, pJNI->GetMethodID( DisplayCutoutClass, "getSafeInsetBottom", "()I" ) );
		}
		else
		{
			// TODO: There are other ways to obtain the safe area
			Adapter.WorkRect = Adapter.FullRect;
		}

		DisplayAdapters.emplace_back( std::move( Adapter ) );

		pJNI->ReleaseStringUTFChars( Name, pNameUTF );
	}

	rContext.pPlatformImpl->pNativeActivity->vm->DetachCurrentThread();

#elif defined( XY_OS_IOS ) // XY_OS_ANDROID

	for( UIScreen* pScreen in [ UIScreen screens ] )
	{
		NSString*        pScreenName = [ pScreen debugDescription ];
		CGRect           Bounds      = [ pScreen bounds ];
		xyDisplayAdapter MainDisplay = { .Name     = [ pScreenName UTF8String ],
		                                 .FullRect = { .Left=CGRectGetMinX( Bounds ), .Top=CGRectGetMinY( Bounds ), .Right=CGRectGetMaxX( Bounds ), .Bottom=CGRectGetMaxY( Bounds ) } };

		// TODO: Obtain the safe area

		DisplayAdapters.emplace_back( std::move( MainDisplay ) );
	}

#endif // XY_OS_IOS

	return DisplayAdapters;

} // xyGetDisplayAdapters
