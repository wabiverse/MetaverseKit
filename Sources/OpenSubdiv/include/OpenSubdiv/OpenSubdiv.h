/* ----------------------------------------------------------------
 * :: :  M  E  T  A  V  E  R  S  E  :                            ::
 * ----------------------------------------------------------------
 * This software is Licensed under the terms of the Apache License,
 * version 2.0 (the "Apache License") with the following additional
 * modification; you may not use this file except within compliance
 * of the Apache License and the following modification made to it.
 * Section 6. Trademarks. is deleted and replaced with:
 *
 * Trademarks. This License does not grant permission to use any of
 * its trade names, trademarks, service marks, or the product names
 * of this Licensor or its affiliates, except as required to comply
 * with Section 4(c.) of this License, and to reproduce the content
 * of the NOTICE file.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND without even an
 * implied warranty of MERCHANTABILITY, or FITNESS FOR A PARTICULAR
 * PURPOSE. See the Apache License for more details.
 *
 * You should have received a copy for this software license of the
 * Apache License along with this program; or, if not, please write
 * to the Free Software Foundation Inc., with the following address
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *         Copyright (C) 2024 Wabi Foundation. All Rights Reserved.
 * ----------------------------------------------------------------
 *  . x x x . o o o . x x x . : : : .    o  x  o    . : : : .
 * ---------------------------------------------------------------- */

#ifndef __OPENSUBDIV_H__
#define __OPENSUBDIV_H__

/* version. */
#include <OpenSubdiv/OSDVersion.h>

/* core. */
#include <OpenSubdiv/OSDCoreBilinearScheme.h>
#include <OpenSubdiv/OSDCoreCatmarkScheme.h>
#include <OpenSubdiv/OSDCoreCrease.h>
#include <OpenSubdiv/OSDCoreLoopScheme.h>
#include <OpenSubdiv/OSDCoreOptions.h>
#include <OpenSubdiv/OSDCoreScheme.h>
#include <OpenSubdiv/OSDCoreTypes.h>

/* adaptive refinement. */
#include <OpenSubdiv/OSDAdaptiveBilinearPatchBuilder.h>
#include <OpenSubdiv/OSDAdaptiveCatmarkPatchBuilder.h>
#include <OpenSubdiv/OSDAdaptiveError.h>
#include <OpenSubdiv/OSDAdaptiveLoopPatchBuilder.h>
#include <OpenSubdiv/OSDAdaptivePatchBasis.h>
#include <OpenSubdiv/OSDAdaptivePatchBuilder.h>
#include <OpenSubdiv/OSDAdaptivePatchDescriptor.h>
#include <OpenSubdiv/OSDAdaptivePatchMap.h>
#include <OpenSubdiv/OSDAdaptivePatchParam.h>
#include <OpenSubdiv/OSDAdaptivePatchTable.h>
#include <OpenSubdiv/OSDAdaptivePatchTableFactory.h>
#include <OpenSubdiv/OSDAdaptivePrimvarRefiner.h>
#include <OpenSubdiv/OSDAdaptivePtexIndices.h>
#include <OpenSubdiv/OSDAdaptiveSparseMatrix.h>
#include <OpenSubdiv/OSDAdaptiveStencilBuilder.h>
#include <OpenSubdiv/OSDAdaptiveStencilTable.h>
#include <OpenSubdiv/OSDAdaptiveStencilTableFactory.h>
#include <OpenSubdiv/OSDAdaptiveTopologyDescriptor.h>
#include <OpenSubdiv/OSDAdaptiveTopologyLevel.h>
#include <OpenSubdiv/OSDAdaptiveTopologyRefiner.h>
#include <OpenSubdiv/OSDAdaptiveTopologyRefinerFactory.h>
#include <OpenSubdiv/OSDAdaptiveTypes.h>

