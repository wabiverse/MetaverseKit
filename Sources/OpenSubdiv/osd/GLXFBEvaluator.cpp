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

#include "OpenSubdiv/OSDSurfaceGLXFBEvaluator.h"
#include "OpenSubdiv/OSDSurfaceGLSLPatchShaderSource.h"

#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

#include "OpenSubdiv/OSDAdaptiveError.h"
#include "OpenSubdiv/OSDAdaptiveStencilTable.h"

#if _MSC_VER
#define snprintf _snprintf
#endif

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
"uniform samplerBuffer vertexBuffer;\n"
"uniform int srcOffset = 0;\n"
"out float outVertexBuffer[LENGTH];\n"
"\n"
"//------------------------------------------------------------------------------\n"
"\n"
"struct Vertex {\n"
"    float vertexData[LENGTH];\n"
"};\n"
"\n"
"void clear(out Vertex v) {\n"
"    for (int i = 0; i < LENGTH; i++) {\n"
"        v.vertexData[i] = 0;\n"
"    }\n"
"}\n"
"\n"
"void addWithWeight(inout Vertex v, Vertex src, float weight) {\n"
"    for(int j = 0; j < LENGTH; j++) {\n"
"        v.vertexData[j] += weight * src.vertexData[j];\n"
"    }\n"
"}\n"
"\n"
"Vertex readVertex(int index) {\n"
"    Vertex v;\n"
"    int vertexIndex = srcOffset + index * SRC_STRIDE;\n"
"    for(int j = 0; j < LENGTH; j++) {\n"
"        v.vertexData[j] = texelFetch(vertexBuffer, vertexIndex+j).x;\n"
"    }\n"
"    return v;\n"
"}\n"
"\n"
"void writeVertex(Vertex v) {\n"
"    for(int i = 0; i < LENGTH; i++) {\n"
"        outVertexBuffer[i] = v.vertexData[i];\n"
"    }\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(OPENSUBDIV_GLSL_XFB_USE_1ST_DERIVATIVES) && "
"    defined(OPENSUBDIV_GLSL_XFB_INTERLEAVED_1ST_DERIVATIVE_BUFFERS)\n"
"out float outDeriv1Buffer[2*LENGTH];\n"
"\n"
"void writeDu(Vertex v) {\n"
"    for(int i = 0; i < LENGTH; i++) {\n"
"        outDeriv1Buffer[i] = v.vertexData[i];\n"
"    }\n"
"}\n"
"\n"
"void writeDv(Vertex v) {\n"
"    for(int i = 0; i < LENGTH; i++) {\n"
"        outDeriv1Buffer[i+LENGTH] = v.vertexData[i];\n"
"    }\n"
"}\n"
"#elif defined(OPENSUBDIV_GLSL_XFB_USE_1ST_DERIVATIVES)\n"
"out float outDuBuffer[LENGTH];\n"
"out float outDvBuffer[LENGTH];\n"
"\n"
"void writeDu(Vertex v) {\n"
"    for(int i = 0; i < LENGTH; i++) {\n"
"        outDuBuffer[i] = v.vertexData[i];\n"
"    }\n"
"}\n"
"\n"
"void writeDv(Vertex v) {\n"
"    for(int i = 0; i < LENGTH; i++) {\n"
"        outDvBuffer[i] = v.vertexData[i];\n"
"    }\n"
"}\n"
"#endif\n"
"\n"
"#if defined(OPENSUBDIV_GLSL_XFB_USE_2ND_DERIVATIVES) && "
"    defined(OPENSUBDIV_GLSL_XFB_INTERLEAVED_2ND_DERIVATIVE_BUFFERS)\n"
"out float outDeriv2Buffer[3*LENGTH];\n"
"\n"
"void writeDuu(Vertex v) {\n"
"    for(int i = 0; i < LENGTH; i++) {\n"
"        outDeriv2Buffer[i] = v.vertexData[i];\n"
"    }\n"
"}\n"
"\n"
"void writeDuv(Vertex v) {\n"
"    for(int i = 0; i < LENGTH; i++) {\n"
"        outDeriv2Buffer[i+LENGTH] = v.vertexData[i];\n"
"    }\n"
"}\n"
"\n"
"void writeDvv(Vertex v) {\n"
"    for(int i = 0; i < LENGTH; i++) {\n"
"        outDeriv2Buffer[i+2*LENGTH] = v.vertexData[i];\n"
"    }\n"
"}\n"
"#elif defined(OPENSUBDIV_GLSL_XFB_USE_2ND_DERIVATIVES)\n"
"out float outDuuBuffer[LENGTH];\n"
"out float outDuvBuffer[LENGTH];\n"
"out float outDvvBuffer[LENGTH];\n"
"\n"
"void writeDuu(Vertex v) {\n"
"    for(int i = 0; i < LENGTH; i++) {\n"
"        outDuuBuffer[i] = v.vertexData[i];\n"
"    }\n"
"}\n"
"\n"
"void writeDuv(Vertex v) {\n"
"    for(int i = 0; i < LENGTH; i++) {\n"
"        outDuvBuffer[i] = v.vertexData[i];\n"
"    }\n"
"}\n"
"\n"
"void writeDvv(Vertex v) {\n"
"    for(int i = 0; i < LENGTH; i++) {\n"
"        outDvvBuffer[i] = v.vertexData[i];\n"
"    }\n"
"}\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(OPENSUBDIV_GLSL_XFB_KERNEL_EVAL_STENCILS)\n"
"\n"
"uniform usamplerBuffer sizes;\n"
"uniform isamplerBuffer offsets;\n"
"uniform isamplerBuffer indices;\n"
"uniform samplerBuffer  weights;\n"
"\n"
"#if defined(OPENSUBDIV_GLSL_XFB_USE_1ST_DERIVATIVES)\n"
"uniform samplerBuffer  duWeights;\n"
"uniform samplerBuffer  dvWeights;\n"
"#endif\n"
"\n"
"#if defined(OPENSUBDIV_GLSL_XFB_USE_2ND_DERIVATIVES)\n"
"uniform samplerBuffer  duuWeights;\n"
"uniform samplerBuffer  duvWeights;\n"
"uniform samplerBuffer  dvvWeights;\n"
"#endif\n"
"\n"
"uniform int batchStart = 0;\n"
"uniform int batchEnd = 0;\n"
"\n"
"void main() {\n"
"    int current = gl_VertexID + batchStart;\n"
"\n"
"    if (current>=batchEnd) {\n"
"        return;\n"
"    }\n"
"\n"
"    Vertex dst, du, dv, duu, duv, dvv;\n"
"    clear(dst);\n"
"    clear(du);\n"
"    clear(dv);\n"
"    clear(duu);\n"
"    clear(duv);\n"
"    clear(dvv);\n"
"\n"
"    int offset = texelFetch(offsets, current).x;\n"
"    uint size = texelFetch(sizes, current).x;\n"
"\n"
"    for (int stencil=0; stencil<size; ++stencil) {\n"
"        int index = texelFetch(indices, offset+stencil).x;\n"
"        float weight = texelFetch(weights, offset+stencil).x;\n"
"        addWithWeight(dst, readVertex( index ), weight);\n"
"\n"
"#if defined(OPENSUBDIV_GLSL_XFB_USE_1ST_DERIVATIVES)\n"
"        float duWeight = texelFetch(duWeights, offset+stencil).x;\n"
"        float dvWeight = texelFetch(dvWeights, offset+stencil).x;\n"
"        addWithWeight(du,  readVertex(index), duWeight);\n"
"        addWithWeight(dv,  readVertex(index), dvWeight);\n"
"#endif\n"
"#if defined(OPENSUBDIV_GLSL_XFB_USE_2ND_DERIVATIVES)\n"
"        float duuWeight = texelFetch(duuWeights, offset+stencil).x;\n"
"        float duvWeight = texelFetch(duvWeights, offset+stencil).x;\n"
"        float dvvWeight = texelFetch(dvvWeights, offset+stencil).x;\n"
"        addWithWeight(duu,  readVertex(index), duuWeight);\n"
"        addWithWeight(duv,  readVertex(index), duvWeight);\n"
"        addWithWeight(dvv,  readVertex(index), dvvWeight);\n"
"#endif\n"
"    }\n"
"    writeVertex(dst);\n"
"\n"
"#if defined(OPENSUBDIV_GLSL_XFB_USE_1ST_DERIVATIVES)\n"
"    writeDu(du);\n"
"    writeDv(dv);\n"
"#endif\n"
"#if defined(OPENSUBDIV_GLSL_XFB_USE_2ND_DERIVATIVES)\n"
"    writeDuu(duu);\n"
"    writeDuv(duv);\n"
"    writeDvv(dvv);\n"
"#endif\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(OPENSUBDIV_GLSL_XFB_KERNEL_EVAL_PATCHES)\n"
"\n"
"layout (location = 0) in ivec3 patchHandles;\n"
"layout (location = 1) in vec2  patchCoords;\n"
"\n"
"layout (std140) uniform PatchArrays {\n"
"    OsdPatchArray patchArrays[2];\n"
"};\n"
"uniform isamplerBuffer patchParamBuffer;\n"
"uniform isamplerBuffer patchIndexBuffer;\n"
"\n"
"OsdPatchArray GetPatchArray(int arrayIndex) {\n"
"    return patchArrays[arrayIndex];\n"
"}\n"
"\n"
"OsdPatchParam GetPatchParam(int patchIndex) {\n"
"    ivec3 patchParamBits = texelFetch(patchParamBuffer, patchIndex).xyz;\n"
"    return OsdPatchParamInit(patchParamBits.x, patchParamBits.y, patchParamBits.z);\n"
"}\n"
"\n"
"void main() {\n"
"    int current = gl_VertexID;\n"
"\n"
"    ivec3 handle = patchHandles;\n"
"    int arrayIndex = handle.x;\n"
"    int patchIndex = handle.y;\n"
"\n"
"    vec2 coord = patchCoords;\n"
"\n"
"    OsdPatchArray array = GetPatchArray(arrayIndex);\n"
"    OsdPatchParam param = GetPatchParam(patchIndex);\n"
"\n"
"    int patchType = OsdPatchParamIsRegular(param) ? array.regDesc : array.desc;\n"
"\n"
"    float wP[20], wDu[20], wDv[20], wDuu[20], wDuv[20], wDvv[20];\n"
"    int nPoints = OsdEvaluatePatchBasis(patchType, param,\n"
"        coord.x, coord.y, wP, wDu, wDv, wDuu, wDuv, wDvv);\n"
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
"                (patchIndex - array.primitiveIdBase);\n"
"\n"
"    for (int cv = 0; cv < nPoints; ++cv) {\n"
"        int index = texelFetch(patchIndexBuffer, indexBase + cv).x;\n"
"        addWithWeight(dst, readVertex(index), wP[cv]);\n"
"        addWithWeight(du,  readVertex(index), wDu[cv]);\n"
"        addWithWeight(dv,  readVertex(index), wDv[cv]);\n"
"        addWithWeight(duu, readVertex(index), wDuu[cv]);\n"
"        addWithWeight(duv, readVertex(index), wDuv[cv]);\n"
"        addWithWeight(dvv, readVertex(index), wDvv[cv]);\n"
"    }\n"
"\n"
"    writeVertex(dst);\n"
"\n"
"#if defined(OPENSUBDIV_GLSL_XFB_USE_1ST_DERIVATIVES)\n"
"    writeDu(du);\n"
"    writeDv(dv);\n"
"#endif\n"
"#if defined(OPENSUBDIV_GLSL_XFB_USE_2ND_DERIVATIVES)\n"
"    writeDuu(duu);\n"
"    writeDuv(duv);\n"
"    writeDvv(dvv);\n"
"#endif\n"
"}\n"
"\n"
"#endif\n"
"\n"
"\n"
;

