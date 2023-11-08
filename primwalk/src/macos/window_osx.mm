// std
#include <cassert>
#include <iostream>
#include <chrono>
#include <cmath>

// primwalk
#include "../window.hpp"
#include "../common/rendering/graphicsDevice.hpp"
#include "../common/graphicsPipeline.hpp"
#include "../common/rendering/renderer.hpp"
#include "../common/rendering/frameInfo.hpp"
#include "../common/rendering/systems/uiRenderSystem.hpp"

// Vendor
#include <glm/gtc/matrix_transform.hpp>

// MacOS
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

  @interface ContentView : NSView <NSTextInputClient>
  {
      pw::WindowOSX* window;
      NSTrackingArea* trackingArea;
      NSMutableAttributedString* markedText;
  }

  - (instancetype)initWithWindow:(pw::WindowOSX*)initWindow;

  @end

  @implementation ContentView

  - (instancetype)initWithWindow:(pw::WindowOSX*)initWindow
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

namespace pw {
  WindowOSX::WindowOSX(const std::string& name, int width, int height)
    : WindowBase(name, width, height) {
    assert(width > 0 && "ASSERTION FAILED: Width must be greater than 0");
    assert(height > 0 && "ASSERTION FAILED: Height must be greater than 0");

    init();
  }

  WindowOSX::~WindowOSX() {
  }

  int WindowOSX::run() {
    WindowDelegate* windowDelegate = [[WindowDelegate alloc] init];
    windowDelegate.shouldQuit = &m_ShouldClose;
    [m_Object setDelegate:windowDelegate];
    [m_Object setAcceptsMouseMovedEvents:YES];
    [m_Object makeKeyAndOrderFront:nil];

    // Run the event loop
    while (!m_ShouldClose) {
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
        }
      }
    }
  }

  int WindowOSX::init() {
    m_Object = [
      [NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, m_Width, m_Height)
      styleMask:NSWindowStyleMaskResizable |
                NSWindowStyleMaskTitled |
                NSWindowStyleMaskClosable |
                NSWindowStyleMaskMiniaturizable |
                NSWindowStyleMaskFullSizeContentView
      backing:NSBackingStoreBuffered
      defer:NO
    ];
    
    [m_Object setTitlebarAppearsTransparent:YES]; // hide default title bar
    [m_Object setTitleVisibility:NSWindowTitleHidden]; // hide default title text

    // TODO: Investigate character encoding (potential issue)
    NSString* titleString = @(m_Name.c_str());
    [m_Object setTitle:titleString];
    [m_Object setMinSize:NSMakeSize(200, 100)];

    m_View = [[ContentView alloc] initWithWindow:this];
    [m_Object setContentView:m_View];
    [m_Object makeFirstResponder:m_View];

    NSBundle* bundle = [NSBundle bundleWithPath:@"/System/Library/Frameworks/QuartzCore.framework"];
    m_Layer = [[bundle classNamed:@"CAMetalLayer"] layer];
    
    if (!m_Layer) {
      std::cout << "METAL ERROR: Failed to create layer for view\n";
    }

    [m_View setLayer:m_Layer];
    [m_View setWantsLayer:YES];
  }

  // ------ Event functions ------
  void WindowOSX::onCreate() {
  }

  void WindowOSX::onUpdate(float dt) {
  }

  bool WindowOSX::shouldClose() {
    return m_ShouldClose;
  }
}