/* base face representation. */
#include <OpenSubdiv/OSDBaseFaceSurface.h>
#include <OpenSubdiv/OSDBaseFaceTopology.h>
#include <OpenSubdiv/OSDBaseFaceVertex.h>
#include <OpenSubdiv/OSDBaseFaceVertexSubset.h>
#include <OpenSubdiv/OSDBaseHash.h>
#include <OpenSubdiv/OSDBaseIrregularPatchBuilder.h>
#include <OpenSubdiv/OSDBaseIrregularPatchType.h>
#include <OpenSubdiv/OSDBaseLimits.h>
#include <OpenSubdiv/OSDBaseParameterization.h>
#include <OpenSubdiv/OSDBasePatchTree.h>
#include <OpenSubdiv/OSDBasePatchTreeBuilder.h>
#include <OpenSubdiv/OSDBasePointOperations.h>
#include <OpenSubdiv/OSDBaseRefinerSurfaceFactory.h>
#include <OpenSubdiv/OSDBaseRegularPatchBuilder.h>
#include <OpenSubdiv/OSDBaseSurface.h>
#include <OpenSubdiv/OSDBaseSurfaceData.h>
#include <OpenSubdiv/OSDBaseSurfaceFactory.h>
#include <OpenSubdiv/OSDBaseSurfaceFactoryCache.h>
#include <OpenSubdiv/OSDBaseSurfaceFactoryMeshAdapter.h>
#include <OpenSubdiv/OSDBaseTessellation.h>
#include <OpenSubdiv/OSDBaseVertexDescriptor.h>
#include <OpenSubdiv/OSDBaseVertexTag.h>

/* hierarchical boundary representation. */
#include <OpenSubdiv/OSDBoundHierarchicalEdit.h>
#include <OpenSubdiv/OSDBoundMesh.h>
#include <OpenSubdiv/OSDBoundVertex.h>
#include <OpenSubdiv/OSDBoundVertexEdit.h>
#include <OpenSubdiv/OSDBoundAllocator.h>
#include <OpenSubdiv/OSDBoundBilinear.h>
#include <OpenSubdiv/OSDBoundCatmark.h>
#include <OpenSubdiv/OSDBoundCornerEdit.h>
#include <OpenSubdiv/OSDBoundCreaseEdit.h>
#include <OpenSubdiv/OSDBoundFace.h>
#include <OpenSubdiv/OSDBoundFaceEdit.h>
#include <OpenSubdiv/OSDBoundFvarData.h>
#include <OpenSubdiv/OSDBoundFvarEdit.h>
#include <OpenSubdiv/OSDBoundHalfEdge.h>

#include <OpenSubdiv/OSDBoundHoleEdit.h>
#include <OpenSubdiv/OSDBoundLoop.h>
#include <OpenSubdiv/OSDBoundSubdivision.h>

