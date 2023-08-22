<h1 align="center">primwalk</h1>
<p align="center">
<img width=100% height=20% src="https://github.com/JackiBackiBoy/primwalk/assets/44316688/7c92edb4-616e-4d88-a1e3-be1595c8c7f4">
</p>

<p align="center">
Cross platform graphics engine powered by Vulkan® and C++.
</p>

# Compatibility
The graphics engine offers full Vulkan support on all modern versions of Windows, excluding versions released before Windows 10. When it comes to the macOS version of the engine there is at the time writing no native Vulkan support from Apple, since Apple expects the Metal graphics API to be used instead.

However, the Vulkan SDK provides *partial* Vulkan support through the use of the MoltenVK library which is a "translation" or "porting" library that maps most of the Vulkan functionality to the underlying graphics support (via Metal) on macOS, iOS, and tvOS platforms. It is **not** a fully-conforming Vulkan driver for macOS, iOS, or tvOS devices but it works as a preliminary solution until the Metal API has been integrated in the engine.

# Build Instructions
## Prerequisites
- [Vulkan® SDK](https://www.lunarg.com/vulkan-sdk/) version 1.3.204 or newer
