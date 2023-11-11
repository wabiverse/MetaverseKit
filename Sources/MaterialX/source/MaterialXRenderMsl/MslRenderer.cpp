//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <Foundation/Foundation.hpp>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXRender/TinyObjLoader.h>
#include <MaterialXRenderHw/SimpleWindow.h>
#include <MaterialXRenderMsl/MslRenderer.h>

#include <iostream>

#include <Metal/Metal.hpp>

MATERIALX_NAMESPACE_BEGIN

//
// MslRenderer methods
//

MslRendererPtr MslRenderer::create(unsigned int width, unsigned int height,
                                   Image::BaseType baseType) {
  return MslRendererPtr(new MslRenderer(width, height, baseType));
}

MTL::Device *MslRenderer::getMetalDevice() const { return _device; }

MslRenderer::MslRenderer(unsigned int width, unsigned int height,
                         Image::BaseType baseType)
    : ShaderRenderer(width, height, baseType, MatrixConvention::Metal),
      _initialized(false), _screenColor(DEFAULT_SCREEN_COLOR_LIN_REC709) {
  _program = MslProgram::create();

  _geometryHandler = GeometryHandler::create();
  _geometryHandler->addLoader(TinyObjLoader::create());
}

void MslRenderer::initialize(RenderContextHandle) {
  if (!_initialized) {
    // Create window
    _window = SimpleWindow::create();

    if (!_window->initialize("Renderer Window", _width, _height, nullptr)) {
      throw ExceptionRenderError("Failed to initialize renderer window");
    }

    _device = MTL::CreateSystemDefaultDevice();
    _cmdQueue = _device->newCommandQueue();
    createFrameBuffer(true);

    _initialized = true;
  }
}

void MslRenderer::createProgram(ShaderPtr shader) {
  _program = MslProgram::create();
  _program->setStages(shader);
  _program->build(_device, _framebuffer);
}

void MslRenderer::createProgram(const StageMap &stages) {
  for (const auto &it : stages) {
    _program->addStage(it.first, it.second);
  }
  _program->build(_device, _framebuffer);
}

void MslRenderer::renderTextureSpace(const Vector2 &uvMin,
                                     const Vector2 &uvMax) {
  bool captureRenderTextureSpace = false;
  if (captureRenderTextureSpace)
    triggerProgrammaticCapture();

  MTL::RenderPassDescriptor *desc = MTL::RenderPassDescriptor::alloc()->init();
  _framebuffer->bind(desc);

  _cmdBuffer = _cmdQueue->commandBuffer();

  MTL::RenderCommandEncoder *rendercmdEncoder =
      _cmdBuffer->renderCommandEncoder(desc);
  _program->bind(rendercmdEncoder);
  _program->prepareUsedResources(rendercmdEncoder, _camera, _geometryHandler,
                                 _imageHandler, _lightHandler);

  MeshPtr mesh = _geometryHandler->createQuadMesh(uvMin, uvMax, true);
  _program->bindMesh(rendercmdEncoder, mesh);
  MeshPartitionPtr part = mesh->getPartition(0);
  _program->bindPartition(part);
  MeshIndexBuffer &indexData = part->getIndices();
  rendercmdEncoder->drawIndexedPrimitives(
      MTL::PrimitiveTypeTriangle, indexData.size(), MTL::IndexTypeUInt32,
      _program->getIndexBuffer(part), 0);

  _framebuffer->unbind();
  rendercmdEncoder->endEncoding();

  _cmdBuffer->commit();
  _cmdBuffer->waitUntilCompleted();

  desc->release();

  if (captureRenderTextureSpace)
    stopProgrammaticCapture();
}

void MslRenderer::validateInputs() {
  // Check that the generated uniforms and attributes are valid
  _program->getUniformsList();
  _program->getAttributesList();
}

void MslRenderer::updateUniform(const string &name, ConstValuePtr value) {
  _program->bindUniform(name, value);
}

void MslRenderer::createFrameBuffer(bool encodeSrgb) {
  _framebuffer = MetalFramebuffer::create(_device, _width, _height, 4,
                                          _baseType, nil, encodeSrgb);
}