template <class T>
GLuint createGLTextureBuffer(std::vector<T> const &src, GLenum type) {
  if (src.empty()) {
    return 0;
  }

  GLint size = static_cast<int>(src.size() * sizeof(T));
  void const *ptr = &src.at(0);

  GLuint buffer = 0;
  GLuint devicePtr = 0;

#if defined(GL_ARB_direct_state_access)
  if (OSD_OPENGL_HAS(ARB_direct_state_access)) {
    glCreateBuffers(1, &buffer);
    glNamedBufferData(buffer, size, ptr, GL_STATIC_DRAW);
    glCreateTextures(GL_TEXTURE_BUFFER, 1, &devicePtr);
    glTextureBuffer(devicePtr, type, buffer);
  } else
#endif
  {
    GLint prev = 0;

    OpenSubdiv::internal::GLApi::glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prev);
    OpenSubdiv::internal::GLApi::glGenBuffers(1, &buffer);
    OpenSubdiv::internal::GLApi::glBindBuffer(GL_ARRAY_BUFFER, buffer);
    OpenSubdiv::internal::GLApi::glBufferData(GL_ARRAY_BUFFER, size, ptr, GL_STATIC_DRAW);
    OpenSubdiv::internal::GLApi::glBindBuffer(GL_ARRAY_BUFFER, prev);

    OpenSubdiv::internal::GLApi::glGetIntegerv(GL_TEXTURE_BINDING_BUFFER, &prev);
    OpenSubdiv::internal::GLApi::glGenTextures(1, &devicePtr);
    OpenSubdiv::internal::GLApi::glBindTexture(GL_TEXTURE_BUFFER, devicePtr);
    glTexBuffer(GL_TEXTURE_BUFFER, type, buffer);
    OpenSubdiv::internal::GLApi::glBindTexture(GL_TEXTURE_BUFFER, prev);
  }

  OpenSubdiv::internal::GLApi::glDeleteBuffers(1, &buffer);

  return devicePtr;
}

