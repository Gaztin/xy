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

#include <memory>


//////////////////////////////////////////////////////////////////////////
/// Classes

class xyWindow;

class xyRenderContext
{
public:

	/// Constructors/Destructors
	xyRenderContext( const xyWindow& rWindow );
	xyRenderContext( const xyRenderContext& ) = delete;
	xyRenderContext( xyRenderContext&& rrOther );

	/// Assignment operators
	xyRenderContext& operator=( xyRenderContext&& ) = delete;
	xyRenderContext& operator=( const xyRenderContext& ) = delete;

private:

	// Member variables
	std::shared_ptr< void > m_pImpl;

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
#if defined( XY_HAS_D3D11 )
#include <d3d11.h>
#endif // XY_HAS_D3D11


//////////////////////////////////////////////////////////////////////////
/// Data structures

#if defined( XY_HAS_VULKAN )

struct ImplVulkan
{
	~ImplVulkan()
	{
		if( Device ) vkDestroyDevice( Device, nullptr );
		if( Instance ) vkDestroyInstance( Instance, nullptr );

	} // ~ImplVulkan

	VkInstance Instance = VK_NULL_HANDLE;
	VkDevice   Device   = VK_NULL_HANDLE;

}; // ImplVulkan

#endif // XY_HAS_VULKAN
#if defined( XY_HAS_D3D11 )

struct ImplD3D11
{
	~ImplD3D11()
	{
		if( pSwapChain ) pSwapChain->Release();
		if( pDeviceContext ) pDeviceContext->Release();
		if( pDevice ) pDevice->Release();

	} // ~ImplD3D11

	ID3D11Device*        pDevice        = nullptr;
	ID3D11DeviceContext* pDeviceContext = nullptr;
	IDXGISwapChain*      pSwapChain     = nullptr;

}; // ImplD3D11

#endif // XY_HAS_D3D11


//////////////////////////////////////////////////////////////////////////
/// Functions

#if defined( XY_HAS_VULKAN )

static std::shared_ptr< ImplVulkan > VulkanInit()
{
	auto                 pImpl = std::make_shared< ImplVulkan >();
	VkApplicationInfo    ApplicationInfo{ .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .pNext = nullptr, .pApplicationName = "xy", .applicationVersion = 0, .pEngineName = "xy-renderer", .engineVersion = 0, .apiVersion = VK_API_VERSION_1_0 };
	VkInstanceCreateInfo InstanceCreateInfo{ .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, .pNext = nullptr, .flags = 0, .pApplicationInfo = &ApplicationInfo, .enabledLayerCount = 0, .ppEnabledLayerNames = nullptr, .enabledExtensionCount = 0, .ppEnabledExtensionNames = nullptr };
	VkResult             Result;
	if( ( Result = vkCreateInstance( &InstanceCreateInfo, nullptr, &pImpl->Instance ) ) != VK_SUCCESS ) return {};

	uint32_t GPUCount;
	if( ( Result = vkEnumeratePhysicalDevices( pImpl->Instance, &GPUCount, nullptr ) ) != VK_SUCCESS ) return {};

	std::vector< VkPhysicalDevice > GPUs( GPUCount );
	if( ( Result = vkEnumeratePhysicalDevices( pImpl->Instance, &GPUCount, &GPUs[ 0 ] ) ) != VK_SUCCESS ) return {};

	VkPhysicalDevice GPU = GPUs.front();
	uint32_t         QueueFamilyPropertyCount;
	vkGetPhysicalDeviceQueueFamilyProperties( GPU, &QueueFamilyPropertyCount, nullptr );

	std::vector< VkQueueFamilyProperties > QueueFamilyProperties( QueueFamilyPropertyCount );
	vkGetPhysicalDeviceQueueFamilyProperties( GPU, &QueueFamilyPropertyCount, &QueueFamilyProperties[ 0 ] );

	auto GraphicsQueueFamily = std::find_if( QueueFamilyProperties.begin(), QueueFamilyProperties.end(), []( auto& rProperty ) { return ( rProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT ) != 0; } );
	if( GraphicsQueueFamily == QueueFamilyProperties.end() ) return {};

	const uint32_t          GraphicsQueueFamilyIndex = static_cast< uint32_t >( std::distance( QueueFamilyProperties.begin(), GraphicsQueueFamily ) );
	VkDeviceQueueCreateInfo DeviceQueueCreateInfo{ .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, .pNext = nullptr, .queueFamilyIndex = GraphicsQueueFamilyIndex, .queueCount = 1, .pQueuePriorities = std::begin( { 0.0f } ) };
	VkDeviceCreateInfo      DeviceCreateInfo{ .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, .pNext = nullptr, .queueCreateInfoCount = 1, .pQueueCreateInfos = &DeviceQueueCreateInfo, .enabledLayerCount = 0, .ppEnabledLayerNames = nullptr, .enabledExtensionCount = 0, .ppEnabledExtensionNames = nullptr, .pEnabledFeatures = nullptr };
	if( ( Result = vkCreateDevice( GPU, &DeviceCreateInfo, nullptr, &pImpl->Device ) ) != VK_SUCCESS ) return {};

	return pImpl;

} // VulkanInit

#endif // XY_HAS_VULKAN
#if defined( XY_HAS_D3D11 )

static std::shared_ptr< ImplD3D11 > D3D11Init( const xyWindow& rWindow )
{
	const xySize             WindowSize   = rWindow.GetSize();
	auto                     pImpl        = std::make_shared< ImplD3D11 >();
	D3D_FEATURE_LEVEL        FeatureLevel = D3D_FEATURE_LEVEL_11_0;
	D3D11_CREATE_DEVICE_FLAG DeviceFlags  = XY_IF_DEBUG( D3D11_CREATE_DEVICE_DEBUG, 0 );
	DXGI_SWAP_CHAIN_DESC     SwapChainDesc{ .BufferDesc = { .Width = WindowSize.Width, .Height = WindowSize.Height, .RefreshRate = 60, .Format = DXGI_FORMAT_R8G8B8A8_UNORM, .ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE, .Scaling = DXGI_MODE_SCALING_STRETCHED }, .SampleDesc = { .Count = 1, .Quality = 0 }, .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, .BufferCount = 2, .OutputWindow = ( HWND )rWindow.GetNativeHandle(), .Windowed = TRUE, .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD };
	HRESULT                  Result;

	if( ( Result = D3D11CreateDeviceAndSwapChain( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, DeviceFlags, NULL, 0, D3D11_SDK_VERSION, &SwapChainDesc, &pImpl->pSwapChain, &pImpl->pDevice, &FeatureLevel, &pImpl->pDeviceContext ) ) != S_OK )
		return nullptr;

	return pImpl;

} // D3D11Init

#endif // XY_HAS_D3D11


//////////////////////////////////////////////////////////////////////////
/// Class methods

xyRenderContext::xyRenderContext( const xyWindow& rWindow )
{
	if( false );
#ifdef XY_HAS_VULKAN
	else if( auto Vulkan = VulkanInit() ) m_pImpl = std::move( Vulkan );
#endif // XY_HAS_VULKAN
#ifdef XY_HAS_D3D11
	else if( auto D3D11 = D3D11Init( rWindow ) ) m_pImpl = std::move( D3D11 );
#endif // XY_HAS_D3D11

} // xyRenderContext

//////////////////////////////////////////////////////////////////////////

xyRenderContext::xyRenderContext( xyRenderContext&& rrOther )
	: m_pImpl( std::move( rrOther.m_pImpl ) )
{

} // xyRenderContext


#endif // XY_IMPLEMENT
