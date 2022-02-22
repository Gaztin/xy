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

#pragma once

//////////////////////////////////////////////////////////////////////////
/// Includes

#include "xy-renderers.h"


//////////////////////////////////////////////////////////////////////////
/// Classes

class xyWindow;

class xyRenderContext
{
public:

	/// Constructors/Destructors
	xyRenderContext( const xyWindow& rWindow );
	xyRenderContext( const xyRenderContext& ) = delete;
	xyRenderContext( xyRenderContext&& )      = delete;
	~xyRenderContext();

	/// Assignment operators
	xyRenderContext& operator=( xyRenderContext&& ) = delete;
	xyRenderContext& operator=( const xyRenderContext& ) = delete;

private:

	// Member variables
	void* m_pImpl = nullptr;

}; // xyRenderContext


//////////////////////////////////////////////////////////////////////////
/*

██╗███╗   ███╗██████╗ ██╗     ███████╗███╗   ███╗███████╗███╗   ██╗████████╗ █████╗ ████████╗██╗ ██████╗ ███╗   ██╗
██║████╗ ████║██╔══██╗██║     ██╔════╝████╗ ████║██╔════╝████╗  ██║╚══██╔══╝██╔══██╗╚══██╔══╝██║██╔═══██╗████╗  ██║
██║██╔████╔██║██████╔╝██║     █████╗  ██╔████╔██║█████╗  ██╔██╗ ██║   ██║   ███████║   ██║   ██║██║   ██║██╔██╗ ██║
██║██║╚██╔╝██║██╔═══╝ ██║     ██╔══╝  ██║╚██╔╝██║██╔══╝  ██║╚██╗██║   ██║   ██╔══██║   ██║   ██║██║   ██║██║╚██╗██║
██║██║ ╚═╝ ██║██║     ███████╗███████╗██║ ╚═╝ ██║███████╗██║ ╚████║   ██║   ██║  ██║   ██║   ██║╚██████╔╝██║ ╚████║
╚═╝╚═╝     ╚═╝╚═╝     ╚══════╝╚══════╝╚═╝     ╚═╝╚══════╝╚═╝  ╚═══╝   ╚═╝   ╚═╝  ╚═╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝
*/
#if defined( XY_IMPLEMENT )

//////////////////////////////////////////////////////////////////////////
/// Includes

#if defined( XY_HAS_VULKAN )
#include <vulkan/vulkan.h>
#endif // XY_HAS_VULKAN


//////////////////////////////////////////////////////////////////////////
/// Data structures

#if defined( XY_HAS_VULKAN )

struct ImplVulkan
{
	VkInstance Instance = VK_NULL_HANDLE;

}; // ImplVulkan

#endif // XY_HAS_VULKAN


//////////////////////////////////////////////////////////////////////////
/// Class methods

xyRenderContext::xyRenderContext( const xyWindow& /*rWindow*/ )
{

#ifdef XY_HAS_VULKAN

	VkApplicationInfo    ApplicationInfo{ .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .pNext = nullptr, .pApplicationName = "xy", .applicationVersion = 0, .pEngineName = "xy-renderer", .engineVersion = 0, .apiVersion = VK_API_VERSION_1_0 };
	VkInstanceCreateInfo InstanceCreateInfo{ .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, .pNext = nullptr, .flags = 0, .pApplicationInfo = &ApplicationInfo, .enabledLayerCount = 0, .ppEnabledLayerNames = nullptr, .enabledExtensionCount = 0, .ppEnabledExtensionNames = nullptr };
	ImplVulkan&          rImpl = *static_cast< ImplVulkan* >( m_pImpl = new ImplVulkan() );
	VkResult             Result;

	if( ( Result = vkCreateInstance( &InstanceCreateInfo, nullptr, &rImpl.Instance ) ) != VK_SUCCESS )
		return;

#endif // XY_HAS_VULKAN

} // xyRenderContext

//////////////////////////////////////////////////////////////////////////

xyRenderContext::~xyRenderContext()
{

#ifdef XY_HAS_VULKAN

	ImplVulkan& rImpl = *static_cast< ImplVulkan* >( m_pImpl );

	if( rImpl.Instance )
		vkDestroyInstance( rImpl.Instance, nullptr );

	delete &rImpl;

#endif // XY_HAS_VULKAN

} // ~xyRenderContext


#endif // XY_IMPLEMENT
