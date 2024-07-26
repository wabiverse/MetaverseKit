// swift-tools-version: 5.10
import PackageDescription

/* ------------------------------------------------------
 *  :: :  💫 The Open Source Metaverse  :   ::
 * ------------------------------------------------------ */

let package = Package(
  name: "MetaverseKit",
  platforms: [
    .macOS(.v14),
    .visionOS(.v1),
    .iOS(.v17),
    .tvOS(.v17),
    .watchOS(.v10),
  ],
  products: getConfig(for: .all).products,
  dependencies: [
    .package(url: "https://github.com/furby-tm/swift-bundler", from: "2.0.8"),
  ] + Arch.OS.packageDeps(),
  targets: [
    .target(
      name: "Eigen",
      publicHeadersPath: "include"
    ),

    .target(
      name: "Draco",
      dependencies: [
        .target(name: "Eigen"),
      ],
      publicHeadersPath: "include"
    ),

    .target(
      name: "TBBMallocProxy",
      dependencies: [
        .target(name: "OneTBB"),
      ],
      publicHeadersPath: ".",
      cxxSettings: [
        .define("_XOPEN_SOURCE", to: "1", .when(platforms: Arch.OS.apple.platform)),
        .define("_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH", .when(platforms: [.windows])),
      ]
    ),

    .target(
      name: "TBBMalloc",
      dependencies: [
        .target(name: "OneTBB"),
        .target(name: "TBBMallocProxy"),
      ],
      exclude: [],
      publicHeadersPath: ".",
      cxxSettings: [
        .define("_XOPEN_SOURCE", to: "1", .when(platforms: Arch.OS.apple.platform)),
        .define("__TBBMALLOC_BUILD", to: "1"),
        .define("_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH", .when(platforms: [.windows])),
      ]
    ),

    .target(
      name: "OneTBB",
      publicHeadersPath: "include",
      cxxSettings: [
        .define("_XOPEN_SOURCE", to: "1", .when(platforms: Arch.OS.apple.platform)),
        .define("TBB_ALLOCATOR_TRAITS_BROKEN", to: "1", .when(platforms: Arch.OS.linux.platform)),
        .define("_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH", .when(platforms: [.windows])),
      ]
    ),

    .target(
      name: "MetaTBB",
      dependencies: [
        .target(name: "OneTBB"),
        .target(name: "TBBMalloc"),
        .target(name: "TBBMallocProxy"),
      ],
      exclude: [],
      cxxSettings: [
        .define("_XOPEN_SOURCE"),
        .define("TBB_USE_PROFILING_TOOLS", to: "2"),
      ],
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),

    .target(
      name: "ZStandard",
      publicHeadersPath: "."
    ),

    .target(
      name: "LZMA2",
      dependencies: [],
      exclude: getConfig(for: .lzma2).exclude,
      publicHeadersPath: "include",
      cSettings: getConfig(for: .lzma2).cSettings
    ),

    .target(
      name: "Yaml",
      dependencies: [],
      exclude: [],
      publicHeadersPath: "include",
      cxxSettings: [
        .define("_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH", .when(platforms: [.windows])),
        .define("yaml_cpp_EXPORTS", .when(platforms: [.windows]))
      ]
    ),

    .target(
      name: "MiniZip",
      dependencies: Arch.OS.minizipDeps(),
      exclude: getConfig(for: .minizip).exclude,
      publicHeadersPath: "include",
      cSettings: [
        .define("HAVE_ZLIB", to: "1"),
        .define("ZLIB_COMPAT", to: "1"),
        .define("MZ_ZIP_NO_CRYPTO", to: "1"),
      ],
      linkerSettings: [
        .linkedLibrary("bz2"),
      ]
    ),

    .target(
      name: "ZLibDataCompression",
      dependencies: [],
      exclude: [],
      publicHeadersPath: "include",
      cSettings: [
        .headerSearchPath("."),
        .define("HAVE_ATTRIBUTE_ALIGNED", to: "1"),
        .define("WITH_GZFILEOP", to: "1"),
        .define("HAVE_UNISTD_H", to: "1", .when(platforms: Arch.OS.nix.platform)),
        .define("Z_HAVE_STDARG_H", to: "1"),
      ]
    ),

    .target(
      name: "Raw",
      dependencies: [],
      exclude: [],
      publicHeadersPath: "include",
      cxxSettings: [
        .headerSearchPath("."),
        .define("_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH", .when(platforms: [.windows])),
        .define("LIBRAW_BUILDLIB", .when(platforms: [.windows])),
      ]
    ),

    .target(
      name: "LibPNG",
      dependencies: [
        .target(name: "ZLibDataCompression"),
      ],
      exclude: getConfig(for: .png).exclude,
      publicHeadersPath: "include",
      cSettings: getConfig(for: .png).cSettings
    ),

    .target(
      name: "TurboJPEG",
      exclude: getConfig(for: .turbojpeg).exclude,
      publicHeadersPath: "include/turbo",
      cSettings: [
        .headerSearchPath("."),
        .define("C_LOSSLESS_SUPPORTED", to: "1"),
        .define("D_LOSSLESS_SUPPORTED", to: "1"),
        .define("PPM_SUPPORTED", to: "1"),
        .define("BITS_IN_JSAMPLE", to: "12"),
      ]
    ),

    .target(
      name: "TIFF",
      dependencies: [
        .target(name: "ZLibDataCompression"),
        .target(name: "WebP", condition: .when(platforms: Arch.OS.nix.platform)),
        .target(name: "LZMA2"),
        .target(name: "ZStandard"),
        .target(name: "TurboJPEG"),
      ],
      exclude: getConfig(for: .tiff).exclude,
      publicHeadersPath: "include",
      cxxSettings: [
        .define("FROM_TIF_JPEG_12", to: "1"),
        .define("HAVE_JPEGTURBO_DUAL_MODE_8_12", to: "1"),
        .define("BITS_IN_JSAMPLE", to: "12"),
      ]
    ),

    .target(
      name: "WebP",
      publicHeadersPath: "include",
      cSettings: [
        .headerSearchPath("."),
      ]
    ),

    .target(
      name: "OpenMP",
      dependencies: [
        Arch.OS.python(),
      ],
      exclude: getConfig(for: .openmp).exclude,
      publicHeadersPath: "include",
      cxxSettings: [
        .headerSearchPath("runtime"),
      ]
    ),

    .target(
      name: "Apple",
      publicHeadersPath: "include"
    ),

    .target(
      name: "DEFLATE",
      exclude: [
        // better cross-platform compatibility if we remove gzip
        // support for now, gzip builds fine for macOS and even
        // on iOS -- it is only when a user chooses to build an
        // archive using one of the generic devices, that makes
        // building gzip more complicated.
        "crc32.c",
        "gzip_compress.c",
        "gzip_decompress.c",
      ],
      publicHeadersPath: "include",
      cxxSettings: [
        .headerSearchPath("."),
        .define("HAVE_UNISTD_H", to: "1", .when(platforms: Arch.OS.nix.platform)),
        .define("Z_HAVE_STDARG_H", to: "1"),
      ]
    ),

    .target(
      name: "GLFW",
      dependencies: Arch.OS.glfwDeps(),
      exclude: getConfig(for: .glfw).exclude,
      publicHeadersPath: "include",
      cxxSettings: getConfig(for: .glfw).cxxSettings,
      linkerSettings: getConfig(for: .glfw).linkerSettings
    ),

    .target(
      name: "ImGui",
      dependencies: [
        .target(name: "GLFW"),
      ],
      exclude: getConfig(for: .imgui).exclude,
      publicHeadersPath: "include",
      cxxSettings: [
        .define("VK_USE_PLATFORM_MACOS_MVK", to: "1", .when(platforms: [.macOS])),
        .define("VK_USE_PLATFORM_IOS_MVK", to: "1", .when(platforms: [.iOS, .visionOS])),
        .headerSearchPath("."),
        .headerSearchPath("backends"),
      ],
      linkerSettings: [
        .linkedFramework("Cocoa", .when(platforms: [.macOS])),
        .linkedFramework("GLUT", .when(platforms: [.macOS])),
        .linkedFramework("GameController", .when(platforms: Arch.OS.apple.platform)),
      ]
    ),

    .target(
      name: "MXResources",
      exclude: [
        // FIXME:
        // temporary workaround for metal shaders
        // erroring on compilation, so we don't
        // gunk up SwiftUSD and everything else.
        "libraries",
        "Resources/Lights",
        "Resources/Geometry",
        "Resources/Materials",
      ],
      resources: [
        .process("Resources/Images"),
      ],
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),

    .target(
      name: "MaterialX",
      dependencies: [
        .target(name: "Apple", condition: .when(platforms: Arch.OS.apple.platform)),
        .target(name: "ImGui"),
        .target(name: "OpenImageIO"),
        .target(name: "OpenImageIO_Util"),
        .target(name: "PyBind11"),
        .target(name: "MXResources"),
        Arch.OS.python(),
      ],
      exclude: getConfig(for: .materialx).exclude,
      publicHeadersPath: "include",
      cxxSettings: [
        .define("GL_SILENCE_DEPRECATION", to: "1"),
      ]
    ),

    .executableTarget(
      name: "MXGraphEditor",
      dependencies: [
        .target(name: "MaterialX"),
      ],
      exclude: getConfig(for: .mxGraphEditor).exclude
    ),

    .target(
      name: "OpenSubdiv",
      dependencies: [
        .target(name: "MetaTBB"),
        .target(name: "OpenMP"),
      ],
      exclude: getConfig(for: .osd).exclude,
      publicHeadersPath: "include",
      cxxSettings: getConfig(for: .osd).cxxSettings
    ),

    .target(
      name: "pystring",
      publicHeadersPath: "include",
      cxxSettings: [
        .headerSearchPath("include/pystring"),
        .define("_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH", .when(platforms: [.windows]))
      ]
    ),

    .target(
      name: "Imath",
      dependencies: [
        .target(name: "PyBind11"),
        .target(name: "pystring"),
        Arch.OS.python(),
      ],
      exclude: [
        /* metapy builds pyimath. */
        "src/python/PyBindImath",
        "src/python/PyImath",
        "src/python/PyImathTest",
        "src/python/PyImathNumpy",
        "src/python/PyImathNumpyTest",
        "src/python/PyImathPythonTest",
        "src/python/PyImathSpeedTest",
        "src/python/PyImathTest",
        "src/ImathTest",
        "website",
      ],
      publicHeadersPath: "src/Imath",
      cxxSettings: [
        .headerSearchPath("src/python/PyImath"),
      ],
      linkerSettings: [
        .linkedFramework("OpenGL", .when(platforms: [.macOS])),
        .linkedFramework("OpenGLES", .when(platforms: [.iOS, .watchOS, .tvOS])),
      ]
    ),

    .target(
      name: "any",
      publicHeadersPath: "include",
      cxxSettings: [
        .define("_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH", .when(platforms: [.windows]))
      ]
    ),

    .target(
      name: "nonstd",
      publicHeadersPath: "include",
      cxxSettings: [
        .define("_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH", .when(platforms: [.windows]))
      ]
    ),

    .target(
      name: "rapidjson",
      publicHeadersPath: "include",
      cxxSettings: [
        .define("_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH", .when(platforms: [.windows]))
      ]
    ),

    .target(
      name: "OpenTime",
      dependencies: [
        .target(name: "PyBind11"),
        .target(name: "any"),
        .target(name: "nonstd"),
        .target(name: "rapidjson"),
        .target(name: "Imath"),
        Arch.OS.python(),
      ],
      path: "Sources/OpenTimelineIO/src/opentime",
      publicHeadersPath: "include"
    ),

    .target(
      name: "OpenTimelineIO",
      dependencies: [
        .target(name: "OpenTime"),
      ],
      path: "Sources/OpenTimelineIO/src/opentimelineio",
      exclude: [
        "CORE_VERSION_MAP.last.cpp",
      ],
      publicHeadersPath: "include",
      cxxSettings: [
        .headerSearchPath("include/opentimelineio"),
      ]
    ),

    .target(
      name: "OpenEXR",
      dependencies: [
        .target(name: "Imath"),
        .target(name: "DEFLATE", condition: .when(platforms: Arch.OS.apple.platform)),
      ],
      publicHeadersPath: "include",
      cxxSettings: [
        .headerSearchPath("."),
        .headerSearchPath("OpenEXRCore"),
        .headerSearchPath("include/OpenEXR"),
      ],
      linkerSettings: [
        .linkedLibrary("deflate", .when(platforms: Arch.OS.linux.platform)),
      ]
    ),

    .target(
      name: "OpenImageIO_Util",
      dependencies: [
        .target(name: "OpenImageIO"),
      ],
      exclude: getConfig(for: .oiioUtil).exclude,
      publicHeadersPath: "include",
      cxxSettings: [
        .headerSearchPath("."),
        .define("OpenImageIO_Util_EXPORTS", to: "1"),
      ],
      linkerSettings: getConfig(for: .oiioUtil).linkerSettings
    ),

    .target(
      name: "OpenImageIO",
      dependencies: [
        .target(name: "WebP", condition: .when(platforms: Arch.OS.nix.platform)),
        .target(name: "TIFF", condition: .when(platforms: Arch.OS.nix.platform)),
        .target(name: "Raw"),
        .target(name: "Ptex"),
        .target(name: "LibPNG"),
        .target(name: "OpenVDB"),
        .target(name: "Imath"),
        .target(name: "OpenEXR"),
        .target(name: "PyBind11"),
      ],
      exclude: getConfig(for: .oiio).exclude,
      publicHeadersPath: "include",
      cxxSettings: [
        .headerSearchPath("."),
        .headerSearchPath("include/OpenImageIO/detail"),
        .define("EMBED_PLUGINS", to: "1"),
        .define("OpenImageIO_EXPORTS", to: "1"),
        .define("DISABLE_PSD", to: "1"),
        .define("DISABLE_BMP", to: "1"),
        .define("DISABLE_TERM", to: "1"),
        .define("DISABLE_PNM", to: "1"),
        // FIXME: We broke the ABI for fmt::detail::get_file
        // to stop the swift compiler from crashing at this
        // function, we should fix this in the future.
        // .define("OIIO_FIX_ABI_FOR_SWIFT", to: "1"),
      ],
      linkerSettings: getConfig(for: .oiio).linkerSettings
    ),

    .target(
      name: "OpenColorIO",
      dependencies: Arch.OS.ocioDeps(),
      exclude: getConfig(for: .ocio).exclude,
      publicHeadersPath: "include",
      cxxSettings: [
        .headerSearchPath("."),
      ],
      linkerSettings: [
        .linkedLibrary("expat"),
      ]
    ),

    .target(
      name: "OCIOBundle",
      dependencies: [
        .target(name: "OpenColorIO"),
      ],
      resources: [
        .copy("Resources/colormanagement"),
      ],
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),

    .target(
      name: "MetaPy",
      dependencies: [
        .target(name: "Boost"),
        .target(name: "Imath"),
        .target(name: "OpenEXR"),
        .target(name: "OpenVDB"),
        .target(name: "Alembic"),
        .target(name: "OpenImageIO"),
        .target(name: "MaterialX"),
        Arch.OS.python(),
      ],
      exclude: getConfig(for: .mpy).exclude,
      publicHeadersPath: "include/python",
      cxxSettings: [
        .headerSearchPath("include/python/PyImath"),
        .headerSearchPath("include/python/PyAlembic"),
        .headerSearchPath("include/python/PyOIIO"),
      ]
    ),

    .target(
      name: "Ptex",
      publicHeadersPath: "include",
      cxxSettings: []
    ),

    .target(
      name: "HDF5",
      publicHeadersPath: "include",
      cSettings: [
        .define("H5_USE_18_API", to: "1"),
        .define("H5_BUILT_AS_DYNAMIC_LIB", to: "1"),
        .define("H5_HAVE_C99_FUNC", to: "1", .when(platforms: Arch.OS.apple.platform)),
        .define("H5_HAVE_FUNCTION", to: "1", .when(platforms: Arch.OS.linux.platform)),
        .define("H5_TIME_WITH_SYS_TIME", to: "1", .when(platforms: Arch.OS.linux.platform)),
      ]
    ),

    .target(
      name: "Alembic",
      dependencies: [
        .target(name: "Boost"),
        .target(name: "HDF5"),
        .target(name: "Imath"),
        .target(name: "OpenEXR"),
        Arch.OS.python(),
      ],
      publicHeadersPath: "include",
      cxxSettings: [
        .headerSearchPath("include/Alembic/AbcMaterial"),
        .define("_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH", .when(platforms: [.windows])),
      ]
    ),

    .target(
      name: "PyBind11",
      dependencies: [
        Arch.OS.python(),
      ],
      publicHeadersPath: "include",
      linkerSettings: [
        .linkedLibrary("python3.10", .when(platforms: Arch.OS.linux.platform)),
      ]
    ),

    .target(
      name: "Blosc",
      dependencies: [
        .target(name: "ZLibDataCompression"),
        .target(name: "ZStandard"),
      ],
      exclude: getConfig(for: .blosc).exclude,
      publicHeadersPath: "include/blosc",
      linkerSettings: [
        .linkedLibrary("pthread", .when(platforms: Arch.OS.linux.platform)),
      ]
    ),

    .target(
      name: "OpenVDB",
      dependencies: [
        .target(name: "Boost"),
        .target(name: "MetaTBB"),
        .target(name: "Blosc"),
        .target(name: "PyBind11"),
        .target(name: "Imath"),
        .target(name: "OpenEXR"),
        .target(name: "ZLibDataCompression"),
        Arch.OS.python(),
      ],
      publicHeadersPath: "include",
      cxxSettings: getConfig(for: .openvdb).cxxSettings,
      linkerSettings: getConfig(for: .openvdb).linkerSettings
    ),

    .target(
      name: "CosmoGraph",
      dependencies: [
        .target(name: "Apple", condition: .when(platforms: Arch.OS.apple.platform)),
        .target(name: "ImGui"),
      ],
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),

    /*
      * Run this from the command line via:
      *
      * swift package --disable-sandbox plugin bundler run -p macOS MetaversalDemo
      *
      * It is pending a pull request which I have submitted here:
      * https://github.com/stackotter/swift-bundler/issues/34
      *
      * Once that is merged, the swift bundler will be included
      * as a swift plugin to all packages that transitively depend
      * on this package, the bundler now also supports the bundling
      * of visionOS and iOS targets, in addition to macOS all from
      * the command line. */
    .executableTarget(
      name: "MetaversalDemo",
      dependencies: [
        .target(name: "ImGui"),
        .target(name: "Imath"),
        .target(name: "OpenEXR"),
        .target(name: "OCIOBundle"),
        .target(name: "OpenColorIO"),
        .target(name: "OpenImageIO"),
        .target(name: "OpenTimelineIO"),
        .target(name: "MaterialX"),
        .target(name: "OpenVDB"),
        .target(name: "OpenSubdiv"),
        .target(name: "Ptex"),
        .target(name: "Draco"),
        Arch.OS.python(),
      ],
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),

    .testTarget(
      name: "MetaverseKitTests",
      dependencies: getConfig(for: .all).dependencies,
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),
  ] + Arch.OS.targets(),
  cLanguageStandard: .gnu17,
  cxxLanguageStandard: .cxx17
)

