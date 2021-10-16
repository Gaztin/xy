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

#elif defined( XY_OS_IOS ) // XY_OS_ANDROID

	NSString* pDeviceName = [ [ UIDevice currentDevice ] name ];

	return { .Name=[ pDeviceName UTF8String ] };

#endif // XY_OS_IOS

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

	return { .Width  = static_cast< uint32_t >( Right - Left )
	       , .Height = static_cast< uint32_t >( Bottom - Top ) };

#elif defined( XY_OS_IOS ) // XY_OS_ANDROID

	CGRect ScreenRect = [ [ UIScreen mainScreen ] bounds ];

	return { .Width  = ScreenRect.size.width
	       , .Height = ScreenRect.size.height };

#endif // XY_OS_IOS

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

	const size_t LocaleNameLength = wcslen( Buffer );
	Language.LocaleName.resize( LocaleNameLength );
	WideCharToMultiByte( CP_UTF8, 0, Buffer, static_cast< int >( LocaleNameLength ), Language.LocaleName.data(), static_cast< int >( Language.LocaleName.size() ), NULL, NULL );

	return Language;

#elif defined( XY_OS_ANDROID ) // XY_OS_WINDOWS

	xyContext& rContext = xyGetContext();
	char       LanguageCode[ 2 ];
	AConfiguration_getLanguage( rContext.pPlatformImpl->pConfiguration, LanguageCode );

	return { .LocaleName=std::string( LanguageCode, 2 ) };

#elif defined( XY_OS_IOS ) // XY_OS_ANDROID

	NSString* pLanguage = [ [ NSLocale preferredLanguages ] firstObject ];

	return { .LocaleName=[ pLanguage UTF8String ] };

#endif // XY_OS_IOS

} // xyGetLanguage
