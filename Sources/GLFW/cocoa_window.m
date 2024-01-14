//========================================================================
// GLFW 3.4 macOS - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2009-2019 Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
// It is fine to use C99 in this file because it will not be built with VS
//========================================================================

#include "internal.h"

#include <float.h>
#include <string.h>

// Returns the style mask corresponding to the window settings
//
static NSUInteger getStyleMask(_GLFWwindow* window)
{
#if !TARGET_OS_VISION
    NSUInteger styleMask = NSWindowStyleMaskMiniaturizable;

    if (window->monitor || !window->decorated)
        styleMask |= NSWindowStyleMaskBorderless;
    else
    {
        styleMask |= NSWindowStyleMaskTitled |
                     NSWindowStyleMaskClosable;

        if (window->resizable)
            styleMask |= NSWindowStyleMaskResizable;
    }
    

#else /* TARGET_OS_VISION */
    NSUInteger styleMask = 0;
#endif /* !TARGET_OS_VISION */
    return styleMask;
}

// Returns whether the cursor is in the content area of the specified window
//
static GLFWbool cursorInContentArea(_GLFWwindow* window)
{
#if !TARGET_OS_VISION
    const NSPoint pos = [window->ns.object mouseLocationOutsideOfEventStream];
    return [window->ns.view mouse:pos inRect:[window->ns.view frame]];
#else /* TARGET_OS_VISION */
    return GLFW_FALSE;
#endif /* !TARGET_OS_VISION */
}

// Hides the cursor if not already hidden
//
static void hideCursor(_GLFWwindow* window)
{
#if !TARGET_OS_VISION
    if (!_glfw.ns.cursorHidden)
    {
        [NSCursor hide];
        _glfw.ns.cursorHidden = GLFW_TRUE;
    }
#endif /* !TARGET_OS_VISION */
}

// Shows the cursor if not already shown
//
static void showCursor(_GLFWwindow* window)
{
#if !TARGET_OS_VISION
    if (_glfw.ns.cursorHidden)
    {
        [NSCursor unhide];
        _glfw.ns.cursorHidden = GLFW_FALSE;
    }
#endif /* !TARGET_OS_VISION */
}

// Updates the cursor image according to its cursor mode
//
static void updateCursorImage(_GLFWwindow* window)
{
#if !TARGET_OS_VISION
    if (window->cursorMode == GLFW_CURSOR_NORMAL)
    {
        showCursor(window);

        if (window->cursor)
            [(NSCursor*) window->cursor->ns.object set];
        else
            [[NSCursor arrowCursor] set];
    }
    else
        hideCursor(window);
#endif /* !TARGET_OS_VISION */
}

// Apply chosen cursor mode to a focused window
//
static void updateCursorMode(_GLFWwindow* window)
{
#if !TARGET_OS_VISION
    if (window->cursorMode == GLFW_CURSOR_DISABLED)
    {
        _glfw.ns.disabledCursorWindow = window;
        _glfwPlatformGetCursorPos(window,
                                  &_glfw.ns.restoreCursorPosX,
                                  &_glfw.ns.restoreCursorPosY);
        _glfwCenterCursorInContentArea(window);
        CGAssociateMouseAndMouseCursorPosition(false);
    }
    else if (_glfw.ns.disabledCursorWindow == window)
    {
        _glfw.ns.disabledCursorWindow = NULL;
        CGAssociateMouseAndMouseCursorPosition(true);
        _glfwPlatformSetCursorPos(window,
                                  _glfw.ns.restoreCursorPosX,
                                  _glfw.ns.restoreCursorPosY);
    }

    if (cursorInContentArea(window))
        updateCursorImage(window);
#endif /* !TARGET_OS_VISION */
}

// Make the specified window and its video mode active on its monitor
//
static void acquireMonitor(_GLFWwindow* window)
{
#if !TARGET_OS_VISION
    _glfwSetVideoModeNS(window->monitor, &window->videoMode);
    const CGRect bounds = CGDisplayBounds(window->monitor->ns.displayID);
    const NSRect frame = NSMakeRect(bounds.origin.x,
                                    _glfwTransformYNS(bounds.origin.y + bounds.size.height - 1),
                                    bounds.size.width,
                                    bounds.size.height);

    [window->ns.object setFrame:frame display:YES];

    _glfwInputMonitorWindow(window->monitor, window);
#endif /* !TARGET_OS_VISION */
}

// Remove the window and restore the original video mode
//
static void releaseMonitor(_GLFWwindow* window)
{
#if !TARGET_OS_VISION
    if (window->monitor->window != window)
        return;

    _glfwInputMonitorWindow(window->monitor, NULL);
    _glfwRestoreVideoModeNS(window->monitor);
#endif /* !TARGET_OS_VISION */
}

// Translates macOS key modifiers into GLFW ones
//
static int translateFlags(NSUInteger flags)
{
    int mods = 0;
  
#if !TARGET_OS_VISION
    if (flags & NSEventModifierFlagShift)
        mods |= GLFW_MOD_SHIFT;
    if (flags & NSEventModifierFlagControl)
        mods |= GLFW_MOD_CONTROL;
    if (flags & NSEventModifierFlagOption)
        mods |= GLFW_MOD_ALT;
    if (flags & NSEventModifierFlagCommand)
        mods |= GLFW_MOD_SUPER;
    if (flags & NSEventModifierFlagCapsLock)
        mods |= GLFW_MOD_CAPS_LOCK;
#endif /* !TARGET_OS_VISION */

    return mods;
}

// Translates a macOS keycode to a GLFW keycode
//
static int translateKey(unsigned int key)
{
#if !TARGET_OS_VISION
    if (key >= sizeof(_glfw.ns.keycodes) / sizeof(_glfw.ns.keycodes[0]))
        return GLFW_KEY_UNKNOWN;

    return _glfw.ns.keycodes[key];
#else /* TARGET_OS_VISION */
    return GLFW_KEY_UNKNOWN;
#endif /* !TARGET_OS_VISION */
}

// Translate a GLFW keycode to a Cocoa modifier flag
//
static NSUInteger translateKeyToModifierFlag(int key)
{
#if !TARGET_OS_VISION
    switch (key)
    {
        case GLFW_KEY_LEFT_SHIFT:
        case GLFW_KEY_RIGHT_SHIFT:
            return NSEventModifierFlagShift;
        case GLFW_KEY_LEFT_CONTROL:
        case GLFW_KEY_RIGHT_CONTROL:
            return NSEventModifierFlagControl;
        case GLFW_KEY_LEFT_ALT:
        case GLFW_KEY_RIGHT_ALT:
            return NSEventModifierFlagOption;
        case GLFW_KEY_LEFT_SUPER:
        case GLFW_KEY_RIGHT_SUPER:
            return NSEventModifierFlagCommand;
        case GLFW_KEY_CAPS_LOCK:
            return NSEventModifierFlagCapsLock;
    }
#endif /* !TARGET_OS_VISION */
    return 0;
}

// Defines a constant for empty ranges in NSTextInputClient
//
static const NSRange kEmptyRange = { NSNotFound, 0 };


//------------------------------------------------------------------------
// Delegate for window related notifications
//------------------------------------------------------------------------

@interface GLFWWindowDelegate : NSObject
{
    _GLFWwindow* window;
}

- (instancetype)initWithGlfwWindow:(_GLFWwindow *)initWindow;

@end

@implementation GLFWWindowDelegate

- (instancetype)initWithGlfwWindow:(_GLFWwindow *)initWindow
{
    self = [super init];
    if (self != nil)
        window = initWindow;

    return self;
}

- (BOOL)windowShouldClose:(id)sender
{
    _glfwInputWindowCloseRequest(window);
    return NO;
}

