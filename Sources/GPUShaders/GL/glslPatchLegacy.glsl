//
//   Copyright 2013-2018 Pixar
//
//   Licensed under the Apache License, Version 2.0 (the "Apache License")
//   with the following modification; you may not use this file except in
//   compliance with the Apache License and the following modification to it:
//   Section 6. Trademarks. is deleted and replaced with:
//
//   6. Trademarks. This License does not grant permission to use the trade
//      names, trademarks, service marks, or product names of the Licensor
//      and its affiliates, except as required to comply with Section 4(c) of
//      the License and to reproduce the content of the NOTICE file.
//
//   You may obtain a copy of the Apache License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the Apache License with the above modification is
//   distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
//   KIND, either express or implied. See the Apache License for the specific
//   language governing permissions and limitations under the Apache License.
//

//----------------------------------------------------------
// Patches.Common
//----------------------------------------------------------

// XXXdyu all handling of varying data can be managed by client code
#ifndef OSD_USER_VARYING_DECLARE
#define OSD_USER_VARYING_DECLARE
// type var;
#endif

#ifndef OSD_USER_VARYING_ATTRIBUTE_DECLARE
#define OSD_USER_VARYING_ATTRIBUTE_DECLARE
// layout(location = loc) in type var;
#endif

#ifndef OSD_USER_VARYING_PER_VERTEX
#define OSD_USER_VARYING_PER_VERTEX()
// output.var = var;
#endif

#ifndef OSD_USER_VARYING_PER_CONTROL_POINT
#define OSD_USER_VARYING_PER_CONTROL_POINT(ID_OUT, ID_IN)
// output[ID_OUT].var = input[ID_IN].var
#endif

#ifndef OSD_USER_VARYING_PER_EVAL_POINT
#define OSD_USER_VARYING_PER_EVAL_POINT(UV, a, b, c, d)
// output.var =
//     mix(mix(input[a].var, input[b].var, UV.x),
//         mix(input[c].var, input[d].var, UV.x), UV.y)
#endif

#ifndef OSD_USER_VARYING_PER_EVAL_POINT_TRIANGLE
#define OSD_USER_VARYING_PER_EVAL_POINT_TRIANGLE(UV, a, b, c)
// output.var =
//     input[a].var * (1.0f-UV.x-UV.y) +
//     input[b].var * UV.x +
//     input[c].var * UV.y;
#endif

#if __VERSION__ < 420
    #define centroid
#endif

struct ControlVertex {
    vec4 position;
#ifdef OSD_ENABLE_PATCH_CULL
    ivec3 clipFlag;
#endif
};

// XXXdyu all downstream data can be handled by client code
struct OutputVertex {
    vec4 position;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec4 patchCoord; // u, v, faceLevel, faceId
    vec2 tessCoord; // tesscoord.st
#if defined OSD_COMPUTE_NORMAL_DERIVATIVES
    vec3 Nu;
    vec3 Nv;
#endif
};

mat4 OsdModelViewProjectionMatrix();
int OsdGregoryQuadOffsetBase();
int OsdPrimitiveIdBase();
int OsdBaseVertex();

#ifndef OSD_DISPLACEMENT_CALLBACK
#define OSD_DISPLACEMENT_CALLBACK
#endif

// These are stored in OsdPatchParamBuffer indexed by the value returned
// from OsdGetPatchIndex() which is a function of the current PrimitiveID
// along with an optional client provided offset.

uniform isamplerBuffer OsdPatchParamBuffer;

int OsdGetPatchIndex(int primitiveId)
{
    return (primitiveId + OsdPrimitiveIdBase());
}

ivec3 OsdGetPatchParam(int patchIndex)
{
    return texelFetch(OsdPatchParamBuffer, patchIndex).xyz;
}

// ----------------------------------------------------------------------------
// patch culling
// ----------------------------------------------------------------------------

#ifdef OSD_ENABLE_PATCH_CULL

