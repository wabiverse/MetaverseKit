// Copyright Contributors to the OpenImageIO project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/OpenImageIO/


#pragma once

// Determine which Imath we're dealing with and include the appropriate path
// to half.h.

#include <half.h>


#if defined(FMT_VERSION) && !defined(OIIO_HALF_FORMATTER)
#if FMT_VERSION >= 100000
#define OIIO_HALF_FORMATTER
FMT_BEGIN_NAMESPACE
template<> struct formatter<half> : ostream_formatter {};
FMT_END_NAMESPACE
#endif
#endif