- (void)windowDidResize:(NSNotification *)notification
{
#if !TARGET_OS_VISION
    if (window->context.client != GLFW_NO_API)
        [window->context.nsgl.object update];

    if (_glfw.ns.disabledCursorWindow == window)
        _glfwCenterCursorInContentArea(window);

    const int maximized = [window->ns.object isZoomed];
    if (window->ns.maximized != maximized)
    {
        window->ns.maximized = maximized;
        _glfwInputWindowMaximize(window, maximized);
    }

    const NSRect contentRect = [window->ns.view frame];
    const NSRect fbRect = [window->ns.view convertRectToBacking:contentRect];

    if (fbRect.size.width != window->ns.fbWidth ||
        fbRect.size.height != window->ns.fbHeight)
    {
        window->ns.fbWidth  = fbRect.size.width;
        window->ns.fbHeight = fbRect.size.height;
        _glfwInputFramebufferSize(window, fbRect.size.width, fbRect.size.height);
    }

    if (contentRect.size.width != window->ns.width ||
        contentRect.size.height != window->ns.height)
    {
        window->ns.width  = contentRect.size.width;
        window->ns.height = contentRect.size.height;
        _glfwInputWindowSize(window, contentRect.size.width, contentRect.size.height);
    }
#endif /* !TARGET_OS_VISION */
}

- (void)windowDidMove:(NSNotification *)notification
{
    if (window->context.client != GLFW_NO_API)
        [window->context.nsgl.object update];

    if (_glfw.ns.disabledCursorWindow == window)
        _glfwCenterCursorInContentArea(window);

    int x, y;
    _glfwPlatformGetWindowPos(window, &x, &y);
    _glfwInputWindowPos(window, x, y);
}

- (void)windowDidMiniaturize:(NSNotification *)notification
{
    if (window->monitor)
        releaseMonitor(window);

    _glfwInputWindowIconify(window, GLFW_TRUE);
}

- (void)windowDidDeminiaturize:(NSNotification *)notification
{
    if (window->monitor)
        acquireMonitor(window);

    _glfwInputWindowIconify(window, GLFW_FALSE);
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    if (_glfw.ns.disabledCursorWindow == window)
        _glfwCenterCursorInContentArea(window);

    _glfwInputWindowFocus(window, GLFW_TRUE);
    updateCursorMode(window);
}

- (void)windowDidResignKey:(NSNotification *)notification
{
    if (window->monitor && window->autoIconify)
        _glfwPlatformIconifyWindow(window);

    _glfwInputWindowFocus(window, GLFW_FALSE);
}

@end


//------------------------------------------------------------------------
// Content view class for the GLFW window
//------------------------------------------------------------------------

#if !TARGET_OS_VISION
@interface GLFWContentView : NSView <NSTextInputClient>
#else /* TARGET_OS_VISION */
@interface GLFWContentView : UIView
#endif /* !TARGET_OS_VISION */
{
    _GLFWwindow* window;
#if !TARGET_OS_VISION
    NSTrackingArea* trackingArea;
#endif /* !TARGET_OS_VISION */
    NSMutableAttributedString* markedText;
}

- (instancetype)initWithGlfwWindow:(_GLFWwindow *)initWindow;

@end

@implementation GLFWContentView

- (instancetype)initWithGlfwWindow:(_GLFWwindow *)initWindow
{
    self = [super init];
    if (self != nil)
    {
        window = initWindow;
#if !TARGET_OS_VISION
        trackingArea = nil;
#endif /* !TARGET_OS_VISION */
        markedText = [[NSMutableAttributedString alloc] init];

        [self updateTrackingAreas];
        // NOTE: kUTTypeURL corresponds to NSPasteboardTypeURL but is available
        //       on 10.7 without having been deprecated yet
#if !TARGET_OS_VISION
        [self registerForDraggedTypes:@[(__bridge NSString*) kUTTypeURL]];
#endif /* !TARGET_OS_VISION */
    }

    return self;
}

- (void)dealloc
{
#if !__has_feature(objc_arc)
  [trackingArea release];
  [markedText release];
  [super dealloc];
#endif
}

- (BOOL)isOpaque
{
    return [window->ns.object isOpaque];
}

