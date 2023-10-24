// swift-tools-version: 5.9

import PackageDescription

/** Platforms, grouped by OS. */
struct Arch
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
      static let device = "unknown"
    #endif
  #elseif os(Windows)
    static let host = "windows"
    static let device = "windows"
  #elseif os(WASI)
    static let host = "wasi"
    static let device = "wasi"
  #endif

  #if arch(arm64)
    static let cpuArch: String = "arm64"
  #elseif arch(x86_64)
    static let cpuArch: String = "x86_64"
  #elseif arch(i386)
    static let cpuArch: String = "i386"
  #elseif arch(arm)
    static let cpuArch: String = "arm"
  #elseif arch(powerpc64)
    static let cpuArch: String = "powerpc64"
  #elseif arch(powerpc64le)
    static let cpuArch: String = "powerpc64le"
  #elseif arch(powerpc)
    static let cpuArch: String = "powerpc"
  #elseif arch(s390x)
    static let cpuArch: String = "s390x"
  #elseif arch(wasm32)
    static let cpuArch: String = "wasm32"
  #elseif arch(arm64_32)
    static let cpuArch: String = "arm64_32"
  #endif
}

#if os(Windows)
  let platformBloscExcludes: [String] = []
  let platformMetaPyExcludes: [String] = []
  let platformMiniZipExcludes: [String] = [
    "mz_strm_os_posix.c",
    "mz_os_posix.c",
    "mz_crypt_apple.c",
  ]
  let platformOCIOExcludes: [String] = [
    "SystemMonitor_macos.cpp",
  ]
  let platformTIFFExcludes: [String] = [
    "tif_unix.c",
    "mkg3states.c",
  ]
  let platformTurboJPEGExcludes: [String] = [
    "turbojpeg-jni.c",
    "tjunittest.c",
  ]
  let platformOsdExcludes: [String] = [
    /* disabled for now. */
    "osd/OpenCLD3D11VertexBuffer.cpp",
    "osd/OpenCLEvaluator.cpp",
    /* disabled DX3D for now. */
    /* I have lots of TODOs on windows... */
    "osd/D3D11ComputeEvaluator.cpp",
    "osd/D3D11LegacyGregoryPatchTable.cpp",
    "osd/D3D11PatchTable.cpp",
    "osd/D3D11VertexBuffer.cpp",
    "osd/CpuD3D11VertexBuffer.cpp",
    /* no metal for windows. */
    "MTLComputeEvaluator.mm",
    "MTLLegacyGregoryPatchTable.mm",
    "MTLMesh.mm",
    "MTLPatchShaderSource.mm",
    "MTLPatchTable.mm",
    "MTLVertexBuffer.mm",
    /* disabled CUDA for now. */
    /* I have lots of TODOs on windows... */
    "osd/CudaD3D11VertexBuffer.cpp",
    "osd/CudaEvaluator.cpp",
    "osd/CudaGLVertexBuffer.cpp",
    "osd/CudaKernel.cu",
    "osd/CudaPatchTable.cpp",
    "osd/CudaVertexBuffer.cpp",
  ]
