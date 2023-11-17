//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialX/MXGenOslBlurNodeOsl.h>

#include <MaterialX/MXGenShaderGenContext.h>
#include <MaterialX/MXGenShaderGenerator.h>
#include <MaterialX/MXGenShaderNode.h>
#include <MaterialX/MXGenShaderStage.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr BlurNodeOsl::create() {
  return std::make_shared<BlurNodeOsl>();
}

void BlurNodeOsl::emitSamplingFunctionDefinition(const ShaderNode & /*node*/,
                                                 GenContext &context,
                                                 ShaderStage &stage) const {
  const ShaderGenerator &shadergen = context.getShaderGenerator();
  shadergen.emitLibraryInclude("stdlib/genosl/lib/mx_sampling.osl", context,
                               stage);
  shadergen.emitLineBreak(stage);
}

MATERIALX_NAMESPACE_END
