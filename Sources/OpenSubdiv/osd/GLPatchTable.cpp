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

#include "OpenSubdiv/OSDSurfaceGLPatchTable.h"

#include "OpenSubdiv/OSDAdaptivePatchTable.h"
#include "OpenSubdiv/OSDSurfaceCpuPatchTable.h"

namespace OpenSubdiv {
namespace OPENSUBDIV_VERSION {

namespace Osd {

GLPatchTable::GLPatchTable()
    : _patchIndexBuffer(0), _patchParamBuffer(0), _patchIndexTexture(0),
      _patchParamTexture(0) {

  // Initialize internal OpenGL loader library if necessary
  OpenSubdiv::internal::GLLoader::libraryInitializeGL();
}

GLPatchTable::~GLPatchTable() {
  if (_patchIndexBuffer)
    OpenSubdiv::internal::GLApi::glDeleteBuffers(1, &_patchIndexBuffer);
  if (_patchParamBuffer)
    OpenSubdiv::internal::GLApi::glDeleteBuffers(1, &_patchParamBuffer);
  if (_patchIndexTexture)
    OpenSubdiv::internal::GLApi::glDeleteTextures(1, &_patchIndexTexture);
  if (_patchParamTexture)
    OpenSubdiv::internal::GLApi::glDeleteTextures(1, &_patchParamTexture);
  if (_varyingIndexBuffer)
    OpenSubdiv::internal::GLApi::glDeleteBuffers(1, &_varyingIndexBuffer);
  if (_varyingIndexTexture)
    OpenSubdiv::internal::GLApi::glDeleteTextures(1, &_varyingIndexTexture);
  for (int fvc = 0; fvc < (int)_fvarIndexBuffers.size(); ++fvc) {
    if (_fvarIndexBuffers[fvc])
      OpenSubdiv::internal::GLApi::glDeleteBuffers(1, &_fvarIndexBuffers[fvc]);
  }
  for (int fvc = 0; fvc < (int)_fvarIndexTextures.size(); ++fvc) {
    if (_fvarIndexTextures[fvc])
      OpenSubdiv::internal::GLApi::glDeleteTextures(1, &_fvarIndexTextures[fvc]);
  }
}

GLPatchTable *GLPatchTable::Create(Far::PatchTable const *farPatchTable,
                                   void * /*deviceContext*/) {
  GLPatchTable *instance = new GLPatchTable();
  if (instance->allocate(farPatchTable))
    return instance;
  delete instance;
  return 0;
}

bool GLPatchTable::allocate(Far::PatchTable const *farPatchTable) {
  OpenSubdiv::internal::GLApi::glGenBuffers(1, &_patchIndexBuffer);
  OpenSubdiv::internal::GLApi::glGenBuffers(1, &_patchParamBuffer);

  CpuPatchTable patchTable(farPatchTable);

  size_t numPatchArrays = patchTable.GetNumPatchArrays();
  GLsizei indexSize = (GLsizei)patchTable.GetPatchIndexSize();
  GLsizei patchParamSize = (GLsizei)patchTable.GetPatchParamSize();

  // copy patch array
  _patchArrays.assign(patchTable.GetPatchArrayBuffer(),
                      patchTable.GetPatchArrayBuffer() + numPatchArrays);

  // copy index buffer
  OpenSubdiv::internal::GLApi::glBindBuffer(GL_ARRAY_BUFFER, _patchIndexBuffer);
  OpenSubdiv::internal::GLApi::glBufferData(GL_ARRAY_BUFFER, indexSize * sizeof(GLint),
               patchTable.GetPatchIndexBuffer(), GL_STATIC_DRAW);

  // copy patchparam buffer
  OpenSubdiv::internal::GLApi::glBindBuffer(GL_ARRAY_BUFFER, _patchParamBuffer);
  OpenSubdiv::internal::GLApi::glBufferData(GL_ARRAY_BUFFER, patchParamSize * sizeof(PatchParam),
               patchTable.GetPatchParamBuffer(), GL_STATIC_DRAW);
  OpenSubdiv::internal::GLApi::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // make both buffer as texture buffers too.
  OpenSubdiv::internal::GLApi::glGenTextures(1, &_patchIndexTexture);
  OpenSubdiv::internal::GLApi::glGenTextures(1, &_patchParamTexture);

  OpenSubdiv::internal::GLApi::glBindTexture(GL_TEXTURE_BUFFER, _patchIndexTexture);
  glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, _patchIndexBuffer);