GLStencilTableTBO::GLStencilTableTBO(Far::StencilTable const *stencilTable) {

  _numStencils = stencilTable->GetNumStencils();
  if (_numStencils > 0) {
    _sizes = createGLTextureBuffer(stencilTable->GetSizes(), GL_R32UI);
    _offsets = createGLTextureBuffer(stencilTable->GetOffsets(), GL_R32I);
    _indices =
        createGLTextureBuffer(stencilTable->GetControlIndices(), GL_R32I);
    _weights = createGLTextureBuffer(stencilTable->GetWeights(), GL_R32F);
    _duWeights = _dvWeights = 0;
    _duuWeights = _duvWeights = _dvvWeights = 0;
  } else {
    _sizes = _offsets = _indices = _weights = 0;
    _duWeights = _dvWeights = 0;
    _duuWeights = _duvWeights = _dvvWeights = 0;
  }
}

GLStencilTableTBO::GLStencilTableTBO(
    Far::LimitStencilTable const *limitStencilTable) {

  _numStencils = limitStencilTable->GetNumStencils();
  if (_numStencils > 0) {
    _sizes = createGLTextureBuffer(limitStencilTable->GetSizes(), GL_R32UI);
    _offsets = createGLTextureBuffer(limitStencilTable->GetOffsets(), GL_R32I);
    _indices =
        createGLTextureBuffer(limitStencilTable->GetControlIndices(), GL_R32I);
    _weights = createGLTextureBuffer(limitStencilTable->GetWeights(), GL_R32F);
    _duWeights =
        createGLTextureBuffer(limitStencilTable->GetDuWeights(), GL_R32F);
    _dvWeights =
        createGLTextureBuffer(limitStencilTable->GetDvWeights(), GL_R32F);
    _duuWeights =
        createGLTextureBuffer(limitStencilTable->GetDuuWeights(), GL_R32F);
    _duvWeights =
        createGLTextureBuffer(limitStencilTable->GetDuvWeights(), GL_R32F);
    _dvvWeights =
        createGLTextureBuffer(limitStencilTable->GetDvvWeights(), GL_R32F);
  } else {
    _sizes = _offsets = _indices = _weights = 0;
    _duWeights = _dvWeights = 0;
    _duuWeights = _duvWeights = _dvvWeights = 0;
  }
}

GLStencilTableTBO::~GLStencilTableTBO() {
  if (_sizes)
    OpenSubdiv::internal::GLApi::glDeleteTextures(1, &_sizes);
  if (_offsets)
    OpenSubdiv::internal::GLApi::glDeleteTextures(1, &_offsets);
  if (_indices)
    OpenSubdiv::internal::GLApi::glDeleteTextures(1, &_indices);
  if (_weights)
    OpenSubdiv::internal::GLApi::glDeleteTextures(1, &_weights);
  if (_duWeights)
    OpenSubdiv::internal::GLApi::glDeleteTextures(1, &_duWeights);
  if (_dvWeights)
    OpenSubdiv::internal::GLApi::glDeleteTextures(1, &_dvWeights);
  if (_duuWeights)
    OpenSubdiv::internal::GLApi::glDeleteTextures(1, &_duuWeights);
  if (_duvWeights)
    OpenSubdiv::internal::GLApi::glDeleteTextures(1, &_duvWeights);
  if (_dvvWeights)
    OpenSubdiv::internal::GLApi::glDeleteTextures(1, &_dvvWeights);
}

// ---------------------------------------------------------------------------

GLXFBEvaluator::GLXFBEvaluator(bool interleavedDerivativeBuffers)
    : _srcBufferTexture(0), _patchArraysUBO(0),
      _interleavedDerivativeBuffers(interleavedDerivativeBuffers) {

  // Initialize internal OpenGL loader library if necessary
  OpenSubdiv::internal::GLLoader::libraryInitializeGL();
}

GLXFBEvaluator::~GLXFBEvaluator() {
  if (_srcBufferTexture) {
    OpenSubdiv::internal::GLApi::glDeleteTextures(1, &_srcBufferTexture);
  }
  if (_patchArraysUBO) {
    OpenSubdiv::internal::GLApi::glDeleteBuffers(1, &_patchArraysUBO);
  }
}

