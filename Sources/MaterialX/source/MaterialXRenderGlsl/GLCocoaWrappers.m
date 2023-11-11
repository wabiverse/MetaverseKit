//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#if defined(__APPLE__)

#import <Cocoa/Cocoa.h>

#import <AppKit/NSApplication.h>

#import "GLCocoaWrappers.h"

void *NSOpenGLChoosePixelFormatWrapper(bool allRenders, int bufferType,
                                       int colorSize, int depthFormat,
                                       int stencilFormat, int auxBuffers,
                                       int accumSize, bool minimumPolicy,
                                       bool accelerated, bool mp_safe,
                                       bool stereo, bool supportMultiSample) {
  // NOTE: the autoreleasepool (for the objc impl) only exists if ARC is enabled.
  // TODO: refactor this objc code to cxx.
  // Create local autorelease pool for any objects that need to be autoreleased.
#if !__has_feature(objc_arc)
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
#endif

  NSOpenGLPixelFormatAttribute list[50];
  int i = 0;
  if (allRenders) {
    list[i++] = NSOpenGLPFAAllRenderers;
  }
  if (bufferType == 1) // On screen
  {
    list[i++] = NSOpenGLPFADoubleBuffer;
  }
  if (colorSize != 0) {
    list[i++] = NSOpenGLPFAColorSize;
    list[i++] = colorSize;
  }
  if (depthFormat != 0) {
    list[i++] = NSOpenGLPFADepthSize;
    list[i++] = depthFormat;
  }
  if (stencilFormat != 0) {
    list[i++] = NSOpenGLPFAStencilSize;
    list[i++] = stencilFormat;
  }
  if (auxBuffers != 0) {
    list[i++] = NSOpenGLPFAAuxBuffers;
    list[i++] = auxBuffers;
  }
  if (accumSize != 0) {
    list[i++] = NSOpenGLPFAAccumSize;
    list[i++] = accumSize;
  }
  if (minimumPolicy) {
    list[i++] = NSOpenGLPFAMinimumPolicy;
  }
  if (accelerated) {
    list[i++] = NSOpenGLPFAAccelerated;
  }

  // Add multisample support to the list of attributes if supported
  //
  int multiSampleAttrIndex = i;
  if (supportMultiSample) {
    // Default to 4 samples
    list[i++] = NSOpenGLPFASampleBuffers;
    list[i++] = TRUE;
    list[i++] = NSOpenGLPFASamples;
    list[i++] = 4;
  }
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 101000
  list[i++] = NSOpenGLPFAOpenGLProfile;
  list[i++] = NSOpenGLProfileVersion4_1Core;
#else
  list[i++] = NSOpenGLPFAOpenGLProfile;
  list[i++] = NSOpenGLProfileVersion3_2Core;
#endif
  list[i++] = 0;

  NSOpenGLPixelFormat *pixelFormat =
      [[NSOpenGLPixelFormat alloc] initWithAttributes:list];
  if (!pixelFormat) {
    // Try again without multisample, if previous try failed
    //
    list[multiSampleAttrIndex++] = 0; // NSOpenGLPFASampleBuffers
    list[multiSampleAttrIndex++] = 0; // NSOpenGLPFASampleBuffers value
    list[multiSampleAttrIndex++] = 0; // NSOpenGLPFASamplesB
    list[multiSampleAttrIndex++] = 0; // NSOpenGLPFASamples value

    pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:list];
  }

// Free up memory (pool only exists if ARC is enabled)
#if !__has_feature(objc_arc)
  [pool release];
#endif

  return (__bridge void*)pixelFormat;
}

void NSOpenGLReleasePixelFormat(void *pPixelFormat) {
  NSOpenGLPixelFormat *pixelFormat = (__bridge NSOpenGLPixelFormat *)pPixelFormat;
  #if !__has_feature(objc_arc)
  [pixelFormat release];
  #endif
}

void NSOpenGLReleaseContext(void *pContext) {
  NSOpenGLContext *context = (__bridge NSOpenGLContext *)pContext;
  #if !__has_feature(objc_arc)
  [context release];
  #endif
}

void *NSOpenGLCreateContextWrapper(void *pPixelFormat, void *pDummyContext) {
  NSOpenGLPixelFormat *pixelFormat = (__bridge NSOpenGLPixelFormat *)pPixelFormat;
  NSOpenGLContext *dummyContext = (__bridge NSOpenGLContext *)pDummyContext;
  NSOpenGLContext *context =
      [[NSOpenGLContext alloc] initWithFormat:pixelFormat
                                 shareContext:dummyContext];

  return (__bridge void*)context;
}

