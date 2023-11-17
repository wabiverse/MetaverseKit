//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialX/MXGenGlslEsslShaderGenerator.h>
#include <MaterialX/MXGenShaderUtil.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

EMSCRIPTEN_BINDINGS(EsslShaderGenerator) {
  ems::class_<mx::EsslShaderGenerator, ems::base<mx::HwShaderGenerator>>(
      "EsslShaderGenerator")
      .smart_ptr_constructor("EsslShaderGenerator",
                             &std::make_shared<mx::EsslShaderGenerator>);
}
