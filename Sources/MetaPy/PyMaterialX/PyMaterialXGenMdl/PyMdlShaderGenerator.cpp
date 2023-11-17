//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialX/MXGenMdlShaderGenerator.h>
#include <MaterialX/MXGenShaderGenerator.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyMdlShaderGenerator(py::module &mod) {
  py::class_<mx::MdlShaderGenerator, mx::ShaderGenerator,
             mx::MdlShaderGeneratorPtr>(mod, "MdlShaderGenerator")
      .def_static("create", &mx::MdlShaderGenerator::create)
      .def(py::init<>())
      .def("getTarget", &mx::MdlShaderGenerator::getTarget);
}
