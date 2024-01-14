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

#include "OpenSubdiv/OSDSurfaceGLLegacyGregoryPatchTable.h"

namespace OpenSubdiv {
namespace OPENSUBDIV_VERSION {

namespace Osd {

GLLegacyGregoryPatchTable::GLLegacyGregoryPatchTable()
    : _vertexTextureBuffer(0), _vertexValenceTextureBuffer(0),
      _quadOffsetsTextureBuffer(0) {
  _quadOffsetsBase[0] = _quadOffsetsBase[1] = 0;

  // Initialize internal OpenGL loader library if necessary
  OpenSubdiv::internal::GLLoader::libraryInitializeGL();
}

GLLegacyGregoryPatchTable::~GLLegacyGregoryPatchTable() {
  if (_vertexTextureBuffer)
    OpenSubdiv::internal::GLApi::glDeleteTextures(1, &_vertexTextureBuffer);
  if (_vertexValenceTextureBuffer)
    OpenSubdiv::internal::GLApi::glDeleteTextures(1, &_vertexValenceTextureBuffer);
  if (_quadOffsetsTextureBuffer)
    OpenSubdiv::internal::GLApi::glDeleteTextures(1, &_quadOffsetsTextureBuffer);
}

GLLegacyGregoryPatchTable *
GLLegacyGregoryPatchTable::Create(Far::PatchTable const *farPatchTable) {

  GLLegacyGregoryPatchTable *result = new GLLegacyGregoryPatchTable();
  OpenSubdiv::internal::GLApi::glGenTextures(1, &result->_vertexTextureBuffer);
  OpenSubdiv::internal::GLApi::glGenTextures(1, &result->_vertexValenceTextureBuffer);
  OpenSubdiv::internal::GLApi::glGenTextures(1, &result->_quadOffsetsTextureBuffer);

  Far::PatchTable::VertexValenceTable const &valenceTable =
      farPatchTable->GetVertexValenceTable();
  Far::PatchTable::QuadOffsetsTable const &quadOffsetsTable =
      farPatchTable->GetQuadOffsetsTable();

  if (!valenceTable.empty()) {
    GLuint buffer;
    OpenSubdiv::internal::GLApi::glGenBuffers(1, &buffer);
    OpenSubdiv::internal::GLApi::glBindBuffer(GL_ARRAY_BUFFER, buffer);
    OpenSubdiv::internal::GLApi::glBufferData(GL_ARRAY_BUFFER, valenceTable.size() * sizeof(int),
                 &valenceTable[0], GL_STATIC_DRAW);

    OpenSubdiv::internal::GLApi::glBindTexture(GL_TEXTURE_BUFFER, result->_vertexValenceTextureBuffer);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, buffer);
    OpenSubdiv::internal::GLApi::glBindTexture(GL_TEXTURE_BUFFER, 0);
    OpenSubdiv::internal::GLApi::glBindBuffer(GL_ARRAY_BUFFER, 0);
    OpenSubdiv::internal::GLApi::glDeleteBuffers(1, &buffer);
  }

  if (!quadOffsetsTable.empty()) {
    GLuint buffer;
    OpenSubdiv::internal::GLApi::glGenBuffers(1, &buffer);
    OpenSubdiv::internal::GLApi::glBindBuffer(GL_ARRAY_BUFFER, buffer);
    OpenSubdiv::internal::GLApi::glBufferData(GL_ARRAY_BUFFER, quadOffsetsTable.size() * sizeof(int),
                 &quadOffsetsTable[0], GL_STATIC_DRAW);

    OpenSubdiv::internal::GLApi::glBindTexture(GL_TEXTURE_BUFFER, result->_quadOffsetsTextureBuffer);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, buffer);
    OpenSubdiv::internal::GLApi::glBindTexture(GL_TEXTURE_BUFFER, 0);
    OpenSubdiv::internal::GLApi::glBindBuffer(GL_ARRAY_BUFFER, 0);
    OpenSubdiv::internal::GLApi::glDeleteBuffers(1, &buffer);
  }

  result->_quadOffsetsBase[0] = 0;
  result->_quadOffsetsBase[1] = 0;
  // scan patchtable to find quadOffsetsBase.
  for (int i = 0; i < farPatchTable->GetNumPatchArrays(); ++i) {
    // GREGORY_BOUNDARY's quadoffsets come after GREGORY's.
    if (farPatchTable->GetPatchArrayDescriptor(i) ==
        Far::PatchDescriptor::GREGORY) {
      result->_quadOffsetsBase[1] = farPatchTable->GetNumPatches(i) * 4;
      break;
    }
  }
  return result;
}

void GLLegacyGregoryPatchTable::UpdateVertexBuffer(GLuint vbo) {
  OpenSubdiv::internal::GLApi::glBindTexture(GL_TEXTURE_BUFFER, _vertexTextureBuffer);
  glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, vbo);
  OpenSubdiv::internal::GLApi::glBindTexture(GL_TEXTURE_BUFFER, 0);
}

} // end namespace Osd

} // end namespace OPENSUBDIV_VERSION
} // end namespace OpenSubdiv
