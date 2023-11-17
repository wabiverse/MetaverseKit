//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialX/MXRenderMslMetalTextureHandler.h>
#include <MaterialX/MXRenderMslPipelineStateObject.h>
#include <MaterialX/MXRenderShaderRenderer.h>

#include <iostream>

MATERIALX_NAMESPACE_BEGIN

MetalTextureHandler::MetalTextureHandler(MTL::Device *device,
                                         ImageLoaderPtr imageLoader)
    : ImageHandler(imageLoader) {
  int maxTextureUnits = 31;
  _boundTextureLocations.resize(maxTextureUnits,
                                MslProgram::UNDEFINED_METAL_RESOURCE_ID);
  _device = device;
}

bool MetalTextureHandler::bindImage(
    ImagePtr image, const ImageSamplingProperties &samplingProperties) {
  // Create renderer resources if needed.
  if (image->getResourceId() == MslProgram::UNDEFINED_METAL_RESOURCE_ID) {
    if (!createRenderResources(image, true)) {
      return false;
    }
  }
  _imageBindingInfo[image->getResourceId()] =
      std::make_pair(image, samplingProperties);
  return true;
}

MTL::SamplerState *MetalTextureHandler::getSamplerState(
    const ImageSamplingProperties &samplingProperties) {
  if (_imageSamplerStateMap.find(samplingProperties) ==
      _imageSamplerStateMap.end()) {
    MTL::SamplerDescriptor *samplerDesc =
        MTL::SamplerDescriptor::alloc()->init();
    samplerDesc->setSAddressMode(
        mapAddressModeToMetal(samplingProperties.uaddressMode));
    samplerDesc->setRAddressMode(
        mapAddressModeToMetal(samplingProperties.uaddressMode));
    samplerDesc->setTAddressMode(
        mapAddressModeToMetal(samplingProperties.vaddressMode));
    samplerDesc->setBorderColor(samplingProperties.defaultColor[0] == 0
                                    ? MTL::SamplerBorderColorOpaqueBlack
                                    : MTL::SamplerBorderColorOpaqueWhite);
    MTL::SamplerMinMagFilter minmagFilter;
    MTL::SamplerMipFilter mipFilter;
    mapFilterTypeToMetal(samplingProperties.filterType,
                         samplingProperties.enableMipmaps, minmagFilter,
                         mipFilter);
    // Magnification filters are more restrictive than minification
    samplerDesc->setMagFilter(MTL::SamplerMinMagFilterLinear);
    samplerDesc->setMinFilter(minmagFilter);
    samplerDesc->setMipFilter(mipFilter);
    samplerDesc->setMaxAnisotropy(16);

    _imageSamplerStateMap[samplingProperties] =
        _device->newSamplerState(samplerDesc);
  }

  return _imageSamplerStateMap[samplingProperties];
}

bool MetalTextureHandler::bindImage(MTL::RenderCommandEncoder *renderCmdEncoder,
                                    int textureUnit, ImagePtr image) {
  // Create renderer resources if needed.
  if (image->getResourceId() == MslProgram::UNDEFINED_METAL_RESOURCE_ID) {
    if (!createRenderResources(image, true)) {
      return false;
    }
  }

  _boundTextureLocations[textureUnit] = image->getResourceId();

  renderCmdEncoder->setFragmentTexture(_metalTextureMap[image->getResourceId()],
                                       textureUnit);
  renderCmdEncoder->setFragmentSamplerState(
      getSamplerState(_imageBindingInfo[image->getResourceId()].second),
      textureUnit);

  return true;
}

MTL::Texture *MetalTextureHandler::getAssociatedMetalTexture(ImagePtr image) {
  if (image) {
    auto tex = _metalTextureMap.find(image->getResourceId());
    if (tex != _metalTextureMap.end())
      return (tex->second);
  }
  return nil;
}

MTL::Texture *
MetalTextureHandler::getMTLTextureForImage(unsigned int index) const {
  auto imageInfo = _imageBindingInfo.find(index);
  if (imageInfo != _imageBindingInfo.end()) {
    if (!imageInfo->second.first)
      return nil;

    auto metalTexture =
        _metalTextureMap.find(imageInfo->second.first->getResourceId());
    if (metalTexture != _metalTextureMap.end())
      return metalTexture->second;
  }

  return nil;
}