- (BOOL)canBecomeKeyView
{
    return YES;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (BOOL)wantsUpdateLayer
{
    return YES;
}

- (void)updateLayer
{
    if (window->context.client != GLFW_NO_API)
        [window->context.nsgl.object update];

    _glfwInputWindowDamage(window);
}

#if !TARGET_OS_VISION
- (void)cursorUpdate:(NSEvent *)event
#else /* TARGET_OS_VISION */
- (void)cursorUpdate:(UIEvent *)event
#endif /* !TARGET_OS_VISION */
{
    updateCursorImage(window);
}

#if !TARGET_OS_VISION
- (BOOL)acceptsFirstMouse:(NSEvent *)event
#else /* TARGET_OS_VISION */
- (BOOL)acceptsFirstMouse:(UIEvent *)event
#endif /* !TARGET_OS_VISION */
{
    return YES;
}

#if !TARGET_OS_VISION
- (void)mouseDown:(NSEvent *)event
#else /* TARGET_OS_VISION */
- (void)mouseDown:(UIEvent *)event
#endif /* !TARGET_OS_VISION */
{
    _glfwInputMouseClick(window,
                         GLFW_MOUSE_BUTTON_LEFT,
                         GLFW_PRESS,
                         translateFlags([event modifierFlags]));
}

#if !TARGET_OS_VISION
- (void)mouseDragged:(NSEvent *)event
#else /* TARGET_OS_VISION */
- (void)mouseDragged:(UIEvent *)event
#endif /* !TARGET_OS_VISION */
{
    [self mouseMoved:event];
}

#if !TARGET_OS_VISION
- (void)mouseUp:(NSEvent *)event
#else /* TARGET_OS_VISION */
- (void)mouseUp:(UIEvent *)event
#endif /* !TARGET_OS_VISION */
{
    _glfwInputMouseClick(window,
                         GLFW_MOUSE_BUTTON_LEFT,
                         GLFW_RELEASE,
                         translateFlags([event modifierFlags]));
}

#if !TARGET_OS_VISION
- (void)mouseMoved:(NSEvent *)event
#else /* TARGET_OS_VISION */
- (void)mouseMoved:(UIEvent *)event
#endif /* !TARGET_OS_VISION */
{
    if (window->cursorMode == GLFW_CURSOR_DISABLED)
    {
#if !TARGET_OS_VISION
      const double dx = [event deltaX] - window->ns.cursorWarpDeltaX;
      const double dy = [event deltaY] - window->ns.cursorWarpDeltaY;
#else /* TARGET_OS_VISION */
      const double dx = 0;
      const double dy = 0;
#endif /* !TARGET_OS_VISION */


        _glfwInputCursorPos(window,
                            window->virtualCursorPosX + dx,
                            window->virtualCursorPosY + dy);
    }
    else
    {
#if !TARGET_OS_VISION
        const NSRect contentRect = [window->ns.view frame];
        // NOTE: The returned location uses base 0,1 not 0,0
        const NSPoint pos = [event locationInWindow];
#else /* TARGET_OS_VISION */
        const CGRect contentRect = [window->ns.view frame];
        // NOTE: The returned location uses base 0,1 not 0,0
        CGPoint pos;
        pos.x = 0;
        pos.y = 0;
#endif /* !TARGET_OS_VISION */

        _glfwInputCursorPos(window, pos.x, contentRect.size.height - pos.y);
    }

    window->ns.cursorWarpDeltaX = 0;
    window->ns.cursorWarpDeltaY = 0;
}

#if !TARGET_OS_VISION
- (void)rightMouseDown:(NSEvent *)event
#else /* TARGET_OS_VISION */
- (void)rightMouseDown:(UIEvent *)event
#endif /* !TARGET_OS_VISION */
{
    _glfwInputMouseClick(window,
                         GLFW_MOUSE_BUTTON_RIGHT,
                         GLFW_PRESS,
                         translateFlags([event modifierFlags]));
}

#if !TARGET_OS_VISION
- (void)rightMouseDragged:(NSEvent *)event
#else /* TARGET_OS_VISION */
- (void)rightMouseDragged:(UIEvent *)event
#endif /* !TARGET_OS_VISION */
{
    [self mouseMoved:event];
}

#if !TARGET_OS_VISION
- (void)rightMouseUp:(NSEvent *)event
#else /* TARGET_OS_VISION */
- (void)rightMouseUp:(UIEvent *)event
#endif /* !TARGET_OS_VISION */
{
    _glfwInputMouseClick(window,
                         GLFW_MOUSE_BUTTON_RIGHT,
                         GLFW_RELEASE,
                         translateFlags([event modifierFlags]));
}

#if !TARGET_OS_VISION
- (void)otherMouseDown:(NSEvent *)event
#else /* TARGET_OS_VISION */
- (void)otherMouseDown:(UIEvent *)event
#endif /* !TARGET_OS_VISION */
{
    _glfwInputMouseClick(window,
#if !TARGET_OS_VISION
                         (int) [event buttonNumber],
#else /* TARGET_OS_VISION */
                         0,
#endif /* !TARGET_OS_VISION */
                         GLFW_PRESS,
                         translateFlags([event modifierFlags]));
}

#if !TARGET_OS_VISION
- (void)otherMouseDragged:(NSEvent *)event
#else /* TARGET_OS_VISION */
- (void)otherMouseDragged:(UIEvent *)event
#endif /* !TARGET_OS_VISION */
{
    [self mouseMoved:event];
}

#if !TARGET_OS_VISION
- (void)otherMouseUp:(NSEvent *)event
#else /* TARGET_OS_VISION */
- (void)otherMouseUp:(UIEvent *)event
#endif /* !TARGET_OS_VISION */
{
    _glfwInputMouseClick(window,
#if !TARGET_OS_VISION
                         (int) [event buttonNumber],
#else /* TARGET_OS_VISION */
                         0,
#endif /* !TARGET_OS_VISION */
                         GLFW_RELEASE,
                         translateFlags([event modifierFlags]));
}

#if !TARGET_OS_VISION
- (void)mouseExited:(NSEvent *)event
#else /* TARGET_OS_VISION */
- (void)mouseExited:(UIEvent *)event
#endif /* !TARGET_OS_VISION */
{
    if (window->cursorMode == GLFW_CURSOR_HIDDEN)
        showCursor(window);

    _glfwInputCursorEnter(window, GLFW_FALSE);
}

#if !TARGET_OS_VISION
- (void)mouseEntered:(NSEvent *)event
#else /* TARGET_OS_VISION */
- (void)mouseEntered:(UIEvent *)event
#endif /* !TARGET_OS_VISION */
{
    if (window->cursorMode == GLFW_CURSOR_HIDDEN)
        hideCursor(window);

    _glfwInputCursorEnter(window, GLFW_TRUE);
}

- (void)viewDidChangeBackingProperties
{
#if !TARGET_OS_VISION
    const NSRect contentRect = [window->ns.view frame];
    const NSRect fbRect = [window->ns.view convertRectToBacking:contentRect];
#else /* TARGET_OS_VISION */
    const CGRect contentRect = [window->ns.view frame];
    const CGRect fbRect = [window->ns.view convertRect:contentRect 
                                                toView:window->ns.view];
#endif /* !TARGET_OS_VISION */

    if (fbRect.size.width != window->ns.fbWidth ||
        fbRect.size.height != window->ns.fbHeight)
    {
        window->ns.fbWidth  = fbRect.size.width;
        window->ns.fbHeight = fbRect.size.height;
        _glfwInputFramebufferSize(window, fbRect.size.width, fbRect.size.height);
    }

    const float xscale = fbRect.size.width / contentRect.size.width;
    const float yscale = fbRect.size.height / contentRect.size.height;

    if (xscale != window->ns.xscale || yscale != window->ns.yscale)
    {
        window->ns.xscale = xscale;
        window->ns.yscale = yscale;
        _glfwInputWindowContentScale(window, xscale, yscale);

#if !TARGET_OS_VISION
        if (window->ns.retina && window->ns.layer)
            [window->ns.layer setContentsScale:[window->ns.object backingScaleFactor]];
#endif /* !TARGET_OS_VISION */
    }
}

#if !TARGET_OS_VISION
- (void)drawRect:(NSRect)rect
#else /* TARGET_OS_VISION */
- (void)drawRect:(CGRect)rect
#endif /* !TARGET_OS_VISION */
{
    _glfwInputWindowDamage(window);
}

- (void)updateTrackingAreas
{
#if !TARGET_OS_VISION
    if (trackingArea != nil)
    {
        [self removeTrackingArea:trackingArea];
    #if !__has_feature(objc_arc)
        [trackingArea release];
    #endif
    }

    const NSTrackingAreaOptions options = NSTrackingMouseEnteredAndExited |
                                          NSTrackingActiveInKeyWindow |
                                          NSTrackingEnabledDuringMouseDrag |
                                          NSTrackingCursorUpdate |
                                          NSTrackingInVisibleRect |
                                          NSTrackingAssumeInside;

    trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds]
                                                options:options
                                                  owner:self
                                               userInfo:nil];

    [self addTrackingArea:trackingArea];
    [super updateTrackingAreas];
#endif /* !TARGET_OS_VISION */
}

#if !TARGET_OS_VISION
- (void)keyDown:(NSEvent *)event
#else /* TARGET_OS_VISION */
- (void)keyDown:(UIEvent *)event
#endif /* !TARGET_OS_VISION */
{
#if !TARGET_OS_VISION
    const int key = translateKey([event keyCode]);
    const int mods = translateFlags([event modifierFlags]);

    _glfwInputKey(window, key, [event keyCode], GLFW_PRESS, mods);

    [self interpretKeyEvents:@[event]];
#endif /* !TARGET_OS_VISION */
}

#if !TARGET_OS_VISION
- (void)flagsChanged:(NSEvent *)event
#else /* TARGET_OS_VISION */
- (void)flagsChanged:(UIEvent *)event
#endif /* !TARGET_OS_VISION */
{
#if !TARGET_OS_VISION
    int action;
    const unsigned int modifierFlags =
        [event modifierFlags] & NSEventModifierFlagDeviceIndependentFlagsMask;
    const int key = translateKey([event keyCode]);
    const int mods = translateFlags(modifierFlags);
    const NSUInteger keyFlag = translateKeyToModifierFlag(key);

    if (keyFlag & modifierFlags)
    {
        if (window->keys[key] == GLFW_PRESS)
            action = GLFW_RELEASE;
        else
            action = GLFW_PRESS;
    }
    else
        action = GLFW_RELEASE;

    _glfwInputKey(window, key, [event keyCode], action, mods);
#endif /* !TARGET_OS_VISION */
}

#if !TARGET_OS_VISION
- (void)keyUp:(NSEvent *)event
#else /* TARGET_OS_VISION */
- (void)keyUp:(UIEvent *)event
#endif /* !TARGET_OS_VISION */
{
#if !TARGET_OS_VISION
    const int key = translateKey([event keyCode]);
    const int mods = translateFlags([event modifierFlags]);
    _glfwInputKey(window, key, [event keyCode], GLFW_RELEASE, mods);
#endif /* !TARGET_OS_VISION */
}

#if !TARGET_OS_VISION
- (void)scrollWheel:(NSEvent *)event
#else /* TARGET_OS_VISION */
- (void)scrollWheel:(UIEvent *)event
#endif /* !TARGET_OS_VISION */
{
#if !TARGET_OS_VISION
    double deltaX = [event scrollingDeltaX];
    double deltaY = [event scrollingDeltaY];

    if ([event hasPreciseScrollingDeltas])
    {
        deltaX *= 0.1;
        deltaY *= 0.1;
    }

    if (fabs(deltaX) > 0.0 || fabs(deltaY) > 0.0)
        _glfwInputScroll(window, deltaX, deltaY);
#endif /* !TARGET_OS_VISION */
}

