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

// #if defined(_FIXP_SIMD) && _FIXP_SIMD == _FIXP_SIMD_NEON
#include <cstddef>
#include <cstdint>
#include <arm_neon.h>
#include <concepts>
#include <iostream>
#include <functional>
#include <hints.hpp>
#include <optional>
#include <type_traits>

namespace fixp::internals::simd_neon {
    template<std::signed_integral T>
    struct neon_vector {
            using type = void;
            static constexpr std::size_t lanes = 0;
    };

#define SPECIALIZE(SCALAR, VECTOR, LANES)                    \
    template<>                                                          \
    struct neon_vector<SCALAR> {                                        \
        using type = VECTOR;                                            \
        static constexpr std::size_t lanes = LANES;                     \
    }

    SPECIALIZE(std::int8_t,  int8x16_t, 16);
    SPECIALIZE(std::int16_t, int16x8_t, 8);
    SPECIALIZE(std::int32_t, int32x4_t, 4);
    SPECIALIZE(std::int64_t, int64x2_t, 2);
#undef SPECIALIZE
    
    template<std::signed_integral T>
    using neon_vector_type = neon_vector<T>::type;

    template<std::signed_integral T>
    static constexpr std::size_t neon_lanes = neon_vector<T>::lanes;

    namespace neon_op {
#define SPECIALIZE(NAME, SCALAR, VECTOR, OP)     \
        template<> \
        FIXP_ALWAYS_INLINE inline VECTOR NAME(const SCALAR* p) { return OP(p); }
        
        template<std::signed_integral T, typename V>
        static inline V load(const T*);

        SPECIALIZE(load, std::int8_t,  int8x16_t, vld1q_s8);
        SPECIALIZE(load, std::int16_t, int16x8_t, vld1q_s16);
        SPECIALIZE(load, std::int32_t, int32x4_t, vld1q_s32);
#undef SPECIALIZE

#define SPECIALIZE(NAME, SCALAR, VECTOR, OP)    \
        template<> \
        FIXP_ALWAYS_INLINE inline void NAME(SCALAR* p, VECTOR v) { OP(p, v); }

        template<std::signed_integral T, typename V>
        static inline void store(T*, V);

        SPECIALIZE(store, std::int8_t,  int8x16_t, vst1q_s8);
        SPECIALIZE(store, std::int16_t, int16x8_t, vst1q_s16);
        SPECIALIZE(store, std::int32_t, int32x4_t, vst1q_s32);
#undef SPECIALIZE

#define SPECIALIZE(NAME, VECTOR, OP) \
        template<> \
        FIXP_ALWAYS_INLINE inline VECTOR NAME(VECTOR a, VECTOR b) { return OP(a, b); }

        template<typename V>
        static inline V add(V, V);

        template<typename V>
        static inline V sub(V, V);

        template<typename V>
        static inline V mul(V, V);

        SPECIALIZE(add, int8x16_t, vaddq_s8);
        SPECIALIZE(add, int16x8_t, vaddq_s16);
        SPECIALIZE(add, int32x4_t, vaddq_s32);

        SPECIALIZE(sub, int8x16_t, vsubq_s8);
        SPECIALIZE(sub, int16x8_t, vsubq_s16);
        SPECIALIZE(sub, int32x4_t, vsubq_s32);

        SPECIALIZE(mul, int8x16_t, vmulq_s8);
        SPECIALIZE(mul, int16x8_t, vmulq_s16);
        SPECIALIZE(mul, int32x4_t, vmulq_s32);
#undef SPECIALIZE
    }

    template<std::signed_integral T, const std::size_t IterSize=4>
    FIXP_ALWAYS_INLINE static void
    add(const T* a, const T* b, T* result, std::size_t dim)
    {
        constexpr std::size_t vlanes = neon_lanes<T>;
        constexpr std::size_t vchunk = vlanes * IterSize;

        for (std::size_t i = 0; i < dim / vchunk; i++) {
            neon_vector_type<T> va[IterSize];
            neon_vector_type<T> vb[IterSize];
            neon_vector_type<T> vresult[IterSize];

            #pragma clang loop unroll(full)
            for (std::size_t j = 0; j < IterSize; j++) {
                va[j] = neon_op::load<T, neon_vector_type<T>>(&a[i * vchunk + j * vlanes]);
                vb[j] = neon_op::load<T, neon_vector_type<T>>(&b[i * vchunk + j * vlanes]);
            }

            #pragma clang loop unroll(full)
            for (std::size_t j = 0; j < IterSize; j++) {
                vresult[j] = neon_op::add(va[j], vb[j]);
            }

            #pragma clang loop unroll(full)
            for (std::size_t j = 0; j < IterSize; j++) {
                neon_op::store<T, neon_vector_type<T>>(&result[i * vchunk + j * vlanes], vresult[j]);
            }
        }

        const std::size_t offset = dim / vchunk;
        for (std::size_t i = 0; i < dim % vchunk; i++) {
            result[i + offset] = a[i + offset] + b[i + offset];
        }
    }

