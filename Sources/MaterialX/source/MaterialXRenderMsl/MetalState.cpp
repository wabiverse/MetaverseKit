//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include "MetalState.h"
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>

#include <MaterialXRenderMsl/MetalFramebuffer.h>

std::unique_ptr<MetalState> MetalState::singleton = nullptr;

MetalState::MetalState() {}

void MetalState::initialize(MTL::Device *mtlDevice,
                            MTL::CommandQueue *mtlCmdQueue) {
  device = mtlDevice;
  cmdQueue = mtlCmdQueue;

#if defined(MAC_OS_VERSION_11_0) || defined(IPHONE_OS_VERSION_14_0)
  supportsTiledPipeline = device->supportsFamily(MTL::GPUFamilyApple4);
#else
  supportsTiledPipeline = false;
#endif

  MTL::DepthStencilDescriptor *depthStencilDesc =
      MTL::DepthStencilDescriptor::alloc()->init();
  depthStencilDesc->setDepthWriteEnabled(true);
  depthStencilDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
  opaqueDepthStencilState = device->newDepthStencilState(depthStencilDesc);

  depthStencilDesc->setDepthWriteEnabled(false);
  depthStencilDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
  transparentDepthStencilState = device->newDepthStencilState(depthStencilDesc);

  depthStencilDesc->setDepthWriteEnabled(true);
  depthStencilDesc->setDepthCompareFunction(MTL::CompareFunctionAlways);
  envMapDepthStencilState = device->newDepthStencilState(depthStencilDesc);

  initLinearToSRGBKernel();
}

