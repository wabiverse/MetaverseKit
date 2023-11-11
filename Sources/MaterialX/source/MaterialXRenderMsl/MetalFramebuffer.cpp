//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderMsl/MetalFramebuffer.h>

#include <MaterialXRenderMsl/MetalTextureHandler.h>
#include <MaterialXRenderMsl/MslPipelineStateObject.h>
#include <MaterialXRenderMsl/MslRenderer.h>

MATERIALX_NAMESPACE_BEGIN

//
// MetalFramebuffer methods
//

MetalFramebufferPtr
MetalFramebuffer::create(MTL::Device *device, unsigned int width,
                         unsigned int height, unsigned channelCount,
                         Image::BaseType baseType, MTL::Texture *colorTexture,
                         bool encodeSrgb, MTL::PixelFormat pixelFormat) {
  return MetalFramebufferPtr(
      new MetalFramebuffer(device, width, height, channelCount, baseType,
                           colorTexture, encodeSrgb, pixelFormat));
}

MetalFramebuffer::MetalFramebuffer(MTL::Device *device, unsigned int width,
                                   unsigned int height,
                                   unsigned int channelCount,
                                   Image::BaseType baseType,
                                   MTL::Texture *colorTexture, bool encodeSrgb,
                                   MTL::PixelFormat pixelFormat)
    : _width(0), _height(0), _channelCount(channelCount), _baseType(baseType),
      _encodeSrgb(encodeSrgb), _device(device), _colorTexture(colorTexture),
      _depthTexture(0) {
  StringVec errors;
  const string errorType("Metal target creation failure.");

  resize(width, height, true, pixelFormat, colorTexture);
}

MetalFramebuffer::~MetalFramebuffer() {
  _colorTexture->release();
  _depthTexture->release();
}

void MetalFramebuffer::resize(unsigned int width, unsigned int height,
                              bool forceRecreate, MTL::PixelFormat pixelFormat,
                              MTL::Texture *extColorTexture) {
  if (width * height <= 0) {
    return;
  }
  if (width != _width || _height != height || forceRecreate) {
    // Convert texture format to Metal
    MTL::DataType dataType;
    if (pixelFormat == MTL::PixelFormatInvalid)
      MetalTextureHandler::mapTextureFormatToMetal(
          _baseType, _channelCount, _encodeSrgb, dataType, pixelFormat);

    MTL::TextureDescriptor *texDescriptor =
        MTL::TextureDescriptor::texture2DDescriptor(pixelFormat, width, height,
                                                    NO);
    texDescriptor->setStorageMode(MTL::StorageModePrivate);
    texDescriptor->setUsage(MTL::TextureUsageRenderTarget |
                            MTL::TextureUsageShaderRead);

    if (extColorTexture == nil) {
      _colorTexture = _device->newTexture(texDescriptor);
      _colorTextureOwned = true;
    } else {
      _colorTexture = extColorTexture;
      _colorTextureOwned = false;
    }

    texDescriptor->setPixelFormat(MTL::PixelFormatDepth32Float);
    texDescriptor->setUsage(MTL::TextureUsageRenderTarget);
    _depthTexture = _device->newTexture(texDescriptor);

    _width = width;
    _height = height;
  }
}

void MetalFramebuffer::bind(MTL::RenderPassDescriptor *renderpassDesc) {
  renderpassDesc->colorAttachments()->object(0)->setTexture(getColorTexture());
  renderpassDesc->colorAttachments()->object(0)->setLoadAction(
      MTL::LoadActionClear);
  renderpassDesc->colorAttachments()->object(0)->setStoreAction(
      MTL::StoreActionStore);

  renderpassDesc->depthAttachment()->setTexture(getDepthTexture());
  renderpassDesc->depthAttachment()->setClearDepth(1.0);
  renderpassDesc->depthAttachment()->setLoadAction(MTL::LoadActionClear);
  renderpassDesc->depthAttachment()->setStoreAction(MTL::StoreActionStore);
  renderpassDesc->setStencilAttachment(nil);

  renderpassDesc->setRenderTargetWidth(_width);
  renderpassDesc->setRenderTargetHeight(_height);
}

void MetalFramebuffer::unbind() {}

ImagePtr MetalFramebuffer::getColorImage(MTL::CommandQueue *cmdQueue,
                                         ImagePtr image) {
  if (!image) {
    image = Image::create(_width, _height, _channelCount, _baseType);
    image->createResourceBuffer();
  }

  if (cmdQueue == nil) {
    return image;
  }

  size_t bytesPerRow = _width * _channelCount *
                       MetalTextureHandler::getTextureBaseTypeSize(_baseType);
  size_t bytesPerImage = _height * bytesPerRow;

  MTL::Buffer *buffer =
      _device->newBuffer(bytesPerImage, MTL::ResourceStorageModeShared);

  MTL::CommandBuffer *cmdBuffer = cmdQueue->commandBuffer();

  MTL::BlitCommandEncoder *blitCmdEncoder = cmdBuffer->blitCommandEncoder();
  blitCmdEncoder->copyFromTexture(_colorTexture, 0, 0, MTL::Origin::Make(0, 0, 0), MTL::Size::Make(_width, _height, 1), buffer, 0, bytesPerRow, bytesPerImage, MTL::BlitOptionNone);

  blitCmdEncoder->endEncoding();

  cmdBuffer->commit();
  cmdBuffer->waitUntilCompleted();

  std::vector<unsigned char> imageData(bytesPerImage);
  memcpy(imageData.data(), buffer->contents(), bytesPerImage);

  if (_colorTexture->pixelFormat() == MTL::PixelFormatBGRA8Unorm) {
    for (unsigned int j = 0; j < _height; ++j) {
      unsigned int rawStart = j * (_width * _channelCount);
      for (unsigned int i = 0; i < _width; ++i) {
        unsigned int offset = rawStart + _channelCount * i;
        unsigned char tmp = imageData[offset + 0];
        imageData[offset + 0] = imageData[offset + 2];
        imageData[offset + 2] = tmp;
      }
    }
  }

  memcpy(image->getResourceBuffer(), imageData.data(), bytesPerImage);
  buffer->release();

  return image;
}

MATERIALX_NAMESPACE_END
