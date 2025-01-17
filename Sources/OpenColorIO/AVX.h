// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenColorIO Project.


#ifndef INCLUDED_OCIO_AVX_H
#define INCLUDED_OCIO_AVX_H

#include "CPUInfo.h"
#if OCIO_USE_AVX && defined(__AVX__)

#include <immintrin.h>
#include <stdio.h>

#include <OpenColorIO/OpenColorIO.h>
#include "BitDepthUtils.h"

// Macros for alignment declarations
#define AVX_SIMD_BYTES 32
#define AVX_ALIGN(decl) alignas(AVX_SIMD_BYTES) decl

namespace OCIO_NAMESPACE
{

inline OCIO_TARGET_ATTRIBUTE("avx")
__m256 avx_movelh_ps(__m256 a, __m256 b)
{
    return _mm256_castpd_ps(_mm256_unpacklo_pd(_mm256_castps_pd(a), _mm256_castps_pd(b)));
}

inline OCIO_TARGET_ATTRIBUTE("avx")
__m256 avx_movehl_ps(__m256 a, __m256 b)
{
    // NOTE: this is a and b are reversed to match sse2 movhlps which is different than unpckhpd
    return _mm256_castpd_ps(_mm256_unpackhi_pd(_mm256_castps_pd(b), _mm256_castps_pd(a)));
}

inline OCIO_TARGET_ATTRIBUTE("avx")
__m256 avx_clamp(__m256 value, const __m256& maxValue)
{
    value = _mm256_max_ps(value, _mm256_setzero_ps());
    return _mm256_min_ps(value, maxValue);
}

inline OCIO_TARGET_ATTRIBUTE("avx")
void avxRGBATranspose_4x4_4x4(__m256 row0, __m256 row1, __m256 row2, __m256 row3,
                              __m256 &out_r, __m256 &out_g, __m256 &out_b, __m256 &out_a )
{
    // the rgba transpose result will look this
    //
    //  0   1   2   3    0   1   2   3         0   1   2   3    0   1   2   3
    // r0, g0, b0, a0 | r1, g1, b1, a1        r0, r2, r4, r6 | r1, r3, r5, r7
    // r2, g2, b2, a2 | r3, g3, b3, a3  <==>  g0, g2, g4, g6 | g1, g3, g5, g7
    // r4, g4, b4, a4 | r5, g5, b5, a5  <==>  b0, b2, b4, b6 | b1, b3, b5, b7
    // r6, g6, b6, a6 | r7, g7, b7, a7        a0, a2, a4, a6 | a1, a4, a5, a7

    // each 128 lane is transposed independently,
    // the channel values end up with a even/odd shuffled order because of this.
    // if exact order is important more cross lane shuffling is needed

    __m256 tmp0 = _mm256_unpacklo_ps(row0, row1);
    __m256 tmp2 = _mm256_unpacklo_ps(row2, row3);
    __m256 tmp1 = _mm256_unpackhi_ps(row0, row1);
    __m256 tmp3 = _mm256_unpackhi_ps(row2, row3);

    out_r = avx_movelh_ps(tmp0, tmp2);
    out_g = avx_movehl_ps(tmp2, tmp0);
    out_b = avx_movelh_ps(tmp1, tmp3);
    out_a = avx_movehl_ps(tmp3, tmp1);

}

inline OCIO_TARGET_ATTRIBUTE("avx")
__m256i avx_load_u8(__m128i a)
{
    __m128i b = _mm_shuffle_epi32(a, _MM_SHUFFLE(1,0,3,1));
    b = _mm_cvtepu8_epi32(b);
    a = _mm_cvtepu8_epi32(a);

    return _mm256_insertf128_si256(_mm256_castsi128_si256(a), b, 1);
}

inline OCIO_TARGET_ATTRIBUTE("avx")
__m128i avx_pack_u8(__m256i a, __m256i b)
{
    const __m128i lo0_shuf =  _mm_setr_epi8(  0, 4, 8,12, -1,-1,-1, -1,  -1,-1,-1,-1,   -1,-1,-1,-1);
    const __m128i lo1_shuf =  _mm_setr_epi8( -1,-1,-1,-1,  0, 4, 8, 12,  -1,-1,-1,-1,   -1,-1,-1,-1);
    const __m128i hi0_shuf =  _mm_setr_epi8( -1,-1,-1,-1, -1,-1,-1, -1,   0, 4, 8, 12,  -1,-1,-1,-1);
    const __m128i hi1_shuf =  _mm_setr_epi8( -1,-1,-1,-1, -1,-1,-1, -1,  -1,-1,-1,-1,   0, 4, 8, 12);

    __m128i lo0 = _mm_shuffle_epi8(_mm256_castsi256_si128(a), lo0_shuf);
    __m128i lo1 = _mm_shuffle_epi8(_mm256_extractf128_si256(a, 1), lo1_shuf);

    __m128i hi0 = _mm_shuffle_epi8(_mm256_castsi256_si128(b), hi0_shuf);
    __m128i hi1 = _mm_shuffle_epi8(_mm256_extractf128_si256(b, 1), hi1_shuf);

    return _mm_or_si128(_mm_or_si128(_mm_or_si128(lo0, lo1), hi0), hi1);
}

// Note Packing functions perform no 0.0 - 1.0 normalization
// but perform 0 - max value clamping for integer formats
template<BitDepth BD> struct AVXRGBAPack {};

template <>
struct AVXRGBAPack<BIT_DEPTH_UINT8>
{
    static inline OCIO_TARGET_ATTRIBUTE("avx")
    void Load(const uint8_t *in, __m256& r, __m256& g, __m256& b, __m256& a)
    {
        __m256i rgba_00_07 = _mm256_loadu_si256((const __m256i*)in);

        __m128i rgba_00_03 =_mm256_castsi256_si128(rgba_00_07);
        __m128i rgba_04_07 =_mm256_extractf128_si256(rgba_00_07, 1);

        //          :  0,  1,  2,  3 |  4,  5,  6,  7 |  8,  9, 10, 11 | 12, 13, 14, 15
        // rgba_x03 : r0, g0, b0, a0 | r1, g1, b1, a1 | r2, g2, b2, a2 | r3, g3, b3, a3
        // rgba_x47 : r4, g4, b4, a4 | r5, g5, b5, a5 | r6, g6, b6, a6 | r7, g7, b7, a7

        __m256 rgba0 = _mm256_cvtepi32_ps(avx_load_u8(rgba_00_03));
        __m256 rgba1 = _mm256_cvtepi32_ps(avx_load_u8(_mm_shuffle_epi32(rgba_00_03, _MM_SHUFFLE(3, 2, 3, 2))));

        __m256 rgba2 = _mm256_cvtepi32_ps(avx_load_u8(rgba_04_07));
        __m256 rgba3 = _mm256_cvtepi32_ps(avx_load_u8(_mm_shuffle_epi32(rgba_04_07, _MM_SHUFFLE(3, 2, 3, 2))));

        avxRGBATranspose_4x4_4x4(rgba0, rgba1, rgba2, rgba3, r, g, b, a);
    }

