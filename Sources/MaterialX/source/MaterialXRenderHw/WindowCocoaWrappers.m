//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#if defined(__APPLE__)

#include <TargetConditionals.h>

#if !TARGET_OS_IOS && !TARGET_OS_VISION

#import <AppKit/NSApplication.h>
#import <Cocoa/Cocoa.h>
#import <MaterialX/MXRenderHwWindowCocoaWrappers.h>

void *NSUtilGetView(void *pWindow) {
  NSWindow *window = (__bridge NSWindow *)pWindow;
  NSView *view = [window contentView];
  return (__bridge void *)view;
}

void *NSUtilCreateWindow(unsigned int width, unsigned int height,
                         const char *title, bool batchMode) {
  // In batch mode, ensure that Cocoa is initialized
  if (batchMode) {
    NSApplicationLoad();
  }

  // NOTE: the autoreleasepool (for the objc impl) only exists if ARC is
  // enabled.
  // TODO: refactor this objc code to cxx.
  // Create local autorelease pool for any objects that need to be autoreleased.
#if !__has_feature(objc_arc)
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
#endif
  NSWindow *window = [[NSWindow alloc]
      initWithContentRect:NSMakeRect(0, 0, width, height)
                styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                          NSWindowStyleMaskMiniaturizable |
                          NSWindowStyleMaskResizable
                  backing:NSBackingStoreBuffered
                    defer:NO];
  NSString *string = [NSString stringWithUTF8String:title];

  [window setTitle:string];
  [window setAlphaValue:0.0];

  // Free up memory (pool only exists if ARC is enabled)
#if !__has_feature(objc_arc)
  [pool release];
#endif

  return (__bridge void *)window;
}

void NSUtilShowWindow(void *pWindow) {
  NSWindow *window = (__bridge NSWindow *)pWindow;
  [window orderFront:window];
}

void NSUtilHideWindow(void *pWindow) {
  NSWindow *window = (__bridge NSWindow *)pWindow;
  [window orderOut:window];
}

void NSUtilSetFocus(void *pWindow) {
  NSWindow *window = (__bridge NSWindow *)pWindow;
  [window makeKeyAndOrderFront:window];
}

void NSUtilDisposeWindow(void *pWindow) {
  // NOTE: the autoreleasepool (for the objc impl) only exists if ARC is
  // enabled.
  // TODO: refactor this objc code to cxx.
  // Create local autorelease pool for any objects that need to be autoreleased.
#if !__has_feature(objc_arc)
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
#endif

  NSWindow *window = (__bridge NSWindow *)pWindow;
  [window close];

  // Free up memory (pool only exists if ARC is enabled)
#if !__has_feature(objc_arc)
  [pool release];
#endif
}

#endif /* !TARGET_OS_IOS && !TARGET_OS_VISION */

#endif /* defined(__APPLE__) */
