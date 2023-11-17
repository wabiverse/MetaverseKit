//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialX/MXGenOslShaderGenerator.h>
#include <MaterialX/MXGenOslSurfaceNodeOsl.h>

#include <MaterialX/MXGenShader.h>
#include <MaterialX/MXGenShaderGenContext.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr SurfaceNodeOsl::create() {
  return std::make_shared<SurfaceNodeOsl>();
}

void SurfaceNodeOsl::emitFunctionCall(const ShaderNode &node,
                                      GenContext &context,
                                      ShaderStage &stage) const {
  // Make sure a closure context is set for upstream closure evaluation,
  // and then just rely on the base class source code implementation.
  context.pushClosureContext(&_cct);
  ClosureSourceCodeNode::emitFunctionCall(node, context, stage);
  context.popClosureContext();
}

MATERIALX_NAMESPACE_END
