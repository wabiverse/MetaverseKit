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

#include "OpenSubdiv/OSDAdaptiveError.h"
#include "OpenSubdiv/OSDSurfaceHLSLPatchShaderSource.h"

#include <sstream>
#include <string>

namespace OpenSubdiv {
namespace OPENSUBDIV_VERSION {

namespace Osd {

static const char *commonShaderSource = "";
static const char *commonTessShaderSource = "";
static const char *patchLegacyShaderSource = "";
static const char *patchBasisTypesShaderSource = "";
static const char *patchBasisShaderSource = "";
static const char *boxSplineTriangleShaderSource = "";
static const char *bsplineShaderSource = "";
static const char *gregoryShaderSource = "";
static const char *gregoryBasisShaderSource = "";
static const char *gregoryTriangleShaderSource = "";

/*static*/
std::string HLSLPatchShaderSource::GetPatchDrawingShaderSource() {
  std::stringstream ss;
  ss << std::string(commonShaderSource);
  ss << std::string(commonTessShaderSource);
  return ss.str();
}

/*static*/
std::string HLSLPatchShaderSource::GetCommonShaderSource() {
  std::stringstream ss;
  ss << GetPatchDrawingShaderSource();
  ss << std::string(patchLegacyShaderSource);
  return ss.str();
}

/*static*/
std::string HLSLPatchShaderSource::GetPatchBasisShaderSource() {
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
HLSLPatchShaderSource::GetVertexShaderSource(Far::PatchDescriptor::Type type) {
  switch (type) {
  case Far::PatchDescriptor::REGULAR:
    return bsplineShaderSource;
  case Far::PatchDescriptor::LOOP:
    return boxSplineTriangleShaderSource;
  case Far::PatchDescriptor::GREGORY:
    return gregoryShaderSource;
  case Far::PatchDescriptor::GREGORY_BOUNDARY:
    return std::string("#define OSD_PATCH_GREGORY_BOUNDRY\n") +
           std::string(gregoryShaderSource);
  case Far::PatchDescriptor::GREGORY_BASIS:
    return gregoryBasisShaderSource;
  case Far::PatchDescriptor::GREGORY_TRIANGLE:
    return gregoryTriangleShaderSource;
  default:
    break; // returns empty (points, lines, quads, ...)
  }
  return std::string();
}

/*static*/
std::string
HLSLPatchShaderSource::GetHullShaderSource(Far::PatchDescriptor::Type type) {
  switch (type) {
  case Far::PatchDescriptor::REGULAR:
    return bsplineShaderSource;
  case Far::PatchDescriptor::LOOP:
    return boxSplineTriangleShaderSource;
  case Far::PatchDescriptor::GREGORY:
    return gregoryShaderSource;
  case Far::PatchDescriptor::GREGORY_BOUNDARY:
    return std::string("#define OSD_PATCH_GREGORY_BOUNDRY\n") +
           std::string(gregoryShaderSource);
  case Far::PatchDescriptor::GREGORY_BASIS:
    return gregoryBasisShaderSource;
  case Far::PatchDescriptor::GREGORY_TRIANGLE:
    return gregoryTriangleShaderSource;
  default:
    break; // returns empty (points, lines, quads, ...)
  }
  return std::string();
}

/*static*/
std::string
HLSLPatchShaderSource::GetDomainShaderSource(Far::PatchDescriptor::Type type) {
  switch (type) {
  case Far::PatchDescriptor::REGULAR:
    return bsplineShaderSource;
  case Far::PatchDescriptor::LOOP:
    return boxSplineTriangleShaderSource;
  case Far::PatchDescriptor::GREGORY:
    return gregoryShaderSource;
  case Far::PatchDescriptor::GREGORY_BOUNDARY:
    return std::string("#define OSD_PATCH_GREGORY_BOUNDRY\n") +
           std::string(gregoryShaderSource);
  case Far::PatchDescriptor::GREGORY_BASIS:
    return gregoryBasisShaderSource;
  case Far::PatchDescriptor::GREGORY_TRIANGLE:
    return gregoryTriangleShaderSource;
  default:
    break; // returns empty (points, lines, quads, ...)
  }
  return std::string();
}

} // end namespace Osd

} // end namespace OPENSUBDIV_VERSION
} // end namespace OpenSubdiv
