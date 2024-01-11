// swift-tools-version: 5.9

import PackageDescription

/* ------------------------------------------------------
 *  :: :  💫 The Open Source Metaverse  :   ::
 * ------------------------------------------------------ */

let package = Package(
  name: "MetaverseKit",
  platforms: [
    .macOS(.v12),
    .visionOS(.v1),
    .iOS(.v12),
    .tvOS(.v12),
    .watchOS(.v4),
  ],
  products: getConfig(for: .all).products,
  dependencies: getPlatformDeps(),
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
      exclude: getConfig(for: .draco).exclude,
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
      ]
    ),

    .target(
      name: "OneTBB",
      publicHeadersPath: "include",
      cxxSettings: [
        .define("_XOPEN_SOURCE", to: "1", .when(platforms: Arch.OS.apple.platform)),
        .define("TBB_ALLOCATOR_TRAITS_BROKEN", to: "1", .when(platforms: Arch.OS.linux.platform)),
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
      publicHeadersPath: "include"
    ),

    .target(
      name: "OpenSSL",
      dependencies: [],
      exclude: [],
      publicHeadersPath: "include",
      cSettings: [
        .define("BORINGSSL_NO_STATIC_INITIALIZER", to: "1"),
      ]
    ),

    .target(
      name: "MiniZip",
      dependencies: [
        .target(name: "LZMA2"),
        .target(name: "ZLibDataCompression"),
        .target(name: "ZStandard"),
        .target(name: "OpenSSL"),
      ] + getBZ2(),
      exclude: getConfig(for: .minizip).exclude,
      publicHeadersPath: "include",
      cSettings: [
        .define("HAVE_ZLIB", to: "1"),
        .define("ZLIB_COMPAT", to: "1"),
        .define("HAVE_WZAES", to: "1"),
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
        .define("HAVE_UNISTD_H", to: "1"),
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
      dependencies: [
        .target(name: "OpenSSL"),
      ],
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
        .target(name: "WebP"),
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
        getPython(),
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
      name: "GLFW",
      dependencies: getGLFWDeps(),
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
        .linkedFramework("Cocoa", .when(platforms: Arch.OS.apple.platform)),
        .linkedFramework("GLUT", .when(platforms: Arch.OS.apple.platform)),
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
        .target(name: "PyBind11"),
        .target(name: "MXResources"),
        getPython(),
      ],
      exclude: getConfig(for: .materialx).exclude,
      publicHeadersPath: "include"
    ),

    .executableTarget(
      name: "MXGraphEditor",
      dependencies: [
        .target(name: "MaterialX"),
      ],
      exclude: getConfig(for: .mxGraphEditor).exclude
    ),

    // .target(
    //   name: "GPUShaders",
    //   resources: [
    //     .process("Metal"),
    //     .process("GL"),
    //     .process("DX3D"),
    //   ],
    //   swiftSettings: [
    //     .interoperabilityMode(.Cxx),
    //   ]
    // ),

    .target(
      name: "OpenSubdiv",
      dependencies: [
        .target(name: "MetaTBB"),
        .target(name: "OpenMP"),
        // .target(name: "GPUShaders"),
      ],
      exclude: getConfig(for: .osd).exclude,
      publicHeadersPath: "include",
      cxxSettings: getConfig(for: .osd).cxxSettings
    ),

    .target(
      name: "OpenEXR",
      dependencies: getDEFLATE(),
      publicHeadersPath: "include",
      cSettings: [
        .headerSearchPath("."),
        .headerSearchPath("OpenEXRCore"),
        .headerSearchPath("include/OpenEXR"),
      ],
      linkerSettings: [
        .linkedLibrary("deflate", .when(platforms: Arch.OS.linux.platform)),
      ]
    ),

    .target(
      name: "OpenImageIO",
      dependencies: [
        .target(name: "WebP"),
        .target(name: "TIFF"),
        .target(name: "Raw"),
        .target(name: "Ptex"),
        .target(name: "LibPNG"),
        .target(name: "OpenVDB"),
        .target(name: "OpenEXR"),
        .target(name: "PyBind11"),
      ],
      exclude: getConfig(for: .oiio).exclude,
      publicHeadersPath: "include",
      cxxSettings: [
        .headerSearchPath("."),
        .headerSearchPath("include/OpenImageIO/detail"),
        .headerSearchPath("libOpenImageIO"),
      ],
      linkerSettings: getConfig(for: .oiio).linkerSettings
    ),

    .target(
      name: "OpenColorIO",
      dependencies: [
        .target(name: "OpenEXR"),
        .target(name: "MiniZip"),
        .target(name: "Yaml"),
        getPython(),
      ],
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
      name: "MetaPy",
      dependencies: [
        .target(name: "Boost"),
        .target(name: "OpenEXR"),
        .target(name: "OpenVDB"),
        .target(name: "Alembic"),
        .target(name: "OpenImageIO"),
        .target(name: "MaterialX"),
        getPython(),
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
        .target(name: "OpenEXR"),
        getPython(),
      ],
      publicHeadersPath: "include",
      cxxSettings: [
        .headerSearchPath("include/Alembic/AbcMaterial"),
      ]
    ),

    .target(
      name: "PyBind11",
      dependencies: [
        getPython(),
      ],
      publicHeadersPath: "include",
      linkerSettings: [
        .linkedLibrary("python3.11", .when(platforms: Arch.OS.linux.platform)),
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
        .target(name: "OpenEXR"),
        .target(name: "ZLibDataCompression"),
        getPython(),
      ],
      publicHeadersPath: "include",
      cxxSettings: getConfig(for: .openvdb).cxxSettings,
      linkerSettings: getConfig(for: .openvdb).linkerSettings
    ),

    /*
      * Run this from the command line via:
      *
      * swift bundler run -p macOS MetaversalDemo
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
  ] + getPlatformTargets(),
  cLanguageStandard: .gnu17,
  cxxLanguageStandard: .cxx17
)

/* -------------------------------------------------
 * Platform specific, or more lengthy configurations
 * are in the dedicated function below. This is to
 * keep the main package file clean and readable.
 * ------------------------------------------------- */

func getPlatformTargets() -> [Target]
{
  #if os(macOS)
    return [
      .binaryTarget(
        name: "Boost",
        url: "https://github.com/wabiverse/MetaverseBoostFramework/releases/download/1.81.4/boost.xcframework.zip",
        checksum: "2636f77d3ee22507da4484d7b5ab66645a08b196c0fca8a7af28d36c6948404e"
      ),

      .target(
        name: "DEFLATE",
        publicHeadersPath: "include",
        cxxSettings: [
          .headerSearchPath("."),
          .define("HAVE_UNISTD_H", to: "1"),
          .define("Z_HAVE_STDARG_H", to: "1"),
        ],
        linkerSettings: [
          .linkedLibrary("z", .when(platforms: Arch.OS.linux.platform)),
        ]
      ),
    ]
  #else
    return [
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

      .systemLibrary(
        name: "ZLib",
        pkgConfig: "zlib",
        providers: [
          .apt(["zlib1g-dev"]),
          .yum(["zlib-devel"]),
        ]
      ),
    ]
  #endif
}

func getDEFLATE() -> [Target.Dependency]
{
  #if os(macOS)
    return [
      .target(name: "DEFLATE"),
    ]
  #else
    return []
  #endif
}

func getBZ2() -> [Target.Dependency]
{
  #if os(macOS)
    return []
  #else
    return [
      .target(name: "BZ2", condition: .when(platforms: Arch.OS.linux.platform)),
    ]
  #endif
}

func getGLFWDeps() -> [Target.Dependency]
{
  #if os(macOS)
    return [
      .target(name: "Apple", condition: .when(platforms: Arch.OS.apple.platform)),
      .product(name: "MoltenVK", package: "MetaverseVulkanFramework", condition: .when(platforms: Arch.OS.apple.platform)),
    ]
  #else
    return []
  #endif
}

func getPlatformDeps() -> [Package.Dependency]
{
  #if os(macOS)
    return [
      .package(url: "https://github.com/furby-tm/swift-bundler", from: "2.0.8"),
      .package(url: "https://github.com/wabiverse/MetaversePythonFramework", from: "3.11.4"),
      .package(url: "https://github.com/wabiverse/MetaverseVulkanFramework", from: "1.26.1"),
    ]
  #else
    return [
      .package(url: "https://github.com/furby-tm/swift-bundler", from: "2.0.8"),
    ]
  #endif
}

func getPython() -> Target.Dependency
{
  #if os(macOS)
    .product(name: "Python", package: "MetaversePythonFramework", condition: .when(platforms: Arch.OS.apple.platform))
  #else
    .target(name: "Python")
  #endif
}

func getConfig(for target: PkgTarget) -> TargetInfo
{
  var chipsetExcludeDirs: [String] = []
  var chipsetDefinesC: [CSetting] = []
  var chipsetDefinesCXX: [CXXSetting] = []
  if Arch.cpuArch.family.contains(.arm)
  {
    chipsetExcludeDirs.append("intel")
    chipsetExcludeDirs.append("powerpc")
    chipsetDefinesC.append(.define("WITH_ARM", to: "1"))
    chipsetDefinesCXX.append(.define("WITH_ARM", to: "1"))
  }
  else if Arch.cpuArch.family.contains(.x86_64)
  {
    chipsetExcludeDirs.append("arm")
    chipsetExcludeDirs.append("powerpc")
    chipsetDefinesC.append(.define("WITH_INTEL", to: "1"))
    chipsetDefinesCXX.append(.define("WITH_INTEL", to: "1"))
  }
  else if Arch.cpuArch.family.contains(.powerpc)
  {
    chipsetExcludeDirs.append("arm")
    chipsetExcludeDirs.append("intel")
    chipsetDefinesC.append(.define("WITH_POWERPC", to: "1"))
    chipsetDefinesCXX.append(.define("WITH_POWERPC", to: "1"))
  }
  else /* a unicorn! 🦄 */
  {
    chipsetExcludeDirs.append("arm")
    chipsetExcludeDirs.append("intel")
    chipsetExcludeDirs.append("powerpc")
    chipsetDefinesC.append(.define("WITH_UNICORNS", to: "1"))
    chipsetDefinesCXX.append(.define("WITH_UNICORNS", to: "1"))
  }

  var config = TargetInfo()

  switch target
  {
    case .draco:
      config.exclude = [
        "include/draco/animation/keyframe_animation_encoding_test.cc",
        "include/draco/animation/keyframe_animation_test.cc",
        "include/draco/attributes/point_attribute_test.cc",
        "include/draco/point_cloud/point_cloud_test.cc",
        "include/draco/metadata/metadata_test.cc",
        "include/draco/metadata/metadata_encoder_test.cc",
        "include/draco/point_cloud/point_cloud_builder_test.cc",
        "include/draco/unity/draco_unity_plugin_test.cc",
        "include/draco/mesh/mesh_test.cc",
        "include/draco/mesh/triangle_soup_mesh_builder_test.cc",
        "include/draco/mesh/mesh_are_equivalent_test.cc",
        "include/draco/mesh/corner_table_test.cc",
        "include/draco/mesh/mesh_cleanup_test.cc",
        "include/draco/io/stdio_file_writer_test.cc",
        "include/draco/io/stdio_file_reader_test.cc",
        "include/draco/io/stl_decoder_test.cc",
        "include/draco/io/ply_reader_test.cc",
        "include/draco/io/point_cloud_io_test.cc",
        "include/draco/io/stl_encoder_test.cc",
        "include/draco/io/obj_decoder_test.cc",
        "include/draco/io/obj_encoder_test.cc",
        "include/draco/io/ply_decoder_test.cc",
        "include/draco/io/file_reader_factory_test.cc",
        "include/draco/io/file_writer_factory_test.cc",
        "include/draco/io/file_utils_test.cc",
        "include/draco/io/file_writer_utils_test.cc",
        "include/draco/core/status_test.cc",
        "include/draco/core/vector_d_test.cc",
        "include/draco/core/math_utils_test.cc",
        "include/draco/core/quantization_utils_test.cc",
        "include/draco/core/buffer_bit_coding_test.cc",
        "include/draco/core/draco_test_utils.cc",
        "include/draco/compression/point_cloud/point_cloud_kd_tree_encoding_test.cc",
        "include/draco/compression/point_cloud/point_cloud_sequential_encoding_test.cc",
        "include/draco/compression/mesh/mesh_encoder_test.cc",
        "include/draco/compression/mesh/mesh_edgebreaker_encoding_test.cc",
        "include/draco/compression/entropy/symbol_coding_test.cc",
        "include/draco/compression/entropy/shannon_entropy_test.cc",
        "include/draco/compression/encode_test.cc",
        "include/draco/compression/decode_test.cc",
        "include/draco/compression/config/decoder_options_test.cc",
        "include/draco/compression/bit_coders/rans_coding_test.cc",
        "include/draco/compression/attributes/sequential_integer_attribute_encoding_test.cc",
        "include/draco/compression/attributes/prediction_schemes/prediction_scheme_normal_octahedron_canonicalized_transform_test.cc",
        "include/draco/compression/attributes/prediction_schemes/prediction_scheme_normal_octahedron_transform_test.cc",
        "include/draco/compression/attributes/point_d_vector_test.cc",
        "include/draco/javascript",
        "include/draco/tools",
      ]
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
        .define("MYTHREAD_POSIX", to: "1", .when(platforms: Arch.OS.apple.platform + Arch.OS.linux.platform)),
        .define("MYTHREAD_VISTA", to: "1", .when(platforms: Arch.OS.windows.platform)),
      ]
    case .yaml:
      break
    case .openssl:
      break
    case .minizip:
      config.exclude = [
        "mz_crypt_winxp.c",
      ]
      #if os(macOS)
        config.exclude += [
          "mz_crypt_openssl.c",
        ]
      #endif /* os(macOS) */
      #if !os(Windows)
        config.exclude += [
          "mz_strm_os_win32.c",
          "mz_os_win32.c",
          "mz_crypt_winvista.c",
        ]
      #endif
      #if !os(macOS)
        config.exclude += [
          "mz_strm_libcomp.c",
          "mz_crypt_apple.c",
        ]
      #endif
    case .zlib:
      break
    case .raw:
      break
    case .png:
      config.exclude = ["example.c", "pngtest.c"] + chipsetExcludeDirs
      config.cSettings = [.headerSearchPath(".")] + chipsetDefinesC
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
    case .apple:
      break
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
      #if !os(macOS)
        config.exclude += [
          "nsgl_context.m",
          "cocoa_init.m",
          "cocoa_joystick.m",
          "cocoa_window.m",
          "cocoa_monitor.m",
          "cocoa_time.c",
        ]
      #endif /* !os(macOS) */
      #if !os(Linux)
        config.exclude += [
          "xkb_unicode.c",
          "x11_init.c",
          "x11_monitor.c",
          "x11_window.c",
          "posix_time.c",
          "glx_context.c",
          "linux_joystick.c",
        ]
      #endif /* !os(Linux) */
      config.cxxSettings = [
        .headerSearchPath("."),
        .define("_GLFW_COCOA", to: "1", .when(platforms: Arch.OS.apple.platform)),
        .define("_GLFW_X11", to: "1", .when(platforms: Arch.OS.linux.platform)),
        .define("_GLFW_WIN32", to: "1", .when(platforms: Arch.OS.windows.platform)),
        .define("GL_SILENCE_DEPRECATION", to: "1"),
      ]
      #if os(Linux)
        config.linkerSettings = [
          .linkedLibrary("glut"),
          .linkedLibrary("GL"),
          .linkedLibrary("GLU"),
          .linkedLibrary("m"),
          .linkedLibrary("X11"),
          .linkedLibrary("Xt"),
        ]
      #endif /* os(Linux) */
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
      #if os(Linux)
        config.exclude += [
          // no vulkan (for now)
          "backends/ImplVulkan.cpp",
          "backends/ImplVulkan.h",
        ]
      #endif /* !os(Windows) */
      #if !os(Windows)
        config.exclude += [
          // no win32
          "backends/ImplWin32.cpp",
        ]
      #endif /* !os(Windows) */
      #if !os(macOS)
        config.exclude += [
          // no metal
          "backends/ImplMetal.mm",
          // no apple os
          "backends/ImplMacOS.mm",
        ]
      #endif /* !os(macOS) */
    case .mxGraphEditor:
      #if !os(macOS)
        config.exclude = [
          // no objc
          "FileDialog.mm",
        ]
      #endif /* !os(macOS) */
    case .mxResources:
      break
    case .materialx:
      config.exclude = [
        "source/JsMaterialX",
        "source/MaterialXView",
      ]
      #if !os(macOS)
        /* no metal on linux or windows */
        config.exclude += [
          "source/MaterialXRenderMsl",
        ]
      #endif
    // case .gpuShaders:
    //   break
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
      #if !os(macOS)
        /* metal is only for darwin. */
        config.exclude += [
          "osd/MTLComputeEvaluator.mm",
          "osd/MTLLegacyGregoryPatchTable.mm",
          "osd/MTLMesh.mm",
          "osd/MTLPatchShaderSource.mm",
          "osd/MTLPatchTable.mm",
          "osd/MTLVertexBuffer.mm",
        ]
      #endif /* !os(macOS) */
      config.cxxSettings = [
        .headerSearchPath("glLoader"),
        /* autogenerated shader headers. */
        .define("GPU_SHADERS_SWIFT_OSD_H", to: "../../.build/\(Arch.hostTriplet)/debug/GPUShaders.build/GPUShaders-Swift.h", .when(configuration: .debug)),
        .define("GPU_SHADERS_SWIFT_OSD_H", to: "../../.build/\(Arch.hostTriplet)/release/GPUShaders.build/GPUShaders-Swift.h", .when(configuration: .release)),
      ]
    case .exr:
      break
    case .oiio:
      config.exclude = [
        "nuke",
        "jpeg2000.imageio",
        "iv",
        "heif.imageio",
        "gif.imageio",
        "ffmpeg.imageio",
        "dicom.imageio",
        "cineon.imageio",
      ]
    case .ocio:
      #if !os(macOS)
        config.exclude = [
          "SystemMonitor_macos.cpp",
        ]
      #endif
      #if !os(Windows)
        config.exclude += [
          "SystemMonitor_windows.cpp",
        ]
      #endif
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
    case .deflate:
      break
    case .blosc:
      #if !os(Windows)
        config.exclude = ["include/win32"]
      #endif
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
      #if os(Linux)
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
          .linkedLibrary("boost_python311"),
        ]
      #endif /* os(Linux) */
    case .boost:
      break
    case .python:
      break
    case .moltenVK:
      break
    case .demo:
      break
    case .all:
      config.products = [
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
          name: "OpenColorIO",
          targets: ["OpenColorIO"]
        ),
        .library(
          name: "OpenImageIO",
          targets: ["OpenImageIO"]
        ),
        .library(
          name: "OpenEXR",
          targets: ["OpenEXR"]
        ),
        .library(
          name: "OpenSubdiv",
          targets: ["OpenSubdiv"]
        ),
        // .library(
        //   name: "GPUShaders",
        //   targets: ["GPUShaders"]
        // ),
        .library(
          name: "OpenMP",
          targets: ["OpenMP"]
        ),
        .library(
          name: "GLFW",
          targets: ["GLFW"]
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
          name: "OpenSSL",
          targets: ["OpenSSL"]
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
          targets: ["MetaversalDemo"]
        ),
      ]
      #if os(macOS)
        config.products += [
          .library(
            name: "Boost",
            targets: ["Boost"]
          ),
          .library(
            name: "DEFLATE",
            targets: ["DEFLATE"]
          ),
        ]
      #endif /* os(macOS) */

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
    case apple
    case linux
    case windows
    case web

    public var platform: [Platform]
    {
      switch self
      {
        case .apple: [.macOS, .iOS, .visionOS, .tvOS, .watchOS]
        case .linux: [.linux, .android, .openbsd]
        case .windows: [.windows]
        case .web: [.wasi]
      }
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
    #endif
  #elseif os(Linux) || os(Android) || os(OpenBSD)
    static let host = "unknown-linux"
    #if os(Android)
      static let device = "android"
    #else /* os(Linux) || os(OpenBSD) */
      static let device = "gnu"
    #endif
  #elseif os(Windows)
    static let host = "windows"
    static let device = "windows"
  #elseif os(WASI)
    static let host = "wasi"
    static let device = "wasi"
  #endif

  #if arch(arm64)
    #if os(Linux)
      static let cpuArch: CPU = .aarch64
    #else
      static let cpuArch: CPU = .arm64
    #endif
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
  #endif
}