void NSOpenGLSetDrawable(void *pContext, void *pWindow) {
  // NOTE: the autoreleasepool (for the objc impl) only exists if ARC is enabled.
  // TODO: refactor this objc code to cxx.
  // Create local autorelease pool for any objects that need to be autoreleased.
#if !__has_feature(objc_arc)
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
#endif

  NSOpenGLContext *context = (__bridge NSOpenGLContext *)pContext;
  NSWindow *window = (__bridge NSWindow *)pWindow;
  NSView *view = [window contentView];
  [context setView:view];

// Free up memory (pool only exists if ARC is enabled)
#if !__has_feature(objc_arc)
  [pool release];
#endif
}

void NSOpenGLMakeCurrent(void *pContext) {
  NSOpenGLContext *context = (__bridge NSOpenGLContext *)pContext;
  [context makeCurrentContext];
}

void *NSOpenGLGetCurrentContextWrapper() {
  return (__bridge void*)[NSOpenGLContext currentContext];
}

void NSOpenGLSwapBuffers(void *pContext) {
  NSOpenGLContext *context = (__bridge NSOpenGLContext *)pContext;
  [context flushBuffer];
}

void NSOpenGLClearCurrentContext() { [NSOpenGLContext clearCurrentContext]; }

void NSOpenGLDestroyContext(void **pContext) {
  NSOpenGLContext *context = (__bridge NSOpenGLContext *)*pContext;
  #if !__has_feature(objc_arc)
  [context release];
  #endif
  *pContext = NULL;
}

void NSOpenGLDestroyCurrentContext(void **pContext) {
  [NSOpenGLContext clearCurrentContext];
  NSOpenGLContext *context = (__bridge NSOpenGLContext *)*pContext;
  #if !__has_feature(objc_arc)
  [context release];
  #endif
  *pContext = NULL;
}

void NSOpenGLClearDrawable(void *pContext) {
  // NOTE: the autoreleasepool (for the objc impl) only exists if ARC is enabled.
  // TODO: refactor this objc code to cxx.
  // Create local autorelease pool for any objects that need to be autoreleased.
#if !__has_feature(objc_arc)
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
#endif

  NSOpenGLContext *context = (__bridge NSOpenGLContext *)pContext;
  [context clearDrawable];

// Free up memory (pool only exists if ARC is enabled)
#if !__has_feature(objc_arc)
  [pool release];
#endif
}

void NSOpenGLDescribePixelFormat(void *pPixelFormat, int attrib, int *vals) {
  NSOpenGLPixelFormat *pixelFormat = (__bridge NSOpenGLPixelFormat *)pPixelFormat;
  [pixelFormat getValues:vals forAttribute:attrib forVirtualScreen:0];
}

void NSOpenGLGetInteger(void *pContext, int param, int *vals) {
  NSOpenGLContext *context = (__bridge NSOpenGLContext *)pContext;
  [context getValues:vals forParameter:(NSOpenGLContextParameter)param];
}

void NSOpenGLUpdate(void *pContext) {
  NSOpenGLContext *context = (__bridge NSOpenGLContext *)pContext;
  [context update];
}

void *NSOpenGLGetWindow(void *pView) {
  NSView *view = (__bridge NSView *)pView;
  return (__bridge void*)[view window];
}

void NSOpenGLInitializeGLLibrary() {
  // NOTE: the autoreleasepool (for the objc impl) only exists if ARC is enabled.
  // TODO: refactor this objc code to cxx.
  // Create local autorelease pool for any objects that need to be autoreleased.
  // (needed in batch mode).
#if !__has_feature(objc_arc)
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
#endif
  NSOpenGLPixelFormatAttribute attrib[] = {NSOpenGLPFAAllRenderers,
                                           NSOpenGLPFADoubleBuffer, 0};
  NSOpenGLPixelFormat *dummyPixelFormat =
      [[NSOpenGLPixelFormat alloc] initWithAttributes:attrib];
  if (nil != dummyPixelFormat) {
    #if !__has_feature(objc_arc)
    [dummyPixelFormat release];
    #endif
  }
// Free up memory (pool only exists if ARC is enabled)
#if !__has_feature(objc_arc)
  [pool release];
#endif
}

#endif
