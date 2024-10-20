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

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#include "NSGLDefines.hpp"

#include <objc/runtime.h>

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#define _NSGL_PRIVATE_CLS(symbol) (Private::Class::s_k##symbol)
#define _NSGL_PRIVATE_SEL(accessor) (Private::Selector::s_k##accessor)

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#if defined(NSGL_PRIVATE_IMPLEMENTATION)

#ifdef METALCPP_SYMBOL_VISIBILITY_HIDDEN
#define _NSGL_PRIVATE_VISIBILITY __attribute__((visibility("hidden")))
#else
#define _NSGL_PRIVATE_VISIBILITY __attribute__((visibility("default")))
#endif // METALCPP_SYMBOL_VISIBILITY_HIDDEN

#define _NSGL_PRIVATE_IMPORT __attribute__((weak_import))

#ifdef __OBJC__
#define _NSGL_PRIVATE_OBJC_LOOKUP_CLASS(symbol) ((__bridge void*)objc_lookUpClass(#symbol))
#define _NSGL_PRIVATE_OBJC_GET_PROTOCOL(symbol) ((__bridge void*)objc_getProtocol(#symbol))
#else
#define _NSGL_PRIVATE_OBJC_LOOKUP_CLASS(symbol) objc_lookUpClass(#symbol)
#define _NSGL_PRIVATE_OBJC_GET_PROTOCOL(symbol) objc_getProtocol(#symbol)
#endif // __OBJC__

#define _NSGL_PRIVATE_DEF_CLS(symbol) void* s_k##symbol _NSGL_PRIVATE_VISIBILITY = _NSGL_PRIVATE_OBJC_LOOKUP_CLASS(symbol)
#define _NSGL_PRIVATE_DEF_PRO(symbol) void* s_k##symbol _NSGL_PRIVATE_VISIBILITY = _NSGL_PRIVATE_OBJC_GET_PROTOCOL(symbol)
#define _NSGL_PRIVATE_DEF_SEL(accessor, symbol) SEL s_k##accessor _NSGL_PRIVATE_VISIBILITY = sel_registerName(symbol)

#include <dlfcn.h>
#define NSGL_DEF_FUNC( name, signature ) \
    using Fn##name = signature; \
    Fn##name name = reinterpret_cast< Fn##name >( dlsym( RTLD_DEFAULT, #name ) )

namespace NSGL::Private
{
    template <typename _Type>
    inline _Type const LoadSymbol(const char* pSymbol)
    {
        const _Type* pAddress = static_cast<_Type*>(dlsym(RTLD_DEFAULT, pSymbol));

        return pAddress ? *pAddress : nullptr;
    }
} // NSGL::Private

#if defined(__MAC_10_16) || defined(__MAC_11_0) || defined(__MAC_12_0) || defined(__MAC_13_0) || defined(__MAC_14_0) || defined(__IPHONE_14_0) || defined(__IPHONE_15_0) || defined(__IPHONE_16_0) || defined(__IPHONE_17_0) || defined(__TVOS_14_0) || defined(__TVOS_15_0) || defined(__TVOS_16_0) || defined(__TVOS_17_0)