/* -------------------------------------------------
 * Platform specific, or more lengthy configurations
 * are in the dedicated function below. This is to
 * keep the main package file clean and readable.
 * ------------------------------------------------- */

func getConfig(for target: PkgTarget) -> TargetInfo
{
  var config = TargetInfo()

  switch target
  {
    case .cosmo:
      break
    case .draco:
      break
    case .tbbMallocProxy:
      break
    case .tbbMalloc:
      break
    case .oneTbb:
      break
    case .metaTbb:
      break
    case .zstd:
      break
    case .pystring:
      break
    case .imath:
      break
    case .lzma2:
      config.exclude = ["check/crc32_small.c"]
      config.cSettings = [
        .headerSearchPath("common"),
        .headerSearchPath("lzmacommon"),
        .headerSearchPath("check"),
        .headerSearchPath("delta"),
        .headerSearchPath("lz"),
        .headerSearchPath("lzma"),
        .headerSearchPath("rangecoder"),
        .headerSearchPath("simple"),
        .define("HAVE_STDBOOL_H", to: "1"),
        .define("HAVE_INTTYPES_H", to: "1", .when(platforms: Arch.OS.linux.platform)),
        .define("MYTHREAD_POSIX", to: "1", .when(platforms: Arch.OS.nix.platform)),
        .define("MYTHREAD_VISTA", to: "1", .when(platforms: Arch.OS.windows.platform)),
      ]
    case .yaml:
      break
    case .minizip:
      config.exclude = [
        "mz_crypt_winxp.c",
        "mz_crypt_openssl.c",
        "mz_crypt_apple.c",
        "mz_strm_wzaes.c",
      ]
      #if !os(Windows)
        config.exclude += [
          "mz_strm_os_win32.c",
          "mz_os_win32.c",
          "mz_crypt_winvista.c",
        ]
      #endif /* !os(Windows) */
      #if !os(macOS) && !os(visionOS) && !os(iOS) && !os(tvOS) && !os(watchOS)
        config.exclude += [
          "mz_strm_libcomp.c"
        ]
      #endif /* !os(macOS) && !os(visionOS) && !os(iOS) && !os(tvOS) && !os(watchOS) */
    case .zlib:
      break
    case .raw:
      break
    case .png:
      if Arch.cpuArch.family.contains(.arm)
      {
        config.exclude = [
          "example.c",
          "pngtest.c",
          "intel",
          "powerpc",
        ]
        config.cSettings = [
          .headerSearchPath("."),
          .define("WITH_ARM", to: "1"),
        ]
      }
      else if Arch.cpuArch.family.contains(.x86_64)
      {
        config.exclude = [
          "example.c",
          "pngtest.c",
          "arm",
          "powerpc",
        ]
        config.cSettings = [
          .headerSearchPath("."),
          .define("WITH_INTEL", to: "1"),
        ]
      }
      else if Arch.cpuArch.family.contains(.powerpc)
      {
        config.exclude = [
          "example.c",
          "pngtest.c",
          "arm",
          "intel",
        ]
        config.cSettings = [
          .headerSearchPath("."),
          .define("WITH_POWERPC", to: "1"),
        ]
      }
      else /* a unicorn! 🦄 */
      {
        config.exclude = [
          "example.c",
          "pngtest.c",
          "arm",
          "intel",
          "powerpc",
        ]
        config.cSettings = [
          .headerSearchPath("."),
          .define("WITH_UNICORNS", to: "1"),
        ]
      }
    case .turbojpeg:
      config.exclude = [
        "cjpeg.c",
        "djpeg.c",
        "turbojpeg-jni.c",
        "tjunittest.c",
        "tjexample.c",
        "example.c",
      ]
    case .tiff:
      config.exclude = ["tif_win32.c", "mkg3states.c"]
    case .webp:
      break
    case .openmp:
      config.exclude = [
        "runtime/extractExternal.cpp",
        "runtime/z_Windows_NT_util.cpp",
        "runtime/z_Windows_NT-586_util.cpp",
        "runtime/z_Windows_NT-586_asm.asm",
        "runtime/kmp_stub.cpp",
        "runtime/kmp_import.cpp",
        "runtime/test-touch.c",
      ]
    case .glfw:
      config.exclude = [
        "wl_init.c",
        "wl_window.c",
        "wl_monitor.c",
        "wgl_context.c",
        "null_init.c",
        "null_joystick.c",
        "null_monitor.c",
        "null_window.c",
      ]
      #if !os(Windows)
        config.exclude += [
          "win32_init.c",
          "win32_joystick.c",
          "win32_monitor.c",
          "win32_thread.c",
          "win32_time.c",
          "win32_window.c",
        ]
      #endif /* !os(Windows) */
      #if !os(macOS) && !os(visionOS) && !os(iOS) && !os(tvOS) && !os(watchOS)
        config.exclude += [
          "nsgl_context.m",
          "cocoa_init.m",
          "cocoa_joystick.m",
          "cocoa_window.m",
          "cocoa_monitor.m",
          "cocoa_time.c",
        ]
      #endif /* !os(macOS) && !os(visionOS) && !os(iOS) && !os(tvOS) && !os(watchOS) */
      #if !os(Linux) && !os(Android) && !os(OpenBSD) && !os(FreeBSD)
        config.exclude += [
          "xkb_unicode.c",
          "x11_init.c",
          "x11_monitor.c",
          "x11_window.c",
          "posix_time.c",
          "glx_context.c",
          "linux_joystick.c",
        ]
      #endif /* !os(Linux) && !os(Android) && !os(OpenBSD) && !os(FreeBSD) */
      config.cxxSettings = [
        .headerSearchPath("."),
        .define("_GLFW_COCOA", to: "1", .when(platforms: Arch.OS.apple.platform)),
        .define("_GLFW_X11", to: "1", .when(platforms: Arch.OS.linux.platform)),
        .define("_GLFW_WIN32", to: "1", .when(platforms: Arch.OS.windows.platform)),
        .define("GL_SILENCE_DEPRECATION", to: "1"),
      ]
      #if os(Linux) || os(Android) || os(OpenBSD) || os(FreeBSD)
        config.linkerSettings = [
          .linkedLibrary("glut"),
          .linkedLibrary("GL"),
          .linkedLibrary("GLU"),
          .linkedLibrary("m"),
          .linkedLibrary("X11"),
          .linkedLibrary("Xt"),
        ]
      #endif /* os(Linux) || os(Android) || os(OpenBSD) || os(FreeBSD) */
    case .imgui:
      config.exclude = [
        // no wgpu (for now)
        "backends/ImplWGPU.cpp",
        // no allegro
        "backends/ImplAllegro5.cpp",
        // no android
        "backends/ImplAndroid.cpp",
        // no directx (for now)
        "backends/ImplDX9.cpp",
        "backends/ImplDX10.cpp",
        "backends/ImplDX11.cpp",
        "backends/ImplDX12.cpp",
        // no sdl
        "backends/ImplSDL.cpp",
        "backends/ImplSDLRenderer.cpp",
      ]
      #if !os(macOS) && !os(iOS) && !os(tvOS)
        /* fixup metaversalvulkanframework,
         we should be able to rig moltenvk
         to work on visionOS, but disabled
         for now. */
        config.exclude += [
          // no vulkan (for now)
          "backends/ImplVulkan.cpp",
          "backends/ImplVulkan.h",
        ]
      #endif /* !os(macOS) && !os(iOS) && !os(tvOS) */
      #if !os(Windows)
        config.exclude += [
          // no win32
          "backends/ImplWin32.cpp",
        ]
      #endif /* !os(Windows) */
      #if !os(macOS) && !os(visionOS) && !os(iOS) && !os(tvOS) && !os(watchOS)
        config.exclude += [
          // no metal
          "backends/ImplMetal.mm",
          // no apple os
          "backends/ImplMacOS.mm",
        ]
      #endif /* !os(macOS) && !os(visionOS) && !os(iOS) && !os(tvOS) && !os(watchOS) */
    case .mxGraphEditor:
      #if !os(macOS) && !os(visionOS) && !os(iOS) && !os(tvOS) && !os(watchOS)
        config.exclude = [
          // no objc
          "FileDialog.mm",
        ]
      #endif /* !os(macOS) && !os(visionOS) && !os(iOS) && !os(tvOS) && !os(watchOS) */
    case .mxResources:
      break
    case .materialx:
      config.exclude = [
        "source/JsMaterialX",
        "source/MaterialXView",
      ]
      #if !os(macOS) && !os(visionOS) && !os(iOS) && !os(tvOS) && !os(watchOS)
        /* no metal on linux or windows or wasm */
        config.exclude += [
          "source/MaterialXRenderMsl",
        ]
      #endif /* !os(macOS) && !os(visionOS) && !os(iOS) && !os(tvOS) && !os(watchOS) */
    case .osd:
      config.exclude = [
        /* disabled for now */
        "osd/OpenCLD3D11VertexBuffer.cpp",
        "osd/OpenCLEvaluator.cpp",
        "osd/OpenCLPatchTable.cpp",
        "osd/OpenCLVertexBuffer.cpp",
        "osd/OpenCLGLVertexBuffer.cpp",
        /* peek into apple's late DX3D work
          on macOS, we may want to tap into this. */
        "osd/D3D11ComputeEvaluator.cpp",
        "osd/D3D11LegacyGregoryPatchTable.cpp",
        "osd/D3D11PatchTable.cpp",
        "osd/D3D11VertexBuffer.cpp",
        "osd/CpuD3D11VertexBuffer.cpp",
        /* no CUDA support on macOS */
        "osd/CudaD3D11VertexBuffer.cpp",
        "osd/CudaEvaluator.cpp",
        "osd/CudaGLVertexBuffer.cpp",
        "osd/CudaKernel.cu",
        "osd/CudaPatchTable.cpp",
        "osd/CudaVertexBuffer.cpp",
      ]
      #if !os(macOS) && !os(visionOS) && !os(iOS) && !os(tvOS) && !os(watchOS)
        /* metal is only for darwin. */
        config.exclude += [
          "osd/MTLComputeEvaluator.mm",
          "osd/MTLLegacyGregoryPatchTable.mm",
          "osd/MTLMesh.mm",
          "osd/MTLPatchShaderSource.mm",
          "osd/MTLPatchTable.mm",
          "osd/MTLVertexBuffer.mm",
        ]
      #endif /* !os(macOS) && !os(visionOS) && !os(iOS) && !os(tvOS) && !os(watchOS) */
      config.cxxSettings = [
        .headerSearchPath("glLoader"),
      ]
    case .exr:
      break
    case .oiioUtil:
      config.exclude = [
        "nuke",
        "iv",
      ]
    case .oiio:
      config.exclude = [
        "jpeg2000.imageio",
        "heif.imageio",
        "gif.imageio",
        "ffmpeg.imageio",
        "dicom.imageio",
        "bmp.imageio",
        "term.imageio",
        "psd.imageio",
        "pnm.imageio",
        "oiiotool",
      ]
    case .ocioBundle:
      break
    case .ocio:
      #if !os(macOS) && !os(visionOS) && !os(iOS) && !os(tvOS) && !os(watchOS)
        config.exclude = [
          "SystemMonitor_macos.cpp",
        ]
      #endif /* !os(macOS) && !os(visionOS) && !os(iOS) && !os(tvOS) && !os(watchOS) */
      #if !os(Windows)
        config.exclude += [
          "SystemMonitor_windows.cpp",
        ]
      #endif /* !os(Windows) */
    case .mpy:
      config.exclude = [
        "PyAlembic/msvc14fixes.cpp",
        "PyOpenVDB",
        "PyMaterialX",
      ]
    case .ptex:
      break
    case .hdf5:
      break
    case .alembic:
      break
    case .pybind11:
      break
    case .blosc:
      #if !os(Windows)
        config.exclude = ["include/win32"]
      #endif /* !os(Windows) */
    case .openvdb:
      config.cxxSettings = [
        .headerSearchPath("include/openvdb/io"),
        .headerSearchPath("include/openvdb/points"),
        .headerSearchPath("include/openvdb/thread"),
        .headerSearchPath("include/openvdb/tools"),
        .headerSearchPath("include/openvdb/tree"),
        .headerSearchPath("include/openvdb/util"),
        /* -------------------------------------------
           NOTICE: the lack of the math dir search path
           otherwise, openvdbs math.h header will clash
           with the system stdlib math.h header. Source
           files within openvdb (***.cc) which included
           math have been prefixed to math/(***.h), and
           original openvdb headers remain unchanged.
          ------------------------------------------- */
        .headerSearchPath("include/openvdb"),
        .define("OPENVDB_USE_DELAYED_LOADING", to: "1"),
        .define("OPENVDB_USE_BLOSC", to: "1"),
        .define("OPENVDB_USE_ZLIB", to: "1"),
      ]
      #if os(Linux) || os(Android) || os(OpenBSD) || os(FreeBSD)
        config.linkerSettings = [
          .linkedLibrary("boost_system"),
          .linkedLibrary("boost_filesystem"),
          .linkedLibrary("boost_thread"),
          .linkedLibrary("boost_regex"),
          .linkedLibrary("boost_date_time"),
          .linkedLibrary("boost_chrono"),
          .linkedLibrary("boost_atomic"),
          .linkedLibrary("boost_iostreams"),
          .linkedLibrary("boost_program_options"),
          .linkedLibrary("boost_python310"),
        ]
      #endif /* os(Linux) || os(Android) || os(OpenBSD) || os(FreeBSD) */
    case .boost:
      break
    case .python:
      break
    case .demo:
      break
    case .all:
      config.products = [
        .library(
          name: "CosmoGraph",
          targets: ["CosmoGraph"]
        ),
        .library(
          name: "OneTBB",
          targets: ["OneTBB"]
        ),
        .library(
          name: "MetaTBB",
          targets: ["MetaTBB"]
        ),
        .library(
          name: "TBBMalloc",
          targets: ["TBBMalloc"]
        ),
        .library(
          name: "TBBMallocProxy",
          targets: ["TBBMallocProxy"]
        ),
        .library(
          name: "Apple",
          targets: ["Apple"]
        ),
        .library(
          name: "DEFLATE",
          targets: ["DEFLATE"]
        ),
        .library(
          name: "MetaPy",
          targets: ["MetaPy"]
        ),
        .library(
          name: "HDF5",
          targets: ["HDF5"]
        ),
        .library(
          name: "OpenVDB",
          targets: ["OpenVDB"]
        ),
        .library(
          name: "OCIOBundle",
          targets: ["OCIOBundle"]
        ),
        .library(
          name: "OpenColorIO",
          targets: ["OpenColorIO", "OCIOBundle"]
        ),
        .library(
          name: "OpenImageIO_Util",
          targets: ["OpenImageIO_Util"]
        ),
        .library(
          name: "OpenImageIO",
          targets: ["OpenImageIO"]
        ),
        .library(
          name: "OpenTimelineIO",
          targets: ["any", "nonstd", "rapidjson", "OpenTime", "OpenTimelineIO"]
        ),
        .library(
          name: "OpenEXR",
          targets: ["OpenEXR"]
        ),
        .library(
          name: "OpenSubdiv",
          targets: ["OpenSubdiv"]
        ),
        .library(
          name: "OpenMP",
          targets: ["OpenMP"]
        ),
        .library(
          name: "GLFW",
          targets: ["GLFW"]
        ),
        .library(
          name: "Imath",
          targets: ["pystring", "Imath"]
        ),
        .library(
          name: "ImGui",
          targets: ["ImGui"]
        ),
        .library(
          name: "MaterialX",
          targets: ["MaterialX"]
        ),
        .library(
          name: "WebP",
          targets: ["WebP"]
        ),
        .library(
          name: "TurboJPEG",
          targets: ["TurboJPEG"]
        ),
        .library(
          name: "Raw",
          targets: ["Raw"]
        ),
        .library(
          name: "TIFF",
          targets: ["TIFF"]
        ),
        .library(
          name: "LibPNG",
          targets: ["LibPNG"]
        ),
        .library(
          name: "ZStandard",
          targets: ["ZStandard"]
        ),
        .library(
          name: "ZLibDataCompression",
          targets: ["ZLibDataCompression"]
        ),
        .library(
          name: "LZMA2",
          targets: ["LZMA2"]
        ),
        .library(
          name: "MiniZip",
          targets: ["MiniZip"]
        ),
        .library(
          name: "Yaml",
          targets: ["Yaml"]
        ),
        .library(
          name: "PyBind11",
          targets: ["PyBind11"]
        ),
        .library(
          name: "Blosc",
          targets: ["Blosc"]
        ),
        .library(
          name: "Ptex",
          targets: ["Ptex"]
        ),
        .library(
          name: "Alembic",
          targets: ["Alembic"]
        ),
        .library(
          name: "Eigen",
          targets: ["Eigen"]
        ),
        .library(
          name: "Draco",
          targets: ["Draco"]
        ),
        .library(
          name: "MXResources",
          targets: ["MXResources"]
        ),
        .executable(
          name: "MXGraphEditor",
          targets: ["MXGraphEditor"]
        ),
        .executable(
          name: "MetaversalDemo",
          targets: [
            "MetaversalDemo",
            "OpenImageIO",
            "OpenImageIO_Util",
          ]
        ),
      ]

      config.products += Arch.OS.products()

      config.dependencies = config.products.map
      {
        .init(stringLiteral: $0.name)
      }
  }

  return config
}

