// std
#include <cassert>
#include <iostream>
#include <chrono>
#include <cmath>

// primwalk
#include "primwalk/window.hpp"
#include "primwalk/mouse.hpp"
#include "primwalk/rendering/graphicsDevice.hpp"
#include "primwalk/rendering/graphicsPipeline.hpp"
#include "primwalk/rendering/renderer.hpp"
#include "primwalk/rendering/frameInfo.hpp"
#include "primwalk/rendering/systems/uiRenderSystem.hpp"

// Vendor
#include <glm/gtc/matrix_transform.hpp>

// MacOS
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

  @interface FZContentView : NSView <NSTextInputClient>
  {
      fz::WindowOSX* window;
      NSTrackingArea* trackingArea;
      NSMutableAttributedString* markedText;
  }

  - (instancetype)initWithFzWindow:(fz::WindowOSX*)initWindow;

  @end

  @implementation FZContentView

  - (instancetype)initWithFzWindow:(fz::WindowOSX*)initWindow
  {
      self = [super init];
      if (self != nil)
      {
          window = initWindow;
          trackingArea = nil;
          markedText = [[NSMutableAttributedString alloc] init];

          [self updateTrackingAreas];
          [self registerForDraggedTypes:@[NSPasteboardTypeURL]];
      }

      return self;
  }

  - (BOOL)wantsUpdateLayer
  {
      return YES;
  }

  - (NSArray*)validAttributesForMarkedText
  {
      return [NSArray array];
  }

  @end

  @interface ApplicationDelegate : NSObject<NSApplicationDelegate>
  @end

  @implementation ApplicationDelegate

  - (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication {
    return YES;
  }
  @end

  @interface WindowDelegate : NSObject<NSWindowDelegate>
  @property bool* shouldQuit;
  @end

  @implementation WindowDelegate
  - (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize {
    return frameSize;
  }

  - (BOOL)windowShouldClose:(NSWindow *)sender {
    *self.shouldQuit = true;
    return YES;
  }
  @end

namespace fz {
  WindowOSX::WindowOSX(const std::string& name, const int& width, const int& height, WindowOSX* parent)
    : WindowBase() {
    assert(width > 0 && "ASSERTION FAILED: Width must be greater than 0");
    assert(height > 0 && "ASSERTION FAILED: Height must be greater than 0");

    m_Name = name;
    m_Width = width;
    m_Height = height;

    //init();
  }

  WindowOSX::~WindowOSX() {
  }

  int WindowOSX::run() {
    bool shouldQuit = false;

    m_Object = [
      [NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, m_Width, m_Height)
      styleMask:NSWindowStyleMaskResizable |
                NSWindowStyleMaskTitled |
                NSWindowStyleMaskClosable |
                NSWindowStyleMaskMiniaturizable |
                NSWindowStyleMaskFullSizeContentView
      backing:NSBackingStoreBuffered
      defer:NO];
    
    [m_Object setTitlebarAppearsTransparent:YES];
    [m_Object setTitleVisibility:NSWindowTitleHidden]; // hide default title text

    // TODO: Investigate character encoding (potential issue)
    NSString* titleString = @(m_Name.c_str());
    [m_Object setTitle:titleString];
    [m_Object setMinSize:NSMakeSize(200, 100)];

    WindowDelegate* windowDelegate = [[WindowDelegate alloc] init];
    windowDelegate.shouldQuit = &shouldQuit;
    [m_Object setDelegate:windowDelegate];
    [m_Object setAcceptsMouseMovedEvents:YES];

    // Rendering
    createGraphicsContext();
    VkRenderPass renderPass = m_Renderer->getSwapChainRenderPass();
    std::vector<VkDescriptorSetLayout> setLayouts;
    m_UIRenderSystem = std::make_unique<UIRenderSystem>(*m_GraphicsDevice, renderPass, setLayouts);

    // Show the window
    [m_Object makeKeyAndOrderFront:nil];

    static float dt = 0.0f;
    auto currentTime = std::chrono::high_resolution_clock::now();

    // Run the event loop
    while (!shouldQuit) {
      @autoreleasepool {
        NSEvent* event = [[NSApplication sharedApplication] nextEventMatchingMask:NSEventMaskAny
                      untilDate:nil
                      inMode:NSDefaultRunLoopMode
                      dequeue:YES];
        if (event) {
          [[NSApplication sharedApplication] sendEvent:event];
        }
        else {
          NSRect windowFrame = [[m_Object contentView] frame];
          CGFloat width = windowFrame.size.width;
          CGFloat height = windowFrame.size.height;
          m_Width = width;
          m_Height = height;

          // TODO: Make frame timing consistent when resizing window
          auto newTime = std::chrono::high_resolution_clock::now();
          float dt = std::chrono::duration<float, std::chrono::seconds::period>(
              newTime - currentTime).count();
          currentTime = newTime;

          if (auto commandBuffer = m_Renderer->beginFrame()) {
            int frameIndex = m_Renderer->getFrameIndex();
            // Update
            FrameInfo frameInfo{};
            frameInfo.frameIndex = frameIndex;
            frameInfo.frameTime = dt;
            frameInfo.commandBuffer = commandBuffer;
            frameInfo.windowWidth = (float)m_Renderer->getSwapChainWidth();
            frameInfo.windowHeight = (float)m_Renderer->getSwapChainHeight();

            m_UIRenderSystem->onUpdate(frameInfo);

            // Render
            m_Renderer->beginSwapChainRenderPass(commandBuffer);
            m_UIRenderSystem->onRender(frameInfo);
            m_Renderer->endSwapChainRenderPass(commandBuffer);
            
            m_Renderer->endFrame();
          }
        }
      }
    }

    vkDeviceWaitIdle(m_GraphicsDevice->getDevice());
  }

  std::vector<std::string> WindowOSX::getRequiredVulkanInstanceExtensions() {
    std::vector<std::string> extensions = {
      "VK_KHR_surface",
      "VK_EXT_metal_surface"
    };

    // TODO: Add alternative MVK extension

    return extensions;
  }

  VkResult WindowOSX::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
    m_View = [[FZContentView alloc] initWithFzWindow:this];
    [m_Object setContentView: m_View];
    [m_Object makeFirstResponder: m_View];

    NSBundle* bundle = [NSBundle bundleWithPath:@"/System/Library/Frameworks/QuartzCore.framework"];
    m_Layer = [[bundle classNamed:@"CAMetalLayer"] layer];
    
    if (!m_Layer) {
      std::cout << "METAL ERROR: Failed to create layer for view\n";
    }

    [m_View setLayer:m_Layer];
    [m_View setWantsLayer:YES];

    VkMetalSurfaceCreateInfoEXT sci;

    PFN_vkCreateMetalSurfaceEXT vkCreateMetalSurfaceEXT;
    vkCreateMetalSurfaceEXT = (PFN_vkCreateMetalSurfaceEXT)
        vkGetInstanceProcAddr(instance, "vkCreateMetalSurfaceEXT");
    if (!vkCreateMetalSurfaceEXT)
    {
      std::cout << "Metal ERROR!" << std::endl;
    }

    memset(&sci, 0, sizeof(sci));
    sci.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    sci.pLayer = m_Layer;

    return vkCreateMetalSurfaceEXT(instance, &sci, nullptr, surface);
  }

  int WindowOSX::init() {
  }

  void WindowOSX::createGraphicsContext() {
    m_GraphicsDevice = std::make_unique<GraphicsDevice_Vulkan>(*this);
    m_Renderer = std::make_unique<Renderer>(*this, *m_GraphicsDevice);
  }

  // ------ Event functions ------
  void WindowOSX::onCreate() {
  }

  void WindowOSX::onUpdate(float dt) {
  }

  void WindowOSX::onRender(float dt) {
  }

  // Getters
  int WindowOSX::getWidth() const {
    return m_Width;
  }

  int WindowOSX::getHeight() const {
    return m_Height;
  }
}