#define OSD_PATCH_CULL_COMPUTE_CLIPFLAGS(P)                     \
    vec4 clipPos = OsdModelViewProjectionMatrix() * P;          \
    bvec3 clip0 = lessThan(clipPos.xyz, vec3(clipPos.w));       \
    bvec3 clip1 = greaterThan(clipPos.xyz, -vec3(clipPos.w));   \
    outpt.v.clipFlag = ivec3(clip0) + 2*ivec3(clip1);           \

#define OSD_PATCH_CULL(N)                            \
    ivec3 clipFlag = ivec3(0);                       \
    for(int i = 0; i < N; ++i) {                     \
        clipFlag |= inpt[i].v.clipFlag;              \
    }                                                \
    if (clipFlag != ivec3(3) ) {                     \
        gl_TessLevelInner[0] = 0;                    \
        gl_TessLevelInner[1] = 0;                    \
        gl_TessLevelOuter[0] = 0;                    \
        gl_TessLevelOuter[1] = 0;                    \
        gl_TessLevelOuter[2] = 0;                    \
        gl_TessLevelOuter[3] = 0;                    \
        return;                                      \
    }

#else
#define OSD_PATCH_CULL_COMPUTE_CLIPFLAGS(P)
#define OSD_PATCH_CULL(N)
#endif

// ----------------------------------------------------------------------------
// Legacy Gregory
// ----------------------------------------------------------------------------
#if defined(OSD_PATCH_GREGORY) || defined(OSD_PATCH_GREGORY_BOUNDARY)

#define M_PI 3.14159265359f

// precomputed catmark coefficient table up to valence 29
uniform float OsdCatmarkCoefficient[30] = float[](
    0, 0, 0, 0.812816, 0.500000, 0.363644, 0.287514,
    0.238688, 0.204544, 0.179229, 0.159657,
    0.144042, 0.131276, 0.120632, 0.111614,
    0.103872, 0.09715, 0.0912559, 0.0860444,
    0.0814022, 0.0772401, 0.0734867, 0.0700842,
    0.0669851, 0.0641504, 0.0615475, 0.0591488,
    0.0569311, 0.0548745, 0.0529621
    );

float
OsdComputeCatmarkCoefficient(int valence)
{
#if OSD_MAX_VALENCE < 30
    return OsdCatmarkCoefficient[valence];
#else
    if (valence < 30) {
        return OsdCatmarkCoefficient[valence];
    } else {
        float t = 2.0f * float(M_PI) / float(valence);
        return 1.0f / (valence * (cos(t) + 5.0f +
                                  sqrt((cos(t) + 9) * (cos(t) + 1)))/16.0f);
    }
#endif
}

float cosfn(int n, int j) {
    return cos((2.0f * M_PI * j)/float(n));
}

float sinfn(int n, int j) {
    return sin((2.0f * M_PI * j)/float(n));
}

#if !defined OSD_MAX_VALENCE || OSD_MAX_VALENCE < 1
#undef OSD_MAX_VALENCE
#define OSD_MAX_VALENCE 4
#endif

struct OsdPerVertexGregory {
    vec3 P;
    ivec3 clipFlag;
    int  valence;
    vec3 e0;
    vec3 e1;
#ifdef OSD_PATCH_GREGORY_BOUNDARY
    int zerothNeighbor;
    vec3 org;
#endif
    vec3 r[OSD_MAX_VALENCE];
};

struct OsdPerPatchVertexGregory {
    ivec3 patchParam;
    vec3 P;
    vec3 Ep;
    vec3 Em;
    vec3 Fp;
    vec3 Fm;
};

#ifndef OSD_NUM_ELEMENTS
#define OSD_NUM_ELEMENTS 3
#endif

uniform samplerBuffer OsdVertexBuffer;
uniform isamplerBuffer OsdValenceBuffer;

vec3 OsdReadVertex(int vertexIndex)
{
    int index = int(OSD_NUM_ELEMENTS * (vertexIndex + OsdBaseVertex()));
    return vec3(texelFetch(OsdVertexBuffer, index).x,
                texelFetch(OsdVertexBuffer, index+1).x,
                texelFetch(OsdVertexBuffer, index+2).x);
}