/** Platforms, grouped by OS. */
enum Arch
{
  static let hostTriplet: String = "\(cpuArch)-\(host)-\(device)"

  enum OS
  {
    /// apple devices (macOS, visionOS, iOS, etc).
    case apple
    /// linux distros (ubuntu, centos, etc).
    case linux
    /// microsoft windows.
    case windows
    /// web (wasm).
    case web
    /// everything not windows (apple + linux).
    case nix

    public var platform: [Platform]
    {
      switch self
      {
        case .apple: [.macOS, .iOS, .visionOS, .tvOS, .watchOS]
        case .linux: [.linux, .android, .openbsd]
        case .windows: [.windows]
        case .web: [.wasi]
        case .nix: [.macOS, .iOS, .visionOS, .tvOS, .watchOS, .linux, .android, .openbsd]
      }
    }

    public static func packageDeps() -> [Package.Dependency]
    {
      #if os(macOS) || os(visionOS) || os(iOS) || os(tvOS) || os(watchOS)
        [
          .package(url: "https://github.com/wabiverse/MetaversePythonFramework", from: "3.11.7"),
          .package(url: "https://github.com/wabiverse/MetaverseVulkanFramework", from: "1.26.2"),
        ]
      #else /* os(Linux) || os(Android) || os(OpenBSD) || os(FreeBSD) || os(Windows) || os(Cygwin) || os(WASI) */
        []
      #endif /* os(Linux) || os(Android) || os(OpenBSD) || os(FreeBSD) || os(Windows) || os(Cygwin) || os(WASI) */
    }

