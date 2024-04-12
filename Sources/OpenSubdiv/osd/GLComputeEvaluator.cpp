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

#include "glLoader.h"

#include "OpenSubdiv/OSDSurfaceGLComputeEvaluator.h"
#include "OpenSubdiv/OSDSurfaceGLSLPatchShaderSource.h"

#include "OpenSubdiv/OSDAdaptiveError.h"
#include "OpenSubdiv/OSDAdaptiveStencilTable.h"

#include <cassert>
#include <cstring>
#include <sstream>
#include <vector>

namespace OpenSubdiv {
namespace OPENSUBDIV_VERSION {

namespace Osd {

static const char *shaderSource =
"//\n"
"//   Copyright 2013 Pixar\n"
"//\n"
"//   Licensed under the Apache License, Version 2.0 (the \"Apache License\")\n"
"//   with the following modification; you may not use this file except in\n"
"//   compliance with the Apache License and the following modification to it:\n"
"//   Section 6. Trademarks. is deleted and replaced with:\n"
"//\n"
"//   6. Trademarks. This License does not grant permission to use the trade\n"
"//      names, trademarks, service marks, or product names of the Licensor\n"
"//      and its affiliates, except as required to comply with Section 4(c) of\n"
"//      the License and to reproduce the content of the NOTICE file.\n"
"//\n"
"//   You may obtain a copy of the Apache License at\n"
"//\n"
"//       http://www.apache.org/licenses/LICENSE-2.0\n"
"//\n"
"//   Unless required by applicable law or agreed to in writing, software\n"
"//   distributed under the Apache License with the above modification is\n"
"//   distributed on an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY\n"
"//   KIND, either express or implied. See the Apache License for the specific\n"
"//   language governing permissions and limitations under the Apache License.\n"
"//\n"
"\n"
"//------------------------------------------------------------------------------\n"
"\n"
"\n"
"layout(local_size_x=WORK_GROUP_SIZE, local_size_y=1, local_size_z=1) in;\n"
"layout(std430) buffer;\n"
"\n"
"// source and destination buffers\n"
"\n"
"uniform int srcOffset = 0;\n"
"uniform int dstOffset = 0;\n"
"layout(binding=0) buffer src_buffer      { float    srcVertexBuffer[]; };\n"
"layout(binding=1) buffer dst_buffer      { float    dstVertexBuffer[]; };\n"
"\n"
"// derivative buffers (if needed)\n"
"\n"
"#if defined(OPENSUBDIV_GLSL_COMPUTE_USE_1ST_DERIVATIVES)\n"
"uniform ivec3 duDesc;\n"
"uniform ivec3 dvDesc;\n"
"layout(binding=2) buffer du_buffer   { float duBuffer[]; };\n"
"layout(binding=3) buffer dv_buffer   { float dvBuffer[]; };\n"
"#endif\n"
"\n"
"#if defined(OPENSUBDIV_GLSL_COMPUTE_USE_2ND_DERIVATIVES)\n"
"uniform ivec3 duuDesc;\n"
"uniform ivec3 duvDesc;\n"
"uniform ivec3 dvvDesc;\n"
"layout(binding=10) buffer duu_buffer   { float duuBuffer[]; };\n"
"layout(binding=11) buffer duv_buffer   { float duvBuffer[]; };\n"
"layout(binding=12) buffer dvv_buffer   { float dvvBuffer[]; };\n"
"#endif\n"
"\n"
"// stencil buffers\n"
"\n"
"#if defined(OPENSUBDIV_GLSL_COMPUTE_KERNEL_EVAL_STENCILS)\n"
"\n"
"uniform int batchStart = 0;\n"
"uniform int batchEnd = 0;\n"
"layout(binding=4) buffer stencilSizes    { int      _sizes[];   };\n"
"layout(binding=5) buffer stencilOffsets  { int      _offsets[]; };\n"
"layout(binding=6) buffer stencilIndices  { int      _indices[]; };\n"
"layout(binding=7) buffer stencilWeights  { float    _weights[]; };\n"
"\n"
"#if defined(OPENSUBDIV_GLSL_COMPUTE_USE_1ST_DERIVATIVES)\n"
"layout(binding=8) buffer stencilDuWeights { float  _duWeights[]; };\n"
"layout(binding=9) buffer stencilDvWeights { float  _dvWeights[]; };\n"
"#endif\n"
"\n"
"#if defined(OPENSUBDIV_GLSL_COMPUTE_USE_2ND_DERIVATIVES)\n"
"layout(binding=13) buffer stencilDuuWeights { float  _duuWeights[]; };\n"
"layout(binding=14) buffer stencilDuvWeights { float  _duvWeights[]; };\n"
"layout(binding=15) buffer stencilDvvWeights { float  _dvvWeights[]; };\n"
"#endif\n"
"\n"
"#endif\n"
"\n"
"// patch buffers\n"
"\n"
"#if defined(OPENSUBDIV_GLSL_COMPUTE_KERNEL_EVAL_PATCHES)\n"
"\n"
"layout(binding=4) buffer patchArray_buffer { OsdPatchArray patchArrayBuffer[]; };\n"
"layout(binding=5) buffer patchCoord_buffer { OsdPatchCoord patchCoords[]; };\n"
"layout(binding=6) buffer patchIndex_buffer { int patchIndexBuffer[]; };\n"
"layout(binding=7) buffer patchParam_buffer { OsdPatchParam patchParamBuffer[]; };\n"
"\n"
"OsdPatchCoord GetPatchCoord(int coordIndex)\n"
"{\n"
"    return patchCoords[coordIndex];\n"
"}\n"
"\n"
"OsdPatchArray GetPatchArray(int arrayIndex)\n"
"{\n"
"    return patchArrayBuffer[arrayIndex];\n"
"}\n"
"\n"
"OsdPatchParam GetPatchParam(int patchIndex)\n"
"{\n"
"    return patchParamBuffer[patchIndex];\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"\n"
"struct Vertex {\n"
"    float vertexData[LENGTH];\n"
"};\n"
"\n"
"void clear(out Vertex v) {\n"
"    for (int i = 0; i < LENGTH; ++i) {\n"
"        v.vertexData[i] = 0;\n"
"    }\n"
"}\n"
"\n"
"Vertex readVertex(int index) {\n"
"    Vertex v;\n"
"    int vertexIndex = srcOffset + index * SRC_STRIDE;\n"
"    for (int i = 0; i < LENGTH; ++i) {\n"
"        v.vertexData[i] = srcVertexBuffer[vertexIndex + i];\n"
"    }\n"
"    return v;\n"
"}\n"
"\n"
"void writeVertex(int index, Vertex v) {\n"
"    int vertexIndex = dstOffset + index * DST_STRIDE;\n"
"    for (int i = 0; i < LENGTH; ++i) {\n"
"        dstVertexBuffer[vertexIndex + i] = v.vertexData[i];\n"
"    }\n"
"}\n"
"\n"
"void addWithWeight(inout Vertex v, const Vertex src, float weight) {\n"
"    for (int i = 0; i < LENGTH; ++i) {\n"
"        v.vertexData[i] += weight * src.vertexData[i];\n"
"    }\n"
"}\n"
"\n"
"#if defined(OPENSUBDIV_GLSL_COMPUTE_USE_1ST_DERIVATIVES)\n"
"void writeDu(int index, Vertex du) {\n"
"    int duIndex = duDesc.x + index * duDesc.z;\n"
"    for (int i = 0; i < LENGTH; ++i) {\n"
"        duBuffer[duIndex + i] = du.vertexData[i];\n"
"    }\n"
"}\n"
"\n"
"void writeDv(int index, Vertex dv) {\n"
"    int dvIndex = dvDesc.x + index * dvDesc.z;\n"
"    for (int i = 0; i < LENGTH; ++i) {\n"
"        dvBuffer[dvIndex + i] = dv.vertexData[i];\n"
"    }\n"
"}\n"
"#endif\n"
"\n"
"#if defined(OPENSUBDIV_GLSL_COMPUTE_USE_2ND_DERIVATIVES)\n"
"void writeDuu(int index, Vertex duu) {\n"
"    int duuIndex = duuDesc.x + index * duuDesc.z;\n"
"    for (int i = 0; i < LENGTH; ++i) {\n"
"        duuBuffer[duuIndex + i] = duu.vertexData[i];\n"
"    }\n"
"}\n"
"\n"
"void writeDuv(int index, Vertex duv) {\n"
"    int duvIndex = duvDesc.x + index * duvDesc.z;\n"
"    for (int i = 0; i < LENGTH; ++i) {\n"
"        duvBuffer[duvIndex + i] = duv.vertexData[i];\n"
"    }\n"
"}\n"
"\n"
"void writeDvv(int index, Vertex dvv) {\n"
"    int dvvIndex = dvvDesc.x + index * dvvDesc.z;\n"
"    for (int i = 0; i < LENGTH; ++i) {\n"
"        dvvBuffer[dvvIndex + i] = dvv.vertexData[i];\n"
"    }\n"
"}\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"#if defined(OPENSUBDIV_GLSL_COMPUTE_KERNEL_EVAL_STENCILS)\n"
"\n"
"void main() {\n"
"\n"
"    int current = int(gl_GlobalInvocationID.x) + batchStart;\n"
"\n"
"    if (current>=batchEnd) {\n"
"        return;\n"
"    }\n"
"\n"
"    Vertex dst;\n"
"    clear(dst);\n"
"\n"
"    int offset = _offsets[current],\n"
"        size   = _sizes[current];\n"
"\n"
"    for (int stencil = 0; stencil < size; ++stencil) {\n"
"        int vindex = offset + stencil;\n"
"        addWithWeight(\n"
"            dst, readVertex(_indices[vindex]), _weights[vindex]);\n"
"    }\n"
"\n"
"    writeVertex(current, dst);\n"
"\n"
"#if defined(OPENSUBDIV_GLSL_COMPUTE_USE_1ST_DERIVATIVES)\n"
"    Vertex du, dv;\n"
"    clear(du);\n"
"    clear(dv);\n"
"    for (int i=0; i<size; ++i) {\n"
"        // expects the compiler optimizes readVertex out here.\n"
"        Vertex src = readVertex(_indices[offset+i]);\n"
"        addWithWeight(du, src, _duWeights[offset+i]);\n"
"        addWithWeight(dv, src, _dvWeights[offset+i]);\n"
"    }\n"
"\n"
"    if (duDesc.y > 0) { // length\n"
"        writeDu(current, du);\n"
"    }\n"
"    if (dvDesc.y > 0) {\n"
"        writeDv(current, dv);\n"
"    }\n"
"#endif\n"
"#if defined(OPENSUBDIV_GLSL_COMPUTE_USE_2ND_DERIVATIVES)\n"
"    Vertex duu, duv, dvv;\n"
"    clear(duu);\n"
"    clear(duv);\n"
"    clear(dvv);\n"
"    for (int i=0; i<size; ++i) {\n"
"        // expects the compiler optimizes readVertex out here.\n"
"        Vertex src = readVertex(_indices[offset+i]);\n"
"        addWithWeight(duu, src, _duuWeights[offset+i]);\n"
"        addWithWeight(duv, src, _duvWeights[offset+i]);\n"
"        addWithWeight(dvv, src, _dvvWeights[offset+i]);\n"
"    }\n"
"\n"
"    if (duuDesc.y > 0) { // length\n"
"        writeDuu(current, duu);\n"
"    }\n"
"    if (duvDesc.y > 0) {\n"
"        writeDuv(current, duv);\n"
"    }\n"
"    if (dvvDesc.y > 0) {\n"
"        writeDvv(current, dvv);\n"
"    }\n"
"#endif\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"#if defined(OPENSUBDIV_GLSL_COMPUTE_KERNEL_EVAL_PATCHES)\n"
"\n"
"// PERFORMANCE: stride could be constant, but not as significant as length\n"
"\n"
"void main() {\n"
"\n"
"    int current = int(gl_GlobalInvocationID.x);\n"
"\n"
"    OsdPatchCoord coord = GetPatchCoord(current);\n"
"    OsdPatchArray array = GetPatchArray(coord.arrayIndex);\n"
"    OsdPatchParam param = GetPatchParam(coord.patchIndex);\n"
"\n"
"    int patchType = OsdPatchParamIsRegular(param) ? array.regDesc : array.desc;\n"
"\n"
"    float wP[20], wDu[20], wDv[20], wDuu[20], wDuv[20], wDvv[20];\n"
"    int nPoints = OsdEvaluatePatchBasis(patchType, param,\n"
"        coord.s, coord.t, wP, wDu, wDv, wDuu, wDuv, wDvv);\n"
"\n"
"    Vertex dst, du, dv, duu, duv, dvv;\n"
"    clear(dst);\n"
"    clear(du);\n"
"    clear(dv);\n"
"    clear(duu);\n"
"    clear(duv);\n"
"    clear(dvv);\n"
"\n"
"    int indexBase = array.indexBase + array.stride *\n"
"                (coord.patchIndex - array.primitiveIdBase);\n"
"\n"
"    for (int cv = 0; cv < nPoints; ++cv) {\n"
"        int index = patchIndexBuffer[indexBase + cv];\n"
"        addWithWeight(dst, readVertex(index), wP[cv]);\n"
"        addWithWeight(du, readVertex(index), wDu[cv]);\n"
"        addWithWeight(dv, readVertex(index), wDv[cv]);\n"
"        addWithWeight(duu, readVertex(index), wDuu[cv]);\n"
"        addWithWeight(duv, readVertex(index), wDuv[cv]);\n"
"        addWithWeight(dvv, readVertex(index), wDvv[cv]);\n"
"    }\n"
"    writeVertex(current, dst);\n"
"\n"
"#if defined(OPENSUBDIV_GLSL_COMPUTE_USE_1ST_DERIVATIVES)\n"
"    if (duDesc.y > 0) { // length\n"
"        writeDu(current, du);\n"
"    }\n"
"    if (dvDesc.y > 0) {\n"
"        writeDv(current, dv);\n"
"    }\n"
"#endif\n"
"#if defined(OPENSUBDIV_GLSL_COMPUTE_USE_2ND_DERIVATIVES)\n"
"    if (duuDesc.y > 0) { // length\n"
"        writeDuu(current, duu);\n"
"    }\n"
"    if (duvDesc.y > 0) { // length\n"
"        writeDuv(current, duv);\n"
"    }\n"
"    if (dvvDesc.y > 0) {\n"
"        writeDvv(current, dvv);\n"
"    }\n"
"#endif\n"
"}\n"
"\n"
"#endif\n"
"\n"
;

template <class T> GLuint createSSBO(std::vector<T> const &src) {
  if (src.empty()) {
    return 0;
  }

  GLuint devicePtr = 0;

#if defined(GL_ARB_direct_state_access)
  if (OSD_OPENGL_HAS(ARB_direct_state_access)) {
    glCreateBuffers(1, &devicePtr);
    glNamedBufferData(devicePtr, src.size() * sizeof(T), &src.at(0),
                      GL_STATIC_DRAW);
  } else
#endif
  {
    GLint prev = 0;
    OpenSubdiv::internal::GLApi::glGetIntegerv(GL_SHADER_STORAGE_BUFFER_BINDING, &prev);
    OpenSubdiv::internal::GLApi::glGenBuffers(1, &devicePtr);
    OpenSubdiv::internal::GLApi::glBindBuffer(GL_SHADER_STORAGE_BUFFER, devicePtr);
    OpenSubdiv::internal::GLApi::glBufferData(GL_SHADER_STORAGE_BUFFER, src.size() * sizeof(T), &src.at(0),
                                              GL_STATIC_DRAW);
    OpenSubdiv::internal::GLApi::glBindBuffer(GL_SHADER_STORAGE_BUFFER, prev);
  }

  return devicePtr;
}

GLStencilTableSSBO::GLStencilTableSSBO(Far::StencilTable const *stencilTable) {
  _numStencils = stencilTable->GetNumStencils();
  if (_numStencils > 0) {
    _sizes = createSSBO(stencilTable->GetSizes());
    _offsets = createSSBO(stencilTable->GetOffsets());
    _indices = createSSBO(stencilTable->GetControlIndices());
    _weights = createSSBO(stencilTable->GetWeights());
    _duWeights = _dvWeights = 0;
    _duuWeights = _duvWeights = _dvvWeights = 0;
  } else {
    _sizes = _offsets = _indices = _weights = 0;
    _duWeights = _dvWeights = 0;
    _duuWeights = _duvWeights = _dvvWeights = 0;
  }
}

GLStencilTableSSBO::GLStencilTableSSBO(
    Far::LimitStencilTable const *limitStencilTable) {
  _numStencils = limitStencilTable->GetNumStencils();
  if (_numStencils > 0) {
    _sizes = createSSBO(limitStencilTable->GetSizes());
    _offsets = createSSBO(limitStencilTable->GetOffsets());
    _indices = createSSBO(limitStencilTable->GetControlIndices());
    _weights = createSSBO(limitStencilTable->GetWeights());
    _duWeights = createSSBO(limitStencilTable->GetDuWeights());
    _dvWeights = createSSBO(limitStencilTable->GetDvWeights());
    _duuWeights = createSSBO(limitStencilTable->GetDuuWeights());
    _duvWeights = createSSBO(limitStencilTable->GetDuvWeights());
    _dvvWeights = createSSBO(limitStencilTable->GetDvvWeights());
  } else {
    _sizes = _offsets = _indices = _weights = 0;
    _duWeights = _dvWeights = 0;
    _duuWeights = _duvWeights = _dvvWeights = 0;
  }
}

GLStencilTableSSBO::~GLStencilTableSSBO() {
  if (_sizes)
    OpenSubdiv::internal::GLApi::glDeleteBuffers(1, &_sizes);
  if (_offsets)
    OpenSubdiv::internal::GLApi::glDeleteBuffers(1, &_offsets);
  if (_indices)
    OpenSubdiv::internal::GLApi::glDeleteBuffers(1, &_indices);
  if (_weights)
    OpenSubdiv::internal::GLApi::glDeleteBuffers(1, &_weights);
  if (_duWeights)
    OpenSubdiv::internal::GLApi::glDeleteBuffers(1, &_duWeights);
  if (_dvWeights)
    OpenSubdiv::internal::GLApi::glDeleteBuffers(1, &_dvWeights);
  if (_duuWeights)
    OpenSubdiv::internal::GLApi::glDeleteBuffers(1, &_duuWeights);
  if (_duvWeights)
    OpenSubdiv::internal::GLApi::glDeleteBuffers(1, &_duvWeights);
  if (_dvvWeights)
    OpenSubdiv::internal::GLApi::glDeleteBuffers(1, &_dvvWeights);
}

// ---------------------------------------------------------------------------

GLComputeEvaluator::GLComputeEvaluator()
    : _workGroupSize(64), _patchArraysSSBO(0) {
  std::memset((void *)&_stencilKernel, 0, sizeof(_stencilKernel));
  std::memset((void *)&_patchKernel, 0, sizeof(_patchKernel));

  // Initialize internal OpenGL loader library if necessary
  OpenSubdiv::internal::GLLoader::libraryInitializeGL();
}

GLComputeEvaluator::~GLComputeEvaluator() {
  if (_patchArraysSSBO) {
    OpenSubdiv::internal::GLApi::glDeleteBuffers(1, &_patchArraysSSBO);
  }
}

static GLuint
compileKernel(BufferDescriptor const &srcDesc, BufferDescriptor const &dstDesc,
              BufferDescriptor const &duDesc, BufferDescriptor const &dvDesc,
              BufferDescriptor const &duuDesc, BufferDescriptor const &duvDesc,
              BufferDescriptor const &dvvDesc, const char *kernelDefine,
              int workGroupSize) {
  GLuint program = OpenSubdiv::internal::GLApi::glCreateProgram();

  GLuint shader = OpenSubdiv::internal::GLApi::glCreateShader(GL_COMPUTE_SHADER);

  std::string patchBasisShaderSource =
      GLSLPatchShaderSource::GetPatchBasisShaderSource();
  const char *patchBasisShaderSourceDefine = "#define OSD_PATCH_BASIS_GLSL\n";

  std::ostringstream defines;
  defines << "#define LENGTH " << srcDesc.length << "\n"
          << "#define SRC_STRIDE " << srcDesc.stride << "\n"
          << "#define DST_STRIDE " << dstDesc.stride << "\n"
          << "#define WORK_GROUP_SIZE " << workGroupSize << "\n"
          << kernelDefine << "\n"
          << patchBasisShaderSourceDefine << "\n";

  bool deriv1 = (duDesc.length > 0 || dvDesc.length > 0);
  bool deriv2 =
      (duuDesc.length > 0 || duvDesc.length > 0 || dvvDesc.length > 0);
  if (deriv1) {
    defines << "#define OPENSUBDIV_GLSL_COMPUTE_USE_1ST_DERIVATIVES\n";
  }
  if (deriv2) {
    defines << "#define OPENSUBDIV_GLSL_COMPUTE_USE_2ND_DERIVATIVES\n";
  }

  std::string defineStr = defines.str();

  const char *shaderSources[4] = {"#version 430\n", 0, 0, 0};

  shaderSources[1] = defineStr.c_str();
  shaderSources[2] = patchBasisShaderSource.c_str();
  shaderSources[3] = shaderSource;
  OpenSubdiv::internal::GLApi::glShaderSource(shader, 4, shaderSources, NULL);
  OpenSubdiv::internal::GLApi::glCompileShader(shader);
  OpenSubdiv::internal::GLApi::glAttachShader(program, shader);

  GLint linked = 0;
  OpenSubdiv::internal::GLApi::glLinkProgram(program);
  OpenSubdiv::internal::GLApi::glGetProgramiv(program, GL_LINK_STATUS, &linked);

  if (linked == GL_FALSE) {
    char buffer[1024];
    OpenSubdiv::internal::GLApi::glGetShaderInfoLog(shader, 1024, NULL, buffer);
    Far::Error(Far::FAR_RUNTIME_ERROR, buffer);

    OpenSubdiv::internal::GLApi::glGetProgramInfoLog(program, 1024, NULL, buffer);
    Far::Error(Far::FAR_RUNTIME_ERROR, buffer);

    OpenSubdiv::internal::GLApi::glDeleteProgram(program);
    return 0;
  }

  OpenSubdiv::internal::GLApi::glDeleteShader(shader);

  return program;
}

bool GLComputeEvaluator::Compile(BufferDescriptor const &srcDesc,
                                 BufferDescriptor const &dstDesc,
                                 BufferDescriptor const &duDesc,
                                 BufferDescriptor const &dvDesc,
                                 BufferDescriptor const &duuDesc,
                                 BufferDescriptor const &duvDesc,
                                 BufferDescriptor const &dvvDesc) {

  // create a stencil kernel
  if (!_stencilKernel.Compile(srcDesc, dstDesc, duDesc, dvDesc, duuDesc,
                              duvDesc, dvvDesc, _workGroupSize)) {
    return false;
  }

  // create a patch kernel
  if (!_patchKernel.Compile(srcDesc, dstDesc, duDesc, dvDesc, duuDesc, duvDesc,
                            dvvDesc, _workGroupSize)) {
    return false;
  }

  // create a patch arrays buffer
  if (!_patchArraysSSBO) {
    OpenSubdiv::internal::GLApi::glGenBuffers(1, &_patchArraysSSBO);
  }

  return true;
}

/* static */
void GLComputeEvaluator::Synchronize(void * /*kernel*/) {
  // XXX: this is currently just for the performance measuring purpose.
  // need to be reimplemented by fence and sync.
  OpenSubdiv::internal::GLApi::glFinish();
}

bool GLComputeEvaluator::EvalStencils(
    GLuint srcBuffer, BufferDescriptor const &srcDesc, GLuint dstBuffer,
    BufferDescriptor const &dstDesc, GLuint duBuffer,
    BufferDescriptor const &duDesc, GLuint dvBuffer,
    BufferDescriptor const &dvDesc, GLuint sizesBuffer, GLuint offsetsBuffer,
    GLuint indicesBuffer, GLuint weightsBuffer, GLuint duWeightsBuffer,
    GLuint dvWeightsBuffer, int start, int end) const {

  return EvalStencils(srcBuffer, srcDesc, dstBuffer, dstDesc, duBuffer, duDesc,
                      dvBuffer, dvDesc, 0, BufferDescriptor(), 0,
                      BufferDescriptor(), 0, BufferDescriptor(), sizesBuffer,
                      offsetsBuffer, indicesBuffer, weightsBuffer,
                      duWeightsBuffer, dvWeightsBuffer, 0, 0, 0, start, end);
}

bool GLComputeEvaluator::EvalStencils(
    GLuint srcBuffer, BufferDescriptor const &srcDesc, GLuint dstBuffer,
    BufferDescriptor const &dstDesc, GLuint duBuffer,
    BufferDescriptor const &duDesc, GLuint dvBuffer,
    BufferDescriptor const &dvDesc, GLuint duuBuffer,
    BufferDescriptor const &duuDesc, GLuint duvBuffer,
    BufferDescriptor const &duvDesc, GLuint dvvBuffer,
    BufferDescriptor const &dvvDesc, GLuint sizesBuffer, GLuint offsetsBuffer,
    GLuint indicesBuffer, GLuint weightsBuffer, GLuint duWeightsBuffer,
    GLuint dvWeightsBuffer, GLuint duuWeightsBuffer, GLuint duvWeightsBuffer,
    GLuint dvvWeightsBuffer, int start, int end) const {

  if (!_stencilKernel.program)
    return false;
  int count = end - start;
  if (count <= 0) {
    return true;
  }

  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, srcBuffer);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, dstBuffer);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, duBuffer);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, dvBuffer);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, duuBuffer);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, duvBuffer);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, dvvBuffer);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, sizesBuffer);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, offsetsBuffer);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, indicesBuffer);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, weightsBuffer);
  if (duWeightsBuffer)
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, duWeightsBuffer);
  if (dvWeightsBuffer)
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, dvWeightsBuffer);
  if (duuWeightsBuffer)
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, duuWeightsBuffer);
  if (duvWeightsBuffer)
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 14, duvWeightsBuffer);
  if (dvvWeightsBuffer)
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 15, dvvWeightsBuffer);

  GLint activeProgram;
  OpenSubdiv::internal::GLApi::glGetIntegerv(GL_CURRENT_PROGRAM, &activeProgram);
  OpenSubdiv::internal::GLApi::glUseProgram(_stencilKernel.program);

  OpenSubdiv::internal::GLApi::glUniform1i(_stencilKernel.uniformStart, start);
  OpenSubdiv::internal::GLApi::glUniform1i(_stencilKernel.uniformEnd, end);
  OpenSubdiv::internal::GLApi::glUniform1i(_stencilKernel.uniformSrcOffset, srcDesc.offset);
  OpenSubdiv::internal::GLApi::glUniform1i(_stencilKernel.uniformDstOffset, dstDesc.offset);
  if (_stencilKernel.uniformDuDesc > 0) {
    OpenSubdiv::internal::GLApi::glUniform3i(_stencilKernel.uniformDuDesc, duDesc.offset, duDesc.length,
                                             duDesc.stride);
  }
  if (_stencilKernel.uniformDvDesc > 0) {
    OpenSubdiv::internal::GLApi::glUniform3i(_stencilKernel.uniformDvDesc, dvDesc.offset, dvDesc.length,
                                             dvDesc.stride);
  }
  if (_stencilKernel.uniformDuuDesc > 0) {
    OpenSubdiv::internal::GLApi::glUniform3i(_stencilKernel.uniformDuuDesc, duuDesc.offset, duuDesc.length,
                                             duuDesc.stride);
  }
  if (_stencilKernel.uniformDuvDesc > 0) {
    OpenSubdiv::internal::GLApi::glUniform3i(_stencilKernel.uniformDuvDesc, duvDesc.offset, duvDesc.length,
                                             duvDesc.stride);
  }
  if (_stencilKernel.uniformDvvDesc > 0) {
    OpenSubdiv::internal::GLApi::glUniform3i(_stencilKernel.uniformDvvDesc, dvvDesc.offset, dvvDesc.length,
                                             dvvDesc.stride);
  }

  glDispatchCompute((count + _workGroupSize - 1) / _workGroupSize, 1, 1);

  OpenSubdiv::internal::GLApi::glUseProgram(activeProgram);

  glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
  for (int i = 0; i < 16; ++i) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i, 0);
  }

  return true;
}

