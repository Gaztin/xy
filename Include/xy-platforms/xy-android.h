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

#if defined( XY_OS_ANDROID )

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
