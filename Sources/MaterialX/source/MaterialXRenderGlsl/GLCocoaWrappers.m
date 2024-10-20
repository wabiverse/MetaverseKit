//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#if defined(__APPLE__)
# include <TargetConditionals.h>

#if !TARGET_OS_IPHONE
# import <Cocoa/Cocoa.h>
# import <AppKit/NSApplication.h>
#else /* TARGET_OS_IPHONE */
# import <UIKit/UIApplication.h>
#endif /* !TARGET_OS_IPHONE */

#import <MaterialX/MXRenderGlslGLCocoaWrappers.h>

void *NSOpenGLChoosePixelFormatWrapper(bool allRenders, int bufferType,
                                       int colorSize, int depthFormat,
                                       int stencilFormat, int auxBuffers,
                                       int accumSize, bool minimumPolicy,
                                       bool accelerated, bool mp_safe,
                                       bool stereo, bool supportMultiSample) {
#if !TARGET_OS_IPHONE
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
#else /* TARGET_OS_IPHONE */
  return nil;
#endif /* !TARGET_OS_IPHONE */
}

void NSOpenGLReleasePixelFormat(void *pPixelFormat) {
#if !TARGET_OS_IPHONE
  NSOpenGLPixelFormat *pixelFormat = (__bridge NSOpenGLPixelFormat *)pPixelFormat;
  #if !__has_feature(objc_arc)
  [pixelFormat release];
  #endif
#endif /* !TARGET_OS_IPHONE */
}

void NSOpenGLReleaseContext(void *pContext) {
#if !TARGET_OS_IPHONE
  NSOpenGLContext *context = (__bridge NSOpenGLContext *)pContext;
  #if !__has_feature(objc_arc)
  [context release];
  #endif
#endif /* !TARGET_OS_IPHONE */
}

void *NSOpenGLCreateContextWrapper(void *pPixelFormat, void *pDummyContext) {
#if !TARGET_OS_IPHONE
  NSOpenGLPixelFormat *pixelFormat = (__bridge NSOpenGLPixelFormat *)pPixelFormat;
  NSOpenGLContext *dummyContext = (__bridge NSOpenGLContext *)pDummyContext;
  NSOpenGLContext *context =
      [[NSOpenGLContext alloc] initWithFormat:pixelFormat
                                 shareContext:dummyContext];

  return (__bridge void*)context;
#else /* TARGET_OS_IPHONE */
  return nil;
#endif /* !TARGET_OS_IPHONE */
}

void NSOpenGLSetDrawable(void *pContext, void *pWindow) {
#if !TARGET_OS_IPHONE
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
#endif /* !TARGET_OS_IPHONE */
}

void NSOpenGLMakeCurrent(void *pContext) {
#if !TARGET_OS_IPHONE
  NSOpenGLContext *context = (__bridge NSOpenGLContext *)pContext;
  [context makeCurrentContext];
#endif /* !TARGET_OS_IPHONE */
}

void *NSOpenGLGetCurrentContextWrapper() {
#if !TARGET_OS_IPHONE
  return (__bridge void*)[NSOpenGLContext currentContext];
#endif /* !TARGET_OS_IPHONE */
}

void NSOpenGLSwapBuffers(void *pContext) {
#if !TARGET_OS_IPHONE
  NSOpenGLContext *context = (__bridge NSOpenGLContext *)pContext;
  [context flushBuffer];
#endif /* !TARGET_OS_IPHONE */
}

void NSOpenGLClearCurrentContext()
{
#if !TARGET_OS_IPHONE
  [NSOpenGLContext clearCurrentContext];
#endif /* !TARGET_OS_IPHONE */
}

void NSOpenGLDestroyContext(void **pContext)
{
#if !TARGET_OS_IPHONE
  NSOpenGLContext *context = (__bridge NSOpenGLContext *)*pContext;
  #if !__has_feature(objc_arc)
  [context release];
  #endif
  *pContext = NULL;
#endif /* !TARGET_OS_IPHONE */
}

void NSOpenGLDestroyCurrentContext(void **pContext) {
#if !TARGET_OS_IPHONE
  [NSOpenGLContext clearCurrentContext];
  NSOpenGLContext *context = (__bridge NSOpenGLContext *)*pContext;
  #if !__has_feature(objc_arc)
  [context release];
  #endif
  *pContext = NULL;
#endif /* !TARGET_OS_IPHONE */
}

void NSOpenGLClearDrawable(void *pContext) {
#if !TARGET_OS_IPHONE
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
#endif /* !TARGET_OS_IPHONE */
}

void NSOpenGLDescribePixelFormat(void *pPixelFormat, int attrib, int *vals) {
#if !TARGET_OS_IPHONE
  NSOpenGLPixelFormat *pixelFormat = (__bridge NSOpenGLPixelFormat *)pPixelFormat;
  [pixelFormat getValues:vals forAttribute:attrib forVirtualScreen:0];
#endif /* !TARGET_OS_IPHONE */
}

void NSOpenGLGetInteger(void *pContext, int param, int *vals) {
#if !TARGET_OS_IPHONE
  NSOpenGLContext *context = (__bridge NSOpenGLContext *)pContext;
  [context getValues:vals forParameter:(NSOpenGLContextParameter)param];
#endif /* !TARGET_OS_IPHONE */
}

void NSOpenGLUpdate(void *pContext) {
#if !TARGET_OS_IPHONE
  NSOpenGLContext *context = (__bridge NSOpenGLContext *)pContext;
  [context update];
#endif /* !TARGET_OS_IPHONE */
}

void *NSOpenGLGetWindow(void *pView) {
#if !TARGET_OS_IPHONE
  NSView *view = (__bridge NSView *)pView;
  return (__bridge void*)[view window];
#endif /* !TARGET_OS_IPHONE */
}

void NSOpenGLInitializeGLLibrary() {
#if !TARGET_OS_IPHONE
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
#endif /* !TARGET_OS_IPHONE */
}

#endif /* defined(__APPLE__) */
