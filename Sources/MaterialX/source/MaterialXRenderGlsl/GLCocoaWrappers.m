//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#if defined(__APPLE__)
# include <TargetConditionals.h>

#if !defined(TARGET_OS_VISION)
# import <Cocoa/Cocoa.h>
# import <AppKit/NSApplication.h>
#else /* defined(TARGET_OS_VISION) */
# import <UIKit/UIApplication.h>
#endif /* !defined(TARGET_OS_VISION) */

#import <MaterialX/MXRenderGlslGLCocoaWrappers.h>

void *NSOpenGLChoosePixelFormatWrapper(bool allRenders, int bufferType,
                                       int colorSize, int depthFormat,
                                       int stencilFormat, int auxBuffers,
                                       int accumSize, bool minimumPolicy,
                                       bool accelerated, bool mp_safe,
                                       bool stereo, bool supportMultiSample) {
#if !defined(TARGET_OS_VISION)
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
#else /* defined(TARGET_OS_VISION) */
  return nil;
#endif /* !defined(TARGET_OS_VISION) */
}

void NSOpenGLReleasePixelFormat(void *pPixelFormat) {
#if !defined(TARGET_OS_VISION)
  NSOpenGLPixelFormat *pixelFormat = (__bridge NSOpenGLPixelFormat *)pPixelFormat;
  #if !__has_feature(objc_arc)
  [pixelFormat release];
  #endif
#endif /* !defined(TARGET_OS_VISION) */
}

void NSOpenGLReleaseContext(void *pContext) {
#if !defined(TARGET_OS_VISION)
  NSOpenGLContext *context = (__bridge NSOpenGLContext *)pContext;
  #if !__has_feature(objc_arc)
  [context release];
  #endif
#endif /* !defined(TARGET_OS_VISION) */
}

void *NSOpenGLCreateContextWrapper(void *pPixelFormat, void *pDummyContext) {
#if !defined(TARGET_OS_VISION)
  NSOpenGLPixelFormat *pixelFormat = (__bridge NSOpenGLPixelFormat *)pPixelFormat;
  NSOpenGLContext *dummyContext = (__bridge NSOpenGLContext *)pDummyContext;
  NSOpenGLContext *context =
      [[NSOpenGLContext alloc] initWithFormat:pixelFormat
                                 shareContext:dummyContext];

  return (__bridge void*)context;
#else /* defined(TARGET_OS_VISION) */
  return nil;
#endif /* !defined(TARGET_OS_VISION) */
}

void NSOpenGLSetDrawable(void *pContext, void *pWindow) {
#if !defined(TARGET_OS_VISION)
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
#endif /* !defined(TARGET_OS_VISION) */
}

void NSOpenGLMakeCurrent(void *pContext) {
#if !defined(TARGET_OS_VISION)
  NSOpenGLContext *context = (__bridge NSOpenGLContext *)pContext;
  [context makeCurrentContext];
#endif /* !defined(TARGET_OS_VISION) */
}

void *NSOpenGLGetCurrentContextWrapper() {
#if !defined(TARGET_OS_VISION)
  return (__bridge void*)[NSOpenGLContext currentContext];
#endif /* !defined(TARGET_OS_VISION) */
}

void NSOpenGLSwapBuffers(void *pContext) {
#if !defined(TARGET_OS_VISION)
  NSOpenGLContext *context = (__bridge NSOpenGLContext *)pContext;
  [context flushBuffer];
#endif /* !defined(TARGET_OS_VISION) */
}

void NSOpenGLClearCurrentContext()
{
#if !defined(TARGET_OS_VISION)
  [NSOpenGLContext clearCurrentContext];
#endif /* !defined(TARGET_OS_VISION) */
}

void NSOpenGLDestroyContext(void **pContext)
{
#if !defined(TARGET_OS_VISION)
  NSOpenGLContext *context = (__bridge NSOpenGLContext *)*pContext;
  #if !__has_feature(objc_arc)
  [context release];
  #endif
  *pContext = NULL;
#endif /* !defined(TARGET_OS_VISION) */
}

void NSOpenGLDestroyCurrentContext(void **pContext) {
#if !defined(TARGET_OS_VISION)
  [NSOpenGLContext clearCurrentContext];
  NSOpenGLContext *context = (__bridge NSOpenGLContext *)*pContext;
  #if !__has_feature(objc_arc)
  [context release];
  #endif
  *pContext = NULL;
#endif /* !defined(TARGET_OS_VISION) */
}

void NSOpenGLClearDrawable(void *pContext) {
#if !defined(TARGET_OS_VISION)
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
#endif /* !defined(TARGET_OS_VISION) */
}

void NSOpenGLDescribePixelFormat(void *pPixelFormat, int attrib, int *vals) {
#if !defined(TARGET_OS_VISION)
  NSOpenGLPixelFormat *pixelFormat = (__bridge NSOpenGLPixelFormat *)pPixelFormat;
  [pixelFormat getValues:vals forAttribute:attrib forVirtualScreen:0];
#endif /* !defined(TARGET_OS_VISION) */
}

void NSOpenGLGetInteger(void *pContext, int param, int *vals) {
#if !defined(TARGET_OS_VISION)
  NSOpenGLContext *context = (__bridge NSOpenGLContext *)pContext;
  [context getValues:vals forParameter:(NSOpenGLContextParameter)param];
#endif /* !defined(TARGET_OS_VISION) */
}

void NSOpenGLUpdate(void *pContext) {
#if !defined(TARGET_OS_VISION)
  NSOpenGLContext *context = (__bridge NSOpenGLContext *)pContext;
  [context update];
#endif /* !defined(TARGET_OS_VISION) */
}

void *NSOpenGLGetWindow(void *pView) {
#if !defined(TARGET_OS_VISION)
  NSView *view = (__bridge NSView *)pView;
  return (__bridge void*)[view window];
#endif /* !defined(TARGET_OS_VISION) */
}

void NSOpenGLInitializeGLLibrary() {
#if !defined(TARGET_OS_VISION)
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
#endif /* !defined(TARGET_OS_VISION) */
}

#endif /* defined(__APPLE__) */
