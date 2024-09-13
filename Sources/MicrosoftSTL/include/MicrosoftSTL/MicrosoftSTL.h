#ifndef __WABI_MICROSOFT_STL_H__
#define __WABI_MICROSOFT_STL_H__

#if defined(_WIN32)
// otherwise, MicrosoftSTL completely fails when
// we are using anything below C++20, and that's
// just rediculous, specifically for the include
// of the STL's <version> header anywhere within
// MetaverseKit due to a modulemap requiring the
// C++20 language standard for that header, this
// workaround replaces every: #include <version>
// with: #include <MicrosoftSTL/STLVersion.h>
# include <MicrosoftSTL/STLVersion.h>
#endif // defined(_WIN32)

#endif // __WABI_MICROSOFT_STL_H__
