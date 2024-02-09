#ifndef __OPENIMAGEIO_H__
#define __OPENIMAGEIO_H__

// TODO: swift hates this include (OpenImageIO/detail/fmt.h) with an error in the form of:
// 1.	Apple Swift version 5.9.2 (swiftlang-5.9.2.2.56 clang-1500.1.0.2.5)
// 2.	Compiling with the current language version
// 3.	/Users/furby/Wabi/MetaverseKit/Sources/OpenImageIO/include/OpenImageIO/detail/fmt/ostream.h:32:7: importing 'fmt::detail::file_access'
// 4.	/Users/furby/Wabi/MetaverseKit/Sources/OpenImageIO/include/OpenImageIO/detail/fmt/ostream.h:32:7: importing 'fmt::detail::file_access'
// 5.	/Users/furby/Wabi/MetaverseKit/Sources/OpenImageIO/include/OpenImageIO/detail/fmt/ostream.h:33:15: importing 'fmt::detail::get_file'

// In the meantime, leave the OpenImageIO import stubbed out
// so a developer can begin slotting in headers to fill the
// swift api as needed.

#include <OpenImageIO/oiioversion.h>
#include <OpenImageIO/version.h>
#include <OpenImageIO/half.h>

// #include <OpenImageIO/argparse.h>
// #include <OpenImageIO/array_view.h>
// #include <OpenImageIO/atomic.h>
// #include <OpenImageIO/attrdelegate.h>
// #include <OpenImageIO/benchmark.h>
// #include <OpenImageIO/color.h>
// #include <OpenImageIO/dassert.h>
// #include <OpenImageIO/deepdata.h>
// #include <OpenImageIO/errorhandler.h>
// #include <OpenImageIO/export.h>
// #include <OpenImageIO/filesystem.h>
// #include <OpenImageIO/filter.h>
// #include <OpenImageIO/fmath.h>
// #include <OpenImageIO/fstream_mingw.h>
// #include <OpenImageIO/function_view.h>
// #include <OpenImageIO/hash.h>
// #include <OpenImageIO/image_view.h>
// #include <OpenImageIO/imagebuf.h>
// #include <OpenImageIO/imagebufalgo.h>
// #include <OpenImageIO/imagebufalgo_util.h>
// #include <OpenImageIO/imagecache.h>
// #include <OpenImageIO/imageio.h>
// #include <OpenImageIO/missing_math.h>
// #include <OpenImageIO/oiioversion.h>
// #include <OpenImageIO/optparser.h>
// #include <OpenImageIO/parallel.h>
// #include <OpenImageIO/paramlist.h>
// #include <OpenImageIO/platform.h>
// #include <OpenImageIO/plugin.h>
// #include <OpenImageIO/refcnt.h>
// #include <OpenImageIO/simd.h>
// #include <OpenImageIO/span.h>
// #include <OpenImageIO/strided_ptr.h>
// #include <OpenImageIO/string_view.h>
// #include <OpenImageIO/strongparam.h>
// #include <OpenImageIO/strutil.h>
// #include <OpenImageIO/sysutil.h>
// #include <OpenImageIO/texture.h>
// #include <OpenImageIO/thread.h>
// #include <OpenImageIO/tiffutils.h>
// #include <OpenImageIO/timer.h>
// #include <OpenImageIO/type_traits.h>
// #include <OpenImageIO/typedesc.h>
// #include <OpenImageIO/unittest.h>
// #include <OpenImageIO/unordered_map_concurrent.h>
// #include <OpenImageIO/ustring.h>
// #include <OpenImageIO/varyingref.h>
// #include <OpenImageIO/vecparam.h>

#endif // __OPENIMAGEIO_H__
