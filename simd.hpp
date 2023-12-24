/*
 * Copyright (C) 2023  Alister Sanders
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#ifdef _FIXP_INTERNAL
#    define _FIXP_SIMD_NONE -1
#    define _FIXP_SIMD_AVX  0
#    define _FIXP_SIMD_SSE4 1
#    define _FIXP_SIMD_NEON 2

#    if defined(__ARM_NEON)
#        define _FIXP_SIMD _FIXP_SIMD_NEON
#    elif defined(__AVX2__)
#        define _FIXP_SIMD _FIXP_SIMD_AVX
#    else
#        define _FIXP_SIMD _FIXP_SIMD_NONE
#        warning "Vectorization is not supported on this platform!"
#    endif
#else
#    error "This file should not be included directly!"
#endif

#if _FIXP_SIMD == _FIXP_SIMD_NEON
#include <arm_neon.h>
#endif

namespace fixp::internals::simd {
    #if _FIXP_SIMD == _FIXP_SIMD_NEON
    #endif
}

#undef _FIXP_SIMD
#undef _FIXP_SIMD_NONE
#undef _FIXP_SIMD_AVX
#undef _FIXP_SIMD_SSE4
#undef _FIXP_SIMD_NEON