MTL::SamplerState *
MetalTextureHandler::getMTLSamplerStateForImage(unsigned int index) {
  auto imageInfo = _imageBindingInfo.find(index);
  if (imageInfo != _imageBindingInfo.end()) {
    return getSamplerState(imageInfo->second.second);
  }
  return nil;
}

bool MetalTextureHandler::unbindImage(ImagePtr image) {
  if (image->getResourceId() != MslProgram::UNDEFINED_METAL_RESOURCE_ID) {
    int textureUnit = getBoundTextureLocation(image->getResourceId());
    if (textureUnit >= 0) {
      _boundTextureLocations[textureUnit] =
          MslProgram::UNDEFINED_METAL_RESOURCE_ID;
      return true;
    }
  }
  return false;
}

bool MetalTextureHandler::createRenderResources(ImagePtr image,
                                                bool generateMipMaps) {
  MTL::Texture *texture = nil;

  MTL::PixelFormat pixelFormat;
  MTL::DataType dataType;

  if (image->getResourceId() == MslProgram::UNDEFINED_METAL_RESOURCE_ID) {
    static unsigned int resourceId = 0;
    ++resourceId;

    mapTextureFormatToMetal(image->getBaseType(), image->getChannelCount(),
                            false, dataType, pixelFormat);

    MTL::TextureDescriptor *texDesc = MTL::TextureDescriptor::alloc()->init();
    texDesc->setTextureType(MTL::TextureType2D);
    texDesc->setWidth(image->getWidth());
    texDesc->setHeight(image->getHeight());
    texDesc->setMipmapLevelCount(generateMipMaps ? image->getMaxMipCount() : 1);
    texDesc->setUsage(MTL::TextureUsageShaderRead |
                      // For now, we set generate mip maps flag off,
                      // when we want to use the texture as render target
                      (!generateMipMaps ? MTL::TextureUsageRenderTarget : 0));
    texDesc->setResourceOptions(MTL::ResourceStorageModePrivate);
    texDesc->setPixelFormat(pixelFormat);
    if (generateMipMaps) {
      if (image->getChannelCount() == 1) {
        texDesc->setSwizzle(MTL::TextureSwizzleChannels{
            MTL::TextureSwizzleRed, MTL::TextureSwizzleRed,
            MTL::TextureSwizzleRed, MTL::TextureSwizzleRed});
      } else if (image->getChannelCount() == 2) {
        texDesc->setSwizzle(MTL::TextureSwizzleChannels{
            MTL::TextureSwizzleRed, MTL::TextureSwizzleGreen,
            MTL::TextureSwizzleRed, MTL::TextureSwizzleGreen});
      }
    }
    texture = _device->newTexture(texDesc);
    _metalTextureMap[resourceId] = texture;
    image->setResourceId(resourceId);
  } else {
    mapTextureFormatToMetal(image->getBaseType(), image->getChannelCount(),
                            false, dataType, pixelFormat);

    texture = _metalTextureMap[image->getResourceId()];
  }

  MTL::CommandQueue *cmdQueue = _device->newCommandQueue();
  MTL::CommandBuffer *cmdBuffer = cmdQueue->commandBuffer();

  MTL::BlitCommandEncoder *blitCmdEncoder = cmdBuffer->blitCommandEncoder();

  NS::UInteger channelCount = image->getChannelCount();

  NS::UInteger sourceBytesPerRow = image->getWidth() * channelCount *
                                   getTextureBaseTypeSize(image->getBaseType());
  NS::UInteger sourceBytesPerImage = sourceBytesPerRow * image->getHeight();

  std::vector<float> rearrangedDataF;
  std::vector<unsigned char> rearrangedDataC;
  void *imageData = image->getResourceBuffer();

  if ((pixelFormat == MTL::PixelFormatRGBA32Float ||
       pixelFormat == MTL::PixelFormatRGBA8Unorm) &&
      channelCount == 3) {
    bool isFloat = pixelFormat == MTL::PixelFormatRGBA32Float;

    sourceBytesPerRow = sourceBytesPerRow / 3 * 4;
    sourceBytesPerImage = sourceBytesPerImage / 3 * 4;

    size_t srcIdx = 0;

    if (isFloat) {
      rearrangedDataF.resize(sourceBytesPerImage / sizeof(float));
      for (size_t dstIdx = 0; dstIdx < rearrangedDataF.size(); ++dstIdx) {
        if ((dstIdx & 0x3) == 3) {
          rearrangedDataF[dstIdx] = 1.0f;
          continue;
        }

        rearrangedDataF[dstIdx] = ((float *)imageData)[srcIdx++];
      }

      imageData = rearrangedDataF.data();
    } else {
      rearrangedDataC.resize(sourceBytesPerImage);
      for (size_t dstIdx = 0; dstIdx < rearrangedDataC.size(); ++dstIdx) {
        if ((dstIdx & 0x3) == 3) {
          rearrangedDataC[dstIdx] = 255;
          continue;
        }

        rearrangedDataC[dstIdx] = ((unsigned char *)imageData)[srcIdx++];
      }

      imageData = rearrangedDataC.data();
    }

    channelCount = 4;
  }

  MTL::Buffer *buffer = nil;
  if (imageData) {
    buffer = _device->newBuffer(imageData, sourceBytesPerImage,
                                MTL::StorageModeShared);
    blitCmdEncoder->copyFromBuffer(
        buffer, 0, sourceBytesPerRow, sourceBytesPerImage,
        MTL::Size::Make(image->getWidth(), image->getHeight(), 1), texture, 0,
        0, MTL::Origin::Make(0, 0, 0));
  }

  if (generateMipMaps && image->getMaxMipCount() > 1)
    blitCmdEncoder->generateMipmaps(texture);

  blitCmdEncoder->endEncoding();

  cmdBuffer->commit();
  cmdBuffer->waitUntilCompleted();

  if (buffer)
    buffer->release();

  return true;
}

