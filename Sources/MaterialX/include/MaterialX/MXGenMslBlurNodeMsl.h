//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_BLURNODEMSL_H
#define MATERIALX_BLURNODEMSL_H

#include <MaterialX/MXGenMslExport.h>

#include <MaterialX/MXGenShaderBlurNode.h>

MATERIALX_NAMESPACE_BEGIN

/// Blur node implementation for MSL
class MX_GENMSL_API BlurNodeMsl : public BlurNode {
public:
  static ShaderNodeImplPtr create();
  void emitSamplingFunctionDefinition(const ShaderNode &node,
                                      GenContext &context,
                                      ShaderStage &stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