#define _NSGL_PRIVATE_DEF_STR(type, symbol)                  \
    _NSGL_EXTERN type const NS##symbol _NSGL_PRIVATE_IMPORT; \
    type const                         NS::symbol = (nullptr != &NS##symbol) ? NS##symbol : nullptr

#define _NSGL_PRIVATE_DEF_CONST(type, symbol)              \
    _NSGL_EXTERN type const NS##symbol _NSGL_PRIVATE_IMPORT; \
    type const                         NS::symbol = (nullptr != &NS##symbol) ? NS##symbol : nullptr

#define _NSGL_PRIVATE_DEF_WEAK_CONST(type, symbol) \
    _NSGL_EXTERN type const NS##symbol;    \
    type const             NS::symbol = Private::LoadSymbol<type>("NS" #symbol)

#else

#define _NSGL_PRIVATE_DEF_STR(type, symbol) \
    _NSGL_EXTERN type const NS##symbol;    \
    type const             NS::symbol = Private::LoadSymbol<type>("NS" #symbol)

#define _NSGL_PRIVATE_DEF_CONST(type, symbol) \
    _NSGL_EXTERN type const NS##symbol;    \
    type const             NS::symbol = Private::LoadSymbol<type>("NS" #symbol)

#define _NSGL_PRIVATE_DEF_WEAK_CONST(type, symbol) _NSGL_PRIVATE_DEF_CONST(type, symbol)

#endif // defined(__MAC_10_16) || defined(__MAC_11_0) || defined(__MAC_12_0) || defined(__MAC_13_0) || defined(__MAC_14_0) || defined(__IPHONE_14_0) || defined(__IPHONE_15_0) || defined(__IPHONE_16_0) || defined(__IPHONE_17_0) || defined(__TVOS_14_0) || defined(__TVOS_15_0) || defined(__TVOS_16_0) || defined(__TVOS_17_0)

#else

#define _NSGL_PRIVATE_DEF_CLS(symbol) extern void* s_k##symbol
#define _NSGL_PRIVATE_DEF_PRO(symbol) extern void* s_k##symbol
#define _NSGL_PRIVATE_DEF_SEL(accessor, symbol) extern SEL s_k##accessor
#define _NSGL_PRIVATE_DEF_STR(type, symbol) extern type const NS::symbol
#define _NSGL_PRIVATE_DEF_CONST(type, symbol) extern type const NS::symbol
#define _NSGL_PRIVATE_DEF_WEAK_CONST(type, symbol) extern type const NS::symbol

#endif // NSGL_PRIVATE_IMPLEMENTATION

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace NSGL
{
  namespace Private
  {
    namespace Class
    {
      _NSGL_PRIVATE_DEF_CLS(NSOpenGLPixelFormat);
      _NSGL_PRIVATE_DEF_CLS(NSOpenGLContext);
      _NSGL_PRIVATE_DEF_CLS(NSOpenGLPixelBuffer);
    } // Class
  } // Private
} // NSGL

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace NSGL
{
  namespace Private
  {
    namespace Protocol
    {
      _NSGL_PRIVATE_DEF_PRO(NSCoding);
      _NSGL_PRIVATE_DEF_PRO(NSLocking);
    } // Protocol
  } // Private
} // NSGL

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace NSGL
{
  namespace Private
  {
    namespace Selector
    {
      _NSGL_PRIVATE_DEF_SEL(initWithAttributes_,
          "initWithAttributes:");
      _NSGL_PRIVATE_DEF_SEL(initWithData_,
          "initWithData:");
      _NSGL_PRIVATE_DEF_SEL(attributes,
          "attributes");
      _NSGL_PRIVATE_DEF_SEL(getValues_forAttribute_forVirtualScreen_,
          "getValues:forAttribute:forVirtualScreen:");
      _NSGL_PRIVATE_DEF_SEL(numberOfVirtualScreens,
          "numberOfVirtualScreens");
      _NSGL_PRIVATE_DEF_SEL(CGLPixelFormatObj,
          "CGLPixelFormatObj");
      _NSGL_PRIVATE_DEF_SEL(initWithFormat_shareContext_,
          "initWithFormat:shareContext:");
      _NSGL_PRIVATE_DEF_SEL(pixelFormat,
          "pixelFormat");
      _NSGL_PRIVATE_DEF_SEL(setOffScreen_width_height_rowbytes_,
          "setOffScreen:width:height:rowbytes:");
      _NSGL_PRIVATE_DEF_SEL(clearDrawable,
          "clearDrawable");
      _NSGL_PRIVATE_DEF_SEL(update,
          "update");
      _NSGL_PRIVATE_DEF_SEL(flushBuffer,
          "flushBuffer");
      _NSGL_PRIVATE_DEF_SEL(makeCurrentContext,
          "makeCurrentContext");
      _NSGL_PRIVATE_DEF_SEL(clearCurrentContext,
          "clearCurrentContext");
      _NSGL_PRIVATE_DEF_SEL(currentContext,
          "currentContext");
      _NSGL_PRIVATE_DEF_SEL(setFullScreen,
          "setFullScreen");
      _NSGL_PRIVATE_DEF_SEL(clearFullScreen,
          "clearFullScreen");
      _NSGL_PRIVATE_DEF_SEL(setValues_forParameter_,
          "setValues:forParameter:");
      _NSGL_PRIVATE_DEF_SEL(getValues_forParameter_,
          "getValues:forParameter:");
      _NSGL_PRIVATE_DEF_SEL(currentVirtualScreen,
          "currentVirtualScreen");
      _NSGL_PRIVATE_DEF_SEL(CGLContextObj,
          "CGLContextObj");
    } // Class
  } // Private
} // NSGL

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