    static inline OCIO_TARGET_ATTRIBUTE("avx")
    void Store(uint8_t *out, __m256 r, __m256 g, __m256 b, __m256 a)
    {
        __m256 rgba0, rgba1, rgba2, rgba3;
        const __m256 maxValue = _mm256_set1_ps(255.0f);

        avxRGBATranspose_4x4_4x4(r, g, b, a, rgba0, rgba1, rgba2, rgba3);

        rgba0 = avx_clamp(rgba0, maxValue);
        rgba1 = avx_clamp(rgba1, maxValue);
        rgba2 = avx_clamp(rgba2, maxValue);
        rgba3 = avx_clamp(rgba3, maxValue);

        // NOTE note using cvtps which will round based on MXCSR register defaults to _MM_ROUND_NEAREST
        __m256i rgba01 = _mm256_cvtps_epi32(rgba0);
        __m256i rgba23 = _mm256_cvtps_epi32(rgba1);
        __m256i rgba45 = _mm256_cvtps_epi32(rgba2);
        __m256i rgba67 = _mm256_cvtps_epi32(rgba3);

        __m128i lo = avx_pack_u8(rgba01, rgba23);
        __m128i hi = avx_pack_u8(rgba45, rgba67);

        __m256i rgba = _mm256_insertf128_si256(_mm256_castsi128_si256(lo), hi, 1);

        _mm256_storeu_si256((__m256i*)out, rgba);
    }
};

inline OCIO_TARGET_ATTRIBUTE("avx")
__m256i avx_unpack_u16(__m128i a)
{
    __m128i b = _mm_shuffle_epi32(a, _MM_SHUFFLE(1,0,3,2));
    b = _mm_cvtepu16_epi32(b);
    a = _mm_cvtepu16_epi32(a);

    return _mm256_insertf128_si256(_mm256_castsi128_si256(a), b, 1);
}

inline OCIO_TARGET_ATTRIBUTE("avx")
__m128i avx_pack_u16(__m256i a)
{
    const __m128i lo_pack_shuffle16 = _mm_setr_epi8( 0,  1,  4,  5,
                                                     8,  9, 12, 13,
                                                    -1, -1, -1, -1,
                                                    -1, -1, -1, -1);

    const __m128i hi_pack_shuffle16 = _mm_setr_epi8(-1, -1, -1, -1,
                                                    -1, -1, -1, -1,
                                                     0,  1,  4,  5,
                                                     8,  9, 12, 13);

    __m128i hi = _mm256_extractf128_si256(a, 1);
    __m128i lo = _mm256_castsi256_si128(a);

    lo = _mm_shuffle_epi8(lo, lo_pack_shuffle16);
    hi = _mm_shuffle_epi8(hi, hi_pack_shuffle16);

    return _mm_or_si128(lo, hi);
}

template<BitDepth BD>
struct AVXRGBAPack16
{
    typedef typename BitDepthInfo<BD>::Type Type;

