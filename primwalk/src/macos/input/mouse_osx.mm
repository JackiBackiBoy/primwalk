#include "primwalk/mouse.hpp"
#include <iostream>

// Objective-C
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

namespace fz {
  MouseOSX& MouseOSX::Instance() {
    static MouseOSX instance;
    return instance;
  }

  glm::vec2 MouseOSX::getRelativePos() {
    return m_RelativePos;
  }

  glm::vec2 MouseOSX::getAbsolutePos() {
    NSPoint mouseLoc = [NSEvent mouseLocation];
    NSRect screenFrame = [[NSScreen mainScreen] frame];
    NSPoint screenLoc = NSMakePoint(mouseLoc.x,
                                    screenFrame.size.height - mouseLoc.y);
    return { (int)screenLoc.x, (int)screenLoc.y };
  }
}