static GLuint
compileKernel(BufferDescriptor const &srcDesc, BufferDescriptor const &dstDesc,
              BufferDescriptor const &duDesc, BufferDescriptor const &dvDesc,
              BufferDescriptor const &duuDesc, BufferDescriptor const &duvDesc,
              BufferDescriptor const &dvvDesc, const char *kernelDefine,
              bool interleavedDerivativeBuffers) {

  GLuint program = OpenSubdiv::internal::GLApi::glCreateProgram();

  GLuint vertexShader = OpenSubdiv::internal::GLApi::glCreateShader(GL_VERTEX_SHADER);

  std::string patchBasisShaderSource =
      GLSLPatchShaderSource::GetPatchBasisShaderSource();
  const char *patchBasisShaderSourceDefine = "#define OSD_PATCH_BASIS_GLSL\n";

  std::ostringstream defines;
  defines << "#define LENGTH " << srcDesc.length << "\n"
          << "#define SRC_STRIDE " << srcDesc.stride << "\n"
          << "#define VERTEX_SHADER\n"
          << kernelDefine << "\n"
          << patchBasisShaderSourceDefine << "\n";

  bool deriv1 = (duDesc.length > 0 || dvDesc.length > 0);
  bool deriv2 =
      (duuDesc.length > 0 || duvDesc.length > 0 || dvvDesc.length > 0);
  if (deriv1) {
    defines << "#define OPENSUBDIV_GLSL_XFB_USE_1ST_DERIVATIVES\n";
    if (interleavedDerivativeBuffers) {
      defines
          << "#define OPENSUBDIV_GLSL_XFB_INTERLEAVED_1ST_DERIVATIVE_BUFFERS\n";
    }
  }
  if (deriv2) {
    defines << "#define OPENSUBDIV_GLSL_XFB_USE_2ND_DERIVATIVES\n";
    if (interleavedDerivativeBuffers) {
      defines
          << "#define OPENSUBDIV_GLSL_XFB_INTERLEAVED_2ND_DERIVATIVE_BUFFERS\n";
    }
  }

  std::string defineStr = defines.str();

  const char *shaderSources[4] = {"#version 410\n", NULL, NULL, NULL};

  shaderSources[1] = defineStr.c_str();
  shaderSources[2] = patchBasisShaderSource.c_str();
  shaderSources[3] = shaderSource;
  OpenSubdiv::internal::GLApi::glShaderSource(vertexShader, 4, shaderSources, NULL);
  OpenSubdiv::internal::GLApi::glCompileShader(vertexShader);
  OpenSubdiv::internal::GLApi::glAttachShader(program, vertexShader);

  std::vector<std::string> outputs;
  char attrName[32];
  {
    // vertex data (may include custom vertex data) and varying data
    // are stored into the same buffer, interleaved.
    //
    // (gl_SkipComponents1)
    // outVertexData[0]
    // outVertexData[1]
    // outVertexData[2]
    // (gl_SkipComponents1)
    //
    // note that "primvarOffset" in shader is still needed to read
    // interleaved components even if gl_SkipComponents is used.
    //
    int primvarOffset = (dstDesc.offset % dstDesc.stride);
    for (int i = 0; i < primvarOffset; ++i) {
      outputs.push_back("gl_SkipComponents1");
    }
    for (int i = 0; i < dstDesc.length; ++i) {
      snprintf(attrName, sizeof(attrName), "outVertexBuffer[%d]", i);
      outputs.push_back(attrName);
    }
    for (int i = primvarOffset + dstDesc.length; i < dstDesc.stride; ++i) {
      outputs.push_back("gl_SkipComponents1");
    }
  }
  //
  // For derivatives, we use another buffer bindings so gl_NextBuffer
  // is inserted here to switch the destination of transform feedback.
  //
  // Note that the destination buffers may or may not be interleaved between
  // vertex and each derivatives. gl_NextBuffer seems still works well
  // in either case.
  //
  // If we know that the buffers for derivatives are interleaved, then we
  // can use fewer buffer bindings. This can be important, since most GL
  // implementations will support only up to 4 transform feedback bindings.
  //
  if (deriv1 && interleavedDerivativeBuffers) {
    outputs.push_back("gl_NextBuffer");

    int primvar1Offset = (duDesc.offset % duDesc.stride);
    int primvar2Offset = (dvDesc.offset % dvDesc.stride);

    for (int i = 0; i < primvar1Offset; ++i) {
      outputs.push_back("gl_SkipComponents1");
    }
    for (int i = 0; i < duDesc.length; ++i) {
      snprintf(attrName, sizeof(attrName), "outDeriv1Buffer[%d]", i);
      outputs.push_back(attrName);
    }
    for (int i = primvar1Offset + duDesc.length; i < primvar2Offset; ++i) {
      outputs.push_back("gl_SkipComponents1");
    }
    for (int i = 0; i < dvDesc.length; ++i) {
      snprintf(attrName, sizeof(attrName), "outDeriv1Buffer[%d]",
               i + duDesc.length);
      outputs.push_back(attrName);
    }
    for (int i = primvar2Offset + dvDesc.length; i < dvDesc.stride; ++i) {
      outputs.push_back("gl_SkipComponents1");
    }
  } else {
    if (duDesc.length) {
      outputs.push_back("gl_NextBuffer");
      int primvarOffset = (duDesc.offset % duDesc.stride);
      for (int i = 0; i < primvarOffset; ++i) {
        outputs.push_back("gl_SkipComponents1");
      }
      for (int i = 0; i < duDesc.length; ++i) {
        snprintf(attrName, sizeof(attrName), "outDuBuffer[%d]", i);
        outputs.push_back(attrName);
      }
      for (int i = primvarOffset + duDesc.length; i < duDesc.stride; ++i) {
        outputs.push_back("gl_SkipComponents1");
      }
    }
    if (dvDesc.length) {
      outputs.push_back("gl_NextBuffer");
      int primvarOffset = (dvDesc.offset % dvDesc.stride);
      for (int i = 0; i < primvarOffset; ++i) {
        outputs.push_back("gl_SkipComponents1");
      }
      for (int i = 0; i < dvDesc.length; ++i) {
        snprintf(attrName, sizeof(attrName), "outDvBuffer[%d]", i);
        outputs.push_back(attrName);
      }
      for (int i = primvarOffset + dvDesc.length; i < dvDesc.stride; ++i) {
        outputs.push_back("gl_SkipComponents1");
      }
    }
  }
  if (deriv2 && interleavedDerivativeBuffers) {
    outputs.push_back("gl_NextBuffer");

    int primvar1Offset = (duuDesc.offset % duuDesc.stride);
    int primvar2Offset = (duvDesc.offset % duvDesc.stride);
    int primvar3Offset = (dvvDesc.offset % dvvDesc.stride);

    for (int i = 0; i < primvar1Offset; ++i) {
      outputs.push_back("gl_SkipComponents1");
    }
    for (int i = 0; i < duuDesc.length; ++i) {
      snprintf(attrName, sizeof(attrName), "outDeriv2Buffer[%d]", i);
      outputs.push_back(attrName);
    }

    for (int i = primvar1Offset + duuDesc.length; i < primvar2Offset; ++i) {
      outputs.push_back("gl_SkipComponents1");
    }
    for (int i = 0; i < duvDesc.length; ++i) {
      snprintf(attrName, sizeof(attrName), "outDeriv2Buffer[%d]",
               i + duuDesc.length);
      outputs.push_back(attrName);
    }

    for (int i = primvar2Offset + duvDesc.length; i < primvar3Offset; ++i) {
      outputs.push_back("gl_SkipComponents1");
    }
    for (int i = 0; i < dvvDesc.length; ++i) {
      snprintf(attrName, sizeof(attrName), "outDeriv2Buffer[%d]",
               i + duuDesc.length + duvDesc.length);
      outputs.push_back(attrName);
    }

    for (int i = primvar3Offset + dvvDesc.length; i < dvvDesc.stride; ++i) {
      outputs.push_back("gl_SkipComponents1");
    }
  } else {
    if (duuDesc.length) {
      outputs.push_back("gl_NextBuffer");
      int primvarOffset = (duuDesc.offset % duuDesc.stride);
      for (int i = 0; i < primvarOffset; ++i) {
        outputs.push_back("gl_SkipComponents1");
      }
      for (int i = 0; i < duuDesc.length; ++i) {
        snprintf(attrName, sizeof(attrName), "outDuuBuffer[%d]", i);
        outputs.push_back(attrName);
      }
      for (int i = primvarOffset + duuDesc.length; i < duuDesc.stride; ++i) {
        outputs.push_back("gl_SkipComponents1");
      }
    }
    if (duvDesc.length) {
      outputs.push_back("gl_NextBuffer");
      int primvarOffset = (duvDesc.offset % duvDesc.stride);
      for (int i = 0; i < primvarOffset; ++i) {
        outputs.push_back("gl_SkipComponents1");
      }
      for (int i = 0; i < duvDesc.length; ++i) {
        snprintf(attrName, sizeof(attrName), "outDuvBuffer[%d]", i);
        outputs.push_back(attrName);
      }
      for (int i = primvarOffset + duvDesc.length; i < duvDesc.stride; ++i) {
        outputs.push_back("gl_SkipComponents1");
      }
    }
    if (dvvDesc.length) {
      outputs.push_back("gl_NextBuffer");
      int primvarOffset = (dvvDesc.offset % dvvDesc.stride);
      for (int i = 0; i < primvarOffset; ++i) {
        outputs.push_back("gl_SkipComponents1");
      }
      for (int i = 0; i < dvvDesc.length; ++i) {
        snprintf(attrName, sizeof(attrName), "outDvvBuffer[%d]", i);
        outputs.push_back(attrName);
      }
      for (int i = primvarOffset + dvvDesc.length; i < dvvDesc.stride; ++i) {
        outputs.push_back("gl_SkipComponents1");
      }
    }
  }
  // convert to char* array
  std::vector<const char *> pOutputs;
  for (size_t i = 0; i < outputs.size(); ++i) {
    pOutputs.push_back(&outputs[i][0]);
  }

  glTransformFeedbackVaryings(program, (GLsizei)outputs.size(), &pOutputs[0],
                              GL_INTERLEAVED_ATTRIBS);

  GLint linked = 0;
  OpenSubdiv::internal::GLApi::glLinkProgram(program);
  OpenSubdiv::internal::GLApi::glGetProgramiv(program, GL_LINK_STATUS, &linked);

  if (linked == GL_FALSE) {
    char buffer[1024];
    OpenSubdiv::internal::GLApi::glGetShaderInfoLog(vertexShader, 1024, NULL, buffer);
    Far::Error(Far::FAR_RUNTIME_ERROR, buffer);

    OpenSubdiv::internal::GLApi::glGetProgramInfoLog(program, 1024, NULL, buffer);
    Far::Error(Far::FAR_RUNTIME_ERROR, buffer);

    OpenSubdiv::internal::GLApi::glDeleteProgram(program);
    program = 0;
  }

  OpenSubdiv::internal::GLApi::glDeleteShader(vertexShader);

  return program;
}