    template<std::signed_integral T, const std::size_t IterSize=4>
    FIXP_ALWAYS_INLINE static void
    sub(const T* a, const T* b, T* result, std::size_t dim)
    {
        constexpr std::size_t vlanes = neon_lanes<T>;
        constexpr std::size_t vchunk = vlanes * IterSize;

        for (std::size_t i = 0; i < dim / vchunk; i++) {
            neon_vector_type<T> va[IterSize];
            neon_vector_type<T> vb[IterSize];
            neon_vector_type<T> vresult[IterSize];

            #pragma clang loop unroll(full)
            for (std::size_t j = 0; j < IterSize; j++) {
                va[j] = neon_op::load<T, neon_vector_type<T>>(&a[i * vchunk + j * vlanes]);
                vb[j] = neon_op::load<T, neon_vector_type<T>>(&b[i * vchunk + j * vlanes]);
            }

            #pragma clang loop unroll(full)
            for (std::size_t j = 0; j < IterSize; j++) {
                vresult[j] = neon_op::sub(va[j], vb[j]);
            }

            #pragma clang loop unroll(full)
            for (std::size_t j = 0; j < IterSize; j++) {
                neon_op::store<T, neon_vector_type<T>>(&result[i * vchunk + j * vlanes], vresult[j]);
            }
        }

        const std::size_t offset = dim / vchunk;
        for (std::size_t i = 0; i < dim % vchunk; i++) {
            result[i + offset] = a[i + offset] - b[i + offset];
        }
    }

    template<std::signed_integral T, const std::size_t IterSize=4>
    FIXP_ALWAYS_INLINE static void
    mul(const T* a, const T* b, T* result, std::size_t dim)
    {
        constexpr std::size_t vlanes = neon_lanes<T>;
        constexpr std::size_t vchunk = vlanes * IterSize;

        for (std::size_t i = 0; i < dim / vchunk; i++) {
            neon_vector_type<T> va[IterSize];
            neon_vector_type<T> vb[IterSize];
            neon_vector_type<T> vresult[IterSize];

            #pragma clang loop unroll(full)
            for (std::size_t j = 0; j < IterSize; j++) {
                va[j] = neon_op::load<T, neon_vector_type<T>>(&a[i * vchunk + j * vlanes]);
                vb[j] = neon_op::load<T, neon_vector_type<T>>(&b[i * vchunk + j * vlanes]);
            }

            #pragma clang loop unroll(full)
            for (std::size_t j = 0; j < IterSize; j++) {
                vresult[j] = neon_op::mul(va[j], vb[j]);
            }

            #pragma clang loop unroll(full)
            for (std::size_t j = 0; j < IterSize; j++) {
                neon_op::store<T, neon_vector_type<T>>(&result[i * vchunk + j * vlanes], vresult[j]);
            }
        }

        const std::size_t offset = dim / vchunk;
        for (std::size_t i = 0; i < dim % vchunk; i++) {
            result[i + offset] = a[i + offset] * b[i + offset];
        }
    }

    template<std::signed_integral T, const T Bits, const std::size_t IterSize=4>
    FIXP_ALWAYS_INLINE static void
    shl_immediate(const T* a, T* result, std::size_t dim)
        requires (Bits >= 0 && sizeof(T) < 8)
    {
        constexpr std::size_t vlanes = neon_lanes<T>;
        constexpr std::size_t vchunk = vlanes * IterSize;

        for (std::size_t i = 0; i < dim / vchunk; i++) {
            neon_vector_type<T> va[IterSize];
            neon_vector_type<T> vresult[IterSize];

            #pragma clang loop unroll(full)
            for (std::size_t j = 0; j < IterSize; j++) {
                va[j] = neon_op::load<T, neon_vector_type<T>>(&a[i * vchunk + j * vlanes]);
            }

            #pragma clang loop unroll(full)
            for (std::size_t j = 0; j < IterSize; j++) {
                if constexpr (typeid(T) == typeid(std::int8_t)) {
                    vresult[j] = vshlq_n_s8(va[j], Bits);
                } else if (typeid(T) == typeid(std::int16_t)) {
                    vresult[j] = vshlq_n_s16(va[j], Bits);
                } else if (typeid(T) == typeid(std::int32_t)) {
                    vresult[j] = vshlq_n_s32(va[j], Bits);
                }
            }

            #pragma clang loop unroll(full)
            for (std::size_t j = 0; j < IterSize; j++) {
                neon_op::store<T, neon_vector_type<T>>(&result[i * vchunk + j * vlanes], vresult[j]);
            }
        }

        const std::size_t offset = dim / vchunk;
        for (std::size_t i = 0; i < dim % vchunk; i++) {
            result[i + offset] = a[i + offset] << Bits;
        }
    }

    template<std::signed_integral T, const T Bits, const std::size_t IterSize=4>
    FIXP_ALWAYS_INLINE static void
    shr_immediate(const T* a, T* result, std::size_t dim)
        requires (Bits >= 0 && sizeof(T) < 8)
    {
        constexpr std::size_t vlanes = neon_lanes<T>;
        constexpr std::size_t vchunk = vlanes * IterSize;

        for (std::size_t i = 0; i < dim / vchunk; i++) {
            neon_vector_type<T> va[IterSize];
            neon_vector_type<T> vresult[IterSize];

            #pragma clang loop unroll(full)
            for (std::size_t j = 0; j < IterSize; j++) {
                va[j] = neon_op::load<T, neon_vector_type<T>>(&a[i * vchunk + j * vlanes]);
            }

            #pragma clang loop unroll(full)
            for (std::size_t j = 0; j < IterSize; j++) {
                if constexpr (typeid(T) == typeid(std::int8_t)) {
                    vresult[j] = vshrq_n_s8(va[j], Bits);
                } else if (typeid(T) == typeid(std::int16_t)) {
                    vresult[j] = vshrq_n_s16(va[j], Bits);
                } else if (typeid(T) == typeid(std::int32_t)) {
                    vresult[j] = vshrq_n_s32(va[j], Bits);
                }
            }

            #pragma clang loop unroll(full)
            for (std::size_t j = 0; j < IterSize; j++) {
                neon_op::store<T, neon_vector_type<T>>(&result[i * vchunk + j * vlanes], vresult[j]);
            }
        }

        const std::size_t offset = dim / vchunk;
        for (std::size_t i = 0; i < dim % vchunk; i++) {
            result[i + offset] = a[i + offset] << Bits;
        }
    }
}
// #endif
