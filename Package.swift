// swift-tools-version: 5.9

import PackageDescription

/* --------------------------------------------------------------------------------------------- *
 * We will need to migrate these include paths once we provide each of Pixar's USD dependencies, *
 * we are also looking to the industry & community for guidance as to which versions we support. *
 * --------------------------------------------------------------------------------------------- */

#if os(macOS)
  let platformIncludes: [String] = []
  let platformExcludes: [String] = []
#elseif os(Linux)
  let platformIncludes: [String] = []
  let platformExcludes: [String] = []
#else /* os(Windows) */
  let platformIncludes: [String] = []
  let platformExcludes: [String] = []
#endif /* os(Windows) */

let package = Package(
  name: "MetaverseKit",
  products: [
    .library(
      name: "Eigen",
      targets: ["Eigen"]
    ),
    .library(
      name: "Draco",
      targets: ["Draco"]
    ),
    .library(
      name: "Python",
      targets: ["Python"]
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

    .binaryTarget(
      name: "Python", 
      url: "https://github.com/wabiverse/Kraken/releases/download/1.50a/Python.xcframework.zip", 
      checksum: "11c2238d09cf559340ce3fd240235b08f227e8b9c6e60f48d4187cd6de52fa7a"
    ),
  ],
  cxxLanguageStandard: .cxx17
)
