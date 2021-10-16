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

#include "xy.h"

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