bool GLComputeEvaluator::EvalPatches(
    GLuint srcBuffer, BufferDescriptor const &srcDesc, GLuint dstBuffer,
    BufferDescriptor const &dstDesc, GLuint duBuffer,
    BufferDescriptor const &duDesc, GLuint dvBuffer,
    BufferDescriptor const &dvDesc, int numPatchCoords,
    GLuint patchCoordsBuffer, const PatchArrayVector &patchArrays,
    GLuint patchIndexBuffer, GLuint patchParamsBuffer) const {

  return EvalPatches(srcBuffer, srcDesc, dstBuffer, dstDesc, duBuffer, duDesc,
                     dvBuffer, dvDesc, 0, BufferDescriptor(), 0,
                     BufferDescriptor(), 0, BufferDescriptor(), numPatchCoords,
                     patchCoordsBuffer, patchArrays, patchIndexBuffer,
                     patchParamsBuffer);
}

bool GLComputeEvaluator::EvalPatches(
    GLuint srcBuffer, BufferDescriptor const &srcDesc, GLuint dstBuffer,
    BufferDescriptor const &dstDesc, GLuint duBuffer,
    BufferDescriptor const &duDesc, GLuint dvBuffer,
    BufferDescriptor const &dvDesc, GLuint duuBuffer,
    BufferDescriptor const &duuDesc, GLuint duvBuffer,
    BufferDescriptor const &duvDesc, GLuint dvvBuffer,
    BufferDescriptor const &dvvDesc, int numPatchCoords,
    GLuint patchCoordsBuffer, const PatchArrayVector &patchArrays,
    GLuint patchIndexBuffer, GLuint patchParamsBuffer) const {

  if (!_patchKernel.program)
    return false;

  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, srcBuffer);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, dstBuffer);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, duBuffer);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, dvBuffer);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, duuBuffer);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, duvBuffer);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, dvvBuffer);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, patchCoordsBuffer);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, patchIndexBuffer);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, patchParamsBuffer);

  GLint activeProgram;
      OpenSubdiv::internal::GLApi::glGetIntegerv(GL_CURRENT_PROGRAM, &activeProgram);
      OpenSubdiv::internal::GLApi::glUseProgram(_patchKernel.program);

      OpenSubdiv::internal::GLApi::glUniform1i(_patchKernel.uniformSrcOffset, srcDesc.offset);
      OpenSubdiv::internal::GLApi::glUniform1i(_patchKernel.uniformDstOffset, dstDesc.offset);

  int patchArraySize = sizeof(PatchArray);
      OpenSubdiv::internal::GLApi::glBindBuffer(GL_SHADER_STORAGE_BUFFER, _patchArraysSSBO);
      OpenSubdiv::internal::GLApi::glBufferData(GL_SHADER_STORAGE_BUFFER, patchArrays.size() * patchArraySize,
               NULL, GL_STATIC_DRAW);
  for (int i = 0; i < (int)patchArrays.size(); ++i) {
    OpenSubdiv::internal::GLApi::glBufferSubData(GL_SHADER_STORAGE_BUFFER, i * patchArraySize,
                                                 sizeof(PatchArray), &patchArrays[i]);
  }
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, _patchArraysSSBO);

  if (_patchKernel.uniformDuDesc > 0) {
    OpenSubdiv::internal::GLApi::glUniform3i(_patchKernel.uniformDuDesc, duDesc.offset, duDesc.length,
                                             duDesc.stride);
  }
  if (_patchKernel.uniformDvDesc > 0) {
    OpenSubdiv::internal::GLApi::glUniform3i(_patchKernel.uniformDvDesc, dvDesc.offset, dvDesc.length,
                                             dvDesc.stride);
  }
  if (_patchKernel.uniformDuuDesc > 0) {
    OpenSubdiv::internal::GLApi::glUniform3i(_patchKernel.uniformDuuDesc, duuDesc.offset, duuDesc.length,
                                             duuDesc.stride);
  }
  if (_patchKernel.uniformDuvDesc > 0) {
    OpenSubdiv::internal::GLApi::glUniform3i(_patchKernel.uniformDuvDesc, duvDesc.offset, duvDesc.length,
                                             duvDesc.stride);
  }
  if (_patchKernel.uniformDvvDesc > 0) {
    OpenSubdiv::internal::GLApi::glUniform3i(_patchKernel.uniformDvvDesc, dvvDesc.offset, dvvDesc.length,
                                             dvvDesc.stride);
  }

  glDispatchCompute((numPatchCoords + _workGroupSize - 1) / _workGroupSize, 1,
                    1);

  OpenSubdiv::internal::GLApi::glUseProgram(activeProgram);

  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, 0);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, 0);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, 0);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, 0);

  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, 0);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, 0);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, 0);

  return true;
}
// ---------------------------------------------------------------------------