  OpenSubdiv::internal::GLApi::glBindTexture(GL_TEXTURE_BUFFER, _patchParamTexture);
  glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32I, _patchParamBuffer);

  // varying
  _varyingPatchArrays.assign(patchTable.GetVaryingPatchArrayBuffer(),
                             patchTable.GetVaryingPatchArrayBuffer() +
                                 numPatchArrays);

  OpenSubdiv::internal::GLApi::glGenBuffers(1, &_varyingIndexBuffer);
  OpenSubdiv::internal::GLApi::glBindBuffer(GL_ARRAY_BUFFER, _varyingIndexBuffer);
  OpenSubdiv::internal::GLApi::glBufferData(GL_ARRAY_BUFFER,
                                            patchTable.GetVaryingPatchIndexSize() * sizeof(GLint),
                                            patchTable.GetVaryingPatchIndexBuffer(), GL_STATIC_DRAW);

  OpenSubdiv::internal::GLApi::glGenTextures(1, &_varyingIndexTexture);
  OpenSubdiv::internal::GLApi::glBindTexture(GL_TEXTURE_BUFFER, _varyingIndexTexture);
  OpenSubdiv::internal::GLApi::glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, _varyingIndexBuffer);

  // face-varying
  int numFVarChannels = patchTable.GetNumFVarChannels();
  _fvarPatchArrays.resize(numFVarChannels);
  _fvarIndexBuffers.resize(numFVarChannels);
  _fvarIndexTextures.resize(numFVarChannels);
  _fvarParamBuffers.resize(numFVarChannels);
  _fvarParamTextures.resize(numFVarChannels);
  for (int fvc = 0; fvc < numFVarChannels; ++fvc) {
    _fvarPatchArrays[fvc].assign(patchTable.GetFVarPatchArrayBuffer(fvc),
                                 patchTable.GetFVarPatchArrayBuffer(fvc) +
                                     numPatchArrays);

    OpenSubdiv::internal::GLApi::glGenBuffers(1, &_fvarIndexBuffers[fvc]);
    OpenSubdiv::internal::GLApi::glBindBuffer(GL_ARRAY_BUFFER, _fvarIndexBuffers[fvc]);
    OpenSubdiv::internal::GLApi::glBufferData(GL_ARRAY_BUFFER,
                 patchTable.GetFVarPatchIndexSize(fvc) * sizeof(GLint),
                 patchTable.GetFVarPatchIndexBuffer(fvc), GL_STATIC_DRAW);

    OpenSubdiv::internal::GLApi::glGenTextures(1, &_fvarIndexTextures[fvc]);
    OpenSubdiv::internal::GLApi::glBindTexture(GL_TEXTURE_BUFFER, _fvarIndexTextures[fvc]);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, _fvarIndexBuffers[fvc]);

    OpenSubdiv::internal::GLApi::glGenBuffers(1, &_fvarParamBuffers[fvc]);
    OpenSubdiv::internal::GLApi::glBindBuffer(GL_ARRAY_BUFFER, _fvarParamBuffers[fvc]);
    OpenSubdiv::internal::GLApi::glBufferData(GL_ARRAY_BUFFER,
                 patchTable.GetFVarPatchParamSize(fvc) * sizeof(PatchParam),
                 patchTable.GetFVarPatchParamBuffer(fvc), GL_STATIC_DRAW);

    OpenSubdiv::internal::GLApi::glGenTextures(1, &_fvarParamTextures[fvc]);
    OpenSubdiv::internal::GLApi::glBindTexture(GL_TEXTURE_BUFFER, _fvarParamTextures[fvc]);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32I, _fvarParamBuffers[fvc]);
    OpenSubdiv::internal::GLApi::glBindTexture(GL_TEXTURE_BUFFER, 0);
  }

  OpenSubdiv::internal::GLApi::glBindBuffer(GL_ARRAY_BUFFER, 0);
  OpenSubdiv::internal::GLApi::glBindTexture(GL_TEXTURE_BUFFER, 0);

  return true;
}

} // end namespace Osd

} // end namespace OPENSUBDIV_VERSION
} // end namespace OpenSubdiv
