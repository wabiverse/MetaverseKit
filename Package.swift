// swift-tools-version: 5.9

import PackageDescription

#if os(Windows)
let platformBloscExcludes: [String] = []
let platformMetaPyExcludes: [String] = []
#else /* os(Windows) */
let platformBloscExcludes: [String] = [
  "include/win32"
]
let platformMetaPyExcludes: [String] = [
  "PyAlembic/msvc14fixes.cpp"
]
#endif /* !os(Windows) */

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
    .watchOS(.v4)
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
      name: "IMath",
      targets: ["IMath"]
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
    )
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
        "Eigen"
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
        "include/draco/tools"
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
        .define("ENABLE_HLSL", to: "1")
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
        .define("TBB_USE_PROFILING_TOOLS", to: "2")
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
        .define("ENABLE_HLSL", to: "1")
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
      name: "IMath",
      dependencies: [
        .target(name: "Python"),
        .target(name: "Boost"),
      ],
      exclude: [],
      publicHeadersPath: "include/Imath",
      swiftSettings: [
        .interoperabilityMode(.Cxx),
      ]
    ),

    .target(
      name: "MetaPy",
      dependencies: [
        .target(name: "Python"),
        .target(name: "Boost"),
        .target(name: "IMath"),
        .target(name: "Alembic"),
      ],
      exclude: platformMetaPyExcludes,
      publicHeadersPath: "include/python",
      cxxSettings: [
        .headerSearchPath("include/python/PyImath"),
        .headerSearchPath("include/python/PyAlembic")
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
      exclude: [
        "c++"
      ],
      publicHeadersPath: "include",
      cxxSettings: [
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
        .target(name: "IMath")
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
      name: "Blosc",
      exclude: platformBloscExcludes,
      publicHeadersPath: "include/blosc",
      cxxSettings: [],
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
        .target(name: "PyBind11"),
        .target(name: "IMath"),
      ],
      exclude: [
        "include/openvdb/unittest",
        "include/openvdb/unittest/notmain.cc"
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
    )
  ],
  cxxLanguageStandard: .cxx17
)
