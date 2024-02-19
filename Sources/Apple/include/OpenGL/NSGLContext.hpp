/* ----------------------------------------------------------------
 * :: :  M  E  T  A  V  E  R  S  E  :                            ::
 * ----------------------------------------------------------------
 * This software is Licensed under the terms of the Apache License,
 * version 2.0 (the "Apache License") with the following additional
 * modification; you may not use this file except within compliance
 * of the Apache License and the following modification made to it.
 * Section 6. Trademarks. is deleted and replaced with:
 *
 * Trademarks. This License does not grant permission to use any of
 * its trade names, trademarks, service marks, or the product names
 * of this Licensor or its affiliates, except as required to comply
 * with Section 4(c.) of this License, and to reproduce the content
 * of the NOTICE file.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND without even an
 * implied warranty of MERCHANTABILITY, or FITNESS FOR A PARTICULAR
 * PURPOSE. See the Apache License for more details.
 *
 * You should have received a copy for this software license of the
 * Apache License along with this program; or, if not, please write
 * to the Free Software Foundation Inc., with the following address
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *         Copyright (C) 2024 Wabi Foundation. All Rights Reserved.
 * ----------------------------------------------------------------
 *  . x x x . o o o . x x x . : : : .    o  x  o    . : : : .
 * ---------------------------------------------------------------- */

#pragma once

#include "NSGLDefines.hpp"
#include "NSGLPrivate.hpp"
#include "NSGLPixelFormat.hpp"

#include <Foundation/Foundation.hpp>

#include "OpenGL.hpp"

namespace NSGL
{
_NSGL_ENUM(NS::Integer, OpenGLContextParameter) {
  OpenGLContextParameterSwapInterval = 222,
  OpenGLContextParameterSurfaceOrder = 235,
  OpenGLContextParameterSurfaceOpacity = 236,
  OpenGLContextParameterSurfaceBackingSize = 304,
  OpenGLContextParameterReclaimResources = 308,
  OpenGLContextParameterCurrentRendererID = 309,
  OpenGLContextParameterGPUVertexProcessing = 310,
  OpenGLContextParameterGPUFragmentProcessing = 311,
  OpenGLContextParameterHasDrawable = 314,
  OpenGLContextParameterMPSwapsInFlight = 315,
  OpenGLContextParameterSwapRectangle = 200,
  OpenGLContextParameterSwapRectangleEnable = 201,
  OpenGLContextParameterRasterizationEnable = 221,
  OpenGLContextParameterStateValidation = 301,
  OpenGLContextParameterSurfaceSurfaceVolatile = 306,
};

class OpenGLContext : public NS::Referencing<OpenGLContext>
{
 public:
  static class OpenGLContext* alloc();

  NSGL::OpenGLContext*          init(const NSGL::OpenGLPixelFormat* format, NSGL::OpenGLContext* share);

  NSGL::OpenGLPixelFormat*      pixelFormat() const;

#if 0
  void setView(NS::View* view);
  NS::View* view() const;

  void setPixelBuffer(NS::PixelBuffer* pixelBuffer);
  NS::PixelBuffer* pixelBuffer() const;
#endif // 0

  void setOffScreen(void* baseaddr, NS::Integer width, NS::Integer height, NS::Integer rowbytes);
  
  void clearDrawable();
  void update();

  /* Flush draw buffer */
  void flushBuffer();

  /* Current context control */
  void makeCurrentContext();
  static void clearCurrentContext();
  static NSGL::OpenGLContext* currentContext();

  void setFullScreen();
  void clearFullScreen();

  void setValues(const NS::Integer* vals, NSGL::OpenGLContextParameter param);
  void getValues(NS::Integer* vals, NSGL::OpenGLContextParameter param);

  NS::Integer currentVirtualScreen() const;

  void* CGLContextObj() const;
};
} // namespace NSGL

// static method: alloc
_NSGL_INLINE NSGL::OpenGLContext* NSGL::OpenGLContext::alloc()
{
    return NS::Object::alloc<NSGL::OpenGLContext>(_NSGL_PRIVATE_CLS(NSOpenGLContext));
}

