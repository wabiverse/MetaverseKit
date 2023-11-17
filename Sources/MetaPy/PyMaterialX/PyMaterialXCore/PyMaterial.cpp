//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialX/MXCoreLook.h>
#include <MaterialX/MXCoreMaterial.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyMaterial(py::module &mod) {
  mod.def("getShaderNodes", &mx::getShaderNodes, py::arg("materialNode"),
          py::arg("nodeType") = mx::SURFACE_SHADER_TYPE_STRING,
          py::arg("target") = mx::EMPTY_STRING);
  mod.def("getConnectedOutputs", &mx::getConnectedOutputs);
}