int OsdReadVertexValence(int vertexID)
{
    int index = int(vertexID * (2 * OSD_MAX_VALENCE + 1));
    return texelFetch(OsdValenceBuffer, index).x;
}

int OsdReadVertexIndex(int vertexID, int valenceVertex)
{
    int index = int(vertexID * (2 * OSD_MAX_VALENCE + 1) + 1 + valenceVertex);
    return texelFetch(OsdValenceBuffer, index).x;
}

uniform isamplerBuffer OsdQuadOffsetBuffer;

int OsdReadQuadOffset(int primitiveID, int offsetVertex)
{
    int index = int(4*primitiveID+OsdGregoryQuadOffsetBase() + offsetVertex);
    return texelFetch(OsdQuadOffsetBuffer, index).x;
}

void
OsdComputePerVertexGregory(int vID, vec3 P, out OsdPerVertexGregory v)
{
    v.clipFlag = ivec3(0);

    int ivalence = OsdReadVertexValence(vID);
    v.valence = ivalence;
    int valence = abs(ivalence);

    vec3 f[OSD_MAX_VALENCE];
    vec3 pos = P;
    vec3 opos = vec3(0);

#ifdef OSD_PATCH_GREGORY_BOUNDARY
    v.org = pos;
    int boundaryEdgeNeighbors[2];
    int currNeighbor = 0;
    int ibefore = 0;
    int zerothNeighbor = 0;
#endif

    for (int i=0; i<valence; ++i) {
        int im = (i+valence-1)%valence;
        int ip = (i+1)%valence;

        int idx_neighbor = OsdReadVertexIndex(vID, 2*i);

#ifdef OSD_PATCH_GREGORY_BOUNDARY
        bool isBoundaryNeighbor = false;
        int valenceNeighbor = OsdReadVertexValence(idx_neighbor);

        if (valenceNeighbor < 0) {
            isBoundaryNeighbor = true;
            if (currNeighbor<2) {
                boundaryEdgeNeighbors[currNeighbor] = idx_neighbor;
            }
            currNeighbor++;
            if (currNeighbor == 1) {
                ibefore = i;
                zerothNeighbor = i;
            } else {
                if (i-ibefore == 1) {
                    int tmp = boundaryEdgeNeighbors[0];
                    boundaryEdgeNeighbors[0] = boundaryEdgeNeighbors[1];
                    boundaryEdgeNeighbors[1] = tmp;
                    zerothNeighbor = i;
                }
            }
        }
#endif

        vec3 neighbor = OsdReadVertex(idx_neighbor);

        int idx_diagonal = OsdReadVertexIndex(vID, 2*i + 1);
        vec3 diagonal = OsdReadVertex(idx_diagonal);

        int idx_neighbor_p = OsdReadVertexIndex(vID, 2*ip);
        vec3 neighbor_p = OsdReadVertex(idx_neighbor_p);

        int idx_neighbor_m = OsdReadVertexIndex(vID, 2*im);
        vec3 neighbor_m = OsdReadVertex(idx_neighbor_m);

        int idx_diagonal_m = OsdReadVertexIndex(vID, 2*im + 1);
        vec3 diagonal_m = OsdReadVertex(idx_diagonal_m);

        f[i] = (pos * float(valence) + (neighbor_p + neighbor)*2.0f + diagonal) / (float(valence)+5.0f);

        opos += f[i];
        v.r[i] = (neighbor_p-neighbor_m)/3.0f + (diagonal - diagonal_m)/6.0f;
    }

    opos /= valence;
    v.P = vec4(opos, 1.0f).xyz;

    vec3 e;
    v.e0 = vec3(0);
    v.e1 = vec3(0);

    for(int i=0; i<valence; ++i) {
        int im = (i + valence -1) % valence;
        e = 0.5f * (f[i] + f[im]);
        v.e0 += cosfn(valence, i)*e;
        v.e1 += sinfn(valence, i)*e;
    }
    float ef = OsdComputeCatmarkCoefficient(valence);
    v.e0 *= ef;
    v.e1 *= ef;

#ifdef OSD_PATCH_GREGORY_BOUNDARY
    v.zerothNeighbor = zerothNeighbor;
    if (currNeighbor == 1) {
        boundaryEdgeNeighbors[1] = boundaryEdgeNeighbors[0];
    }

    if (ivalence < 0) {
        if (valence > 2) {
            v.P = (OsdReadVertex(boundaryEdgeNeighbors[0]) +
                   OsdReadVertex(boundaryEdgeNeighbors[1]) +
                   4.0f * pos)/6.0f;
        } else {
            v.P = pos;
        }

        v.e0 = (OsdReadVertex(boundaryEdgeNeighbors[0]) -
                OsdReadVertex(boundaryEdgeNeighbors[1]))/6.0;

        float k = float(float(valence) - 1.0f);    //k is the number of faces
        float c = cos(M_PI/k);
        float s = sin(M_PI/k);
        float gamma = -(4.0f*s)/(3.0f*k+c);
        float alpha_0k = -((1.0f+2.0f*c)*sqrt(1.0f+c))/((3.0f*k+c)*sqrt(1.0f-c));
        float beta_0 = s/(3.0f*k + c);

        int idx_diagonal = OsdReadVertexIndex(vID, 2*zerothNeighbor + 1);
        vec3 diagonal = OsdReadVertex(idx_diagonal);

        v.e1 = gamma * pos +
            alpha_0k * OsdReadVertex(boundaryEdgeNeighbors[0]) +
            alpha_0k * OsdReadVertex(boundaryEdgeNeighbors[1]) +
            beta_0 * diagonal;

        for (int x=1; x<valence - 1; ++x) {
            int curri = ((x + zerothNeighbor)%valence);
            float alpha = (4.0f*sin((M_PI * float(x))/k))/(3.0f*k+c);
            float beta = (sin((M_PI * float(x))/k) + sin((M_PI * float(x+1))/k))/(3.0f*k+c);

            int idx_neighbor = OsdReadVertexIndex(vID, 2*curri);
            vec3 neighbor = OsdReadVertex(idx_neighbor);

            idx_diagonal = OsdReadVertexIndex(vID, 2*curri + 1);
            diagonal = OsdReadVertex(idx_diagonal);

            v.e1 += alpha * neighbor + beta * diagonal;
        }

        v.e1 /= 3.0f;
    }
#endif
}