#if !TARGET_OS_VISION
- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
    // HACK: We don't know what to say here because we don't know what the
    //       application wants to do with the paths
    return NSDragOperationGeneric;
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
    const NSRect contentRect = [window->ns.view frame];
    // NOTE: The returned location uses base 0,1 not 0,0
    const NSPoint pos = [sender draggingLocation];
    _glfwInputCursorPos(window, pos.x, contentRect.size.height - pos.y);

    NSPasteboard* pasteboard = [sender draggingPasteboard];
    NSDictionary* options = @{NSPasteboardURLReadingFileURLsOnlyKey:@YES};
    NSArray* urls = [pasteboard readObjectsForClasses:@[[NSURL class]]
                                              options:options];
    const NSUInteger count = [urls count];
    if (count)
    {
        char** paths = calloc(count, sizeof(char*));

        for (NSUInteger i = 0;  i < count;  i++)
            paths[i] = _glfw_strdup([urls[i] fileSystemRepresentation]);

        _glfwInputDrop(window, (int) count, (const char**) paths);

        for (NSUInteger i = 0;  i < count;  i++)
            free(paths[i]);
        free(paths);
    }

    return YES;
}
#endif /* !TARGET_OS_VISION */

- (BOOL)hasMarkedText
{
    return [markedText length] > 0;
}

- (NSRange)markedRange
{
    if ([markedText length] > 0)
        return NSMakeRange(0, [markedText length] - 1);
    else
        return kEmptyRange;
}

- (NSRange)selectedRange
{
    return kEmptyRange;
}

- (void)setMarkedText:(id)string
        selectedRange:(NSRange)selectedRange
     replacementRange:(NSRange)replacementRange
{
#if !__has_feature(objc_arc)
    [markedText release];
#endif
    if ([string isKindOfClass:[NSAttributedString class]])
        markedText = [[NSMutableAttributedString alloc] initWithAttributedString:string];
    else
        markedText = [[NSMutableAttributedString alloc] initWithString:string];
}

- (void)unmarkText
{
    [[markedText mutableString] setString:@""];
}

- (NSArray*)validAttributesForMarkedText
{
    return [NSArray array];
}

- (NSAttributedString*)attributedSubstringForProposedRange:(NSRange)range
                                               actualRange:(NSRangePointer)actualRange
{
    return nil;
}

#if !TARGET_OS_VISION
- (NSUInteger)characterIndexForPoint:(NSPoint)point
#else /* TARGET_OS_VISION */
- (NSUInteger)characterIndexForPoint:(CGPoint)point
#endif /* !TARGET_OS_VISION */
{
    return 0;
}

#if !TARGET_OS_VISION
- (NSRect)firstRectForCharacterRange:(NSRange)range
                         actualRange:(NSRangePointer)actualRange
#else /* TARGET_OS_VISION */
- (CGRect)firstRectForCharacterRange:(NSRange)range
                         actualRange:(NSRangePointer)actualRange
#endif /* !TARGET_OS_VISION */
{
#if !TARGET_OS_VISION
    const NSRect frame = [window->ns.view frame];
    return NSMakeRect(frame.origin.x, frame.origin.y, 0.0, 0.0);
#else /* TARGET_OS_VISION */
  const CGRect frame = [window->ns.view frame];
  return CGRectMake(frame.origin.x, frame.origin.y, 0.0, 0.0);
#endif /* !TARGET_OS_VISION */
}

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange
{
#if !TARGET_OS_VISION
    NSString* characters;
    NSEvent* event = [NSApp currentEvent];
    const int mods = translateFlags([event modifierFlags]);
    const int plain = !(mods & GLFW_MOD_SUPER);

    if ([string isKindOfClass:[NSAttributedString class]])
        characters = [string string];
    else
        characters = (NSString*) string;

    const NSUInteger length = [characters length];
    for (NSUInteger i = 0;  i < length;  i++)
    {
        const unichar codepoint = [characters characterAtIndex:i];
        if ((codepoint & 0xff00) == 0xf700)
            continue;

        _glfwInputChar(window, codepoint, mods, plain);
    }
#endif /* !TARGET_OS_VISION */
}

- (void)doCommandBySelector:(SEL)selector
{
}

@end


//------------------------------------------------------------------------
// GLFW window class
//------------------------------------------------------------------------

#if !TARGET_OS_VISION
@interface GLFWWindow : NSWindow {}
#else /* TARGET_OS_VISION */
@interface GLFWWindow : UIWindow {}
#endif /* !TARGET_OS_VISION */
@end

@implementation GLFWWindow

- (BOOL)canBecomeKeyWindow
{
    // Required for NSWindowStyleMaskBorderless windows
    return YES;
}

- (BOOL)canBecomeMainWindow
{
    return YES;
}

@end


// Create the Cocoa window
//
static GLFWbool createNativeWindow(_GLFWwindow* window,
                                   const _GLFWwndconfig* wndconfig,
                                   const _GLFWfbconfig* fbconfig)
{
    window->ns.delegate = [[GLFWWindowDelegate alloc] initWithGlfwWindow:window];
    if (window->ns.delegate == nil)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Cocoa: Failed to create window delegate");
        return GLFW_FALSE;
    }

#if !TARGET_OS_VISION
    NSRect contentRect;
#else /* TARGET_OS_VISION */
    CGRect contentRect;
#endif /* !TARGET_OS_VISION */

    if (window->monitor)
    {
        GLFWvidmode mode;
        int xpos, ypos;

        _glfwPlatformGetVideoMode(window->monitor, &mode);
        _glfwPlatformGetMonitorPos(window->monitor, &xpos, &ypos);
      
#if !TARGET_OS_VISION
        contentRect = NSMakeRect(xpos, ypos, mode.width, mode.height);
#else /* TARGET_OS_VISION */
        contentRect = CGRectMake(xpos, ypos, mode.width, mode.height);
#endif /* !TARGET_OS_VISION */
    }
    else
#if !TARGET_OS_VISION
        contentRect = NSMakeRect(0, 0, wndconfig->width, wndconfig->height);
  
        window->ns.object = [[GLFWWindow alloc]
                             initWithContentRect:contentRect
                             styleMask:getStyleMask(window)
                             backing:NSBackingStoreBuffered
                             defer:NO];
#else /* TARGET_OS_VISION */
        contentRect = CGRectMake(0, 0, wndconfig->width, wndconfig->height);
  
        window->ns.object = [[GLFWWindow alloc] initWithFrame:contentRect];
#endif /* !TARGET_OS_VISION */

    if (window->ns.object == nil)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "Cocoa: Failed to create window");
        return GLFW_FALSE;
    }

#if !TARGET_OS_VISION
    if (window->monitor)
        [window->ns.object setLevel:NSMainMenuWindowLevel + 1];
    else
    {
        [(NSWindow*) window->ns.object center];
        _glfw.ns.cascadePoint =
            NSPointToCGPoint([window->ns.object cascadeTopLeftFromPoint:
                              NSPointFromCGPoint(_glfw.ns.cascadePoint)]);

        if (wndconfig->resizable)
        {
            const NSWindowCollectionBehavior behavior =
                NSWindowCollectionBehaviorFullScreenPrimary |
                NSWindowCollectionBehaviorManaged;
            [window->ns.object setCollectionBehavior:behavior];
        }

        if (wndconfig->floating)
            [window->ns.object setLevel:NSFloatingWindowLevel];

        if (wndconfig->maximized)
            [window->ns.object zoom:nil];
    }

    if (strlen(wndconfig->ns.frameName))
        [window->ns.object setFrameAutosaveName:@(wndconfig->ns.frameName)];
#endif /* !TARGET_OS_VISION */
    window->ns.view = [[GLFWContentView alloc] initWithGlfwWindow:window];
    window->ns.retina = wndconfig->ns.retina;

    if (fbconfig->transparent)
    {
        [window->ns.object setOpaque:NO];
#if !TARGET_OS_VISION
        [window->ns.object setHasShadow:NO];
        [window->ns.object setBackgroundColor:[NSColor clearColor]];
#endif /* !TARGET_OS_VISION */
    }

#if !TARGET_OS_VISION
    [window->ns.object setContentView:window->ns.view];
    [window->ns.object makeFirstResponder:window->ns.view];
