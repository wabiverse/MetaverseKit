//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_GLTEXTUREHANDLER_H
#define MATERIALX_GLTEXTUREHANDLER_H

/// @file
/// Metal texture handler

#include <MaterialX/MXRenderMslExport.h>

#include <MaterialX/MXRenderImageHandler.h>

#include <stack>
#include <unordered_map>

#include <Metal/Metal.hpp>

MATERIALX_NAMESPACE_BEGIN

/// Shared pointer to an Metal texture handler
using MetalTextureHandlerPtr = std::shared_ptr<class MetalTextureHandler>;

/// @class MetalTextureHandler
/// An Metal texture handler class
class MX_RENDERMSL_API MetalTextureHandler : public ImageHandler {
  friend class MslProgram;

public:
  static MetalTextureHandlerPtr create(MTL::Device *device,
                                       ImageLoaderPtr imageLoader) {
    return MetalTextureHandlerPtr(new MetalTextureHandler(device, imageLoader));
  }

  /// This method binds image and its corresponding sampling properties.
  /// It also creates the underlying resource if needed.
  /// Actual binding of texture and sampler to command encoder happens autoamt
  bool bindImage(ImagePtr image,
                 const ImageSamplingProperties &samplingProperties) override;

protected:
  /// Bind an image. This method will bind the texture to an active texture
  /// unit as defined by the corresponding image description. The method
  /// will fail if there are not enough available image units to bind to.
  bool bindImage(MTL::RenderCommandEncoder *renderCmdEncoder, int textureUnit,
                 ImagePtr image);

public:
  MTL::SamplerState *
  getSamplerState(const ImageSamplingProperties &samplingProperties);

  /// Unbind an image.
  bool unbindImage(ImagePtr image) override;

  MTL::Texture *getMTLTextureForImage(unsigned int index) const;
  MTL::SamplerState *getMTLSamplerStateForImage(unsigned int index);

  /// Create rendering resources for the given image.
  bool createRenderResources(ImagePtr image, bool generateMipMaps) override;

  /// Release rendering resources for the given image, or for all cached images
  /// if no image pointer is specified.
  void releaseRenderResources(ImagePtr image = nullptr) override;

  /// Return the bound texture location for a given resource
  int getBoundTextureLocation(unsigned int resourceId);

  /// Utility to map an address mode enumeration to an Metal address mode
  static MTL::SamplerAddressMode
  mapAddressModeToMetal(ImageSamplingProperties::AddressMode addressModeEnum);

  /// Utility to map a filter type enumeration to an Metal filter type
  static void mapFilterTypeToMetal(
      ImageSamplingProperties::FilterType filterTypeEnum, bool enableMipmaps,
      MTL::SamplerMinMagFilter &minMagFilter, MTL::SamplerMipFilter &mipFilter);

  /// Utility to map generic texture properties to Metal texture formats.
  static void mapTextureFormatToMetal(Image::BaseType baseType,
                                      unsigned int channelCount, bool srgb,
                                      MTL::DataType &dataType,
                                      MTL::PixelFormat &pixelFormat);

  static size_t getTextureBaseTypeSize(Image::BaseType baseType);

  MTL::Texture *getAssociatedMetalTexture(ImagePtr image);

protected:
  // Protected constructor
  MetalTextureHandler(MTL::Device *device, ImageLoaderPtr imageLoader);

protected:
  std::vector<unsigned int> _boundTextureLocations;

  std::unordered_map<unsigned int, MTL::Texture *> _metalTextureMap;
  std::unordered_map<unsigned int, std::pair<ImagePtr, ImageSamplingProperties>>
      _imageBindingInfo;
  std::unordered_map<ImageSamplingProperties, MTL::SamplerState *,
                     ImageSamplingKeyHasher>
      _imageSamplerStateMap;

  MTL::Device *_device = nil;
};

MATERIALX_NAMESPACE_END

#endif