GLComputeEvaluator::_StencilKernel::_StencilKernel() : program(0) {}
GLComputeEvaluator::_StencilKernel::~_StencilKernel() {
  if (program) {
    OpenSubdiv::internal::GLApi::glDeleteProgram(program);
  }
}

bool GLComputeEvaluator::_StencilKernel::Compile(
    BufferDescriptor const &srcDesc, BufferDescriptor const &dstDesc,
    BufferDescriptor const &duDesc, BufferDescriptor const &dvDesc,
    BufferDescriptor const &duuDesc, BufferDescriptor const &duvDesc,
    BufferDescriptor const &dvvDesc, int workGroupSize) {
  // create stencil kernel
  if (program) {
    OpenSubdiv::internal::GLApi::glDeleteProgram(program);
  }

  const char *kernelDefine =
      "#define OPENSUBDIV_GLSL_COMPUTE_KERNEL_EVAL_STENCILS\n";

  program = compileKernel(srcDesc, dstDesc, duDesc, dvDesc, duuDesc, duvDesc,
                          dvvDesc, kernelDefine, workGroupSize);
  if (program == 0)
    return false;

  // cache uniform locations (TODO: use uniform block)
  uniformStart = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "batchStart");
  uniformEnd = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "batchEnd");
  uniformSrcOffset = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "srcOffset");
  uniformDstOffset = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "dstOffset");
  uniformDuDesc = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "duDesc");
  uniformDvDesc = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "dvDesc");
  uniformDuuDesc = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "duuDesc");
  uniformDuvDesc = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "duvDesc");
  uniformDvvDesc = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "dvvDesc");

  return true;
}