#endif /* !TARGET_OS_VISION */
    [window->ns.object setTitle:@(wndconfig->title)];
    [window->ns.object setDelegate:window->ns.delegate];
#if !TARGET_OS_VISION
    [window->ns.object setAcceptsMouseMovedEvents:YES];
    [window->ns.object setRestorable:NO];
#endif /* !TARGET_OS_VISION */

#if !TARGET_OS_VISION && MAC_OS_X_VERSION_MAX_ALLOWED >= 101200
    if ([window->ns.object respondsToSelector:@selector(setTabbingMode:)])
        [window->ns.object setTabbingMode:NSWindowTabbingModeDisallowed];
#endif /* !TARGET_OS_VISION && MAC_OS_X_VERSION_MAX_ALLOWED >= 101200 */

    _glfwPlatformGetWindowSize(window, &window->ns.width, &window->ns.height);
    _glfwPlatformGetFramebufferSize(window, &window->ns.fbWidth, &window->ns.fbHeight);

    return GLFW_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Transforms a y-coordinate between the CG display and NS screen spaces
//
float _glfwTransformYNS(float y)
{
#if !TARGET_OS_VISION
    return CGDisplayBounds(CGMainDisplayID()).size.height - y - 1;
#else /* TARGET_OS_VISION */
    return 0;
#endif /* !TARGET_OS_VISION */
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformCreateWindow(_GLFWwindow* window,
                              const _GLFWwndconfig* wndconfig,
                              const _GLFWctxconfig* ctxconfig,
                              const _GLFWfbconfig* fbconfig)
{
    @autoreleasepool {

#if !TARGET_OS_VISION
    if (!_glfw.ns.finishedLaunching)
        [NSApp run];
#endif /* !TARGET_OS_VISION */

    if (!createNativeWindow(window, wndconfig, fbconfig))
        return GLFW_FALSE;

    if (ctxconfig->client != GLFW_NO_API)
    {
        if (ctxconfig->source == GLFW_NATIVE_CONTEXT_API)
        {
            if (!_glfwInitNSGL())
                return GLFW_FALSE;
            if (!_glfwCreateContextNSGL(window, ctxconfig, fbconfig))
                return GLFW_FALSE;
        }
        else if (ctxconfig->source == GLFW_EGL_CONTEXT_API)
        {
            if (!_glfwInitEGL())
                return GLFW_FALSE;
            if (!_glfwCreateContextEGL(window, ctxconfig, fbconfig))
                return GLFW_FALSE;
        }
        else if (ctxconfig->source == GLFW_OSMESA_CONTEXT_API)
        {
            if (!_glfwInitOSMesa())
                return GLFW_FALSE;
            if (!_glfwCreateContextOSMesa(window, ctxconfig, fbconfig))
                return GLFW_FALSE;
        }
    }

    if (window->monitor)
    {
        _glfwPlatformShowWindow(window);
        _glfwPlatformFocusWindow(window);
        acquireMonitor(window);
    }

    return GLFW_TRUE;

    } // autoreleasepool
}

void _glfwPlatformDestroyWindow(_GLFWwindow* window)
{
    @autoreleasepool {

    if (_glfw.ns.disabledCursorWindow == window)
        _glfw.ns.disabledCursorWindow = NULL;

#if !TARGET_OS_VISION
    [window->ns.object orderOut:nil];
#endif /* !TARGET_OS_VISION */

    if (window->monitor)
        releaseMonitor(window);

    if (window->context.destroy)
        window->context.destroy(window);

    [window->ns.object setDelegate:nil];
#if !__has_feature(objc_arc)
    [window->ns.delegate release];
#endif
    window->ns.delegate = nil;

#if !__has_feature(objc_arc)
    [window->ns.view release];
#endif
    window->ns.view = nil;

    [window->ns.object close];
    window->ns.object = nil;

    // HACK: Allow Cocoa to catch up before returning
    _glfwPlatformPollEvents();

    } // autoreleasepool
}

void _glfwPlatformSetWindowTitle(_GLFWwindow* window, const char* title)
{
    @autoreleasepool {
    NSString* string = @(title);
    [window->ns.object setTitle:string];
    // HACK: Set the miniwindow title explicitly as setTitle: doesn't update it
    //       if the window lacks NSWindowStyleMaskTitled
#if !TARGET_OS_VISION
    [window->ns.object setMiniwindowTitle:string];
#endif /* !TARGET_OS_VISION */
    } // autoreleasepool
}

void _glfwPlatformSetWindowIcon(_GLFWwindow* window,
                                int count, const GLFWimage* images)
{
    // Regular windows do not have icons
}

void _glfwPlatformGetWindowPos(_GLFWwindow* window, int* xpos, int* ypos)
{
    @autoreleasepool {

#if !TARGET_OS_VISION
      const NSRect contentRect = [window->ns.object contentRectForFrameRect:[window->ns.object frame]];
#else /* TARGET_OS_VISION */
      CGPoint origin;
      origin.x = 0;
      origin.y = 0;
      
      CGRect contentRect;
      contentRect.origin = origin;
#endif /* !TARGET_OS_VISION */

    if (xpos)
        *xpos = contentRect.origin.x;
    if (ypos)
        *ypos = _glfwTransformYNS(contentRect.origin.y + contentRect.size.height - 1);

    } // autoreleasepool
}

void _glfwPlatformSetWindowPos(_GLFWwindow* window, int x, int y)
{
#if !TARGET_OS_VISION
    @autoreleasepool {

    const NSRect contentRect = [window->ns.view frame];
    const NSRect dummyRect = NSMakeRect(x, _glfwTransformYNS(y + contentRect.size.height - 1), 0, 0);
    const NSRect frameRect = [window->ns.object frameRectForContentRect:dummyRect];
    [window->ns.object setFrameOrigin:frameRect.origin];

    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformGetWindowSize(_GLFWwindow* window, int* width, int* height)
{
#if !TARGET_OS_VISION
    @autoreleasepool {

    const NSRect contentRect = [window->ns.view frame];

    if (width)
        *width = contentRect.size.width;
    if (height)
        *height = contentRect.size.height;

    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformSetWindowSize(_GLFWwindow* window, int width, int height)
{
    @autoreleasepool {

    if (window->monitor)
    {
        if (window->monitor->window == window)
            acquireMonitor(window);
    }
    else
    {
#if !TARGET_OS_VISION
        NSRect contentRect =
            [window->ns.object contentRectForFrameRect:[window->ns.object frame]];
        contentRect.origin.y += contentRect.size.height - height;
        contentRect.size = NSMakeSize(width, height);
        [window->ns.object setFrame:[window->ns.object frameRectForContentRect:contentRect]
                            display:YES];
#endif /* !TARGET_OS_VISION */
    }

    } // autoreleasepool
}

void _glfwPlatformSetWindowSizeLimits(_GLFWwindow* window,
                                      int minwidth, int minheight,
                                      int maxwidth, int maxheight)
{
#if !TARGET_OS_VISION
    @autoreleasepool {

    if (minwidth == GLFW_DONT_CARE || minheight == GLFW_DONT_CARE)
        [window->ns.object setContentMinSize:NSMakeSize(0, 0)];
    else
        [window->ns.object setContentMinSize:NSMakeSize(minwidth, minheight)];

    if (maxwidth == GLFW_DONT_CARE || maxheight == GLFW_DONT_CARE)
        [window->ns.object setContentMaxSize:NSMakeSize(DBL_MAX, DBL_MAX)];
    else
        [window->ns.object setContentMaxSize:NSMakeSize(maxwidth, maxheight)];

    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformSetWindowAspectRatio(_GLFWwindow* window, int numer, int denom)
{
#if !TARGET_OS_VISION
    @autoreleasepool {
    if (numer == GLFW_DONT_CARE || denom == GLFW_DONT_CARE)
        [window->ns.object setResizeIncrements:NSMakeSize(1.0, 1.0)];
    else
        [window->ns.object setContentAspectRatio:NSMakeSize(numer, denom)];
    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformGetFramebufferSize(_GLFWwindow* window, int* width, int* height)
{
#if !TARGET_OS_VISION
    @autoreleasepool {

    const NSRect contentRect = [window->ns.view frame];
    const NSRect fbRect = [window->ns.view convertRectToBacking:contentRect];

    if (width)
        *width = (int) fbRect.size.width;
    if (height)
        *height = (int) fbRect.size.height;

    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformGetWindowFrameSize(_GLFWwindow* window,
                                     int* left, int* top,
                                     int* right, int* bottom)
{
#if !TARGET_OS_VISION
    @autoreleasepool {

    const NSRect contentRect = [window->ns.view frame];
    const NSRect frameRect = [window->ns.object frameRectForContentRect:contentRect];

    if (left)
        *left = contentRect.origin.x - frameRect.origin.x;
    if (top)
        *top = frameRect.origin.y + frameRect.size.height -
               contentRect.origin.y - contentRect.size.height;
    if (right)
        *right = frameRect.origin.x + frameRect.size.width -
                 contentRect.origin.x - contentRect.size.width;
    if (bottom)
        *bottom = contentRect.origin.y - frameRect.origin.y;

    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformGetWindowContentScale(_GLFWwindow* window,
                                        float* xscale, float* yscale)
{
#if !TARGET_OS_VISION
    @autoreleasepool {

    const NSRect points = [window->ns.view frame];
    const NSRect pixels = [window->ns.view convertRectToBacking:points];

    if (xscale)
        *xscale = (float) (pixels.size.width / points.size.width);
    if (yscale)
        *yscale = (float) (pixels.size.height / points.size.height);

    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformIconifyWindow(_GLFWwindow* window)
{
#if !TARGET_OS_VISION
    @autoreleasepool {
    [window->ns.object miniaturize:nil];
    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformRestoreWindow(_GLFWwindow* window)
{
#if !TARGET_OS_VISION
    @autoreleasepool {
    if ([window->ns.object isMiniaturized])
        [window->ns.object deminiaturize:nil];
    else if ([window->ns.object isZoomed])
        [window->ns.object zoom:nil];
    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformMaximizeWindow(_GLFWwindow* window)
{
#if !TARGET_OS_VISION
    @autoreleasepool {
    if (![window->ns.object isZoomed])
        [window->ns.object zoom:nil];
    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformShowWindow(_GLFWwindow* window)
{
#if !TARGET_OS_VISION
    @autoreleasepool {
    [window->ns.object orderFront:nil];
    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformHideWindow(_GLFWwindow* window)
{
#if !TARGET_OS_VISION
    @autoreleasepool {
    [window->ns.object orderOut:nil];
    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformRequestWindowAttention(_GLFWwindow* window)
{
#if !TARGET_OS_VISION
    @autoreleasepool {
    [NSApp requestUserAttention:NSInformationalRequest];
    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformFocusWindow(_GLFWwindow* window)
{
#if !TARGET_OS_VISION
    @autoreleasepool {
    // Make us the active application
    // HACK: This is here to prevent applications using only hidden windows from
    //       being activated, but should probably not be done every time any
    //       window is shown
    [NSApp activateIgnoringOtherApps:YES];
    [window->ns.object makeKeyAndOrderFront:nil];
    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformSetWindowMonitor(_GLFWwindow* window,
                                   _GLFWmonitor* monitor,
                                   int xpos, int ypos,
                                   int width, int height,
                                   int refreshRate)
{
    @autoreleasepool {

    if (window->monitor == monitor)
    {
        if (monitor)
        {
            if (monitor->window == window)
                acquireMonitor(window);
        }
        else
        {
#if !TARGET_OS_VISION
            const NSRect contentRect =
                NSMakeRect(xpos, _glfwTransformYNS(ypos + height - 1), width, height);
            const NSRect frameRect =
                [window->ns.object frameRectForContentRect:contentRect
                                                 styleMask:getStyleMask(window)];

            [window->ns.object setFrame:frameRect display:YES];
#endif /* !TARGET_OS_VISION */
        }

        return;
    }

    if (window->monitor)
        releaseMonitor(window);

    _glfwInputWindowMonitor(window, monitor);

    // HACK: Allow the state cached in Cocoa to catch up to reality
    // TODO: Solve this in a less terrible way
    _glfwPlatformPollEvents();

#if !TARGET_OS_VISION
    const NSUInteger styleMask = getStyleMask(window);
    [window->ns.object setStyleMask:styleMask];
    // HACK: Changing the style mask can cause the first responder to be cleared
    [window->ns.object makeFirstResponder:window->ns.view];

    if (window->monitor)
    {
        [window->ns.object setLevel:NSMainMenuWindowLevel + 1];
        [window->ns.object setHasShadow:NO];

        acquireMonitor(window);
    }
    else
    {
        NSRect contentRect = NSMakeRect(xpos, _glfwTransformYNS(ypos + height - 1),
                                        width, height);
        NSRect frameRect = [window->ns.object frameRectForContentRect:contentRect
                                                            styleMask:styleMask];
        [window->ns.object setFrame:frameRect display:YES];

        if (window->numer != GLFW_DONT_CARE &&
            window->denom != GLFW_DONT_CARE)
        {
            [window->ns.object setContentAspectRatio:NSMakeSize(window->numer,
                                                                window->denom)];
        }

        if (window->minwidth != GLFW_DONT_CARE &&
            window->minheight != GLFW_DONT_CARE)
        {
            [window->ns.object setContentMinSize:NSMakeSize(window->minwidth,
                                                            window->minheight)];
        }

        if (window->maxwidth != GLFW_DONT_CARE &&
            window->maxheight != GLFW_DONT_CARE)
        {
            [window->ns.object setContentMaxSize:NSMakeSize(window->maxwidth,
                                                            window->maxheight)];
        }

        if (window->floating)
            [window->ns.object setLevel:NSFloatingWindowLevel];
        else
            [window->ns.object setLevel:NSNormalWindowLevel];

        [window->ns.object setHasShadow:YES];
        // HACK: Clearing NSWindowStyleMaskTitled resets and disables the window
        //       title property but the miniwindow title property is unaffected
        [window->ns.object setTitle:[window->ns.object miniwindowTitle]];
    }
#endif /* !TARGET_OS_VISION */

    } // autoreleasepool
}

int _glfwPlatformWindowFocused(_GLFWwindow* window)
{
    @autoreleasepool {
    return [window->ns.object isKeyWindow];
    } // autoreleasepool
}

int _glfwPlatformWindowIconified(_GLFWwindow* window)
{
#if !TARGET_OS_VISION
    @autoreleasepool {
    return [window->ns.object isMiniaturized];
    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

int _glfwPlatformWindowVisible(_GLFWwindow* window)
{
    @autoreleasepool {
    return [window->ns.object isVisible];
    } // autoreleasepool
}

int _glfwPlatformWindowMaximized(_GLFWwindow* window)
{
#if !TARGET_OS_VISION
    @autoreleasepool {
    return [window->ns.object isZoomed];
    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

int _glfwPlatformWindowHovered(_GLFWwindow* window)
{
#if !TARGET_OS_VISION
    @autoreleasepool {

    const NSPoint point = [NSEvent mouseLocation];

    if ([NSWindow windowNumberAtPoint:point belowWindowWithWindowNumber:0] !=
        [window->ns.object windowNumber])
    {
        return GLFW_FALSE;
    }

    return NSMouseInRect(point,
        [window->ns.object convertRectToScreen:[window->ns.view frame]], NO);

    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

int _glfwPlatformFramebufferTransparent(_GLFWwindow* window)
{
    @autoreleasepool {
    return ![window->ns.object isOpaque] && ![window->ns.view isOpaque];
    } // autoreleasepool
}

void _glfwPlatformSetWindowResizable(_GLFWwindow* window, GLFWbool enabled)
{
#if !TARGET_OS_VISION
    @autoreleasepool {
    [window->ns.object setStyleMask:getStyleMask(window)];
    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformSetWindowDecorated(_GLFWwindow* window, GLFWbool enabled)
{
#if !TARGET_OS_VISION
    @autoreleasepool {
    [window->ns.object setStyleMask:getStyleMask(window)];
    [window->ns.object makeFirstResponder:window->ns.view];
    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformSetWindowFloating(_GLFWwindow* window, GLFWbool enabled)
{
#if !TARGET_OS_VISION
    @autoreleasepool {
    if (enabled)
        [window->ns.object setLevel:NSFloatingWindowLevel];
    else
        [window->ns.object setLevel:NSNormalWindowLevel];
    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

float _glfwPlatformGetWindowOpacity(_GLFWwindow* window)
{
#if !TARGET_OS_VISION
    @autoreleasepool {
    return (float) [window->ns.object alphaValue];
    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformSetWindowOpacity(_GLFWwindow* window, float opacity)
{
#if !TARGET_OS_VISION
    @autoreleasepool {
    [window->ns.object setAlphaValue:opacity];
    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformSetRawMouseMotion(_GLFWwindow *window, GLFWbool enabled)
{
}

GLFWbool _glfwPlatformRawMouseMotionSupported(void)
{
    return GLFW_FALSE;
}

void _glfwPlatformPollEvents(void)
{
#if !TARGET_OS_VISION
    @autoreleasepool {

    if (!_glfw.ns.finishedLaunching)
        [NSApp run];

    for (;;)
    {
        NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                            untilDate:[NSDate distantPast]
                                               inMode:NSDefaultRunLoopMode
                                              dequeue:YES];
        if (event == nil)
            break;

        [NSApp sendEvent:event];
    }

    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformWaitEvents(void)
{
#if !TARGET_OS_VISION
    @autoreleasepool {

    if (!_glfw.ns.finishedLaunching)
        [NSApp run];

    // I wanted to pass NO to dequeue:, and rely on PollEvents to
    // dequeue and send.  For reasons not at all clear to me, passing
    // NO to dequeue: causes this method never to return.
    NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                        untilDate:[NSDate distantFuture]
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:YES];
    [NSApp sendEvent:event];

    _glfwPlatformPollEvents();

    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformWaitEventsTimeout(double timeout)
{
#if !TARGET_OS_VISION
    @autoreleasepool {

    if (!_glfw.ns.finishedLaunching)
        [NSApp run];

    NSDate* date = [NSDate dateWithTimeIntervalSinceNow:timeout];
    NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                        untilDate:date
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:YES];
    if (event)
        [NSApp sendEvent:event];

    _glfwPlatformPollEvents();

    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformPostEmptyEvent(void)
{
#if !TARGET_OS_VISION
    @autoreleasepool {

    if (!_glfw.ns.finishedLaunching)
        [NSApp run];

    NSEvent* event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined
                                        location:NSMakePoint(0, 0)
                                   modifierFlags:0
                                       timestamp:0
                                    windowNumber:0
                                         context:nil
                                         subtype:0
                                           data1:0
                                           data2:0];
    [NSApp postEvent:event atStart:YES];

    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformGetCursorPos(_GLFWwindow* window, double* xpos, double* ypos)
{
#if !TARGET_OS_VISION
    @autoreleasepool {

    const NSRect contentRect = [window->ns.view frame];
    // NOTE: The returned location uses base 0,1 not 0,0
    const NSPoint pos = [window->ns.object mouseLocationOutsideOfEventStream];

    if (xpos)
        *xpos = pos.x;
    if (ypos)
        *ypos = contentRect.size.height - pos.y;

    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformSetCursorPos(_GLFWwindow* window, double x, double y)
{
#if !TARGET_OS_VISION
    @autoreleasepool {

    updateCursorImage(window);

    const NSRect contentRect = [window->ns.view frame];
    // NOTE: The returned location uses base 0,1 not 0,0
    const NSPoint pos = [window->ns.object mouseLocationOutsideOfEventStream];

    window->ns.cursorWarpDeltaX += x - pos.x;
    window->ns.cursorWarpDeltaY += y - contentRect.size.height + pos.y;

    if (window->monitor)
    {
        CGDisplayMoveCursorToPoint(window->monitor->ns.displayID,
                                   CGPointMake(x, y));
    }
    else
    {
        const NSRect localRect = NSMakeRect(x, contentRect.size.height - y - 1, 0, 0);
        const NSRect globalRect = [window->ns.object convertRectToScreen:localRect];
        const NSPoint globalPoint = globalRect.origin;

        CGWarpMouseCursorPosition(CGPointMake(globalPoint.x,
                                              _glfwTransformYNS(globalPoint.y)));
    }

    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformSetCursorMode(_GLFWwindow* window, int mode)
{
#if !TARGET_OS_VISION
    @autoreleasepool {
    if (_glfwPlatformWindowFocused(window))
        updateCursorMode(window);
    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

const char* _glfwPlatformGetScancodeName(int scancode)
{
#if !TARGET_OS_VISION
    @autoreleasepool {

    if (scancode < 0 || scancode > 0xff ||
        _glfw.ns.keycodes[scancode] == GLFW_KEY_UNKNOWN)
    {
        _glfwInputError(GLFW_INVALID_VALUE, "Invalid scancode");
        return NULL;
    }

    const int key = _glfw.ns.keycodes[scancode];

    UInt32 deadKeyState = 0;
    UniChar characters[4];
    UniCharCount characterCount = 0;

    if (UCKeyTranslate([(NSData*) _glfw.ns.unicodeData bytes],
                       scancode,
                       kUCKeyActionDisplay,
                       0,
                       LMGetKbdType(),
                       kUCKeyTranslateNoDeadKeysBit,
                       &deadKeyState,
                       sizeof(characters) / sizeof(characters[0]),
                       &characterCount,
                       characters) != noErr)
    {
        return NULL;
    }

    if (!characterCount)
        return NULL;

    CFStringRef string = CFStringCreateWithCharactersNoCopy(kCFAllocatorDefault,
                                                            characters,
                                                            characterCount,
                                                            kCFAllocatorNull);
    CFStringGetCString(string,
                       _glfw.ns.keynames[key],
                       sizeof(_glfw.ns.keynames[key]),
                       kCFStringEncodingUTF8);
    CFRelease(string);

    return _glfw.ns.keynames[key];

    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

int _glfwPlatformGetKeyScancode(int key)
{
    return _glfw.ns.scancodes[key];
}

int _glfwPlatformCreateCursor(_GLFWcursor* cursor,
                              const GLFWimage* image,
                              int xhot, int yhot)
{
#if !TARGET_OS_VISION
    @autoreleasepool {

    NSImage* native;
    NSBitmapImageRep* rep;

    rep = [[NSBitmapImageRep alloc]
        initWithBitmapDataPlanes:NULL
                      pixelsWide:image->width
                      pixelsHigh:image->height
                   bitsPerSample:8
                 samplesPerPixel:4
                        hasAlpha:YES
                        isPlanar:NO
                  colorSpaceName:NSCalibratedRGBColorSpace
                    bitmapFormat:NSBitmapFormatAlphaNonpremultiplied
                     bytesPerRow:image->width * 4
                    bitsPerPixel:32];

    if (rep == nil)
        return GLFW_FALSE;

    memcpy([rep bitmapData], image->pixels, image->width * image->height * 4);

    native = [[NSImage alloc] initWithSize:NSMakeSize(image->width, image->height)];
    [native addRepresentation:rep];

    cursor->ns.object = [[NSCursor alloc] initWithImage:native
                                                hotSpot:NSMakePoint(xhot, yhot)];

#if !__has_feature(objc_arc)
    [native release];
    [rep release];
#endif

    if (cursor->ns.object == nil)
        return GLFW_FALSE;

    return GLFW_TRUE;

    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

int _glfwPlatformCreateStandardCursor(_GLFWcursor* cursor, int shape)
{
#if !TARGET_OS_VISION
    @autoreleasepool {

    SEL cursorSelector = NULL;

    // HACK: Try to use a private message
    if (shape == GLFW_RESIZE_EW_CURSOR)
        cursorSelector = NSSelectorFromString(@"_windowResizeEastWestCursor");
    else if (shape == GLFW_RESIZE_NS_CURSOR)
        cursorSelector = NSSelectorFromString(@"_windowResizeNorthSouthCursor");
    else if (shape == GLFW_RESIZE_NWSE_CURSOR)
        cursorSelector = NSSelectorFromString(@"_windowResizeNorthWestSouthEastCursor");
    else if (shape == GLFW_RESIZE_NESW_CURSOR)
        cursorSelector = NSSelectorFromString(@"_windowResizeNorthEastSouthWestCursor");

    if (cursorSelector && [NSCursor respondsToSelector:cursorSelector])
    {
        id object = [NSCursor performSelector:cursorSelector];
        if ([object isKindOfClass:[NSCursor class]])
            cursor->ns.object = object;
    }

    if (!cursor->ns.object)
    {
        if (shape == GLFW_ARROW_CURSOR)
            cursor->ns.object = [NSCursor arrowCursor];
        else if (shape == GLFW_IBEAM_CURSOR)
            cursor->ns.object = [NSCursor IBeamCursor];
        else if (shape == GLFW_CROSSHAIR_CURSOR)
            cursor->ns.object = [NSCursor crosshairCursor];
        else if (shape == GLFW_POINTING_HAND_CURSOR)
            cursor->ns.object = [NSCursor pointingHandCursor];
        else if (shape == GLFW_RESIZE_EW_CURSOR)
            cursor->ns.object = [NSCursor resizeLeftRightCursor];
        else if (shape == GLFW_RESIZE_NS_CURSOR)
            cursor->ns.object = [NSCursor resizeUpDownCursor];
        else if (shape == GLFW_RESIZE_ALL_CURSOR)
            cursor->ns.object = [NSCursor closedHandCursor];
        else if (shape == GLFW_NOT_ALLOWED_CURSOR)
            cursor->ns.object = [NSCursor operationNotAllowedCursor];
    }

    if (!cursor->ns.object)
    {
        _glfwInputError(GLFW_CURSOR_UNAVAILABLE,
                        "Cocoa: Standard cursor shape unavailable");
        return GLFW_FALSE;
    }
#if !__has_feature(objc_arc)
    [cursor->ns.object retain];
#endif
    return GLFW_TRUE;

    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformDestroyCursor(_GLFWcursor* cursor)
{
    @autoreleasepool {
    if (cursor->ns.object)
    {
  #if !__has_feature(objc_arc)
      [(NSCursor*) cursor->ns.object release];
  #endif
    }
    } // autoreleasepool
}

void _glfwPlatformSetCursor(_GLFWwindow* window, _GLFWcursor* cursor)
{
    @autoreleasepool {
    if (cursorInContentArea(window))
        updateCursorImage(window);
    } // autoreleasepool
}

void _glfwPlatformSetClipboardString(const char* string)
{
#if !TARGET_OS_VISION
    @autoreleasepool {
    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
    [pasteboard declareTypes:@[NSPasteboardTypeString] owner:nil];
    [pasteboard setString:@(string) forType:NSPasteboardTypeString];
    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

const char* _glfwPlatformGetClipboardString(void)
{
#if !TARGET_OS_VISION
    @autoreleasepool {

    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];

    if (![[pasteboard types] containsObject:NSPasteboardTypeString])
    {
        _glfwInputError(GLFW_FORMAT_UNAVAILABLE,
                        "Cocoa: Failed to retrieve string from pasteboard");
        return NULL;
    }

    NSString* object = [pasteboard stringForType:NSPasteboardTypeString];
    if (!object)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Cocoa: Failed to retrieve object from pasteboard");
        return NULL;
    }

    free(_glfw.ns.clipboardString);
    _glfw.ns.clipboardString = _glfw_strdup([object UTF8String]);

    return _glfw.ns.clipboardString;

    } // autoreleasepool
#endif /* !TARGET_OS_VISION */
}

void _glfwPlatformGetRequiredInstanceExtensions(char** extensions)
{
    if (_glfw.vk.KHR_surface && _glfw.vk.EXT_metal_surface)
    {
        extensions[0] = "VK_KHR_surface";
        extensions[1] = "VK_EXT_metal_surface";
    }
    else if (_glfw.vk.KHR_surface && _glfw.vk.MVK_macos_surface)
    {
        extensions[0] = "VK_KHR_surface";
        extensions[1] = "VK_MVK_macos_surface";
    }
}

int _glfwPlatformGetPhysicalDevicePresentationSupport(VkInstance instance,
                                                      VkPhysicalDevice device,
                                                      uint32_t queuefamily)
{
    return GLFW_TRUE;
}

VkResult _glfwPlatformCreateWindowSurface(VkInstance instance,
                                          _GLFWwindow* window,
                                          const VkAllocationCallbacks* allocator,
                                          VkSurfaceKHR* surface)
{
    @autoreleasepool {

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 101100
    // HACK: Dynamically load Core Animation to avoid adding an extra
    //       dependency for the majority who don't use MoltenVK
    NSBundle* bundle = [NSBundle bundleWithPath:@"/System/Library/Frameworks/QuartzCore.framework"];
    if (!bundle)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Cocoa: Failed to find QuartzCore.framework");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    // NOTE: Create the layer here as makeBackingLayer should not return nil
    window->ns.layer = [[bundle classNamed:@"CAMetalLayer"] layer];
    if (!window->ns.layer)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Cocoa: Failed to create layer for view");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

#if !TARGET_OS_VISION
    if (window->ns.retina)
        [window->ns.layer setContentsScale:[window->ns.object backingScaleFactor]];
#endif /* !TARGET_OS_VISION */

    [window->ns.view setLayer:window->ns.layer];
#if !TARGET_OS_VISION
    [window->ns.view setWantsLayer:YES];
#endif /* !TARGET_OS_VISION */

    VkResult err;

    if (_glfw.vk.EXT_metal_surface)
    {
        VkMetalSurfaceCreateInfoEXT sci;

        PFN_vkCreateMetalSurfaceEXT vkCreateMetalSurfaceEXT;
        vkCreateMetalSurfaceEXT = (PFN_vkCreateMetalSurfaceEXT)
            vkGetInstanceProcAddr(instance, "vkCreateMetalSurfaceEXT");
        if (!vkCreateMetalSurfaceEXT)
        {
            _glfwInputError(GLFW_API_UNAVAILABLE,
                            "Cocoa: Vulkan instance missing VK_EXT_metal_surface extension");
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        memset(&sci, 0, sizeof(sci));
        sci.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
        sci.pLayer = (__bridge const void *)(window->ns.layer);

        err = vkCreateMetalSurfaceEXT(instance, &sci, allocator, surface);
    }
    else
    {
        VkMacOSSurfaceCreateInfoMVK sci;

        PFN_vkCreateMacOSSurfaceMVK vkCreateMacOSSurfaceMVK;
        vkCreateMacOSSurfaceMVK = (PFN_vkCreateMacOSSurfaceMVK)
            vkGetInstanceProcAddr(instance, "vkCreateMacOSSurfaceMVK");
        if (!vkCreateMacOSSurfaceMVK)
        {
            _glfwInputError(GLFW_API_UNAVAILABLE,
                            "Cocoa: Vulkan instance missing VK_MVK_macos_surface extension");
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        memset(&sci, 0, sizeof(sci));
        sci.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
        sci.pView = (__bridge const void *)(window->ns.view);

        err = vkCreateMacOSSurfaceMVK(instance, &sci, allocator, surface);
    }

    if (err)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Cocoa: Failed to create Vulkan surface: %s",
                        _glfwGetVulkanResultString(err));
    }

    return err;
#else
    return VK_ERROR_EXTENSION_NOT_PRESENT;
#endif

    } // autoreleasepool
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW native API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI id glfwGetCocoaWindow(GLFWwindow* handle)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;
    _GLFW_REQUIRE_INIT_OR_RETURN(nil);
    return window->ns.object;
}