    public static func targets() -> [Target]
    {
      #if os(macOS) || os(visionOS) || os(iOS) || os(tvOS) || os(watchOS)
        var targs: [Target] = [
          .binaryTarget(
            name: "Boost",
            url: "https://github.com/wabiverse/MetaverseBoostFramework/releases/download/1.81.4/boost.xcframework.zip",
            checksum: "2636f77d3ee22507da4484d7b5ab66645a08b196c0fca8a7af28d36c6948404e"
          ),
        ]
      #elseif os(Linux) || os(Android) || os(OpenBSD) || os(FreeBSD) || os(Windows) || os(Cygwin)
        var targs: [Target] = [
          .systemLibrary(
            name: "Boost",
            pkgConfig: "boost",
            providers: [
              .apt(["libboost-all-dev"]),
              .yum(["boost-devel"]),
            ]
          ),

          .systemLibrary(
            name: "BZ2",
            pkgConfig: "libbz2",
            providers: [
              .apt(["libbz2-dev"]),
              .yum(["bzip2-devel"]),
            ]
          ),

          .systemLibrary(
            name: "Python",
            pkgConfig: "python3",
            providers: [
              .apt(["python3-dev"]),
              .yum(["python3-devel"]),
            ]
          ),
        ]
      #else /* os(WASI) */
        var targs: [Target] = []
      #endif /* os(WASI) */

      // targets that depend on arch not os.
      if Arch.cpuArch.family.contains(.arm)
      {
        targs.append(
          .target(
            name: "sse2neon",
            publicHeadersPath: "include"
          )
        )
      }

      return targs
    }

