//
//   Copyright 2015 Pixar
//
//   Licensed under the Apache License, Version 2.0 (the "Apache License")
//   with the following modification; you may not use this file except in
//   compliance with the Apache License and the following modification to it:
//   Section 6. Trademarks. is deleted and replaced with:
//
//   6. Trademarks. This License does not grant permission to use the trade
//      names, trademarks, service marks, or product names of the Licensor
//      and its affiliates, except as required to comply with Section 4(c) of
//      the License and to reproduce the content of the NOTICE file.
//
//   You may obtain a copy of the Apache License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the Apache License with the above modification is
//   distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
//   KIND, either express or implied. See the Apache License for the specific
//   language governing permissions and limitations under the Apache License.
//

#if __has_include("GPUShaders/GPUShaders-Swift.h")
  #include "GPUShaders/GPUShaders-Swift.h"
#else
  #define GPU_SHADERS_INC <GPU_SHADERS_SWIFT_OSD_H>
  #include GPU_SHADERS_INC
#endif

#include "OpenSubdiv/OSDSurfaceGLSLPatchShaderSource.h"
#include <sstream>
#include <string>

#include GPU_SHADERS_INC

namespace OpenSubdiv {
namespace OPENSUBDIV_VERSION {

namespace Osd {

/* and here, we use our Swift implementation right out of the box, in cxx. magic. */
static GPUShaders::GPUShaders glslShaders = GPUShaders::GPUShaders::init();
static swift::String glslCode = glslShaders.setupMetal();
static char source[1024];
static const char *getShaderCode() {
  std::stringstream ss;
  ss << std::string(glslCode);

  const size_t bufSize = ss.str().length() + 1;
  ss.str().copy(source, bufSize);
  return &source[bufSize];
}

static const char *commonShaderSource = getShaderCode();
static const char *commonTessShaderSource = getShaderCode();
static const char *patchLegacyShaderSource = getShaderCode();
static const char *patchBasisTypesShaderSource = getShaderCode();
static const char *patchBasisShaderSource = getShaderCode();
static const char *boxSplineTriangleShaderSource = getShaderCode();
static const char *bsplineShaderSource = getShaderCode();
static const char *gregoryShaderSource = getShaderCode();
static const char *gregoryBasisShaderSource = getShaderCode();
static const char *gregoryTriangleShaderSource = getShaderCode();

/*static*/
std::string GLSLPatchShaderSource::GetPatchDrawingShaderSource() {
  std::stringstream ss;
  ss << std::string(commonShaderSource);
  ss << std::string(commonTessShaderSource);
  return ss.str();
}

/*static*/
std::string GLSLPatchShaderSource::GetCommonShaderSource() {
  std::stringstream ss;
  ss << GetPatchDrawingShaderSource();
  ss << std::string(patchLegacyShaderSource);
  return ss.str();
}

/*static*/
std::string GLSLPatchShaderSource::GetPatchBasisShaderSource() {
  std::stringstream ss;
#if defined(OPENSUBDIV_GREGORY_EVAL_TRUE_DERIVATIVES)
  ss << "#define OPENSUBDIV_GREGORY_EVAL_TRUE_DERIVATIVES\n";
#endif
  ss << std::string(patchBasisTypesShaderSource);
  ss << std::string(patchBasisShaderSource);
  return ss.str();
}

/*static*/
std::string
GLSLPatchShaderSource::GetVertexShaderSource(Far::PatchDescriptor::Type type) {
  std::stringstream ss;
  switch (type) {
  case Far::PatchDescriptor::REGULAR:
    ss << "#define OSD_PATCH_BSPLINE\n"
       << "#define OSD_PATCH_VERTEX_BSPLINE_SHADER\n"
       << bsplineShaderSource;
    break;
  case Far::PatchDescriptor::LOOP:
    ss << "#define OSD_PATCH_BOX_SPLINE_TRIANGLE\n"
       << "#define OSD_PATCH_VERTEX_BOX_SPLINE_TRIANGLE_SHADER\n"
       << boxSplineTriangleShaderSource;
    break;
  case Far::PatchDescriptor::GREGORY:
    ss << "#define OSD_PATCH_GREGORY\n"
       << "#define OSD_PATCH_VERTEX_GREGORY_SHADER\n"
       << gregoryShaderSource;
    break;
  case Far::PatchDescriptor::GREGORY_BOUNDARY:
    ss << "#define OSD_PATCH_GREGORY_BOUNDARY\n"
       << "#define OSD_PATCH_VERTEX_GREGORY_SHADER\n"
       << gregoryShaderSource;
    break;
  case Far::PatchDescriptor::GREGORY_BASIS:
    ss << "#define OSD_PATCH_GREGORY_BASIS\n"
       << "#define OSD_PATCH_VERTEX_GREGORY_BASIS_SHADER\n"
       << gregoryBasisShaderSource;
    break;
  case Far::PatchDescriptor::GREGORY_TRIANGLE:
    ss << "#define OSD_PATCH_GREGORY_TRIANGLE\n"
       << "#define OSD_PATCH_VERTEX_GREGORY_TRIANGLE_SHADER\n"
       << gregoryTriangleShaderSource;
    break;
  default:
    break; // returns empty (points, lines, quads, ...)
  }
  return ss.str();
}

/*static*/
std::string GLSLPatchShaderSource::GetTessControlShaderSource(
    Far::PatchDescriptor::Type type) {
  std::stringstream ss;
  switch (type) {
  case Far::PatchDescriptor::REGULAR:
    ss << "#define OSD_PATCH_BSPLINE\n"
       << "#define OSD_PATCH_TESS_CONTROL_BSPLINE_SHADER\n"
       << bsplineShaderSource;
    break;
  case Far::PatchDescriptor::LOOP:
    ss << "#define OSD_PATCH_BOX_SPLINE_TRIANGLE\n"
       << "#define OSD_PATCH_TESS_CONTROL_BOX_SPLINE_TRIANGLE_SHADER\n"
       << boxSplineTriangleShaderSource;
    break;
  case Far::PatchDescriptor::GREGORY:
    ss << "#define OSD_PATCH_GREGORY\n"
       << "#define OSD_PATCH_TESS_CONTROL_GREGORY_SHADER\n"
       << gregoryShaderSource;
    break;
  case Far::PatchDescriptor::GREGORY_BOUNDARY:
    ss << "#define OSD_PATCH_GREGORY_BOUNDARY\n"
       << "#define OSD_PATCH_TESS_CONTROL_GREGORY_SHADER\n"
       << gregoryShaderSource;
    break;
  case Far::PatchDescriptor::GREGORY_BASIS:
    ss << "#define OSD_PATCH_GREGORY_BASIS\n"
       << "#define OSD_PATCH_TESS_CONTROL_GREGORY_BASIS_SHADER\n"
       << gregoryBasisShaderSource;
    break;
  case Far::PatchDescriptor::GREGORY_TRIANGLE:
    ss << "#define OSD_PATCH_GREGORY_TRIANGLE\n"
       << "#define OSD_PATCH_TESS_CONTROL_GREGORY_TRIANGLE_SHADER\n"
       << gregoryTriangleShaderSource;
    break;
  default:
    break; // returns empty (points, lines, quads, ...)
  }
  return ss.str();
}

/*static*/
std::string GLSLPatchShaderSource::GetTessEvalShaderSource(
    Far::PatchDescriptor::Type type) {
  std::stringstream ss;
  switch (type) {
  case Far::PatchDescriptor::REGULAR:
    ss << "#define OSD_PATCH_BSPLINE\n"
       << "#define OSD_PATCH_TESS_EVAL_BSPLINE_SHADER\n"
       << bsplineShaderSource;
    break;
  case Far::PatchDescriptor::LOOP:
    ss << "#define OSD_PATCH_BOX_SPLINE_TRIANGLE\n"
       << "#define OSD_PATCH_TESS_EVAL_BOX_SPLINE_TRIANGLE_SHADER\n"
       << boxSplineTriangleShaderSource;
    break;
  case Far::PatchDescriptor::GREGORY:
    ss << "#define OSD_PATCH_GREGORY\n"
       << "#define OSD_PATCH_TESS_EVAL_GREGORY_SHADER\n"
       << gregoryShaderSource;
    break;
  case Far::PatchDescriptor::GREGORY_BOUNDARY:
    ss << "#define OSD_PATCH_GREGORY_BOUNDARY\n"
       << "#define OSD_PATCH_TESS_EVAL_GREGORY_SHADER\n"
       << gregoryShaderSource;
    break;
  case Far::PatchDescriptor::GREGORY_BASIS:
    ss << "#define OSD_PATCH_GREGORY_BASIS\n"
       << "#define OSD_PATCH_TESS_EVAL_GREGORY_BASIS_SHADER\n"
       << gregoryBasisShaderSource;
    break;
  case Far::PatchDescriptor::GREGORY_TRIANGLE:
    ss << "#define OSD_PATCH_GREGORY_TRIANGLE\n"
       << "#define OSD_PATCH_TESS_EVAL_GREGORY_TRIANGLE_SHADER\n"
       << gregoryTriangleShaderSource;
    break;
  default:
    break; // returns empty (points, lines, quads, ...)
  }
  return ss.str();
}

} // end namespace Osd

} // end namespace OPENSUBDIV_VERSION
} // end namespace OpenSubdiv