    static inline OCIO_TARGET_ATTRIBUTE("avx")
    void Load(const Type *in, __m256& r, __m256& g, __m256& b, __m256& a)
    {
        // const __m256 scale = _mm256_set1_ps(1.0f / (float)BitDepthInfo<BD>::maxValue);
        __m256i rgba_00_03 = _mm256_loadu_si256((const __m256i*)(in +  0));
        __m256i rgba_04_07 = _mm256_loadu_si256((const __m256i*)(in + 16));

        __m256 rgba0 = _mm256_cvtepi32_ps(avx_unpack_u16(_mm256_castsi256_si128(rgba_00_03)));
        __m256 rgba1 = _mm256_cvtepi32_ps(avx_unpack_u16(_mm256_extractf128_si256(rgba_00_03, 1)));
        __m256 rgba2 = _mm256_cvtepi32_ps(avx_unpack_u16(_mm256_castsi256_si128(rgba_04_07)));
        __m256 rgba3 = _mm256_cvtepi32_ps(avx_unpack_u16(_mm256_extractf128_si256(rgba_04_07, 1)));

        avxRGBATranspose_4x4_4x4(rgba0, rgba1, rgba2, rgba3, r, g, b, a);
    }

    static inline OCIO_TARGET_ATTRIBUTE("avx")
    void Store(Type *out, __m256 r, __m256 g, __m256 b, __m256 a)
    {
        __m256 rgba0, rgba1, rgba2, rgba3;
        __m128i lo, hi;
        __m256i rgba;
        const __m256 maxValue = _mm256_set1_ps((float)BitDepthInfo<BD>::maxValue);

        avxRGBATranspose_4x4_4x4(r, g, b, a, rgba0, rgba1, rgba2, rgba3);

        rgba0 = avx_clamp(rgba0, maxValue);
        rgba1 = avx_clamp(rgba1, maxValue);
        rgba2 = avx_clamp(rgba2, maxValue);
        rgba3 = avx_clamp(rgba3, maxValue);

        // NOTE note using cvtps which will round based on MXCSR register defaults to _MM_ROUND_NEAREST
        __m256i rgba01 = _mm256_cvtps_epi32(rgba0);
        __m256i rgba23 = _mm256_cvtps_epi32(rgba1);
        __m256i rgba45 = _mm256_cvtps_epi32(rgba2);
        __m256i rgba67 = _mm256_cvtps_epi32(rgba3);

        lo = avx_pack_u16(rgba01);
        hi = avx_pack_u16(rgba23);

        rgba = _mm256_insertf128_si256(_mm256_castsi128_si256(lo), hi, 1);
        _mm256_storeu_si256((__m256i*)(out+0), rgba);

        lo = avx_pack_u16(rgba45);
        hi = avx_pack_u16(rgba67);

        rgba = _mm256_insertf128_si256(_mm256_castsi128_si256(lo), hi, 1);
        _mm256_storeu_si256((__m256i*)(out+16), rgba);
    }
};

template <>
struct AVXRGBAPack<BIT_DEPTH_UINT10>
{
    static inline OCIO_TARGET_ATTRIBUTE("avx")
    void Load(const uint16_t *in, __m256& r, __m256& g, __m256& b, __m256& a)
    {
        AVXRGBAPack16<BIT_DEPTH_UINT10>::Load(in, r, g, b, a);
    }
    static inline OCIO_TARGET_ATTRIBUTE("avx")
    void Store(uint16_t *out, __m256 r, __m256 g, __m256 b, __m256 a)
    {
        AVXRGBAPack16<BIT_DEPTH_UINT10>::Store(out, r, g, b, a);
    }
};

template <>
struct AVXRGBAPack<BIT_DEPTH_UINT12>
{
    static inline OCIO_TARGET_ATTRIBUTE("avx")
    void Load(const uint16_t *in, __m256& r, __m256& g, __m256& b, __m256& a)
    {
        AVXRGBAPack16<BIT_DEPTH_UINT12>::Load(in, r, g, b, a);
    }
    static inline OCIO_TARGET_ATTRIBUTE("avx")
    void Store(uint16_t *out, __m256 r, __m256 g, __m256 b, __m256 a)
    {
        AVXRGBAPack16<BIT_DEPTH_UINT12>::Store(out, r, g, b, a);
    }
};

template <>
struct AVXRGBAPack<BIT_DEPTH_UINT16>
{
    static inline OCIO_TARGET_ATTRIBUTE("avx")
    void Load(const uint16_t *in, __m256& r, __m256& g, __m256& b, __m256& a)
    {
        AVXRGBAPack16<BIT_DEPTH_UINT16>::Load(in, r, g, b, a);
    }
    static inline OCIO_TARGET_ATTRIBUTE("avx")
    void Store(uint16_t *out, __m256 r, __m256 g, __m256 b, __m256 a)
    {
        AVXRGBAPack16<BIT_DEPTH_UINT16>::Store(out, r, g, b, a);
    }
};

#if OCIO_USE_F16C

template <>
struct AVXRGBAPack<BIT_DEPTH_F16>
{
    static inline OCIO_TARGET_ATTRIBUTE("avx")
    void Load(const half *in, __m256& r, __m256& g, __m256& b, __m256& a)
    {

        __m256i rgba_00_03 = _mm256_loadu_si256((const __m256i*)(in +  0));
        __m256i rgba_04_07 = _mm256_loadu_si256((const __m256i*)(in + 16));

        __m256 rgba0 = _mm256_cvtph_ps(_mm256_castsi256_si128(rgba_00_03));
        __m256 rgba1 = _mm256_cvtph_ps(_mm256_extractf128_si256(rgba_00_03, 1));
        __m256 rgba2 = _mm256_cvtph_ps(_mm256_castsi256_si128(rgba_04_07));
        __m256 rgba3 = _mm256_cvtph_ps(_mm256_extractf128_si256(rgba_04_07, 1));

        avxRGBATranspose_4x4_4x4(rgba0, rgba1, rgba2, rgba3, r, g, b, a);
    }