void MetalTextureHandler::releaseRenderResources(ImagePtr image) {
  if (!image)
    return;

  if (image->getResourceId() == MslProgram::UNDEFINED_METAL_RESOURCE_ID) {
    return;
  }

  unbindImage(image);
  unsigned int resourceId = image->getResourceId();
  auto tex = _metalTextureMap.find(resourceId);
  if (tex != _metalTextureMap.end()) {
    tex->second->release();
  }
  _metalTextureMap.erase(resourceId);
  image->setResourceId(MslProgram::UNDEFINED_METAL_RESOURCE_ID);
}

int MetalTextureHandler::getBoundTextureLocation(unsigned int resourceId) {
  for (size_t i = 0; i < _boundTextureLocations.size(); i++) {
    if (_boundTextureLocations[i] == resourceId) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

MTL::SamplerAddressMode MetalTextureHandler::mapAddressModeToMetal(
    ImageSamplingProperties::AddressMode addressModeEnum) {
  const vector<MTL::SamplerAddressMode> addressModes{
      // Constant color. Use clamp to border
      // with border color to achieve this
      MTL::SamplerAddressModeClampToBorderColor,

      // Clamp
      MTL::SamplerAddressModeClampToEdge,

      // Repeat
      MTL::SamplerAddressModeRepeat,

      // Mirror
      MTL::SamplerAddressModeMirrorRepeat};

  MTL::SamplerAddressMode addressMode = MTL::SamplerAddressModeRepeat;
  if (addressModeEnum != ImageSamplingProperties::AddressMode::UNSPECIFIED) {
    addressMode = addressModes[static_cast<int>(addressModeEnum)];
  }
  return addressMode;
}

void MetalTextureHandler::mapFilterTypeToMetal(
    ImageSamplingProperties::FilterType filterTypeEnum, bool enableMipmaps,
    MTL::SamplerMinMagFilter &minMagFilter, MTL::SamplerMipFilter &mipFilter) {
  if (enableMipmaps) {
    if (filterTypeEnum == ImageSamplingProperties::FilterType::LINEAR ||
        filterTypeEnum == ImageSamplingProperties::FilterType::CUBIC ||
        filterTypeEnum == ImageSamplingProperties::FilterType::UNSPECIFIED) {
      minMagFilter = MTL::SamplerMinMagFilterLinear;
      mipFilter = MTL::SamplerMipFilterLinear;
    } else {
      minMagFilter = MTL::SamplerMinMagFilterNearest;
      mipFilter = MTL::SamplerMipFilterNearest;
    }
  } else {
    if (filterTypeEnum == ImageSamplingProperties::FilterType::LINEAR ||
        filterTypeEnum == ImageSamplingProperties::FilterType::CUBIC ||
        filterTypeEnum == ImageSamplingProperties::FilterType::UNSPECIFIED) {
      minMagFilter = MTL::SamplerMinMagFilterLinear;
      mipFilter = MTL::SamplerMipFilterNotMipmapped;
    } else {
      minMagFilter = MTL::SamplerMinMagFilterNearest;
      mipFilter = MTL::SamplerMipFilterNotMipmapped;
    }
  }
}

void MetalTextureHandler::mapTextureFormatToMetal(
    Image::BaseType baseType, unsigned int channelCount, bool srgb,
    MTL::DataType &dataType, MTL::PixelFormat &pixelFormat) {
  if (baseType == Image::BaseType::UINT8) {
    dataType = MTL::DataTypeChar;
    switch (channelCount) {
    case 4:
      pixelFormat =
          srgb ? MTL::PixelFormatRGBA8Unorm_sRGB : MTL::PixelFormatRGBA8Unorm;
      dataType = MTL::DataTypeChar4;
      break;
    case 3:
      pixelFormat =
          srgb ? MTL::PixelFormatRGBA8Unorm_sRGB : MTL::PixelFormatRGBA8Unorm;
      dataType = MTL::DataTypeChar3;
      break;
    case 2:
      pixelFormat = MTL::PixelFormatRG8Unorm;
      dataType = MTL::DataTypeChar2;
      break;
    case 1:
      pixelFormat = MTL::PixelFormatR8Unorm;
      dataType = MTL::DataTypeChar;
      break;
    default:
      throw Exception("Unsupported channel count in mapTextureFormatToMetal");
    }
  } else if (baseType == Image::BaseType::UINT16) {
    switch (channelCount) {
    case 4:
      pixelFormat = MTL::PixelFormatRGBA16Uint;
      dataType = MTL::DataTypeShort4;
      break;
    case 3:
      pixelFormat = MTL::PixelFormatRGBA16Uint;
      dataType = MTL::DataTypeShort3;
      break;
    case 2:
      pixelFormat = MTL::PixelFormatRG16Uint;
      dataType = MTL::DataTypeShort2;
      break;
    case 1:
      pixelFormat = MTL::PixelFormatR16Uint;
      dataType = MTL::DataTypeShort;
      break;
    default:
      throw Exception("Unsupported channel count in mapTextureFormatToMetal");
    }
  } else if (baseType == Image::BaseType::HALF) {
    switch (channelCount) {
    case 4:
      pixelFormat = MTL::PixelFormatRGBA16Float;
      dataType = MTL::DataTypeHalf4;
      break;
    case 3:
      pixelFormat = MTL::PixelFormatRGBA16Float;
      dataType = MTL::DataTypeHalf3;
      break;
    case 2:
      pixelFormat = MTL::PixelFormatRG16Float;
      dataType = MTL::DataTypeHalf2;
      break;
    case 1:
      pixelFormat = MTL::PixelFormatR16Float;
      dataType = MTL::DataTypeHalf;
      break;
    default:
      throw Exception("Unsupported channel count in mapTextureFormatToMetal");
    }
  } else if (baseType == Image::BaseType::FLOAT) {
    switch (channelCount) {
    case 4:
      pixelFormat = MTL::PixelFormatRGBA32Float;
      dataType = MTL::DataTypeFloat4;
      break;
    case 3:
      pixelFormat = MTL::PixelFormatRGBA32Float;
      dataType = MTL::DataTypeFloat3;
      break;
    case 2:
      pixelFormat = MTL::PixelFormatRG32Float;
      dataType = MTL::DataTypeFloat2;
      break;
    case 1:
      pixelFormat = MTL::PixelFormatR32Float;
      dataType = MTL::DataTypeFloat;
      break;
    default:
      throw Exception("Unsupported channel count in mapTextureFormatToMetal");
    }
  } else {
    throw Exception("Unsupported base type in mapTextureFormatToMetal");
  }
}

size_t MetalTextureHandler::getTextureBaseTypeSize(Image::BaseType baseType) {
  if (baseType == Image::BaseType::UINT8) {
    return 1;
  } else if (baseType == Image::BaseType::UINT16 ||
             baseType == Image::BaseType::HALF) {
    return 2;
  } else if (baseType == Image::BaseType::FLOAT) {
    return 4;
  } else {
    throw Exception("Unsupported base type in mapTextureFormatToMetal");
  }
}

MATERIALX_NAMESPACE_END
