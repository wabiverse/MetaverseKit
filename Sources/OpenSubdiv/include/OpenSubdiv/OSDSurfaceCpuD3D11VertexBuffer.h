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

#ifndef OPENSUBDIV3_OSD_CPU_D3D11_VERTEX_BUFFER_H
#define OPENSUBDIV3_OSD_CPU_D3D11_VERTEX_BUFFER_H

#include "OpenSubdiv/OSDVersion.h"
#include <cstddef>

struct ID3D11Buffer;
struct ID3D11Device;
struct ID3D11DeviceContext;

namespace OpenSubdiv {
namespace OPENSUBDIV_VERSION {

namespace Osd {

///
/// \brief Concrete vertex buffer class for Cpu subdivision and DirectX drawing.
///
/// CpuD3D11VertexBuffer implements CpuVertexBufferInterface and
/// D3D11VertexBufferInterface.
///
/// An instance of this buffer class can be passed to CpuEvaluator.
///
class CpuD3D11VertexBuffer {
public:
  /// Creator. Returns NULL if error.
  static CpuD3D11VertexBuffer *Create(int numElements, int numVertices,
                                      ID3D11DeviceContext *deviceContext);

  /// Destructor.
  virtual ~CpuD3D11VertexBuffer();

  /// This method is meant to be used in client code in order to provide coarse
  /// vertices data to Osd.
  void UpdateData(const float *src, int startVertex, int numVertices,
                  void *deviceContext = NULL);

  /// Returns how many elements defined in this vertex buffer.
  int GetNumElements() const;

  /// Returns how many vertices allocated in this vertex buffer.
  int GetNumVertices() const;

  /// Returns the address of CPU buffer.
  float *BindCpuBuffer();

  /// Returns the D3D11 buffer object.
  ID3D11Buffer *BindD3D11Buffer(ID3D11DeviceContext *deviceContext);

  /// Returns the D3D11 buffer object (for Osd::Mesh interface)
  ID3D11Buffer *BindVBO(ID3D11DeviceContext *deviceContext) {
    return BindD3D11Buffer(deviceContext);
  }

protected:
  /// Constructor.
  CpuD3D11VertexBuffer(int numElements, int numVertices);

  bool allocate(ID3D11Device *device);

private:
  int _numElements;
  int _numVertices;
  ID3D11Buffer *_d3d11Buffer;
  float *_cpuBuffer;
};

} // end namespace Osd

} // end namespace OPENSUBDIV_VERSION
using namespace OPENSUBDIV_VERSION;

} // end namespace OpenSubdiv

#endif // OPENSUBDIV3_OSD_CPU_D3D11_VERTEX_BUFFER_H