bool GLXFBEvaluator::Compile(BufferDescriptor const &srcDesc,
                             BufferDescriptor const &dstDesc,
                             BufferDescriptor const &duDesc,
                             BufferDescriptor const &dvDesc,
                             BufferDescriptor const &duuDesc,
                             BufferDescriptor const &duvDesc,
                             BufferDescriptor const &dvvDesc) {

  // create a stencil kernel
  _stencilKernel.Compile(srcDesc, dstDesc, duDesc, dvDesc, duuDesc, duvDesc,
                         dvvDesc, _interleavedDerivativeBuffers);

  // create a patch kernel
  _patchKernel.Compile(srcDesc, dstDesc, duDesc, dvDesc, duuDesc, duvDesc,
                       dvvDesc, _interleavedDerivativeBuffers);

  // create a texture for input buffer
  if (!_srcBufferTexture) {
    OpenSubdiv::internal::GLApi::glGenTextures(1, &_srcBufferTexture);
  }
  if (!_patchArraysUBO) {
    OpenSubdiv::internal::GLApi::glGenBuffers(1, &_patchArraysUBO);
  }
  return true;
}

/* static */
void GLXFBEvaluator::Synchronize(void * /*kernel*/) {
  // XXX: this is currently just for the test purpose.
  // need to be reimplemented by fence and sync.
  OpenSubdiv::internal::GLApi::glFinish();
}

static void bindTexture(GLint sampler, GLuint texture, int unit) {
  if (sampler == -1) {
    return;
  }
  OpenSubdiv::internal::GLApi::glUniform1i(sampler, unit);
  OpenSubdiv::internal::GLApi::glActiveTexture(GL_TEXTURE0 + unit);
  OpenSubdiv::internal::GLApi::glBindTexture(GL_TEXTURE_BUFFER, texture);
  OpenSubdiv::internal::GLApi::glActiveTexture(GL_TEXTURE0);
}

