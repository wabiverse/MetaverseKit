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

#ifndef OPENSUBDIV3_OSD_CPU_VERTEX_BUFFER_H
#define OPENSUBDIV3_OSD_CPU_VERTEX_BUFFER_H

#include "OpenSubdiv/OSDVersion.h"

#include <cstddef>

namespace OpenSubdiv {
namespace OPENSUBDIV_VERSION {

namespace Osd {

/// \brief Concrete vertex buffer class for CPU subdivision.
///
/// CpuVertexBuffer implements the VertexBufferInterface. An instance
/// of this buffer class can be passed to CpuEvaluator
///
class CpuVertexBuffer {
public:
  /// Creator. Returns NULL if error.
  static CpuVertexBuffer *Create(int numElements, int numVertices,
                                 void *deviceContext = NULL);

  /// Destructor.
  ~CpuVertexBuffer();

  /// This method is meant to be used in client code in order to provide
  /// coarse vertices data to Osd.
  void UpdateData(const float *src, int startVertex, int numVertices,
                  void *deviceContext = NULL);

  /// Returns how many elements defined in this vertex buffer.
  int GetNumElements() const;

  /// Returns how many vertices allocated in this vertex buffer.
  int GetNumVertices() const;

  /// Returns the address of CPU buffer
  float *BindCpuBuffer();

protected:
  /// Constructor.
  CpuVertexBuffer(int numElements, int numVertices);

private:
  int _numElements;
  int _numVertices;
  float *_cpuBuffer;
};

} // end namespace Osd

} // end namespace OPENSUBDIV_VERSION
using namespace OPENSUBDIV_VERSION;

} // end namespace OpenSubdiv

#endif // OPENSUBDIV3_OSD_CPU_VERTEX_BUFFER_H