void
OsdComputePerPatchVertexGregory(ivec3 patchParam, int ID, int primitiveID,
                                in OsdPerVertexGregory v[4],
                                out OsdPerPatchVertexGregory result)
{
    result.patchParam = patchParam;
    result.P = v[ID].P;

    int i = ID;
    int ip = (i+1)%4;
    int im = (i+3)%4;
    int valence = abs(v[i].valence);
    int n = valence;

    int start = OsdReadQuadOffset(primitiveID, i) & 0xff;
    int prev = (OsdReadQuadOffset(primitiveID, i) >> 8) & 0xff;

    int start_m = OsdReadQuadOffset(primitiveID, im) & 0xff;
    int prev_p = (OsdReadQuadOffset(primitiveID, ip) >> 8) & 0xff;

    int np = abs(v[ip].valence);
    int nm = abs(v[im].valence);

    // Control Vertices based on :
    // "Approximating Subdivision Surfaces with Gregory Patches
    //  for Hardware Tessellation"
    // Loop, Schaefer, Ni, Castano (ACM ToG Siggraph Asia 2009)
    //
    //  P3         e3-      e2+         P2
    //     O--------O--------O--------O
    //     |        |        |        |
    //     |        |        |        |
    //     |        | f3-    | f2+    |
    //     |        O        O        |
    // e3+ O------O            O------O e2-
    //     |     f3+          f2-     |
    //     |                          |
    //     |                          |
    //     |      f0-         f1+     |
    // e0- O------O            O------O e1+
    //     |        O        O        |
    //     |        | f0+    | f1-    |
    //     |        |        |        |
    //     |        |        |        |
    //     O--------O--------O--------O
    //  P0         e0+      e1-         P1
    //

#ifdef OSD_PATCH_GREGORY_BOUNDARY
    vec3 Em_ip;
    if (v[ip].valence < -2) {
        int j = (np + prev_p - v[ip].zerothNeighbor) % np;
        Em_ip = v[ip].P + cos((M_PI*j)/float(np-1))*v[ip].e0 + sin((M_PI*j)/float(np-1))*v[ip].e1;
    } else {
        Em_ip = v[ip].P + v[ip].e0*cosfn(np, prev_p ) + v[ip].e1*sinfn(np, prev_p);
    }

    vec3 Ep_im;
    if (v[im].valence < -2) {
        int j = (nm + start_m - v[im].zerothNeighbor) % nm;
        Ep_im = v[im].P + cos((M_PI*j)/float(nm-1))*v[im].e0 + sin((M_PI*j)/float(nm-1))*v[im].e1;
    } else {
        Ep_im = v[im].P + v[im].e0*cosfn(nm, start_m) + v[im].e1*sinfn(nm, start_m);
    }

    if (v[i].valence < 0) {
        n = (n-1)*2;
    }
    if (v[im].valence < 0) {
        nm = (nm-1)*2;
    }
    if (v[ip].valence < 0) {
        np = (np-1)*2;
    }

    if (v[i].valence > 2) {
        result.Ep = v[i].P + v[i].e0*cosfn(n, start) + v[i].e1*sinfn(n, start);
        result.Em = v[i].P + v[i].e0*cosfn(n, prev ) + v[i].e1*sinfn(n, prev);

        float s1=3-2*cosfn(n,1)-cosfn(np,1);
        float s2=2*cosfn(n,1);

        result.Fp = (cosfn(np,1)*v[i].P + s1*result.Ep + s2*Em_ip + v[i].r[start])/3.0f;
        s1 = 3.0f-2.0f*cos(2.0f*M_PI/float(n))-cos(2.0f*M_PI/float(nm));
        result.Fm = (cosfn(nm,1)*v[i].P + s1*result.Em + s2*Ep_im - v[i].r[prev])/3.0f;

    } else if (v[i].valence < -2) {
        int j = (valence + start - v[i].zerothNeighbor) % valence;

        result.Ep = v[i].P + cos((M_PI*j)/float(valence-1))*v[i].e0 + sin((M_PI*j)/float(valence-1))*v[i].e1;
        j = (valence + prev - v[i].zerothNeighbor) % valence;
        result.Em = v[i].P + cos((M_PI*j)/float(valence-1))*v[i].e0 + sin((M_PI*j)/float(valence-1))*v[i].e1;

        vec3 Rp = ((-2.0f * v[i].org - 1.0f * v[im].org) + (2.0f * v[ip].org + 1.0f * v[(i+2)%4].org))/3.0f;
        vec3 Rm = ((-2.0f * v[i].org - 1.0f * v[ip].org) + (2.0f * v[im].org + 1.0f * v[(i+2)%4].org))/3.0f;

        float s1 = 3-2*cosfn(n,1)-cosfn(np,1);
        float s2 = 2*cosfn(n,1);

        result.Fp = (cosfn(np,1)*v[i].P + s1*result.Ep + s2*Em_ip + v[i].r[start])/3.0f;
        s1 = 3.0f-2.0f*cos(2.0f*M_PI/float(n))-cos(2.0f*M_PI/float(nm));
        result.Fm = (cosfn(nm,1)*v[i].P + s1*result.Em + s2*Ep_im - v[i].r[prev])/3.0f;

        if (v[im].valence < 0) {
            s1 = 3-2*cosfn(n,1)-cosfn(np,1);
            result.Fp = result.Fm = (cosfn(np,1)*v[i].P + s1*result.Ep + s2*Em_ip + v[i].r[start])/3.0f;
        } else if (v[ip].valence < 0) {
            s1 = 3.0f-2.0f*cos(2.0f*M_PI/n)-cos(2.0f*M_PI/nm);
            result.Fm = result.Fp = (cosfn(nm,1)*v[i].P + s1*result.Em + s2*Ep_im - v[i].r[prev])/3.0f;
        }

    } else if (v[i].valence == -2) {
        result.Ep = (2.0f * v[i].org + v[ip].org)/3.0f;
        result.Em = (2.0f * v[i].org + v[im].org)/3.0f;
        result.Fp = result.Fm = (4.0f * v[i].org + v[(i+2)%n].org + 2.0f * v[ip].org + 2.0f * v[im].org)/9.0f;
    }

#else // not OSD_PATCH_GREGORY_BOUNDARY

    result.Ep = v[i].P + v[i].e0 * cosfn(n, start) + v[i].e1*sinfn(n, start);
    result.Em = v[i].P + v[i].e0 * cosfn(n, prev ) + v[i].e1*sinfn(n, prev);

    vec3 Em_ip = v[ip].P + v[ip].e0 * cosfn(np, prev_p ) + v[ip].e1*sinfn(np, prev_p);
    vec3 Ep_im = v[im].P + v[im].e0 * cosfn(nm, start_m) + v[im].e1*sinfn(nm, start_m);

    float s1 = 3-2*cosfn(n,1)-cosfn(np,1);
    float s2 = 2*cosfn(n,1);

    result.Fp = (cosfn(np,1)*v[i].P + s1*result.Ep + s2*Em_ip + v[i].r[start])/3.0f;
    s1 = 3.0f-2.0f*cos(2.0f*M_PI/float(n))-cos(2.0f*M_PI/float(nm));
    result.Fm = (cosfn(nm,1)*v[i].P + s1*result.Em + s2*Ep_im - v[i].r[prev])/3.0f;
#endif
}

