

/*
 * Author: Tomi Jylhä-Ollila, Finland 2017-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_INTRINSICS_H
#define KQT_INTRINSICS_H


#ifdef __SSE__
#include <xmmintrin.h>
#define KQT_SSE 1
#else
#define KQT_SSE 0
#endif

#ifdef __SSE2__
#include <emmintrin.h>
#define KQT_SSE2 1
#else
#define KQT_SSE2 0
#endif

#ifdef __SSE3__
#include <pmmintrin.h>
#define KQT_SSE3 1
#else
#define KQT_SSE3 0
#endif

#ifdef __SSSE3__
#include <tmmintrin.h>
#define KQT_SSSE3 1
#else
#define KQT_SSSE3 0
#endif

#ifdef __SSE4_1__
#include <smmintrin.h>
#define KQT_SSE4_1 1
#else
#define KQT_SSE4_1 0
#endif

#ifdef __SSE4_2__
#include <nmmintrin.h>
#define KQT_SSE4_2 1
#else
#define KQT_SSE4_2 0
#endif

#ifdef __AVX__
#include <immintrin.h>
#define KQT_AVX 1

#ifdef __AVX2__
#define KQT_AVX2 1
#else
#define KQT_AVX2 0
#endif

#else
#define KQT_AVX 0
#endif // __AVX__


#endif // KQT_INTRINSICS_H