void MetalState::initLinearToSRGBKernel() {
  NS::Error *error = nil;
  MTL::CompileOptions *options = MTL::CompileOptions::alloc()->init();
#if defined(MAC_OS_VERSION_11_0) || defined(IPHONE_OS_VERSION_14_0)
  options->setLanguageVersion(MTL::LanguageVersion2_3);
#else
  options->setLanguageVersion(MTL::LanguageVersion2_0);
#endif // defined(MAC_OS_VERSION_11_0) || defined(IPHONE_OS_VERSION_14_0)
  options->setFastMathEnabled(true);

#if defined(MAC_OS_VERSION_11_0) || defined(IPHONE_OS_VERSION_14_0)
  bool useTiledPipeline = supportsTiledPipeline;
  if (useTiledPipeline) {
    NS::String *linearToSRGB_kernel = NS::String::string(
        "#include <metal_stdlib>\n"
        "#include <simd/simd.h>\n"
        "\n"
        "\n"
        "using namespace metal;\n"
        "\n"
        "\n"
        "struct RenderTarget {\n"
        "    half4 colorTarget [[color(0)]];\n"
        "};\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "half4 linearToSRGB(half4 color_linear)\n"
        "{\n"
        "  \n"
        "    half4 color_srgb;\n"
        "  \n"
        "    for(int i = 0; i < 3; ++i)\n"
        "        color_srgb[i] = (color_linear[i] < 0.0031308) ?\n"
        "            (12.92 * color_linear[i])                 :\n"
        "            (1.055 * pow(color_linear[i], 1.0h / 2.2h) - 0.055);\n"
        "    color_srgb[3] = color_linear[3];\n"
        "    return color_srgb;\n"
        "}\n"
        "\n"
        "kernel void LinearToSRGB_kernel(\n"
        "    imageblock<RenderTarget,imageblock_layout_implicit> imageBlock,\n"
        "    ushort2 tid [[ thread_position_in_threadgroup ]])\n"
        "{\n"
        " \n"
        "    RenderTarget linearValue = imageBlock.read(tid);\n"
        "    RenderTarget srgbValue;\n"
        "    srgbValue.colorTarget = linearToSRGB(linearValue.colorTarget);\n"
        "    imageBlock.write(srgbValue, tid);\n"
        "};\n",
        NS::UTF8StringEncoding);
    MTL::Library *library =
        device->newLibrary(linearToSRGB_kernel, options, &error);
    MTL::Function *function = library->newFunction(
        NS::String::string("LinearToSRGB_kernel", NS::UTF8StringEncoding));

    MTL::TileRenderPipelineDescriptor *renderPipelineDescriptor =
        MTL::TileRenderPipelineDescriptor::alloc()->init();
    renderPipelineDescriptor->setRasterSampleCount(1);
    renderPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(
        MTL::PixelFormatBGRA8Unorm);
    renderPipelineDescriptor->setTileFunction(function);
    linearToSRGB_pso = device->newRenderPipelineState(renderPipelineDescriptor,
                                                      0, nil, &error);
  }
#else
  useTiledPipeline = false;
#endif
    if (!useTiledPipeline) {
      NS::String *linearToSRGB_kernel = NS::String::string(
          "#include <metal_stdlib>                                       \n"
          "#include <simd/simd.h>                                        \n"
          "                                                              \n"
          "using namespace metal;                                        \n"
          "                                                              \n"
          "struct VSOutput                                               \n"
          "{                                                             \n"
          "    float4 position [[position]];                             \n"
          "};                                                            \n"
          "                                                              \n"
          "vertex VSOutput VertexMain(uint vertexId [[ vertex_id ]])     \n"
          "{                                                             \n"
          "    VSOutput vsOut;                                           \n"
          "                                                              \n"
          "    switch(vertexId)                                          \n"
          "    {                                                         \n"
          "    case 0: vsOut.position = float4(-1, -1, 0.5, 1); break;   \n"
          "    case 1: vsOut.position = float4(-1,  3, 0.5, 1); break;   \n"
          "    case 2: vsOut.position = float4( 3, -1, 0.5, 1); break;   \n"
          "    };                                                        \n"
          "                                                              \n"
          "    return vsOut;                                             \n"
          "}                                                             \n"
          "                                                              \n"
          "half4 linearToSRGB(half4 color_linear)                        \n"
          "{                                                             \n"
          "    half4 color_srgb;                                         \n"
          "    for(int i = 0; i < 3; ++i)                                \n"
          "        color_srgb[i] = (color_linear[i] < 0.0031308) ?       \n"
          "          (12.92 * color_linear[i])                   :       \n"
          "          (1.055 * pow(color_linear[i], 1.0h / 2.2h) - 0.055);\n"
          "    color_srgb[3] = color_linear[3];                          \n"
          "    return color_srgb;                                        \n"
          "}                                                             \n"
          "                                                              \n"
          "fragment half4 FragmentMain(                                  \n"
          "    texture2d<half>  inputTex  [[ texture(0) ]],              \n"
          "    float4           fragCoord [[ position ]]                 \n"
          ")                                                             \n"
          "{                                                             \n"
          "    constexpr sampler ss(                                     \n"
          "        coord::pixel,                                         \n"
          "        address::clamp_to_border,                             \n"
          "        filter::linear);                                      \n"
          "    return linearToSRGB(inputTex.sample(ss, fragCoord.xy));   \n"
          "}                                                             \n",
          NS::UTF8StringEncoding);

      MTL::Library *library =
          device->newLibrary(linearToSRGB_kernel, options, &error);

      MTL::Function *vertexfunction = library->newFunction(
          NS::String::string("VertexMain", NS::UTF8StringEncoding));
      MTL::Function *Fragmentfunction = library->newFunction(
          NS::String::string("FragmentMain", NS::UTF8StringEncoding));

      MTL::RenderPipelineDescriptor *renderPipelineDesc =
          MTL::RenderPipelineDescriptor::alloc()->init();
      renderPipelineDesc->setVertexFunction(vertexfunction);
      renderPipelineDesc->setFragmentFunction(Fragmentfunction);
      renderPipelineDesc->colorAttachments()->object(0)->setPixelFormat(
          MTL::PixelFormatBGRA8Unorm);
      renderPipelineDesc->setDepthAttachmentPixelFormat(
          MTL::PixelFormatDepth32Float);
      linearToSRGB_pso =
          device->newRenderPipelineState(renderPipelineDesc, &error);
    }
  }

  void MetalState::triggerProgrammaticCapture() {
    MTL::CaptureManager *captureManager =
        MTL::CaptureManager::sharedCaptureManager();
    MTL::CaptureDescriptor *captureDescriptor = MTL::CaptureDescriptor::alloc()->init();

    captureDescriptor->setCaptureObject(device);

    NS::Error *error = nil;
    if (!captureManager->startCapture(captureDescriptor, &error)) {
#if WITH_APPLE_NSLOG
      NS::Log(NS::String::string("Failed to start capture, error %@", error));
#endif /* WITH_APPLE_NSLOG */
    }
  }

  void MetalState::stopProgrammaticCapture() {
    MTL::CaptureManager *captureManager =
        MTL::CaptureManager::sharedCaptureManager();
    captureManager->stopCapture();
  }

  void MetalState::beginCommandBuffer() {
    cmdBuffer = cmdQueue->commandBuffer();
    inFlightCommandBuffers++;
  }

  void MetalState::beginEncoder(MTL::RenderPassDescriptor * renderpassDesc) {
    renderCmdEncoder = cmdBuffer->renderCommandEncoder(renderpassDesc);
  }

  void MetalState::endEncoder() { renderCmdEncoder->endEncoding(); }

  void MetalState::endCommandBuffer() {
    endEncoder();
    cmdBuffer->addCompletedHandler([&](MTL::CommandBuffer *buffer) -> void {
      inFlightCommandBuffers--;
      inFlightCV.notify_one();
    });
    cmdBuffer->commit();
    cmdBuffer->waitUntilCompleted();
  }

  void MetalState::waitForComplition() {
    std::unique_lock<std::mutex> lock(inFlightMutex);
    while (inFlightCommandBuffers != 0) {
      inFlightCV.wait(lock,
                      [this] { return inFlightCommandBuffers.load() == 0; });
    }
  }

  MaterialX::MetalFramebufferPtr MetalState::currentFramebuffer() {
    return framebufferStack.top();
  }
