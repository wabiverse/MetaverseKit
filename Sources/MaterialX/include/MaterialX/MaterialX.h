#ifndef __MATERIALX_H__
#define __MATERIALX_H__

#include <MaterialX/Cgltf.h>
#include <MaterialX/MXCoreDefinition.h>
#include <MaterialX/MXCoreDocument.h>
#include <MaterialX/MXCoreElement.h>
#include <MaterialX/MXCoreException.h>
#include <MaterialX/MXCoreExport.h>
#include <MaterialX/MXCoreGenerated.h>
#include <MaterialX/MXCoreGeom.h>
#include <MaterialX/MXCoreInterface.h>
#include <MaterialX/MXCoreLibrary.h>
#include <MaterialX/MXCoreLook.h>
#include <MaterialX/MXCoreMaterial.h>
#include <MaterialX/MXCoreNode.h>
#include <MaterialX/MXCoreProperty.h>
#include <MaterialX/MXCoreTraversal.h>
#include <MaterialX/MXCoreTypes.h>
#include <MaterialX/MXCoreUnit.h>
#include <MaterialX/MXCoreUtil.h>
#include <MaterialX/MXCoreValue.h>
#include <MaterialX/MXCoreVariant.h>
#include <MaterialX/MXGenMdlBlurNodeMdl.h>
#include <MaterialX/MXGenMdlClosureCompoundNodeMdl.h>
#include <MaterialX/MXGenMdlClosureLayerNodeMdl.h>
#include <MaterialX/MXGenMdlClosureSourceCodeNodeMdl.h>
#include <MaterialX/MXGenMdlCombineNodeMdl.h>
#include <MaterialX/MXGenMdlCompoundNodeMdl.h>
#include <MaterialX/MXGenMdlExport.h>
#include <MaterialX/MXGenMdlHeightToNormalNodeMdl.h>
#include <MaterialX/MXGenMdlImageNodeMdl.h>
#include <MaterialX/MXGenMdlMaterialNodeMdl.h>
#include <MaterialX/MXGenMdlShaderGenerator.h>
#include <MaterialX/MXGenMdlSourceCodeNodeMdl.h>
#include <MaterialX/MXGenMdlSurfaceNodeMdl.h>
#include <MaterialX/MXGenMdlSwizzleNodeMdl.h>
#include <MaterialX/MXGenMdlSyntax.h>
#include <MaterialX/MXGenOslBlurNodeOsl.h>
#include <MaterialX/MXGenOslClosureLayerNodeOsl.h>
#include <MaterialX/MXGenOslExport.h>
#include <MaterialX/MXGenOslMaterialNodeOsl.h>
#include <MaterialX/MXGenOslShaderGenerator.h>
#include <MaterialX/MXGenOslSurfaceNodeOsl.h>
#include <MaterialX/MXGenOslSyntax.h>

#include <MaterialX/MXGraphEditorFileDialog.h>
#include <MaterialX/MXGraphEditorGraph.h>
#include <MaterialX/MXGraphEditorRenderView.h>
#include <MaterialX/MXGraphEditorUiNode.h>
#include <MaterialX/MXRenderCamera.h>
#include <MaterialX/MXRenderCgltfLoader.h>
#include <MaterialX/MXRenderExport.h>
#include <MaterialX/MXRenderGeometryHandler.h>
#include <MaterialX/MXRenderHarmonics.h>
#include <MaterialX/MXRenderHwExport.h>
#include <MaterialX/MXRenderHwSimpleWindow.h>
#include <MaterialX/MXRenderHwWindowCocoaWrappers.h>
#include <MaterialX/MXRenderHwWindowWrapper.h>
#include <MaterialX/MXRenderImage.h>
#include <MaterialX/MXRenderImageHandler.h>
#include <MaterialX/MXRenderLightHandler.h>
#include <MaterialX/MXRenderMesh.h>
#if defined(__APPLE__)
#include <MaterialX/MXRenderMslExport.h>
#include <MaterialX/MXRenderMslMaterial.h>
#include <MaterialX/MXRenderMslMetalFramebuffer.h>
#include <MaterialX/MXRenderMslMetalState.h>
#include <MaterialX/MXRenderMslPipelineStateObject.h>
#endif /* defined(__APPLE__) */
#include <MaterialX/MXRenderOiioImageLoader.h>
#include <MaterialX/MXRenderShaderMaterial.h>
#include <MaterialX/MXRenderShaderRenderer.h>
#include <MaterialX/MXRenderStbImageLoader.h>
#include <MaterialX/MXRenderTextureBaker.h>
#include <MaterialX/MXRenderTimer.h>
#include <MaterialX/MXRenderTinyObjLoader.h>
#include <MaterialX/MXRenderTypes.h>
#include <MaterialX/MXRenderUtil.h>
#include <MaterialX/StbImage.h>
#include <MaterialX/StbImageWrite.h>
#include <MaterialX/TinyObjLoader.h>