    public static func products() -> [Product]
    {
      #if os(macOS) || os(visionOS) || os(iOS) || os(tvOS) || os(watchOS)
        var prods: [Product] = [
          .library(name: "Boost", targets: ["Boost"]),
        ]
      #else /* os(Linux) || os(Android) || os(OpenBSD) || os(FreeBSD) || os(Windows) || os(Cygwin) || os(WASI) */
        var prods: [Product] = []
      #endif /* os(Linux) || os(Android) || os(OpenBSD) || os(FreeBSD) || os(Windows) || os(Cygwin) || os(WASI) */

      // products that depend on arch not os.
      if Arch.cpuArch.family.contains(.arm)
      {
        prods.append(
          .library(
            name: "sse2neon",
            targets: ["sse2neon"]
          )
        )
      }

      return prods
    }

    public static func ocioDeps() -> [Target.Dependency]
    {
      // only add sse2neon on arm arch.
      let sse2neon: [Target.Dependency] = Arch.cpuArch.family.contains(.arm) ? [.target(name: "sse2neon")] : []

      #if os(Linux) || os(Android) || os(OpenBSD) || os(FreeBSD) || os(Windows) || os(Cygwin)
        return [
          .target(name: "pystring"),
          .target(name: "Imath"),
          .target(name: "OpenEXR"),
          .target(name: "MiniZip"),
          .target(name: "Yaml"),
          .target(name: "Python")
        ] + sse2neon
      #else /* os(macOS) || os(visionOS) || os(iOS) || os(tvOS) || os(watchOS) || os(WASI) */
        return [
          .target(name: "pystring"),
          .target(name: "Imath"),
          .target(name: "OpenEXR"),
          .target(name: "MiniZip"),
          .target(name: "Yaml"),
          .product(name: "Python", package: "MetaversePythonFramework", condition: .when(platforms: Arch.OS.apple.platform))
        ] + sse2neon
      #endif /* os(macOS) || os(visionOS) || os(iOS) || os(tvOS) || os(watchOS) || os(Windows) || os(Cygwin) || os(WASI) */
    }