/* device dependent surfaces. */
#include <OpenSubdiv/OSDSurfaceBufferDescriptor.h>
#include <OpenSubdiv/OSDSurfaceCpuEvaluator.h>
#include <OpenSubdiv/OSDSurfaceCpuGLVertexBuffer.h>
#include <OpenSubdiv/OSDSurfaceCpuKernel.h>
#include <OpenSubdiv/OSDSurfaceCpuPatchTable.h>
#include <OpenSubdiv/OSDSurfaceCpuVertexBuffer.h>
#ifdef WITH_OPENCL
# include <OpenSubdiv/OSDSurfaceOpenCLEvaluator.h>
# include <OpenSubdiv/OSDSurfaceOpenCLGLVertexBuffer.h>
# include <OpenSubdiv/OSDSurfaceOpenCLD3D11VertexBuffer.h>
# include <OpenSubdiv/OSDSurfaceOpenCLPatchTable.h>
# include <OpenSubdiv/OSDSurfaceOpenCLVertexBuffer.h>
# include <OpenSubdiv/OSDSurfaceOpenCL.h>
#endif /* WITH_OPENCL */
#ifdef __CUDA__
# include <OpenSubdiv/OSDSurfaceCudaEvaluator.h>
# include <OpenSubdiv/OSDSurfaceCudaD3D11VertexBuffer.h>
# include <OpenSubdiv/OSDSurfaceCudaGLVertexBuffer.h>
# include <OpenSubdiv/OSDSurfaceCudaVertexBuffer.h>
# include <OpenSubdiv/OSDSurfaceCudaPatchTable.h>
#endif /* __CUDA__ */
#include <OpenSubdiv/OSDSurfaceGLComputeEvaluator.h>
#include <OpenSubdiv/OSDSurfaceGLLegacyGregoryPatchTable.h>
#include <OpenSubdiv/OSDSurfaceGLMesh.h>
#include <OpenSubdiv/OSDSurfaceGLPatchTable.h>
#include <OpenSubdiv/OSDSurfaceGLSLPatchShaderSource.h>
#include <OpenSubdiv/OSDSurfaceGLVertexBuffer.h>
#include <OpenSubdiv/OSDSurfaceGLXFBEvaluator.h>
#include <OpenSubdiv/OSDSurfaceHLSLPatchShaderSource.h>
#if defined(__APPLE__)
#include <OpenSubdiv/OSDSurfaceMTLCommon.h>
#include <OpenSubdiv/OSDSurfaceMTLComputeEvaluator.h>
#include <OpenSubdiv/OSDSurfaceMTLLegacyGregoryPatchTable.h>
#include <OpenSubdiv/OSDSurfaceMTLMesh.h>
#include <OpenSubdiv/OSDSurfaceMTLPatchShaderSource.h>
#include <OpenSubdiv/OSDSurfaceMTLPatchTable.h>
#include <OpenSubdiv/OSDSurfaceMTLVertexBuffer.h>
#endif /* defined(__APPLE__) */
#include <OpenSubdiv/OSDSurfaceMesh.h>
#include <OpenSubdiv/OSDSurfaceNonCopyable.h>
#include <OpenSubdiv/OSDSurfaceOmpEvaluator.h>
#include <OpenSubdiv/OSDSurfaceOmpKernel.h>
#include <OpenSubdiv/OSDSurfaceOpenGL.h>
#include <OpenSubdiv/OSDSurfacePatchBasis.h>
#include <OpenSubdiv/OSDSurfacePatchBasisTypes.h>
#include <OpenSubdiv/OSDSurfaceTBBEvaluator.h>
#include <OpenSubdiv/OSDSurfaceTBBKernel.h>
#include <OpenSubdiv/OSDSurfaceTypes.h>
#ifdef WITH_DIRECTX
# include <OpenSubdiv/OSDSurfaceCpuD3D11VertexBuffer.h>
# include <OpenSubdiv/OSDSurfaced3D11ComputeEvaluator.h>
# include <OpenSubdiv/OSDSurfaced3D11LegacyGregoryPatchTable.h>
# include <OpenSubdiv/OSDSurfaced3D11Mesh.h>
# include <OpenSubdiv/OSDSurfaced3D11PatchTable.h>
# include <OpenSubdiv/OSDSurfaced3D11VertexBuffer.h>
#endif /* WITH_DIRECTX */

/* vectorized topology refinement. */
#include <OpenSubdiv/OSDVectorArray.h>
#include <OpenSubdiv/OSDVectorComponentInterfaces.h>
#include <OpenSubdiv/OSDVectorFvarLevel.h>
#include <OpenSubdiv/OSDVectorFvarRefinement.h>
#include <OpenSubdiv/OSDVectorLevel.h>
#include <OpenSubdiv/OSDVectorQuadRefinement.h>
#include <OpenSubdiv/OSDVectorRefinement.h>
#include <OpenSubdiv/OSDVectorSparseSelector.h>
#include <OpenSubdiv/OSDVectorStackBuffer.h>
#include <OpenSubdiv/OSDVectorTriRefinement.h>
#include <OpenSubdiv/OSDVectorTypes.h>

#endif /* __OPENSUBDIV_H__ */
