//
//   Copyright 2013 Pixar
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

#if 0
# if __has_include("GPUShaders/GPUShaders-Swift.h")
  #import "GPUShaders/GPUShaders-Swift.h"
# else
  #define GPU_SHADERS_INC <GPU_SHADERS_SWIFT_OSD_H>
  #import GPU_SHADERS_INC
# endif
#endif // 0

#include "OpenSubdiv/OSDAdaptiveError.h"
#include "OpenSubdiv/OSDSurfaceMTLPatchShaderSource.h"

#include <TargetConditionals.h>
#include <sstream>
#include <string>

namespace OpenSubdiv {
namespace OPENSUBDIV_VERSION {

namespace Osd {

#if 0
/* and here, we use our Swift implementation right out of the box, in cxx. magic. */
static GPUShaders::GPUShaders mtlShaders = GPUShaders::GPUShaders::init();
static swift::String shaderCode = mtlShaders.setupMetal();

static std::string commonShaderSource([shaderCode UTF8String]);
static std::string commonTessShaderSource([shaderCode UTF8String]);
static std::string patchLegacyShaderSource([shaderCode UTF8String]);
static std::string patchBasisTypesShaderSource([shaderCode UTF8String]);
static std::string patchBasisShaderSource([shaderCode UTF8String]);
static std::string bsplineShaderSource([shaderCode UTF8String]);
static std::string boxSplineTriangleShaderSource([shaderCode UTF8String]);
static std::string gregoryShaderSource([shaderCode UTF8String]);
static std::string gregoryBasisShaderSource([shaderCode UTF8String]);
static std::string gregoryTriangleShaderSource([shaderCode UTF8String]);
#else // 0
static std::string commonShaderSource("");
static std::string commonTessShaderSource("");
static std::string patchLegacyShaderSource("");
static std::string patchBasisTypesShaderSource("");
static std::string patchBasisShaderSource("");
static std::string bsplineShaderSource("");
static std::string boxSplineTriangleShaderSource("");
static std::string gregoryShaderSource("");
static std::string gregoryBasisShaderSource("");
static std::string gregoryTriangleShaderSource("");
#endif

static std::string GetPatchTypeDefine(
    Far::PatchDescriptor::Type type,
    Far::PatchDescriptor::Type fvarType = Far::PatchDescriptor::NON_PATCH) {

  std::stringstream ss;
  switch (type) {
  case Far::PatchDescriptor::LINES:
    ss << "#define OSD_PATCH_LINES 1\n";
    break;
  case Far::PatchDescriptor::TRIANGLES:
    ss << "#define OSD_PATCH_TRIANGLES 1\n";
    break;
  case Far::PatchDescriptor::QUADS:
    ss << "#define OSD_PATCH_QUADS 1\n";
    break;
  case Far::PatchDescriptor::REGULAR:
    ss << "#define OSD_PATCH_BSPLINE 1\n"
          "#define OSD_PATCH_REGULAR 1\n";
    break;
  case Far::PatchDescriptor::LOOP:
    ss << "#define OSD_PATCH_BOX_SPLINE_TRIANGLE 1\n";
    break;
  case Far::PatchDescriptor::GREGORY:
    ss << "#define OSD_PATCH_GREGORY 1\n";
    break;
  case Far::PatchDescriptor::GREGORY_BOUNDARY:
    ss << "#define OSD_PATCH_GREGORY_BOUNDARY 1\n";
    break;
  case Far::PatchDescriptor::GREGORY_BASIS:
    ss << "#define OSD_PATCH_GREGORY_BASIS 1\n";
    break;
  case Far::PatchDescriptor::GREGORY_TRIANGLE:
    ss << "#define OSD_PATCH_GREGORY_TRIANGLE 1\n";
    break;
  default:
    assert("Unknown Far::PatchDescriptor::Type" && 0);
    return "";
  }
  switch (fvarType) {
  case Far::PatchDescriptor::REGULAR:
    ss << "#define OSD_FACEVARYING_PATCH_REGULAR 1\n";
    break;
  case Far::PatchDescriptor::GREGORY_BASIS:
    ss << "#define OSD_FACEVARYING_PATCH_GREGORY_BASIS 1\n";
    break;
  default:
    return ss.str();
  }
  return ss.str();
}

static std::string GetPatchTypeSource(Far::PatchDescriptor::Type type) {

  switch (type) {
  case Far::PatchDescriptor::QUADS:
  case Far::PatchDescriptor::TRIANGLES:
    return "";
  case Far::PatchDescriptor::REGULAR:
    return bsplineShaderSource;
  case Far::PatchDescriptor::LOOP:
    return boxSplineTriangleShaderSource;
  case Far::PatchDescriptor::GREGORY:
    return gregoryShaderSource;
  case Far::PatchDescriptor::GREGORY_BOUNDARY:
    return gregoryShaderSource;
  case Far::PatchDescriptor::GREGORY_BASIS:
    return gregoryBasisShaderSource;
  case Far::PatchDescriptor::GREGORY_TRIANGLE:
    return gregoryTriangleShaderSource;
  default:
    assert("Unknown Far::PatchDescriptor::Type" && 0);
    return "";
  }
}

/*static*/
std::string MTLPatchShaderSource::GetPatchDrawingShaderSource() {
#if TARGET_OS_IOS || TARGET_OS_TV
  return std::string("#define OSD_METAL_IOS 1\n")
      .append(commonShaderSource)
      .append(commonTessShaderSource);
#elif TARGET_OS_OSX
  return std::string("#define OSD_METAL_OSX 1\n")
      .append(commonShaderSource)
      .append(commonTessShaderSource);
#endif
}

/*static*/
std::string MTLPatchShaderSource::GetCommonShaderSource() {
  return GetPatchDrawingShaderSource().append(patchLegacyShaderSource);
}

/*static*/
std::string MTLPatchShaderSource::GetPatchBasisShaderSource() {
  std::stringstream ss;
  ss << "#define OSD_PATCH_BASIS_METAL 1\n";
#if defined(OPENSUBDIV_GREGORY_EVAL_TRUE_DERIVATIVES)
  ss << "#define OPENSUBDIV_GREGORY_EVAL_TRUE_DERIVATIVES 1\n";
#endif
  ss << patchBasisTypesShaderSource;
  ss << patchBasisShaderSource;
  return ss.str();
}

/*static*/
std::string
MTLPatchShaderSource::GetVertexShaderSource(Far::PatchDescriptor::Type type) {
  std::stringstream ss;
  ss << GetPatchTypeDefine(type);
  ss << GetCommonShaderSource();
  ss << GetPatchTypeSource(type);
  return ss.str();
}

/*static*/
std::string MTLPatchShaderSource::GetVertexShaderSource(
    Far::PatchDescriptor::Type type, Far::PatchDescriptor::Type fvarType) {
  std::stringstream ss;
  ss << GetPatchTypeDefine(type, fvarType);
  ss << GetCommonShaderSource();
  ss << GetPatchTypeSource(type);
  return ss.str();
}

/*static*/
std::string
MTLPatchShaderSource::GetHullShaderSource(Far::PatchDescriptor::Type type) {
  std::stringstream ss;
  ss << GetPatchTypeDefine(type);
  ss << GetCommonShaderSource();
  ss << GetPatchTypeSource(type);
  return ss.str();
}

/*static*/
std::string
MTLPatchShaderSource::GetHullShaderSource(Far::PatchDescriptor::Type type,
                                          Far::PatchDescriptor::Type fvarType) {
  std::stringstream ss;
  ss << GetPatchTypeDefine(type, fvarType);
  ss << GetCommonShaderSource();
  ss << GetPatchTypeSource(type);
  return ss.str();
}

/*static*/
std::string
MTLPatchShaderSource::GetDomainShaderSource(Far::PatchDescriptor::Type type) {
  std::stringstream ss;
  ss << GetPatchTypeDefine(type);
  ss << GetCommonShaderSource();
  ss << GetPatchTypeSource(type);
  return ss.str();
}

/*static*/
std::string MTLPatchShaderSource::GetDomainShaderSource(
    Far::PatchDescriptor::Type type, Far::PatchDescriptor::Type fvarType) {
  std::stringstream ss;
  ss << GetPatchTypeDefine(type, fvarType);
  ss << GetCommonShaderSource();
  ss << GetPatchTypeSource(type);
  return ss.str();
}

} // end namespace Osd

} // end namespace OPENSUBDIV_VERSION
} // end namespace OpenSubdiv
