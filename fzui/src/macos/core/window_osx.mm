// std
#include <cassert>
#include <iostream>
#include <chrono>
#include <cmath>

// FZUI
#include "fzui/window.hpp"
#include "fzui/rendering/vertexArray.hpp"
#include "fzui/rendering/vertexBuffer.hpp"
#include "fzui/rendering/indexBuffer.hpp"
#include "fzui/rendering/bufferLayout.hpp"
#include "fzui/data/texture.hpp"
#include "fzui/uiButton.hpp"
#include "fzui/data/fonts/fontManager.hpp"
#include "fzui/mouse.hpp"

// Vendor
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

// MacOS
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

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
    delete m_Renderer2D;
  }

  int WindowOSX::run() {
    bool shouldQuit = false;

    // Create an instance of NSWindow
    NSWindow* window = [
      [NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, m_Width, m_Height)
      styleMask:NSWindowStyleMaskResizable |
                NSWindowStyleMaskTitled |
                NSWindowStyleMaskClosable |
                NSWindowStyleMaskMiniaturizable |
                NSWindowStyleMaskFullSizeContentView
      backing:NSBackingStoreBuffered
      defer:NO];
    
    //NSAppearance *appearance = [NSAppearance appearanceNamed:NSAppearanceNameVibrantDark];
    //[window setAppearance:appearance];
    [window setTitlebarAppearsTransparent:YES];
    [window setBackgroundColor:[NSColor colorWithRed:33.0 / 255.0
                                        green:37.0 / 255.0
                                        blue:42.0 / 255.0
                                        alpha:0.0]];
    [window setTitleVisibility:NSWindowTitleHidden]; // hide default title text

    
    // TODO: Investigate character encoding (potential issue)
    NSString* titleString = @(m_Name.c_str());
    [window setTitle:titleString];
    [window setMinSize:NSMakeSize(100, 100)];

    WindowDelegate* windowDelegate = [[WindowDelegate alloc] init];
    windowDelegate.shouldQuit = &shouldQuit;
    //window.delegate = [[WindowDelegate alloc] init];
    [window setDelegate:windowDelegate];

    NSOpenGLPixelFormatAttribute attribs[] =
    {
      NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
      NSOpenGLPFADoubleBuffer,
      NSOpenGLPFAAccelerated,
      NSOpenGLPFAColorSize, 24,
      NSOpenGLPFAAlphaSize, 8,
      NSOpenGLPFADepthSize, 16,
      NSOpenGLPFAStencilSize, 8,
      0
    };

    // Create a pixel format with double buffering and other attributes
    NSOpenGLPixelFormat* pixelFormat =
      [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];

    NSOpenGLContext* openGLContext =
      [[NSOpenGLContext alloc] initWithFormat:pixelFormat
                               shareContext:nil];

    CGFloat scaleFactor = [[window screen] backingScaleFactor];
    std::cout << "Scale factor: " << scaleFactor << std::endl;
    NSRect contentRect = [window contentRectForFrameRect:[window frame]];
    NSOpenGLView* glView = [[NSOpenGLView alloc] initWithFrame:contentRect pixelFormat:pixelFormat];
    [glView setWantsBestResolutionOpenGLSurface:YES];
    [glView setOpenGLContext:openGLContext];
    [window setContentView:glView];

    // Make the OpenGL context the current context
    [openGLContext makeCurrentContext];

    if (!gladLoadGL()) {
      printf("Could not initialize GLAD \n");
    }
    else {
      printf("OpenGL version: %i.%i\n", GLVersion.major, GLVersion.minor);
    }

    // Show the window
    [window makeKeyAndOrderFront:nil];

    m_Renderer2D = new Renderer2D();
    static float dt = 0.0f;
    auto currentTime = std::chrono::high_resolution_clock::now();

    onCreate();

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
          [openGLContext makeCurrentContext];

          NSRect windowFrame = [[window contentView] frame];
          CGFloat width = windowFrame.size.width;
          CGFloat height = windowFrame.size.height;
          m_Width = width;
          m_Height = height;

          glClearColor(m_BackgroundColor.r / 255.0f,
                       m_BackgroundColor.g / 255.0f,
                       m_BackgroundColor.b / 255.0f, 1.0f);
          glClear(GL_COLOR_BUFFER_BIT);
          glViewport(0, 0, m_Width * 2, m_Height * 2);
          m_Renderer2D->setViewport(m_Width, m_Height);

          // TODO: Make frame timing consistent when resizing window
          auto newTime = std::chrono::high_resolution_clock::now();
          float dt = std::chrono::duration<float, std::chrono::seconds::period>(
              newTime - currentTime).count();
          currentTime = newTime;

          // OnUpdate
          NSPoint mouseLoc = [NSEvent mouseLocation];
          NSPoint windowOrigin = [window frame].origin;
          NSPoint relativeMouseLoc = NSMakePoint(mouseLoc.x - windowOrigin.x, NSMaxY([window frame]) - mouseLoc.y);
          Mouse::Instance().m_RelativePos = { (int)relativeMouseLoc.x, (int)relativeMouseLoc.y };
          
          Window::onUpdate(dt);
          onUpdate(dt);

          // OnRender
          onRender(dt);
          Window::onRender(dt);

          [openGLContext flushBuffer];
        }
      }
    }
  }

  // UI
  void WindowOSX::addElement(UIElement* elem) {
    m_UIElements.push_back(elem);
  }

  void WindowOSX::addContainer(UIContainer* container) {
    m_Containers.push_back(container);
  }

  int WindowOSX::init() {
  }

  void WindowOSX::createGraphicsContext() {
  }

  // ------ Event functions ------
  void WindowOSX::onCreate() {
  }

  void WindowOSX::onUpdate(float dt) {
    // Update mouse position

    for (UIElement* element : m_UIElements) {
      element->update(dt); // TODO: Pass delta time instead
    }
  }

  void WindowOSX::onRender(float dt) {
    m_Renderer2D->begin();

    // Render UI containers
    for (UIContainer* container : m_Containers) {
      container->onRender(m_Renderer2D);
    }

    // Render UI elements
    for (UIElement* element : m_UIElements) {
      element->draw(m_Renderer2D);
    }

    m_Renderer2D->drawRect(m_Width, 30, { 0, 0 }, {33, 37, 43 });

    m_Renderer2D->end();
  }

  // Getters
  int WindowOSX::getWidth() const {
    return m_Width;
  }

  int WindowOSX::getHeight() const {
    return m_Height;
  }
}