bool GLXFBEvaluator::EvalStencils(
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

bool GLXFBEvaluator::EvalStencils(
    GLuint srcBuffer, BufferDescriptor const &srcDesc, GLuint dstBuffer,
    BufferDescriptor const &dstDesc, GLuint duBuffer,
    BufferDescriptor const &duDesc, GLuint dvBuffer,
    BufferDescriptor const &dvDesc, GLuint duuBuffer,
    BufferDescriptor const &duuDesc, GLuint duvBuffer,
    BufferDescriptor const &duvDesc, GLuint dvvBuffer,
    BufferDescriptor const &dvvDesc, GLuint sizesTexture, GLuint offsetsTexture,
    GLuint indicesTexture, GLuint weightsTexture, GLuint duWeightsTexture,
    GLuint dvWeightsTexture, GLuint duuWeightsTexture, GLuint duvWeightsTexture,
    GLuint dvvWeightsTexture, int start, int end) const {

  if (!_stencilKernel.program)
    return false;
  int count = end - start;
  if (count <= 0) {
    return true;
  }

  // bind vertex array
  // always create new one, to be safe with multiple contexts (slow though)
  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  OpenSubdiv::internal::GLApi::glEnable(GL_RASTERIZER_DISCARD);
  OpenSubdiv::internal::GLApi::glUseProgram(_stencilKernel.program);

  // Set input VBO as a texture buffer.
  OpenSubdiv::internal::GLApi::glBindTexture(GL_TEXTURE_BUFFER, _srcBufferTexture);
  glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, srcBuffer);
  OpenSubdiv::internal::GLApi::glBindTexture(GL_TEXTURE_BUFFER, 0);

  bindTexture(_stencilKernel.uniformSrcBufferTexture, _srcBufferTexture, 0);

  // bind stencil table textures.
  bindTexture(_stencilKernel.uniformSizesTexture, sizesTexture, 1);
  bindTexture(_stencilKernel.uniformOffsetsTexture, offsetsTexture, 2);
  bindTexture(_stencilKernel.uniformIndicesTexture, indicesTexture, 3);
  bindTexture(_stencilKernel.uniformWeightsTexture, weightsTexture, 4);
  if (_stencilKernel.uniformDuWeightsTexture >= 0 && duWeightsTexture)
    bindTexture(_stencilKernel.uniformDuWeightsTexture, duWeightsTexture, 5);
  if (_stencilKernel.uniformDvWeightsTexture >= 0 && dvWeightsTexture)
    bindTexture(_stencilKernel.uniformDvWeightsTexture, dvWeightsTexture, 6);
  if (_stencilKernel.uniformDuuWeightsTexture >= 0 && duuWeightsTexture)
    bindTexture(_stencilKernel.uniformDuuWeightsTexture, duuWeightsTexture, 7);
  if (_stencilKernel.uniformDuvWeightsTexture >= 0 && duvWeightsTexture)
    bindTexture(_stencilKernel.uniformDuvWeightsTexture, duvWeightsTexture, 8);
  if (_stencilKernel.uniformDvvWeightsTexture >= 0 && dvvWeightsTexture)
    bindTexture(_stencilKernel.uniformDvvWeightsTexture, dvvWeightsTexture, 9);

  // set batch range
  OpenSubdiv::internal::GLApi::glUniform1i(_stencilKernel.uniformStart, start);
  OpenSubdiv::internal::GLApi::glUniform1i(_stencilKernel.uniformEnd, end);
  OpenSubdiv::internal::GLApi::glUniform1i(_stencilKernel.uniformSrcOffset, srcDesc.offset);

  // The destination buffer is bound at vertex boundary.
  //
  // Example: When we have a batched and interleaved vertex buffer
  //
  //  Obj  X    |    Obj Y                                  |
  // -----------+-------------------------------------------+-------
  //            |    vtx 0      |    vtx 1      |           |
  // -----------+---------------+---------------+-----------+-------
  //            | x y z r g b a | x y z r g b a | ....      |
  // -----------+---------------+---------------+-----------+-------
  //                    ^
  //                    srcDesc.offset for Obj Y color
  //
  //            ^-------------------------------------------^
  //                    XFB destination buffer range
  //              S S S * * * *
  //              k k k
  //              i i i
  //              p p p
  //
  //  We use gl_SkipComponents to skip the first 3 XYZ so the
  //  buffer itself needs to be bound for entire section of ObjY.
  //
  //  Note that for the source buffer (texture) we bind the whole
  //  buffer (all VBO range) and use srcOffset=srcDesc.offset for
  //  indexing.
  //
  int dstBufferBindOffset =
      dstDesc.stride ? (dstDesc.offset - (dstDesc.offset % dstDesc.stride)) : 0;
  int duBufferBindOffset =
      duDesc.stride ? (duDesc.offset - (duDesc.offset % duDesc.stride)) : 0;
  int dvBufferBindOffset =
      dvDesc.stride ? (dvDesc.offset - (dvDesc.offset % dvDesc.stride)) : 0;
  int duuBufferBindOffset =
      duuDesc.stride ? (duuDesc.offset - (duuDesc.offset % duuDesc.stride)) : 0;
  int duvBufferBindOffset =
      duvDesc.stride ? (duvDesc.offset - (duvDesc.offset % duvDesc.stride)) : 0;
  int dvvBufferBindOffset =
      dvvDesc.stride ? (dvvDesc.offset - (dvvDesc.offset % dvvDesc.stride)) : 0;

  // bind destination buffer
  glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, dstBuffer,
                    dstBufferBindOffset * sizeof(float),
                    count * dstDesc.stride * sizeof(float));

  if ((duDesc.length > 0) && _interleavedDerivativeBuffers) {
    glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 1, duBuffer,
                      duBufferBindOffset * sizeof(float),
                      count * duDesc.stride * sizeof(float));
  } else {
    if (duDesc.length > 0) {
      glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 1, duBuffer,
                        duBufferBindOffset * sizeof(float),
                        count * duDesc.stride * sizeof(float));
    }

    if (dvDesc.length > 0) {
      glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 2, dvBuffer,
                        dvBufferBindOffset * sizeof(float),
                        count * dvDesc.stride * sizeof(float));
    }
  }

  if ((duuDesc.length > 0) && _interleavedDerivativeBuffers) {
    glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 2, duuBuffer,
                      duuBufferBindOffset * sizeof(float),
                      count * duuDesc.stride * sizeof(float));
  } else {
    if (duuDesc.length > 0) {
      glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 3, duuBuffer,
                        duuBufferBindOffset * sizeof(float),
                        count * duuDesc.stride * sizeof(float));
    }

    if (duvDesc.length > 0) {
      glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 4, duvBuffer,
                        duvBufferBindOffset * sizeof(float),
                        count * duvDesc.stride * sizeof(float));
    }

    if (dvvDesc.length > 0) {
      glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 5, dvvBuffer,
                        dvvBufferBindOffset * sizeof(float),
                        count * dvvDesc.stride * sizeof(float));
    }
  }

  glBeginTransformFeedback(GL_POINTS);
  OpenSubdiv::internal::GLApi::glDrawArrays(GL_POINTS, 0, count);
  glEndTransformFeedback();

  OpenSubdiv::internal::GLApi::glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);

  for (int i = 0; i < 6; ++i) {
    OpenSubdiv::internal::GLApi::glActiveTexture(GL_TEXTURE0 + i);
    OpenSubdiv::internal::GLApi::glBindTexture(GL_TEXTURE_BUFFER, 0);
  }

  OpenSubdiv::internal::GLApi::glDisable(GL_RASTERIZER_DISCARD);
  OpenSubdiv::internal::GLApi::glUseProgram(0);
  OpenSubdiv::internal::GLApi::glActiveTexture(GL_TEXTURE0);

  // revert vao
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &vao);

  return true;
}