enum PkgTarget: String
{
  case draco = "Draco"
  case tbbMallocProxy = "TBBMallocProxy"
  case tbbMalloc = "TBBMalloc"
  case oneTbb = "OneTBB"
  case metaTbb = "MetaTBB"
  case zstd = "ZStandard"
  case lzma2 = "LZMA2"
  case yaml = "Yaml"
  case openssl = "OpenSSL"
  case minizip = "MiniZip"
  case zlib = "ZLibDataCompression"
  case raw = "Raw"
  case png = "LibPNG"
  case turbojpeg = "TurboJPEG"
  case tiff = "TIFF"
  case webp = "WebP"
  case openmp = "OpenMP"
  case apple = "Apple"
  case glfw = "GLFW"
  case imgui = "ImGui"
  case mxGraphEditor = "MXGraphEditor"
  case mxResources = "MXResources"
  case materialx = "MaterialX"
  /// case gpuShaders = "GPUShaders"
  case osd = "OpenSubdiv"
  case exr = "OpenEXR"
  case oiio = "OpenImageIO"
  case ocio = "OpenColorIO"
  case mpy = "MetaPy"
  case ptex = "Ptex"
  case hdf5 = "HDF5"
  case alembic = "Alembic"
  case pybind11 = "PyBind11"
  case deflate = "DEFLATE"
  case blosc = "Blosc"
  case openvdb = "OpenVDB"
  case boost = "Boost"
  case python = "Python"
  case moltenVK = "MoltenVK"
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