    public static func python() -> Target.Dependency
    {
      #if os(macOS) || os(visionOS) || os(iOS) || os(tvOS) || os(watchOS)
        .product(name: "Python", package: "MetaversePythonFramework", condition: .when(platforms: Arch.OS.apple.platform))
      #else /* os(Linux) || os(Android) || os(OpenBSD) || os(FreeBSD) || os(Windows) || os(Cygwin) || os(WASI) */
        .target(name: "Python")
      #endif /* os(Linux) || os(Android) || os(OpenBSD) || os(FreeBSD) || os(Windows) || os(Cygwin) || os(WASI) */
    }

    public static func minizipDeps() -> [Target.Dependency]
    {
      #if os(Linux) || os(Android) || os(OpenBSD) || os(FreeBSD)
        [
          .target(name: "LZMA2"),
          .target(name: "ZLibDataCompression"),
          .target(name: "ZStandard"),
          .target(name: "BZ2"),
        ]
      #else /* os(macOS) || os(visionOS) || os(iOS) || os(tvOS) || os(watchOS) || os(Windows) || os(Cygwin) || os(WASI) */
        [
          .target(name: "LZMA2"),
          .target(name: "ZLibDataCompression"),
          .target(name: "ZStandard"),
        ]
      #endif /* os(macOS) || os(visionOS) || os(iOS) || os(tvOS) || os(watchOS) || os(Windows) || os(Cygwin) || os(WASI) */
    }