bool GLXFBEvaluator::EvalPatches(
    GLuint srcBuffer, BufferDescriptor const &srcDesc, GLuint dstBuffer,
    BufferDescriptor const &dstDesc, GLuint duBuffer,
    BufferDescriptor const &duDesc, GLuint dvBuffer,
    BufferDescriptor const &dvDesc, int numPatchCoords,
    GLuint patchCoordsBuffer, const PatchArrayVector &patchArrays,
    GLuint patchIndexTexture, GLuint patchParamTexture) const {

  return EvalPatches(srcBuffer, srcDesc, dstBuffer, dstDesc, duBuffer, duDesc,
                     dvBuffer, dvDesc, 0, BufferDescriptor(), 0,
                     BufferDescriptor(), 0, BufferDescriptor(), numPatchCoords,
                     patchCoordsBuffer, patchArrays, patchIndexTexture,
                     patchParamTexture);
}

bool GLXFBEvaluator::EvalPatches(
    GLuint srcBuffer, BufferDescriptor const &srcDesc, GLuint dstBuffer,
    BufferDescriptor const &dstDesc, GLuint duBuffer,
    BufferDescriptor const &duDesc, GLuint dvBuffer,
    BufferDescriptor const &dvDesc, GLuint duuBuffer,
    BufferDescriptor const &duuDesc, GLuint duvBuffer,
    BufferDescriptor const &duvDesc, GLuint dvvBuffer,
    BufferDescriptor const &dvvDesc, int numPatchCoords,
    GLuint patchCoordsBuffer, const PatchArrayVector &patchArrays,
    GLuint patchIndexTexture, GLuint patchParamTexture) const {

  bool deriv1 = (duDesc.length > 0 || dvDesc.length > 0);
  bool deriv2 =
      (duuDesc.length > 0 || duvDesc.length > 0 || dvvDesc.length > 0);

  if (!_patchKernel.program)
    return false;

  // bind vertex array
  // always create new one, to be safe with multiple contexts (slow though)
  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  OpenSubdiv::internal::GLApi::glEnable(GL_RASTERIZER_DISCARD);
  OpenSubdiv::internal::GLApi::glUseProgram(_patchKernel.program);

  // Set input VBO as a texture buffer.
  OpenSubdiv::internal::GLApi::glBindTexture(GL_TEXTURE_BUFFER, _srcBufferTexture);
  glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, srcBuffer);
  OpenSubdiv::internal::GLApi::glBindTexture(GL_TEXTURE_BUFFER, 0);

  bindTexture(_patchKernel.uniformSrcBufferTexture, _srcBufferTexture, 0);

  // bind patch index and patch param textures.
  bindTexture(_patchKernel.uniformPatchParamTexture, patchParamTexture, 1);
  bindTexture(_patchKernel.uniformPatchIndexTexture, patchIndexTexture, 2);

  // bind patch arrays UBO (std140 struct size padded to vec4 alignment)
  int patchArraySize =
      sizeof(GLint) * ((sizeof(PatchArray) / sizeof(GLint) + 3) & ~3);
  OpenSubdiv::internal::GLApi::glBindBuffer(GL_UNIFORM_BUFFER, _patchArraysUBO);
  OpenSubdiv::internal::GLApi::glBufferData(GL_UNIFORM_BUFFER, patchArrays.size() * patchArraySize, NULL,
               GL_STATIC_DRAW);
  for (int i = 0; i < (int)patchArrays.size(); ++i) {
    OpenSubdiv::internal::GLApi::glBufferSubData(GL_UNIFORM_BUFFER, i * patchArraySize, sizeof(PatchArray),
                                                 &patchArrays[i]);
  }
  glBindBufferBase(GL_UNIFORM_BUFFER, _patchKernel.uniformPatchArraysUBOBinding,
                   _patchArraysUBO);

  // set other uniforms
      OpenSubdiv::internal::GLApi::glUniform1i(_patchKernel.uniformSrcOffset, srcDesc.offset);

  // input patchcoords
  OpenSubdiv::internal::GLApi::glEnableVertexAttribArray(0);
  OpenSubdiv::internal::GLApi::glEnableVertexAttribArray(1);
  int stride = sizeof(int) * 5; // patchcoord = int*5 struct
  OpenSubdiv::internal::GLApi::glBindBuffer(GL_ARRAY_BUFFER, patchCoordsBuffer);
  glVertexAttribIPointer(0, 3, GL_UNSIGNED_INT, stride, (void *)0);
  OpenSubdiv::internal::GLApi::glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
                                                     (void *)(sizeof(int) * 3));

  int dstBufferBindOffset = dstDesc.offset - (dstDesc.offset % dstDesc.stride);
  int duBufferBindOffset =
      duDesc.stride ? (duDesc.offset - (duDesc.offset % duDesc.stride)) : 0;
  int dvBufferBindOffset =
      dvDesc.stride ? (dvDesc.offset - (dvDesc.offset % dvDesc.stride)) : 0;
  int duuBufferBindOffset =
      duuDesc.stride ? (duuDesc.offset - (duuDesc.offset % duuDesc.stride)) : 0;
  int duvBufferBindOffset =
      duvDesc.stride ? (duvDesc.offset - (duvDesc.offset % duvDesc.stride)) : 0;
  int dvvBufferBindOffset =
      dvvDesc.stride ? (dvvDesc.offset - (dvvDesc.offset % dvvDesc.stride)) : 0;

  // bind destination buffer
  glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, dstBuffer,
                    dstBufferBindOffset * sizeof(float),
                    numPatchCoords * dstDesc.stride * sizeof(float));

  if (deriv1 && _interleavedDerivativeBuffers) {
    glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 1, duBuffer,
                      duBufferBindOffset * sizeof(float),
                      numPatchCoords * duDesc.stride * sizeof(float));
  } else if (deriv1) {
    glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 1, duBuffer,
                      duBufferBindOffset * sizeof(float),
                      numPatchCoords * duDesc.stride * sizeof(float));

    glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 2, dvBuffer,
                      dvBufferBindOffset * sizeof(float),
                      numPatchCoords * dvDesc.stride * sizeof(float));
  }
  if (deriv2 && _interleavedDerivativeBuffers) {
    glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 2, duuBuffer,
                      duuBufferBindOffset * sizeof(float),
                      numPatchCoords * duuDesc.stride * sizeof(float));
  } else if (deriv2) {
    glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 3, duuBuffer,
                      duuBufferBindOffset * sizeof(float),
                      numPatchCoords * duuDesc.stride * sizeof(float));

    glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 4, duvBuffer,
                      duvBufferBindOffset * sizeof(float),
                      numPatchCoords * duvDesc.stride * sizeof(float));

    glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 5, dvvBuffer,
                      dvvBufferBindOffset * sizeof(float),
                      numPatchCoords * dvvDesc.stride * sizeof(float));
  }

  glBeginTransformFeedback(GL_POINTS);
  OpenSubdiv::internal::GLApi::glDrawArrays(GL_POINTS, 0, numPatchCoords);
  glEndTransformFeedback();

  OpenSubdiv::internal::GLApi::glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);

  // unbind textures
  for (int i = 0; i < 6; ++i) {
    OpenSubdiv::internal::GLApi::glActiveTexture(GL_TEXTURE0 + i);
    OpenSubdiv::internal::GLApi::glBindTexture(GL_TEXTURE_BUFFER, 0);
  }

  // unbind UBO
  glBindBufferBase(GL_UNIFORM_BUFFER, _patchKernel.uniformPatchArraysUBOBinding,
                   0);

  OpenSubdiv::internal::GLApi::glDisable(GL_RASTERIZER_DISCARD);
  OpenSubdiv::internal::GLApi::glUseProgram(0);
  OpenSubdiv::internal::GLApi::glActiveTexture(GL_TEXTURE0);

  OpenSubdiv::internal::GLApi::glDisableVertexAttribArray(0);
  OpenSubdiv::internal::GLApi::glDisableVertexAttribArray(1);

  // revert vao
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &vao);

  return true;
}

