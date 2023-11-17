//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialX/MXRenderGlslTextureBaker.h>

#include <MaterialX/MXRenderOiioImageLoader.h>
#include <MaterialX/MXRenderStbImageLoader.h>
#include <MaterialX/MXRenderUtil.h>

#include <MaterialX/MXGenShaderDefaultColorManagementSystem.h>

#include <MaterialX/MXFormatXmlIo.h>

MATERIALX_NAMESPACE_BEGIN
TextureBakerGlsl::TextureBakerGlsl(unsigned int width, unsigned int height,
                                   Image::BaseType baseType)
    : TextureBaker<GlslRenderer, GlslShaderGenerator>(width, height, baseType,
                                                      true) {}
MATERIALX_NAMESPACE_END