void MslRenderer::setSize(unsigned int width, unsigned int height) {
  if (_framebuffer) {
    _framebuffer->resize(width, height);
  } else {
    _width = width;
    _height = height;
    createFrameBuffer(true);
  }
}

void MslRenderer::triggerProgrammaticCapture() {
  MTL::CaptureManager *captureManager =
      MTL::CaptureManager::sharedCaptureManager();
  MTL::CaptureDescriptor *captureDescriptor =
      MTL::CaptureDescriptor::alloc()->init();
  captureDescriptor->setCaptureObject(_device);

  NS::Error *error = nil;
  if (!captureManager->startCapture(captureDescriptor, &error)) {
#if WITH_APPLE_NSLOG
    NS::Log(NS::String::string("Failed to start capture, error %@", error),
            NS::UTF8StringEncoding);
#endif /* WITH_APPLE_NSLOG */
  }
}

void MslRenderer::stopProgrammaticCapture() {
  MTL::CaptureManager *captureManager =
      MTL::CaptureManager::sharedCaptureManager();
  captureManager->stopCapture();
}

void MslRenderer::render() {
  bool captureFrame = false;
  if (captureFrame)
    triggerProgrammaticCapture();

  _cmdBuffer = _cmdQueue->commandBuffer();
  MTL::RenderPassDescriptor *renderpassDesc =
      MTL::RenderPassDescriptor::alloc()->init();

  _framebuffer->bind(renderpassDesc);
  renderpassDesc->colorAttachments()->object(0)->setClearColor(
      MTL::ClearColor::Make(_screenColor[0], _screenColor[1], _screenColor[2],
                            1.0f));

  MTL::RenderCommandEncoder *renderCmdEncoder =
      _cmdBuffer->renderCommandEncoder(renderpassDesc);

  MTL::DepthStencilDescriptor *depthStencilDesc =
      MTL::DepthStencilDescriptor::alloc()->init();
  depthStencilDesc->setDepthWriteEnabled(!(_program->isTransparent()));
  depthStencilDesc->setDepthCompareFunction(MTL::CompareFunctionLess);

  MTL::DepthStencilState *depthStencilState = _device->newDepthStencilState(depthStencilDesc);
  renderCmdEncoder->setDepthStencilState(depthStencilState);

  renderCmdEncoder->setCullMode(MTL::CullModeBack);

  try {
    // Bind program and input parameters
    if (_program) {
      // Bind the program to use
      _program->bind(renderCmdEncoder);
      _program->prepareUsedResources(renderCmdEncoder, _camera,
                                     _geometryHandler, _imageHandler,
                                     _lightHandler);

      // Draw all the partitions of all the meshes in the handler
      for (const auto &mesh : _geometryHandler->getMeshes()) {
        _program->bindMesh(renderCmdEncoder, mesh);

        for (size_t i = 0; i < mesh->getPartitionCount(); i++) {
          auto part = mesh->getPartition(i);
          _program->bindPartition(part);
          MeshIndexBuffer &indexData = part->getIndices();

          if (_program->isTransparent()) {
            renderCmdEncoder->setCullMode(MTL::CullModeFront);
            renderCmdEncoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle,
                                                    (int)indexData.size(),
                                                    MTL::IndexTypeUInt32,
                                                    _program->getIndexBuffer(part),
                                                    0);
            renderCmdEncoder->setCullMode(MTL::CullModeBack);
          }

          renderCmdEncoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle,
                                                  (int)indexData.size(),
                                                  MTL::IndexTypeUInt32,
                                                  _program->getIndexBuffer(part),
                                                  0);
        }
      }
    }
  } catch (ExceptionRenderError &e) {
    _framebuffer->unbind();
    throw e;
  }

  renderCmdEncoder->endEncoding();

  _framebuffer->unbind();

  _cmdBuffer->commit();
  _cmdBuffer->waitUntilCompleted();

  _cmdBuffer->release();
  _cmdBuffer = nil;

  if (captureFrame)
    stopProgrammaticCapture();
}

ImagePtr MslRenderer::captureImage(ImagePtr image) {
  return _framebuffer->getColorImage(_cmdQueue, image);
}

MATERIALX_NAMESPACE_END
