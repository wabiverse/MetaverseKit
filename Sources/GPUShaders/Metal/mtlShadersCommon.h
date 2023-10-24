#ifndef MTL_SHADERSCOMMON_h
#define MTL_SHADERSCOMMON_h

#include <metal_stdlib>
#include <metal_atomic>
using namespace metal;

// The following callback functions are used when evaluating tessellation
// rates and when using legacy patch drawing.
float4x4 OsdModelViewMatrix();
float4x4 OsdProjectionMatrix();
float OsdTessLevel();

int OsdGetPatchFaceId(int3 patchParam);

#endif /* MTL_SHADERSCOMMON_h */