#ifdef WITH_MATERIALX_JAVASCRIPT
#include <MaterialX/JsMXHelpers.h>
#include <MaterialX/JsMXMapHelper.h>
#include <MaterialX/JsMXStrContainerTypeRegistration.h>
#include <MaterialX/JsMXVectorHelper.h>
#endif /* WITH_MATERIALX_JAVASCRIPT */

#include <MaterialX/Glad.h>
#include <MaterialX/KHRPlatform.h>
#include <MaterialX/MXFormatEnviron.h>
#include <MaterialX/MXFormatExport.h>
#include <MaterialX/MXFormatFile.h>
#include <MaterialX/MXFormatUtil.h>
#include <MaterialX/MXFormatXmlIo.h>
#include <MaterialX/MXGenGlslBitangentNodeGlsl.h>
#include <MaterialX/MXGenGlslBlurNodeGlsl.h>
#include <MaterialX/MXGenGlslEsslShaderGenerator.h>
#include <MaterialX/MXGenGlslEsslSyntax.h>
#include <MaterialX/MXGenGlslExport.h>
#include <MaterialX/MXGenGlslFrameNodeGlsl.h>
#include <MaterialX/MXGenGlslGeomColorNodeGlsl.h>
#include <MaterialX/MXGenGlslGeomPropValueNodeGlsl.h>
#include <MaterialX/MXGenGlslHeightToNormalNodeGlsl.h>
#include <MaterialX/MXGenGlslLightCompoundNodeGlsl.h>
#include <MaterialX/MXGenGlslLightNodeGlsl.h>
#include <MaterialX/MXGenGlslLightSamplerNodeGlsl.h>
#include <MaterialX/MXGenGlslLightShaderNodeGlsl.h>
#include <MaterialX/MXGenGlslNormalNodeGlsl.h>
#include <MaterialX/MXGenGlslNumLightsNodeGlsl.h>
#include <MaterialX/MXGenGlslPositionNodeGlsl.h>
#include <MaterialX/MXGenGlslResourceBindingContext.h>
#include <MaterialX/MXGenGlslShaderGenerator.h>
#include <MaterialX/MXGenGlslSurfaceNodeGlsl.h>
#include <MaterialX/MXGenGlslSurfaceShaderNodeGlsl.h>
#include <MaterialX/MXGenGlslSyntax.h>
#include <MaterialX/MXGenGlslTangentNodeGlsl.h>
#include <MaterialX/MXGenGlslTimeNodeGlsl.h>
#include <MaterialX/MXGenGlslUnlitSurfaceNodeGlsl.h>
#include <MaterialX/MXGenGlslVkResourceBindingContext.h>
#include <MaterialX/MXGenGlslVkShaderGenerator.h>
#include <MaterialX/MXGenGlslVkSyntax.h>
#if defined(__APPLE__)
#include <MaterialX/MXGenMslBitangentNodeMsl.h>
#include <MaterialX/MXGenMslBlurNodeMsl.h>
#include <MaterialX/MXGenMslFrameNodeMsl.h>
#include <MaterialX/MXGenMslGeomColorNodeMsl.h>
#include <MaterialX/MXGenMslGeomPropValueNodeMsl.h>
#include <MaterialX/MXGenMslHeightToNormalNodeMsl.h>
#include <MaterialX/MXGenMslLightCompoundNodeMsl.h>
#include <MaterialX/MXGenMslLightNodeMsl.h>
#include <MaterialX/MXGenMslLightSamplerNodeMsl.h>
#include <MaterialX/MXGenMslLightShaderNodeMsl.h>
#include <MaterialX/MXGenMslNormalNodeMsl.h>
#include <MaterialX/MXGenMslNumLightsNodeMsl.h>
#include <MaterialX/MXGenMslPositionNodeMsl.h>
#include <MaterialX/MXGenMslSurfaceShaderNodeMsl.h>
#include <MaterialX/MXGenMslTangentNodeMsl.h>
#include <MaterialX/MXGenMslTimeNodeMsl.h>
#include <MaterialX/MXGenMslUnlitSurfaceNodeMsl.h>
#endif /* defined(__APPLE__) */
#include <MaterialX/MXGenMslExport.h>
#include <MaterialX/MXGenMslShaderGenerator.h>
#include <MaterialX/MXGenMslResourceBindingContext.h>
#include <MaterialX/MXGenMslSurfaceNodeMsl.h>
#include <MaterialX/MXGenMslSyntax.h>
#include <MaterialX/MXGenShader.h>
#include <MaterialX/MXGenShaderBlurNode.h>
#include <MaterialX/MXGenShaderClosureAddNode.h>
#include <MaterialX/MXGenShaderClosureCompoundNode.h>
#include <MaterialX/MXGenShaderClosureLayerNode.h>
#include <MaterialX/MXGenShaderClosureMixNode.h>
#include <MaterialX/MXGenShaderClosureMultiplyNode.h>
#include <MaterialX/MXGenShaderClosureSourceCodeNode.h>
#include <MaterialX/MXGenShaderColorManagementSystem.h>
#include <MaterialX/MXGenShaderCombineNode.h>
#include <MaterialX/MXGenShaderCompoundNode.h>
#include <MaterialX/MXGenShaderConvertNode.h>
#include <MaterialX/MXGenShaderConvolutionNode.h>
#include <MaterialX/MXGenShaderDefaultColorManagementSystem.h>
#include <MaterialX/MXGenShaderExport.h>
#include <MaterialX/MXGenShaderFactory.h>
#include <MaterialX/MXGenShaderGenContext.h>
#include <MaterialX/MXGenShaderGenOptions.h>
#include <MaterialX/MXGenShaderGenUserData.h>
#include <MaterialX/MXGenShaderGenerator.h>
#include <MaterialX/MXGenShaderGraph.h>
#include <MaterialX/MXGenShaderHwImageNode.h>
#include <MaterialX/MXGenShaderHwShaderGenerator.h>
#include <MaterialX/MXGenShaderHwTexCoordNode.h>
#include <MaterialX/MXGenShaderHwTransformNode.h>
#include <MaterialX/MXGenShaderLibrary.h>
#include <MaterialX/MXGenShaderMaterialNode.h>
#include <MaterialX/MXGenShaderNode.h>
#include <MaterialX/MXGenShaderNodeImpl.h>
#include <MaterialX/MXGenShaderSourceCodeNode.h>
#include <MaterialX/MXGenShaderStage.h>
#include <MaterialX/MXGenShaderSwitchNode.h>
#include <MaterialX/MXGenShaderSwizzleNode.h>
#include <MaterialX/MXGenShaderSyntax.h>
#include <MaterialX/MXGenShaderTranslator.h>
#include <MaterialX/MXGenShaderTypeDesc.h>
#include <MaterialX/MXGenShaderUnitSystem.h>
#include <MaterialX/MXGenShaderUtil.h>
#include <MaterialX/MXRenderGlslExport.h>
#include <MaterialX/MXRenderGlslGLCocoaWrappers.h>
#include <MaterialX/MXRenderGlslGLContext.h>
#include <MaterialX/MXRenderGlslGLFramebuffer.h>
#include <MaterialX/MXRenderGlslGLUtil.h>
#include <MaterialX/MXRenderGlslGlslMaterial.h>
#include <MaterialX/MXRenderGlslGlslProgram.h>

#if defined(__APPLE__)
# include <MaterialX/MXRenderMslMetalTextureHandler.h>
# include <MaterialX/MXRenderMslRenderer.h>
# include <MaterialX/MXRenderMslTextureBaker.h>
#else /* defined(__APPLE__) */
# include <MaterialX/MXRenderGlslGLTextureHandler.h>
# include <MaterialX/MXRenderGlslGlslRenderer.h>
# include <MaterialX/MXRenderGlslTextureBaker.h>
#endif /* !defined(__APPLE__) */

#include <MaterialX/MXRenderOslExport.h>
#include <MaterialX/MXRenderOslRenderer.h>

#if WITH_MATERIALX_VIEWER
#include <MaterialX/MXViewEditor.h>
#include <MaterialX/MXViewRenderPipeline.h>
#include <MaterialX/MXViewRenderPipelineGL.h>
#include <MaterialX/MXViewRenderPipelineMetal.h>
#include <MaterialX/MXViewViewer.h>
#endif /* WITH_MATERIALX_VIEWER */

#include <MaterialX/PugiConfig.hpp>
#include <MaterialX/PugiXML.hpp>

#endif /* __MATERIALX_H__ */
