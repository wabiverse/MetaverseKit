//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_COMBINENODEMDL_H
#define MATERIALX_COMBINENODEMDL_H

#include <MaterialX/MXGenMdlExport.h>

#include <MaterialX/MXGenShaderCombineNode.h>

MATERIALX_NAMESPACE_BEGIN

/// Custom combine node implementation for MDL
class MX_GENMDL_API CombineNodeMdl : public CombineNode {
public:
  static ShaderNodeImplPtr create();

  void emitFunctionCall(const ShaderNode &node, GenContext &context,
                        ShaderStage &stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