// method: initWithFormat:shareContext:
_NSGL_INLINE NSGL::OpenGLContext* NSGL::OpenGLContext::init(const NSGL::OpenGLPixelFormat* format, NSGL::OpenGLContext* share)
{
    return Object::sendMessage<NSGL::OpenGLContext*>(this, _NSGL_PRIVATE_SEL(initWithFormat_shareContext_), format, share);
}

// method: pixelFormat
_NSGL_INLINE NSGL::OpenGLPixelFormat* NSGL::OpenGLContext::pixelFormat() const
{
    return Object::sendMessage<NSGL::OpenGLPixelFormat*>(this, _NSGL_PRIVATE_SEL(pixelFormat));
}

// method: setOffScreen:width:height:rowbytes:
_NSGL_INLINE void NSGL::OpenGLContext::setOffScreen(void* baseaddr, NS::Integer width, NS::Integer height, NS::Integer rowbytes)
{
    Object::sendMessage<void>(this, _NSGL_PRIVATE_SEL(setOffScreen_width_height_rowbytes_), baseaddr, width, height, rowbytes);
}

// method: clearDrawable
_NSGL_INLINE void NSGL::OpenGLContext::clearDrawable()
{
    Object::sendMessage<void>(this, _NSGL_PRIVATE_SEL(clearDrawable));
}

// method: update
_NSGL_INLINE void NSGL::OpenGLContext::update()
{
    Object::sendMessage<void>(this, _NSGL_PRIVATE_SEL(update));
}

// method: flushBuffer
_NSGL_INLINE void NSGL::OpenGLContext::flushBuffer()
{
    Object::sendMessage<void>(this, _NSGL_PRIVATE_SEL(flushBuffer));
}

// method: makeCurrentContext
_NSGL_INLINE void NSGL::OpenGLContext::makeCurrentContext()
{
    Object::sendMessage<void>(this, _NSGL_PRIVATE_SEL(makeCurrentContext));
}

// static method: clearCurrentContext
_NSGL_INLINE void NSGL::OpenGLContext::clearCurrentContext()
{
    Object::sendMessage<void>(_NSGL_PRIVATE_CLS(NSOpenGLContext), _NSGL_PRIVATE_SEL(clearCurrentContext));
}

// static method: currentContext
_NSGL_INLINE NSGL::OpenGLContext* NSGL::OpenGLContext::currentContext()
{
    return Object::sendMessage<NSGL::OpenGLContext*>(_NSGL_PRIVATE_CLS(NSOpenGLContext), _NSGL_PRIVATE_SEL(currentContext));
}

// method: setFullScreen
_NSGL_INLINE void NSGL::OpenGLContext::setFullScreen()
{
    Object::sendMessage<void>(this, _NSGL_PRIVATE_SEL(setFullScreen));
}

// method: clearFullScreen
_NSGL_INLINE void NSGL::OpenGLContext::clearFullScreen()
{
    Object::sendMessage<void>(this, _NSGL_PRIVATE_SEL(clearFullScreen));
}

// method: setValues:forParameter:
_NSGL_INLINE void NSGL::OpenGLContext::setValues(const NS::Integer* vals, NSGL::OpenGLContextParameter param)
{
    Object::sendMessage<void>(this, _NSGL_PRIVATE_SEL(setValues_forParameter_), vals, param);
}

// method: getValues:forParameter:
_NSGL_INLINE void NSGL::OpenGLContext::getValues(NS::Integer* vals, NSGL::OpenGLContextParameter param)
{
    Object::sendMessage<void>(this, _NSGL_PRIVATE_SEL(getValues_forParameter_), vals, param);
}

// method: currentVirtualScreen
_NSGL_INLINE NS::Integer NSGL::OpenGLContext::currentVirtualScreen() const
{
    return Object::sendMessage<NS::Integer>(this, _NSGL_PRIVATE_SEL(currentVirtualScreen));
}

// method: CGLContextObj
_NSGL_INLINE void* NSGL::OpenGLContext::CGLContextObj() const
{
    return Object::sendMessage<void*>(this, _NSGL_PRIVATE_SEL(CGLContextObj));
}
