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

  enum CPU: String
  {
    case arm = "arm"
    case arm64 = "arm64"
    case x86_64 = "x86_64"
    case i386 = "i386"
    case powerpc = "powerpc"
    case powerpc64 = "powerpc64"
    case powerpc64le = "powerpc64le"
    case s390x = "s390x"
    case wasm32 = "wasm32"
    case arm64_32 = "arm64_32"

    public var family: [CPU]
    {
      switch self
      {
        case .arm, .arm64, .arm64_32: [.arm, .arm64, .arm64_32]
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
    static let cpuArch: CPU = .arm64
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

var chipsetExcludeDirs: [String] = []
var chipsetDefinesC: [CSetting] = []
var chipsetDefinesCXX: [CXXSetting] = []
if (Arch.cpuArch.family.contains(.arm))
{
  chipsetExcludeDirs.append("intel")
  chipsetExcludeDirs.append("powerpc")
  chipsetDefinesC.append(.define("WITH_ARM", to: "1"))
  chipsetDefinesCXX.append(.define("WITH_ARM", to: "1"))
}
else if (Arch.cpuArch.family.contains(.x86_64))
{
  chipsetExcludeDirs.append("arm")
  chipsetExcludeDirs.append("powerpc")
  chipsetDefinesC.append(.define("WITH_INTEL", to: "1"))
  chipsetDefinesCXX.append(.define("WITH_INTEL", to: "1"))
}
else if (Arch.cpuArch.family.contains(.powerpc))
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

#if os(Windows)
  let platformGLFWExcludes: [String] = [
    "xkb_unicode.c",
    "x11_init.c",
    "x11_monitor.c",
    "x11_window.c",
    "posix_thread.c",
    "wl_init.c",
    "wl_window.c",
    "wl_monitor.c",
    "linux_joystick.c",
    "nsgl_context.m",
    "cocoa_init.m",
  ]
  let platformImGuiExcludes: [String] = [
    // no wgpu (for now)
    "backends/imgui_impl_wgpu.cpp",
    // no allegro
    "backends/imgui_impl_allegro5.cpp",
    // no android
    "backends/imgui_impl_android.cpp",
    // no sdl
    "backends/imgui_impl_sdl2.cpp",
    "backends/imgui_impl_sdl3.cpp",
    "backends/imgui_impl_sdlrenderer2.cpp",
    "backends/imgui_impl_sdlrenderer3.cpp",
    // no metal
    "backends/imgui_impl_metal.mm",
    // no apple os
    "backends/imgui_impl_osx.mm",
  ]
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
    "osd/OpenCLPatchTable.cpp",
    "osd/OpenCLVertexBuffer.cpp",
    "osd/OpenCLGLVertexBuffer.cpp",
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
    let platformGLFWExcludes: [String] = [
      "xkb_unicode.c",
      "x11_init.c",
      "x11_monitor.c",
      "x11_window.c",
      "win32_init.c",
      "win32_joystick.c",
      "win32_monitor.c",
      "win32_thread.c",
      "win32_time.c",
      "win32_window.c",
      "posix_time.c",
      "wl_init.c",
      "wl_window.c",
      "wl_monitor.c",
      "wgl_context.c",
      "glx_context.c",
      "linux_joystick.c",
      "null_init.c",
      "null_joystick.c",
      "null_monitor.c",
      "null_window.c",
    ]
    let platformImGuiExcludes: [String] = [
      // no win32
      "backends/imgui_impl_win32.cpp",
      // no wgpu (for now)
      "backends/imgui_impl_wgpu.cpp",
      // no allegro
      "backends/imgui_impl_allegro5.cpp",
      // no android
      "backends/imgui_impl_android.cpp",
      // no directx (for now)
      "backends/imgui_impl_dx9.cpp",
      "backends/imgui_impl_dx10.cpp",
      "backends/imgui_impl_dx11.cpp",
      "backends/imgui_impl_dx12.cpp",
      // no sdl
      "backends/imgui_impl_sdl2.cpp",
      "backends/imgui_impl_sdl3.cpp",
      "backends/imgui_impl_sdlrenderer2.cpp",
      "backends/imgui_impl_sdlrenderer3.cpp",
      // no opengl2
      "backends/imgui_impl_opengl2.cpp"
    ]
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
  #elseif os(Linux)
    let platformGLFWExcludes: [String] = [
      "win32_init.c",
      "win32_joystick.c",
      "win32_monitor.c",
      "win32_thread.c",
      "win32_time.c",
      "win32_window.c",
      "wgl_context.c",
      "nsgl_context.m",
      "cocoa_init.m",
    ]
    let platformImGuiExcludes: [String] = [
      // no win32
      "backends/imgui_impl_win32.cpp",
      // no wgpu (for now)
      "backends/imgui_impl_wgpu.cpp",
      // no allegro
      "backends/imgui_impl_allegro5.cpp",
      // no android (for now)
      "backends/imgui_impl_android.cpp",
      // no directx
      "backends/imgui_impl_dx9.cpp",
      "backends/imgui_impl_dx10.cpp",
      "backends/imgui_impl_dx11.cpp",
      "backends/imgui_impl_dx12.cpp",
      // no metal
      "backends/imgui_impl_metal.mm",
      // no apple os
      "backends/imgui_impl_osx.mm",
    ]
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
      "osd/OpenCLPatchTable.cpp",
      "osd/OpenCLVertexBuffer.cpp",
      "osd/OpenCLGLVertexBuffer.cpp",
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
    "tjexample.c",
    "example.c"
  ]
#endif /* os(macOS) || os(Linux) */

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
      name: "Python",
      targets: ["Python"]
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
    .library(
      name: "GPUShaders",
      targets: ["GPUShaders"]
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
      name: "ImGui",
      targets: ["ImGui"]
    ),
    .library(
      name: "MXResources",
      targets: ["MXResources"]
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
    .executable(
      name: "MXGraphEditor",
      targets: ["MXGraphEditor"]
    ),
  ],
  targets: [
    .target(
      name: "Eigen",
      dependencies: [],
      exclude: [],
      publicHeadersPath: "include",
      cxxSettings: []
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
      cxxSettings: []
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
      ]
    ),

    .target(
      name: "TBBMallocProxy",
      dependencies: [
        .target(name: "OneTBB"),
      ],
      exclude: [],
      publicHeadersPath: ".",
      cxxSettings: [
        .define("_XOPEN_SOURCE"),
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
        .define("_XOPEN_SOURCE"),
        .define("__TBBMALLOC_BUILD", to: "1"),
      ]
    ),

    .target(
      name: "OneTBB",
      publicHeadersPath: "include",
      cxxSettings: [
        .define("_XOPEN_SOURCE"),
        .define("TBB_USE_PROFILING_TOOLS", to: "2"),
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
      ]
    ),

    .target(
      name: "SPIRVCross",
      dependencies: [],
      exclude: [],
      publicHeadersPath: "include/spirv_cross",
      cxxSettings: []
    ),

    .target(
      name: "ZStandard",
      dependencies: [],
      exclude: [],
      publicHeadersPath: "."
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
      ]
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
      ],
      exclude: platformMiniZipExcludes,
      publicHeadersPath: "include",
      cSettings: [
        .define("HAVE_ZLIB", to: "1"),
        .define("ZLIB_COMPAT", to: "1"),
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
      exclude: [
        "example.c",
        "pngtest.c"
      ] + chipsetExcludeDirs,
      publicHeadersPath: "include",
      cSettings: [
        .headerSearchPath("."),
      ] + chipsetDefinesC
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
        .define("C_LOSSLESS_SUPPORTED", to: "1"),
        .define("D_LOSSLESS_SUPPORTED", to: "1"),
        .define("PPM_SUPPORTED", to: "1"),
        .define("BITS_IN_JSAMPLE", to: "12")
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
      cxxSettings: [
        .define("FROM_TIF_JPEG_12", to: "1"),
        .define("HAVE_JPEGTURBO_DUAL_MODE_8_12", to: "1"),
        .define("BITS_IN_JSAMPLE", to: "12")
      ]
    ),

    .target(
      name: "WebP",
      dependencies: [],
      exclude: [],
      publicHeadersPath: "include",
      cSettings: [
        .headerSearchPath("."),
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
      ]
    ),

    .target(
      name: "Apple",
      dependencies: [],
      publicHeadersPath: "include"
    ),

    .target(
      name: "GLFW",
      dependencies: [
        .target(name: "Apple"),
        .target(name: "MoltenVK"),
      ],
      exclude: platformGLFWExcludes,
      publicHeadersPath: "include",
      cxxSettings: [
        .headerSearchPath("."),
        .define("_GLFW_COCOA", to: "1", .when(platforms: Arch.OS.apple.platform)),
        .define("_GLFW_X11", to: "1", .when(platforms: Arch.OS.linux.platform)),
        .define("_GLFW_WIN32", to: "1", .when(platforms: Arch.OS.windows.platform)),
        .define("GL_SILENCE_DEPRECATION", to: "1"),
      ]
    ),

    .target(
      name: "ImGui",
      dependencies: [
        .target(name: "GLFW"),
      ],
      exclude: platformImGuiExcludes,
      publicHeadersPath: "include",
      cxxSettings: [
        .headerSearchPath("."),
        .headerSearchPath("backends"),
      ],
      linkerSettings: [
        .linkedFramework("Cocoa", .when(platforms: Arch.OS.apple.platform)),
        .linkedFramework("GLUT", .when(platforms: Arch.OS.apple.platform)),
        .linkedFramework("GameController", .when(platforms: Arch.OS.apple.platform))
      ]
    ),

    .target(
      name: "MXResources",
      dependencies: [],
      resources: [
        .copy("libraries"),
        .process("Resources"),
      ],
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),

    .target(
      name: "MaterialX",
      dependencies: [
        .target(name: "Apple"),
        .target(name: "ImGui"),
        .target(name: "OpenImageIO"),
        .target(name: "Python"),
        .target(name: "PyBind11"),
        .target(name: "MXResources"),
      ],
      exclude: [
        "source/JsMaterialX",
        "source/MaterialXView",
        "source/MaterialXTest",
      ],
      publicHeadersPath: "include",
      cxxSettings: [
        .headerSearchPath("include/MaterialXGenGlsl"),
        .headerSearchPath("include/MaterialXGenMdl"),
        .headerSearchPath("include/MaterialXGenMsl"),
        .headerSearchPath("include/MaterialXGenOsl"),
        .headerSearchPath("include/MaterialXGenShader"),
        .headerSearchPath("include/MaterialXRenderGlsl"),
        .headerSearchPath("include/MaterialXRenderHw"),
        .headerSearchPath("include/MaterialXRenderMsl"),
        .headerSearchPath("include/MaterialXRenderOsl"),
        .headerSearchPath("include/MaterialXRenderGlsl/External/Glad"),
        .headerSearchPath("include/MaterialXFormat/External/PugiXML"),
      ]
    ),

    .executableTarget(
      name: "MXGraphEditor",
      dependencies: [
        .target(name: "MaterialX")
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
        .target(name: "MetaTBB"),
        .target(name: "OpenMP"),
        .target(name: "GPUShaders"),
      ],
      exclude: platformOsdExcludes,
      publicHeadersPath: "include",
      cxxSettings: [
        .headerSearchPath("glLoader"),
        /* autogenerated shader headers. */
        .define("GPU_SHADERS_SWIFT_OSD_H", to: "../../.build/\(Arch.hostTriplet)/debug/GPUShaders.build/GPUShaders-Swift.h", .when(configuration: .debug)),
        .define("GPU_SHADERS_SWIFT_OSD_H", to: "../../.build/\(Arch.hostTriplet)/release/GPUShaders.build/GPUShaders-Swift.h", .when(configuration: .release)),
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
      ]
    ),

    .target(
      name: "MetaPy",
      dependencies: [
        .target(name: "Boost"),
        .target(name: "Python"),
        .target(name: "OpenEXR"),
        .target(name: "OpenVDB"),
        .target(name: "Alembic"),
        .target(name: "OpenImageIO"),
        .target(name: "MaterialX"),
      ],
      exclude: platformMetaPyExcludes,
      publicHeadersPath: "include/python",
      cxxSettings: [
        .headerSearchPath("include/python/PyImath"),
        .headerSearchPath("include/python/PyAlembic"),
        .headerSearchPath("include/python/PyOIIO"),
        .headerSearchPath("include/python/PyOpenVDB"),
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
        .define("H5_HAVE_C99_FUNC", to: "1"),
        .define("H5_USE_18_API", to: "1"),
        .define("H5_BUILT_AS_DYNAMIC_LIB", to: "1"),
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
      ]
    ),

    .target(
      name: "PyBind11",
      publicHeadersPath: "include",
      cxxSettings: []
    ),

    .target(
      name: "DEFLATE",
      publicHeadersPath: "include",
      cSettings: [
        .headerSearchPath("."),
      ]
    ),

    .target(
      name: "Blosc",
      dependencies: [
        .target(name: "ZLibDataCompression"),
        .target(name: "ZStandard"),
      ],
      exclude: platformBloscExcludes,
      publicHeadersPath: "include/blosc"
    ),

    .target(
      name: "OpenVDB",
      dependencies: [
        .target(name: "Boost"),
        .target(name: "MetaTBB"),
        .target(name: "Blosc"),
        .target(name: "Python"),
        .target(name: "PyBind11"),
        .target(name: "OpenEXR"),
        .target(name: "ZLibDataCompression"),
      ],
      publicHeadersPath: "include",
      cxxSettings: [
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
    ),

    .binaryTarget(
      name: "Boost",
      url: "https://github.com/wabiverse/MetaverseBoostFramework/releases/download/1.81.3/boost.xcframework.zip",
      checksum: "05025cdac9f49ca8a30a2c9c56f30b85df98e338c12a0dc0ed8e7c8700f2a728"
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