#endif  // OSD_PATCH_GREGORY || OSD_PATCH_GREGORY_BOUNDARY

// ----------------------------------------------------------------------------
// Legacy Face-varying
// ----------------------------------------------------------------------------
uniform samplerBuffer OsdFVarDataBuffer;

#ifndef OSD_FVAR_WIDTH
#define OSD_FVAR_WIDTH 0
#endif

// ------ extract from quads (catmark, bilinear) ---------
// XXX: only linear interpolation is supported

#define OSD_COMPUTE_FACE_VARYING_1(result, fvarOffset, tessCoord)       \
    {                                                                   \
        float v[4];                                                     \
        int primOffset = OsdGetPatchIndex(gl_PrimitiveID) * 4;          \
        for (int i = 0; i < 4; ++i) {                                   \
            int index = (primOffset+i)*OSD_FVAR_WIDTH + fvarOffset;     \
            v[i] = texelFetch(OsdFVarDataBuffer, index).s               \
        }                                                               \
        result = mix(mix(v[0], v[1], tessCoord.s),                      \
                     mix(v[3], v[2], tessCoord.s),                      \
                     tessCoord.t);                                      \
    }

#define OSD_COMPUTE_FACE_VARYING_2(result, fvarOffset, tessCoord)       \
    {                                                                   \
        vec2 v[4];                                                      \
        int primOffset = OsdGetPatchIndex(gl_PrimitiveID) * 4;          \
        for (int i = 0; i < 4; ++i) {                                   \
            int index = (primOffset+i)*OSD_FVAR_WIDTH + fvarOffset;     \
            v[i] = vec2(texelFetch(OsdFVarDataBuffer, index).s,         \
                        texelFetch(OsdFVarDataBuffer, index + 1).s);    \
        }                                                               \
        result = mix(mix(v[0], v[1], tessCoord.s),                      \
                     mix(v[3], v[2], tessCoord.s),                      \
                     tessCoord.t);                                      \
    }

