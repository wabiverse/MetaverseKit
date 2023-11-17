//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialX/MXGenShader.h>
#include <MaterialX/MXGenShaderGenContext.h>
#include <MaterialX/MXGenShaderGenerator.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyShaderGenerator(py::module &mod) {
  py::class_<mx::ShaderGenerator, mx::ShaderGeneratorPtr>(mod,
                                                          "ShaderGenerator")
      .def("getTarget", &mx::ShaderGenerator::getTarget)
      .def("generate", &mx::ShaderGenerator::generate)
      .def("setColorManagementSystem",
           &mx::ShaderGenerator::setColorManagementSystem)
      .def("getColorManagementSystem",
           &mx::ShaderGenerator::getColorManagementSystem)
      .def("setUnitSystem", &mx::ShaderGenerator::setUnitSystem)
      .def("getUnitSystem", &mx::ShaderGenerator::getUnitSystem)
      .def("getTokenSubstitutions", &mx::ShaderGenerator::getTokenSubstitutions)
      .def("registerShaderMetadata",
           &mx::ShaderGenerator::registerShaderMetadata);
}
