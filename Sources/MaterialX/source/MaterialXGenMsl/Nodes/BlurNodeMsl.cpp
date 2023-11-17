//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialX/MXGenMslBlurNodeMsl.h>

#include <MaterialX/MXGenShaderGenContext.h>
#include <MaterialX/MXGenShaderGenerator.h>
#include <MaterialX/MXGenShaderNode.h>
#include <MaterialX/MXGenShaderStage.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr BlurNodeMsl::create() {
  return std::make_shared<BlurNodeMsl>();
}

void BlurNodeMsl::emitSamplingFunctionDefinition(const ShaderNode & /*node*/,
                                                 GenContext &context,
                                                 ShaderStage &stage) const {
  const ShaderGenerator &shadergen = context.getShaderGenerator();
  shadergen.emitLibraryInclude("stdlib/genmsl/lib/mx_sampling.metal", context,
                               stage);
  shadergen.emitLineBreak(stage);
}

MATERIALX_NAMESPACE_END
