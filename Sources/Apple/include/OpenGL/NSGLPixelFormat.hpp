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

#include <Foundation/Foundation.hpp>

#include "OpenGL.hpp"

namespace NSGL
{
_NSGL_ENUM(NS::UInteger, OpenGLPixelFormatAttribute)
{
  OpenGLPFAAllRenderers = 1,
  OpenGLPFATripleBuffer = 3,
  OpenGLPFADoubleBuffer = 5,
  OpenGLPFAAuxBuffers = 7,
  OpenGLPFAColorSize = 8,
  OpenGLPFAAlphaSize = 11,
  OpenGLPFADepthSize = 12,
  OpenGLPFAStencilSize = 13,
  OpenGLPFAAccumSize = 14,
  OpenGLPFAMinimumPolicy = 51,
  OpenGLPFAMaximumPolicy = 52,
  OpenGLPFASampleBuffers = 55,
  OpenGLPFASamples = 56,
  OpenGLPFAAuxDepthStencil = 57,
  OpenGLPFAColorFloat = 58,
  OpenGLPFAMultisample = 59,
  OpenGLPFASupersample = 60,
  OpenGLPFASampleAlpha = 61,
  OpenGLPFARendererID = 70,
  OpenGLPFANoRecovery = 72,
  OpenGLPFAAccelerated = 73,
  OpenGLPFAClosestPolicy = 74,
  OpenGLPFABackingStore = 76,
  OpenGLPFAScreenMask = 84,
  OpenGLPFAAllowOfflineRenderers = 96,
  OpenGLPFAAcceleratedCompute = 97,
  OpenGLPFAOpenGLProfile = 99,
  OpenGLPFAVirtualScreenCount = 128,

  OpenGLPFAStereo = 6,
  OpenGLPFAOffScreen = 53,
  OpenGLPFAFullScreen = 54,
  OpenGLPFASingleRenderer = 71,
  OpenGLPFARobust = 75,
  OpenGLPFAMPSafe = 78,
  OpenGLPFAWindow = 80,
  OpenGLPFAMultiScreen = 81,
  OpenGLPFACompliant = 83,
  OpenGLPFAPixelBuffer = 90,
  OpenGLPFARemotePixelBuffer = 91,

  OpenGLProfileVersionLegacy = 0x1000,  /* choose a Legacy/Pre-OpenGL 3.0 Implementation */
  OpenGLProfileVersion3_2Core = 0x3200, /* choose an OpenGL 3.2 Core Implementation      */
  OpenGLProfileVersion4_1Core = 0x4100, /* choose an OpenGL 4.1 Core Implementation      */
};

class OpenGLPixelFormat : public NS::Referencing<OpenGLPixelFormat>
{
 public:
  static class OpenGLPixelFormat* alloc();

  NSGL::OpenGLPixelFormat*          init(const NSGL::OpenGLPixelFormatAttribute* attribs);
  NSGL::OpenGLPixelFormat*          init(NS::Data* attribs);

  NS::Data* attributes(NSGL::OpenGLPixelFormatAttribute attributes);
  void      getValues(NS::Integer* vals, NSGL::OpenGLPixelFormatAttribute attrib, NS::Integer screen);

  NS::Integer numberOfVirtualScreens() const;

  void* CGLPixelFormatObj() const;
};
} // namespace NSGL

// static method: alloc
_NSGL_INLINE NSGL::OpenGLPixelFormat* NSGL::OpenGLPixelFormat::alloc()
{
  return NS::Object::alloc<NSGL::OpenGLPixelFormat>(_NSGL_PRIVATE_CLS(NSOpenGLPixelFormat));
}

// method: initWithAttributes:
_NSGL_INLINE NSGL::OpenGLPixelFormat* NSGL::OpenGLPixelFormat::init(const NSGL::OpenGLPixelFormatAttribute* attribs)
{
  return Object::sendMessage<NSGL::OpenGLPixelFormat*>(this, _NSGL_PRIVATE_SEL(initWithAttributes_), attribs);
}

// method: initWithData:
_NSGL_INLINE NSGL::OpenGLPixelFormat* NSGL::OpenGLPixelFormat::init(NS::Data* attribs)
{
  return Object::sendMessage<NSGL::OpenGLPixelFormat*>(this, _NSGL_PRIVATE_SEL(initWithData_), attribs);
}

// method: attributes
_NSGL_INLINE NS::Data* NSGL::OpenGLPixelFormat::attributes(NSGL::OpenGLPixelFormatAttribute attributes)
{
  return Object::sendMessage<NS::Data*>(this, _NSGL_PRIVATE_SEL(attributes), attributes);
}

// method: getValues:forAttribute:forVirtualScreen:
_NSGL_INLINE void NSGL::OpenGLPixelFormat::getValues(NS::Integer* vals, NSGL::OpenGLPixelFormatAttribute attrib, NS::Integer screen)
{
  Object::sendMessage<void>(this, _NSGL_PRIVATE_SEL(getValues_forAttribute_forVirtualScreen_), vals, attrib, screen);
}

// property: numberOfVirtualScreens
_NSGL_INLINE NS::Integer NSGL::OpenGLPixelFormat::numberOfVirtualScreens() const
{
  return Object::sendMessage<NS::Integer>(this, _NSGL_PRIVATE_SEL(numberOfVirtualScreens));
}

// property: CGLPixelFormatObj
_NSGL_INLINE void* NSGL::OpenGLPixelFormat::CGLPixelFormatObj() const
{
  return Object::sendMessage<void*>(this, _NSGL_PRIVATE_SEL(CGLPixelFormatObj));
}
