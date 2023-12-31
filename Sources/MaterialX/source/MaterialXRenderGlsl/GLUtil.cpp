//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialX/Glad.h>

#include <MaterialX/MXRenderGlslGLUtil.h>

#include <iostream>

MATERIALX_NAMESPACE_BEGIN

void checkGlErrors(const string &context) {
  for (GLenum error = glGetError(); error; error = glGetError()) {
    std::cerr << "OpenGL error " << context << ": " << std::to_string(error)
              << std::endl;
  }
}

MATERIALX_NAMESPACE_END
