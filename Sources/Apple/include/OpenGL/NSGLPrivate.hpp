//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Metal/NSGLPrivate.hpp
//
// Copyright 2020-2023 Apple Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

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
      _NSGL_PRIVATE_DEF_SEL(initWithFormat_shareContext,
          "initWithFormat:shareContext:");
      _NSGL_PRIVATE_DEF_SEL(currentContext,
          "currentContext");
    } // Class
  } // Private
} // NSGL

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
