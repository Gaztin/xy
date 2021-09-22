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

#include <span>

//////////////////////////////////////////////////////////////////////////
// Data structures

struct xyContext
{
	std::span< char* > CommandLineArgs = { };
	void*              pPlatformHandle = nullptr;

}; // xyContext


//////////////////////////////////////////////////////////////////////////
// Global functions

/*
 * Main entry function
 * Define this in your application's main source file!
 */
extern int xyMain( const xyContext& rContext );


//////////////////////////////////////////////////////////////////////////
// Platform-specific implementations

#if defined( _WIN32 )

#include <windows.h>

INT WINAPI WinMain( _In_ HINSTANCE Instance, _In_opt_ HINSTANCE /*PrevInstance*/, _In_ LPSTR /*CmdLine*/, _In_ int /*ShowCmd*/ )
{
	xyContext Context;
	Context.CommandLineArgs = std::span< char* >( __argv, __argc );
	Context.pPlatformHandle = Instance;

	return xyMain( Context );

} // WinMain

#elif defined( __ANDROID__ ) // _WIN32

#include <android/native_activity.h>

[[maybe_unused]] JNIEXPORT void ANativeActivity_onCreate( ANativeActivity* pActivity, void* /*pSavedState*/, size_t /*SavedStateSize*/ )
{
	xyContext Context;
	Context.pPlatformHandle = pActivity;

	xyMain( Context );

} // ANativeActivity_onCreate

#else // __ANDROID__

int main( int ArgC, char** ppArgV )
{
	xyContext Context;
	Context.CommandLineArgs = std::span< char* >( ppArgV, ArgC );

	return xyMain( Context );

} // main

#endif // !_WIN32