#else /* os(Windows) */
  #if os(macOS)
    let platformOCIOExcludes: [String] = [
      "SystemMonitor_windows.cpp",
    ]
    let platformMiniZipExcludes: [String] = [
      "mz_strm_os_win32.c",
      "mz_os_win32.c",
      "mz_crypt_winvista.c",
      "mz_crypt_winxp.c",
    ]
    let platformOsdExcludes: [String] = [
      /* disabled for now */
      "osd/OpenCLD3D11VertexBuffer.cpp",
      "osd/OpenCLEvaluator.cpp",
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
  #elseif os(Linux)
    let platformOCIOExcludes: [String] = [
      "SystemMonitor_macos.cpp",
      "SystemMonitor_windows.cpp",
    ]
    let platformMiniZipExcludes: [String] = [
      "mz_strm_os_win32.c",
      "mz_os_win32.c",
      "mz_crypt_winvista.c",
      "mz_crypt_winxp.c",
      "mz_crypt_apple.c",
    ]
    let platformOsdExcludes: [String] = [
      /* disabled for now. */
      "osd/OpenCLD3D11VertexBuffer.cpp",
      "osd/OpenCLEvaluator.cpp",
      /* no DirectX3D for linux. */
      "osd/D3D11ComputeEvaluator.cpp",
      "osd/D3D11LegacyGregoryPatchTable.cpp",
      "osd/D3D11PatchTable.cpp",
      "osd/D3D11VertexBuffer.cpp",
      "osd/CpuD3D11VertexBuffer.cpp",
      /* no metal for linux. */
      "MTLComputeEvaluator.mm",
      "MTLLegacyGregoryPatchTable.mm",
      "MTLMesh.mm",
      "MTLPatchShaderSource.mm",
      "MTLPatchTable.mm",
      "MTLVertexBuffer.mm",
      /* disabled CUDA for now. */
      /* I have lots of TODOs on linux... */
      "osd/CudaD3D11VertexBuffer.cpp",
      "osd/CudaEvaluator.cpp",
      "osd/CudaGLVertexBuffer.cpp",
      "osd/CudaKernel.cu",
      "osd/CudaPatchTable.cpp",
      "osd/CudaVertexBuffer.cpp",
    ]
  #endif /* os(Linux) */
  /** ---------------------------------- *
   * Same excludes for macOS and Linux. *
   * ---------------------------------- */
  let platformBloscExcludes: [String] = [
    "include/win32",
  ]
  let platformMetaPyExcludes: [String] = [
    "PyAlembic/msvc14fixes.cpp",
  ]
  let platformTIFFExcludes: [String] = [
    "tif_win32.c",
    "mkg3states.c",
  ]
  let platformTurboJPEGExcludes: [String] = [
    "turbojpeg-jni.c",
    "tjunittest.c",
  ]
#endif /* os(macOS) || os(Linux) */

/* ------------------------------------------------------
 *  :: :  💫 The Open Source Metaverse  :   ::
 * ------------------------------------------------------ */

let package = Package(
  name: "MetaverseKit",
  platforms: [
    .macOS(.v11),
    .visionOS(.v1),
    .iOS(.v12),
    .tvOS(.v12),
    .watchOS(.v4),
  ],
  products: [
    .library(
      name: "OneTBB",
      targets: ["OneTBB"]
    ),
    .library(
      name: "Boost",
      targets: ["Boost"]
    ),
    .library(
      name: "Python",
      targets: ["Python"]
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
    .library(
      name: "GPUShaders",
      targets: ["GPUShaders"]
    ),
    .library(
      name: "OpenMP",
      targets: ["OpenMP"]
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
      name: "DEFLATE",
      targets: ["DEFLATE"]
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
      name: "MoltenVK",
      targets: ["MoltenVK"]
    ),
    .library(
      name: "Glslang",
      targets: ["Glslang"]
    ),
    .library(
      name: "Shaderc",
      targets: ["Shaderc"]
    ),
    .library(
      name: "SPIRVCross",
      targets: ["SPIRVCross"]
    ),
  ],
  targets: [
    .target(
      name: "Eigen",
      dependencies: [],
      exclude: [],
      publicHeadersPath: "include",
      cxxSettings: [],
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),

    .target(
      name: "Draco",
      dependencies: [
        "Eigen",
      ],
      exclude: [
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
      ],
      publicHeadersPath: "include",
      cxxSettings: [],
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),

    .target(
      name: "Glslang",
      exclude: [
        "include/glslang/CMakeLists.txt",
        "include/glslang/ExtensionHeaders/GL_EXT_shader_realtime_clock.glsl",
        "include/glslang/MachineIndependent/glslang.m4",
        "include/glslang/MachineIndependent/glslang.y",
        "include/glslang/OSDependent/Unix/CMakeLists.txt",
        "include/glslang/OSDependent/Web",
        "include/glslang/OSDependent/Windows",
        "include/glslang/updateGrammar",
        "include/SPIRV/CMakeLists.txt",
      ],
      publicHeadersPath: "include",
      cxxSettings: [
        .define("ENABLE_HLSL", to: "1"),
      ],
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),

    .target(
      name: "OneTBB",
      dependencies: [],
      exclude: [],
      publicHeadersPath: "include",
      cxxSettings: [
        .define("_XOPEN_SOURCE"),
        .define("TBB_USE_PROFILING_TOOLS", to: "2"),
      ],
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),

    .target(
      name: "Shaderc",
      dependencies: [
        .target(name: "Glslang"),
      ],
      exclude: [
        "include/shaderc/shaderc_test.cc",
        "include/shaderc/libshaderc_util/string_piece_test.cc",
        "include/shaderc/libshaderc_util/version_profile_test.cc",
        "include/shaderc/shaderc_private_test.cc",
        "include/shaderc/shaderc_cpp_test.cc",
        "include/shaderc/libshaderc_util/message_test.cc",
        "include/shaderc/libshaderc_util/mutex_test.cc",
        "include/shaderc/libshaderc_util/file_finder_test.cc",
        "include/shaderc/libshaderc_util/io_shaderc_test.cc",
        "include/shaderc/libshaderc_util/format_test.cc",
        "include/shaderc/libshaderc_util/counting_includer_test.cc",
        "include/shaderc/libshaderc_util/compiler_test.cc",
        "include/shaderc/glslc/stage_test.cc",
        "include/shaderc/glslc/resource_parse_test.cc",
        "include/shaderc/glslc/file_test.cc",
        "include/shaderc/glslc/notmain.cc",
        "include/shaderc/shaderc_c_smoke_test.c",
      ],
      publicHeadersPath: "include",
      cxxSettings: [
        .headerSearchPath("include/shaderc"),
        .define("ENABLE_HLSL", to: "1"),
      ],
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),

    .target(
      name: "SPIRVCross",
      dependencies: [],
      exclude: [],
      publicHeadersPath: "include/spirv_cross",
      cxxSettings: [],
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),

    .target(
      name: "ZStandard",
      dependencies: [],
      exclude: [],
      publicHeadersPath: ".",
      swiftSettings: [
        .interoperabilityMode(.C),
      ]
    ),

    .target(
      name: "LZMA2",
      dependencies: [],
      exclude: [
        "check/crc32_small.c",
      ],
      publicHeadersPath: "include",
      cSettings: [
        .headerSearchPath("common"),
        .headerSearchPath("lzmacommon"),
        .headerSearchPath("check"),
        .headerSearchPath("delta"),
        .headerSearchPath("lz"),
        .headerSearchPath("lzma"),
        .headerSearchPath("rangecoder"),
        .headerSearchPath("simple"),
        .define("HAVE_STDBOOL_H", to: "1"),
        .define("MYTHREAD_POSIX", to: "1", .when(platforms: Arch.OS.apple.platform + Arch.OS.linux.platform)),
        .define("MYTHREAD_VISTA", to: "1", .when(platforms: Arch.OS.windows.platform)),
      ],
      swiftSettings: [
        .interoperabilityMode(.C),
      ]
    ),

    .target(
      name: "Yaml",
      dependencies: [],
      exclude: [],
      publicHeadersPath: "include",
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),

    .target(
      name: "OpenSSL",
      dependencies: [],
      exclude: [],
      publicHeadersPath: "include",
      cSettings: [
        .define("BORINGSSL_NO_STATIC_INITIALIZER", to: "1"),
      ],
      swiftSettings: [
        .interoperabilityMode(.C),
      ]
    ),

    .target(
      name: "MiniZip",
      dependencies: [
        .target(name: "LZMA2"),
        .target(name: "ZLibDataCompression"),
        .target(name: "ZStandard"),
        .target(name: "OpenSSL"),
      ],
      exclude: platformMiniZipExcludes,
      publicHeadersPath: "include",
      cSettings: [
        .define("HAVE_ZLIB", to: "1"),
        .define("ZLIB_COMPAT", to: "1"),
      ],
      swiftSettings: [
        .interoperabilityMode(.C),
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
      ],
      swiftSettings: [
        .interoperabilityMode(.C),
      ]
    ),

    .target(
      name: "Raw",
      dependencies: [],
      exclude: [],
      publicHeadersPath: "include",
      cxxSettings: [
        .headerSearchPath("."),
      ],
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),

    .target(
      name: "LibPNG",
      dependencies: [
        .target(name: "ZLibDataCompression"),
      ],
      exclude: [],
      publicHeadersPath: "include",
      cSettings: [
        .headerSearchPath("."),
      ],
      swiftSettings: [
        .interoperabilityMode(.C),
      ]
    ),

    .target(
      name: "TurboJPEG",
      dependencies: [
        .target(name: "OpenSSL"),
      ],
      exclude: platformTurboJPEGExcludes,
      publicHeadersPath: "include/turbo",
      cSettings: [
        .headerSearchPath("."),
      ],
      swiftSettings: [
        .interoperabilityMode(.C),
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
      exclude: platformTIFFExcludes,
      publicHeadersPath: "include",
      swiftSettings: [
        .interoperabilityMode(.C),
      ]
    ),

    .target(
      name: "WebP",
      dependencies: [],
      exclude: [],
      publicHeadersPath: "include",
      cSettings: [
        .headerSearchPath("."),
      ],
      swiftSettings: [
        .interoperabilityMode(.C),
      ]
    ),

    .target(
      name: "OpenMP",
      dependencies: [
        .target(name: "Python"),
      ],
      exclude: [
        "runtime/extractExternal.cpp",
        "runtime/z_Windows_NT_util.cpp",
        "runtime/z_Windows_NT-586_util.cpp",
        "runtime/z_Windows_NT-586_asm.asm",
      ],
      publicHeadersPath: "include",
      cxxSettings: [
        .headerSearchPath("runtime"),
      ],
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),

    .target(
      name: "GPUShaders",
      resources: [
        .process("Metal"),
        .process("GL"),
        .process("DX3D"),
      ],
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),

    .target(
      name: "OpenSubdiv",
      dependencies: [
        .target(name: "OneTBB"),
        .target(name: "OpenMP"),
        .target(name: "GPUShaders")
      ],
      exclude: platformOsdExcludes,
      publicHeadersPath: "include",
      cxxSettings: [
        .headerSearchPath("glLoader"),
        /* autogenerated shader headers. */
        .define("GPU_SHADERS_SWIFT_OSD_H", to: "../../../.build/\(Arch.hostTriplet)/debug/GPUShaders.build/GPUShaders-Swift.h", .when(configuration: .debug)),
        .define("GPU_SHADERS_SWIFT_OSD_H", to: "../../../.build/\(Arch.hostTriplet)/release/GPUShaders.build/GPUShaders-Swift.h", .when(configuration: .release))
      ],
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),

    .target(
      name: "OpenEXR",
      dependencies: [
        .target(name: "DEFLATE"),
      ],
      exclude: [],
      publicHeadersPath: "include",
      cSettings: [
        .headerSearchPath("."),
        .headerSearchPath("OpenEXRCore"),
        .headerSearchPath("include/OpenEXR"),
      ],
      swiftSettings: [
        .interoperabilityMode(.Cxx),
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
      exclude: [
        // disabled for now... but always happy to add more in.
        "nuke",
        "jpeg2000.imageio",
        "iv",
        "heif.imageio",
        "gif.imageio",
        "ffmpeg.imageio",
        "dicom.imageio",
        "cineon.imageio",
      ],
      publicHeadersPath: "include",
      cxxSettings: [
        .headerSearchPath("."),
        .headerSearchPath("include/OpenImageIO/detail"),
        .headerSearchPath("libOpenImageIO"),
      ],
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),

    .target(
      name: "OpenColorIO",
      dependencies: [
        .target(name: "OpenEXR"),
        .target(name: "Python"),
        .target(name: "MiniZip"),
        .target(name: "Yaml"),
      ],
      exclude: platformOCIOExcludes,
      publicHeadersPath: "include",
      cxxSettings: [
        .headerSearchPath("."),
      ],
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),

    .target(
      name: "MetaPy",
      dependencies: [
        .target(name: "Python"),
        .target(name: "Boost"),
        .target(name: "OpenEXR"),
        .target(name: "Alembic"),
        .target(name: "OpenImageIO"),
      ],
      exclude: platformMetaPyExcludes,
      publicHeadersPath: "include/python",
      cxxSettings: [
        .headerSearchPath("include/python/PyImath"),
        .headerSearchPath("include/python/PyAlembic"),
        .headerSearchPath("include/python/PyOIIO"),
      ],
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),

    .target(
      name: "Ptex",
      publicHeadersPath: "include",
      cxxSettings: [],
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),

    .target(
      name: "HDF5",
      publicHeadersPath: "include",
      cSettings: [
        .define("H5_HAVE_C99_FUNC", to: "1"),
        .define("H5_USE_18_API", to: "1"),
        .define("H5_BUILT_AS_DYNAMIC_LIB", to: "1"),
      ],
      swiftSettings: [
        .interoperabilityMode(.C),
      ]
    ),

    .target(
      name: "Alembic",
      dependencies: [
        .target(name: "Boost"),
        .target(name: "Python"),
        .target(name: "HDF5"),
        .target(name: "OpenEXR"),
      ],
      publicHeadersPath: "include",
      cxxSettings: [
        .headerSearchPath("include/Alembic/AbcMaterial"),
      ],
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),

    .target(
      name: "PyBind11",
      publicHeadersPath: "include",
      cxxSettings: [],
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),

    .target(
      name: "DEFLATE",
      publicHeadersPath: "include",
      cSettings: [
        .headerSearchPath("."),
      ],
      swiftSettings: [
        .interoperabilityMode(.C),
      ]
    ),

    .target(
      name: "Blosc",
      dependencies: [
        .target(name: "ZLibDataCompression"),
        .target(name: "ZStandard"),
      ],
      exclude: platformBloscExcludes,
      publicHeadersPath: "include/blosc",
      swiftSettings: [
        .interoperabilityMode(.C),
      ]
    ),

    .target(
      name: "OpenVDB",
      dependencies: [
        .target(name: "OneTBB"),
        .target(name: "Boost"),
        .target(name: "Blosc"),
        .target(name: "Python"),
        .target(name: "PyBind11"),
        .target(name: "OpenEXR"),
      ],
      exclude: [
        "include/openvdb/unittest",
        "include/openvdb/unittest/notmain.cc",
      ],
      publicHeadersPath: "include",
      cxxSettings: [],
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),

    .binaryTarget(
      name: "Boost",
      url: "https://github.com/wabiverse/MetaverseBoostFramework/releases/download/1.81.2/boost.xcframework.zip",
      checksum: "a4846beef0b8f335a0fd0de5711aec07674e9c804c066e0090d864a31b99e9de"
    ),

    .binaryTarget(
      name: "Python",
      url: "https://github.com/wabiverse/Kraken/releases/download/1.50a/Python.xcframework.zip",
      checksum: "11c2238d09cf559340ce3fd240235b08f227e8b9c6e60f48d4187cd6de52fa7a"
    ),

    .binaryTarget(
      name: "MoltenVK",
      url: "https://github.com/wabiverse/Kraken/releases/download/1.50a/MoltenVK.xcframework.zip",
      checksum: "d236c4d41f581b6533f2f40eb0f74a6af03b31781cbb451856c5acf2f9f8f491"
    ),
  ],
  cLanguageStandard: .gnu17,
  cxxLanguageStandard: .cxx17
)