    public static func glfwDeps() -> [Target.Dependency]
    {
      #if os(macOS) || os(visionOS) || os(iOS) || os(tvOS) || os(watchOS)
        [
          .target(name: "Apple"),
          .product(name: "MoltenVK", package: "MetaverseVulkanFramework", condition: .when(platforms: [.macOS, .iOS, .tvOS])),
        ]
      #else /* os(Linux) || os(Android) || os(OpenBSD) || os(FreeBSD) || os(Windows) || os(Cygwin) || os(WASI) */
        []
      #endif /* os(Linux) || os(Android) || os(OpenBSD) || os(FreeBSD) || os(Windows) || os(Cygwin) || os(WASI) */
    }
  }

  enum CPU: String
  {
    case arm
    case arm64
    case aarch64
    case x86_64
    case i386
    case powerpc
    case powerpc64
    case powerpc64le
    case s390x
    case wasm32
    case arm64_32

    public var family: [CPU]
    {
      switch self
      {
        case .arm, .arm64, .aarch64, .arm64_32: [.arm, .arm64, .aarch64, .arm64_32]
        case .x86_64, .i386: [.x86_64, .i386]
        case .powerpc, .powerpc64, .powerpc64le: [.powerpc, .powerpc64, .powerpc64le]
        case .s390x: [.s390x]
        case .wasm32: [.wasm32]
      }
    }
  }