// ---------------------------------------------------------------------------

GLComputeEvaluator::_PatchKernel::_PatchKernel() : program(0) {}
GLComputeEvaluator::_PatchKernel::~_PatchKernel() {
  if (program) {
    OpenSubdiv::internal::GLApi::glDeleteProgram(program);
  }
}

bool GLComputeEvaluator::_PatchKernel::Compile(
    BufferDescriptor const &srcDesc, BufferDescriptor const &dstDesc,
    BufferDescriptor const &duDesc, BufferDescriptor const &dvDesc,
    BufferDescriptor const &duuDesc, BufferDescriptor const &duvDesc,
    BufferDescriptor const &dvvDesc, int workGroupSize) {
  // create stencil kernel
  if (program) {
    OpenSubdiv::internal::GLApi::glDeleteProgram(program);
  }

  const char *kernelDefine =
      "#define OPENSUBDIV_GLSL_COMPUTE_KERNEL_EVAL_PATCHES\n";

  program = compileKernel(srcDesc, dstDesc, duDesc, dvDesc, duuDesc, duvDesc,
                          dvvDesc, kernelDefine, workGroupSize);
  if (program == 0)
    return false;

  // cache uniform locations
  uniformSrcOffset = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "srcOffset");
  uniformDstOffset = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "dstOffset");
  uniformPatchArray = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "patchArray");
  uniformDuDesc = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "duDesc");
  uniformDvDesc = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "dvDesc");
  uniformDuuDesc = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "duuDesc");
  uniformDuvDesc = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "duvDesc");
  uniformDvvDesc = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "dvvDesc");

  return true;
}

} // end namespace Osd

} // end namespace OPENSUBDIV_VERSION
} // end namespace OpenSubdiv
