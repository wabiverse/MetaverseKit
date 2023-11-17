//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialX/MXRenderMslTextureBaker.h>

#include <MaterialX/MXRenderOiioImageLoader.h>
#include <MaterialX/MXRenderStbImageLoader.h>
#include <MaterialX/MXRenderUtil.h>

#include <MaterialX/MXGenShaderDefaultColorManagementSystem.h>

#include <MaterialX/MXFormatXmlIo.h>

MATERIALX_NAMESPACE_BEGIN

TextureBakerMsl::TextureBakerMsl(unsigned int width, unsigned int height,
                                 Image::BaseType baseType)
    : TextureBaker<MslRenderer, MslShaderGenerator>(width, height, baseType,
                                                    false) {}

MATERIALX_NAMESPACE_END