  /* -------------------------------------------------------------------------
   * To detect platform triplets, we need to know the OS and CPU architecture.
   * -------------------------------------------------------------------------*/

  #if os(macOS) || os(visionOS) || os(iOS) || os(tvOS) || os(watchOS)
    static let host = "apple"
    #if os(macOS)
      static let device = "macosx"
    #elseif os(visionOS)
      static let device = "xros"
    #elseif os(iOS)
      static let device = "ios"
    #elseif os(tvOS)
      static let device = "tvos"
    #elseif os(watchOS)
      static let device = "watchos"
    #endif /* os(watchOS) */
  #elseif os(Linux) || os(Android) || os(OpenBSD) || os(FreeBSD)
    static let host = "unknown-linux"
    #if os(Android)
      static let device = "android"
    #else /* os(Linux) || os(OpenBSD) || os(FreeBSD) */
      static let device = "gnu"
    #endif /* os(Linux) || os(OpenBSD) || os(FreeBSD) */
  #elseif os(Windows)
    static let host = "windows"
    static let device = "windows"
  #elseif os(Cygwin)
    static let host = "cygwin"
    static let device = "windows"
  #elseif os(WASI)
    static let host = "wasi"
    static let device = "wasi"
  #endif /* os(WASI) */

  #if arch(arm64)
    #if os(Linux) || os(Android) || os(OpenBSD) || os(FreeBSD)
      static let cpuArch: CPU = .aarch64
    #else /* !os(Linux) && !os(Android) && !os(OpenBSD) && !os(FreeBSD) */
      static let cpuArch: CPU = .arm64
    #endif /* !os(Linux) && !os(Android) && !os(OpenBSD) && !os(FreeBSD) */
  #elseif arch(x86_64)
    static let cpuArch: CPU = .x86_64
  #elseif arch(i386)
    static let cpuArch: CPU = .i386
  #elseif arch(arm)
    static let cpuArch: CPU = .arm
  #elseif arch(powerpc64)
    static let cpuArch: CPU = .powerpc64
  #elseif arch(powerpc64le)
    static let cpuArch: CPU = .powerpc64le
  #elseif arch(powerpc)
    static let cpuArch: CPU = .powerpc
  #elseif arch(s390x)
    static let cpuArch: CPU = .s390x
  #elseif arch(wasm32)
    static let cpuArch: CPU = .wasm32
  #elseif arch(arm64_32)
    static let cpuArch: CPU = .arm64_32
  #endif /* arch(arm64_32) */
}

enum PkgTarget: String
{
  case cosmo = "CosmoGraph"
  case draco = "Draco"
  case tbbMallocProxy = "TBBMallocProxy"
  case tbbMalloc = "TBBMalloc"
  case oneTbb = "OneTBB"
  case metaTbb = "MetaTBB"
  case zstd = "ZStandard"
  case lzma2 = "LZMA2"
  case yaml = "Yaml"
  case minizip = "MiniZip"
  case zlib = "ZLibDataCompression"
  case raw = "Raw"
  case png = "LibPNG"
  case turbojpeg = "TurboJPEG"
  case tiff = "TIFF"
  case webp = "WebP"
  case openmp = "OpenMP"
  case glfw = "GLFW"
  case imgui = "ImGui"
  case pystring
  case imath = "Imath"
  case mxGraphEditor = "MXGraphEditor"
  case mxResources = "MXResources"
  case materialx = "MaterialX"
  case osd = "OpenSubdiv"
  case exr = "OpenEXR"
  case oiio = "OpenImageIO"
  case oiioUtil = "OpenImageIO_Util"
  case ocio = "OpenColorIO"
  case ocioBundle = "OCIOBundle"
  case mpy = "MetaPy"
  case ptex = "Ptex"
  case hdf5 = "HDF5"
  case alembic = "Alembic"
  case pybind11 = "PyBind11"
  case blosc = "Blosc"
  case openvdb = "OpenVDB"
  case boost = "Boost"
  case python = "Python"
  case demo = "MetaversalDemo"
  case all
}

struct TargetInfo
{
  var dependencies: [Target.Dependency] = []
  var products: [Product] = []
  var exclude: [String] = []
  var publicHeadersPath: String = ""
  var resources: [Resource] = []
  var cSettings: [CSetting] = []
  var cxxSettings: [CXXSetting] = []
  var swiftSettings: [SwiftSetting] = []
  var linkerSettings: [LinkerSetting] = []
}