#define OSD_COMPUTE_FACE_VARYING_3(result, fvarOffset, tessCoord)       \
    {                                                                   \
        vec3 v[4];                                                      \
        int primOffset = OsdGetPatchIndex(gl_PrimitiveID) * 4;          \
        for (int i = 0; i < 4; ++i) {                                   \
            int index = (primOffset+i)*OSD_FVAR_WIDTH + fvarOffset;     \
            v[i] = vec3(texelFetch(OsdFVarDataBuffer, index).s,         \
                        texelFetch(OsdFVarDataBuffer, index + 1).s,     \
                        texelFetch(OsdFVarDataBuffer, index + 2).s);    \
        }                                                               \
        result = mix(mix(v[0], v[1], tessCoord.s),                      \
                     mix(v[3], v[2], tessCoord.s),                      \
                     tessCoord.t);                                      \
    }

#define OSD_COMPUTE_FACE_VARYING_4(result, fvarOffset, tessCoord)       \
    {                                                                   \
        vec4 v[4];                                                      \
        int primOffset = OsdGetPatchIndex(gl_PrimitiveID) * 4;          \
        for (int i = 0; i < 4; ++i) {                                   \
            int index = (primOffset+i)*OSD_FVAR_WIDTH + fvarOffset;     \
            v[i] = vec4(texelFetch(OsdFVarDataBuffer, index).s,         \
                        texelFetch(OsdFVarDataBuffer, index + 1).s,     \
                        texelFetch(OsdFVarDataBuffer, index + 2).s,     \
                        texelFetch(OsdFVarDataBuffer, index + 3).s);    \
        }                                                               \
        result = mix(mix(v[0], v[1], tessCoord.s),                      \
                     mix(v[3], v[2], tessCoord.s),                      \
                     tessCoord.t);                                      \
    }