// ---------------------------------------------------------------------------

GLXFBEvaluator::_StencilKernel::_StencilKernel() : program(0) {}
GLXFBEvaluator::_StencilKernel::~_StencilKernel() {
  if (program) {
    OpenSubdiv::internal::GLApi::glDeleteProgram(program);
  }
}

bool GLXFBEvaluator::_StencilKernel::Compile(
    BufferDescriptor const &srcDesc, BufferDescriptor const &dstDesc,
    BufferDescriptor const &duDesc, BufferDescriptor const &dvDesc,
    BufferDescriptor const &duuDesc, BufferDescriptor const &duvDesc,
    BufferDescriptor const &dvvDesc, bool interleavedDerivativeBuffers) {
  // create stencil kernel
  if (program) {
    OpenSubdiv::internal::GLApi::glDeleteProgram(program);
  }

  const char *kernelDefines =
      "#define OPENSUBDIV_GLSL_XFB_KERNEL_EVAL_STENCILS\n";

  program = compileKernel(srcDesc, dstDesc, duDesc, dvDesc, duuDesc, duvDesc,
                          dvvDesc, kernelDefines, interleavedDerivativeBuffers);
  if (program == 0)
    return false;

  // cache uniform locations (TODO: use uniform block)
  uniformSrcBufferTexture = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "vertexBuffer");
  uniformSrcOffset = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "srcOffset");
  uniformSizesTexture = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "sizes");
  uniformOffsetsTexture = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "offsets");
  uniformIndicesTexture = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "indices");
  uniformWeightsTexture = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "weights");
  uniformDuWeightsTexture = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "duWeights");
  uniformDvWeightsTexture = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "dvWeights");
  uniformDuuWeightsTexture = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "duuWeights");
  uniformDuvWeightsTexture = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "duvWeights");
  uniformDvvWeightsTexture = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "dvvWeights");
  uniformStart = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "batchStart");
  uniformEnd = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "batchEnd");

  return true;
}

// ---------------------------------------------------------------------------

GLXFBEvaluator::_PatchKernel::_PatchKernel() : program(0) {}
GLXFBEvaluator::_PatchKernel::~_PatchKernel() {
  if (program) {
    OpenSubdiv::internal::GLApi::glDeleteProgram(program);
  }
}

bool GLXFBEvaluator::_PatchKernel::Compile(
    BufferDescriptor const &srcDesc, BufferDescriptor const &dstDesc,
    BufferDescriptor const &duDesc, BufferDescriptor const &dvDesc,
    BufferDescriptor const &duuDesc, BufferDescriptor const &duvDesc,
    BufferDescriptor const &dvvDesc, bool interleavedDerivativeBuffers) {
  // create stencil kernel
  if (program) {
    OpenSubdiv::internal::GLApi::glDeleteProgram(program);
  }

  const char *kernelDefines =
      "#define OPENSUBDIV_GLSL_XFB_KERNEL_EVAL_PATCHES\n";

  program = compileKernel(srcDesc, dstDesc, duDesc, dvDesc, duuDesc, duvDesc,
                          dvvDesc, kernelDefines, interleavedDerivativeBuffers);
  if (program == 0)
    return false;

  // cache uniform locations
  uniformSrcBufferTexture = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "vertexBuffer");
  uniformSrcOffset = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "srcOffset");
  uniformPatchParamTexture = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "patchParamBuffer");
  uniformPatchIndexTexture = OpenSubdiv::internal::GLApi::glGetUniformLocation(program, "patchIndexBuffer");

  uniformPatchArraysUBOBinding = 1;
  int uboIndex = glGetUniformBlockIndex(program, "PatchArrays");
  glUniformBlockBinding(program, uboIndex, uniformPatchArraysUBOBinding);

  return true;
}

} // end namespace Osd

} // end namespace OPENSUBDIV_VERSION
} // end namespace OpenSubdiv