    static inline OCIO_TARGET_ATTRIBUTE("avx,f16c")
    void Store(half *out, __m256 r, __m256 g, __m256 b, __m256 a)
    {
        __m256 rgba0, rgba1, rgba2, rgba3;
        __m256i rgba;

        avxRGBATranspose_4x4_4x4(r, g, b, a, rgba0, rgba1, rgba2, rgba3);

        __m128i rgba00_03 = _mm256_cvtps_ph(rgba0, 0);
        __m128i rgba04_07 = _mm256_cvtps_ph(rgba1, 0);
        __m128i rgba08_11 = _mm256_cvtps_ph(rgba2, 0);
        __m128i rgba12_16 = _mm256_cvtps_ph(rgba3, 0);

        rgba = _mm256_insertf128_si256(_mm256_castsi128_si256(rgba00_03), rgba04_07, 1);
        _mm256_storeu_si256((__m256i*)(out+0), rgba);

        rgba = _mm256_insertf128_si256(_mm256_castsi128_si256(rgba08_11), rgba12_16, 1);
        _mm256_storeu_si256((__m256i*)(out+16), rgba);
    }
};

#endif

template <>
struct AVXRGBAPack<BIT_DEPTH_F32>
{
    static inline OCIO_TARGET_ATTRIBUTE("avx")
    void Load(const float *in, __m256& r, __m256& g, __m256& b, __m256& a)
    {
        __m256 rgba0 = _mm256_loadu_ps(in +  0);
        __m256 rgba1 = _mm256_loadu_ps(in +  8);
        __m256 rgba2 = _mm256_loadu_ps(in + 16);
        __m256 rgba3 = _mm256_loadu_ps(in + 24);

        avxRGBATranspose_4x4_4x4(rgba0, rgba1, rgba2, rgba3, r, g, b, a);

    }

    static inline OCIO_TARGET_ATTRIBUTE("avx")
    void Store(float *out, __m256 r, __m256 g, __m256 b, __m256 a)
    {
        __m256 rgba0, rgba1, rgba2, rgba3;
        avxRGBATranspose_4x4_4x4(r, g, b, a, rgba0, rgba1, rgba2, rgba3);

        _mm256_storeu_ps(out +  0, rgba0);
        _mm256_storeu_ps(out +  8, rgba1);
        _mm256_storeu_ps(out + 16, rgba2);
        _mm256_storeu_ps(out + 24, rgba3);
    }
};

} // namespace OCIO_NAMESPACE

#endif // OCIO_USE_AVX
#endif // INCLUDED_OCIO_AVX_H