// ------ extract from triangles barycentric (loop) ---------
// XXX: no interpolation supported

#define OSD_COMPUTE_FACE_VARYING_TRI_1(result, fvarOffset, triVert)     \
    {                                                                   \
        int primOffset = OsdGetPatchIndex(gl_PrimitiveID) * 3;          \
        int index = (primOffset+triVert)*OSD_FVAR_WIDTH + fvarOffset;   \
        result = texelFetch(OsdFVarDataBuffer, index).s;                \
    }

#define OSD_COMPUTE_FACE_VARYING_TRI_2(result, fvarOffset, triVert)     \
    {                                                                   \
        int primOffset = OsdGetPatchIndex(gl_PrimitiveID) * 3;          \
        int index = (primOffset+triVert)*OSD_FVAR_WIDTH + fvarOffset;   \
        result = vec2(texelFetch(OsdFVarDataBuffer, index).s,           \
                      texelFetch(OsdFVarDataBuffer, index + 1).s);      \
    }

#define OSD_COMPUTE_FACE_VARYING_TRI_3(result, fvarOffset, triVert)     \
    {                                                                   \
        int primOffset = OsdGetPatchIndex(gl_PrimitiveID) * 3;          \
        int index = (primOffset+triVert)*OSD_FVAR_WIDTH + fvarOffset;   \
        result = vec3(texelFetch(OsdFVarDataBuffer, index).s,           \
                      texelFetch(OsdFVarDataBuffer, index + 1).s,       \
                      texelFetch(OsdFVarDataBuffer, index + 2).s);      \
    }

#define OSD_COMPUTE_FACE_VARYING_TRI_4(result, fvarOffset, triVert)     \
    {                                                                   \
        int primOffset = OsdGetPatchIndex(gl_PrimitiveID) * 3;          \
        int index = (primOffset+triVert)*OSD_FVAR_WIDTH + fvarOffset;   \
        result = vec4(texelFetch(OsdFVarDataBuffer, index).s,           \
                      texelFetch(OsdFVarDataBuffer, index + 1).s,       \
                      texelFetch(OsdFVarDataBuffer, index + 2).s,       \
                      texelFetch(OsdFVarDataBuffer, index + 3).s);      \
    }

