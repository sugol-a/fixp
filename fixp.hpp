/*
 * Copyright (C) 2023  Alister Sanders
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef _FIXP_H
#define _FIXP_H

#include <concepts>
#include <functional>
#include <type_traits>

namespace fixp {
    template <typename T>
    concept integral = std::is_integral<T>::value;

    enum class fixed_op {
        ADD,
        SUB,
        MUL,
        DIV
    };

    template<integral T, const std::size_t Scale>
    class fixed;
    
    template<integral T, const std::size_t ScaleA, const std::size_t ScaleB>
    struct fixed_expression {
            fixed<T, ScaleA> a;
            fixed<T, ScaleB> b;
            fixed_op op;
    };

    template <integral T, const std::size_t Scale>
    class fixed final {
        public:
            T raw;
            
        public:
            fixed() { raw = 0; }
            fixed(T raw_value) { raw = raw_value; }
            fixed(T integral, T fractional) {
                raw = (integral << Scale) | fractional;
            }

            inline float as_float() const {
                T upper_mask = (static_cast<T>(-1) >> Scale) << Scale;
                T lower_mask = ~upper_mask;

                T upper = (raw & upper_mask) >> Scale;
                T lower = raw & lower_mask;

                return float(upper) + float(lower) / float(Scale);
            }

            template<const std::size_t ScaleOther>
            inline fixed_expression<T, Scale, ScaleOther>
            operator+(const fixed<T, ScaleOther>& other) const {
                return {
                    .a = *this,
                    .b = other,
                    .op = fixed_op::ADD
                };
            }

            template<const std::size_t ScaleOther>
            inline fixed_expression<T, Scale, ScaleOther>
            operator*(const fixed<T, ScaleOther>& other) const {
                return {
                    .a = *this,
                    .b = other,
                    .op = fixed_op::MUL
                };
            }

            template<const std::size_t ScaleOther>
            inline fixed_expression<T, Scale, ScaleOther>
            operator-(const fixed<T, ScaleOther>& other) const {
                return {
                    .a = *this,
                    .b = other,
                    .op = fixed_op::SUB
                };
            }

            template<const std::size_t ScaleOther>
            inline fixed_expression<T, Scale, ScaleOther>
            operator/(const fixed<T, ScaleOther>& other) const {
                return {
                    .a = *this,
                    .b = other,
                    .op = fixed_op::DIV
                };
            }
    };

    namespace internal {
        template<integral T,
                 const std::size_t Scale>
        static inline fixed<T, Scale>
        execute(const fixed<T, Scale>& a,
                const fixed<T, Scale>& b,
                fixed_op op)
        {
            switch (op) {
                case fixed_op::ADD:
                    return fixed<T, Scale>(a.raw + b.raw);

                case fixed_op::MUL:
                    return fixed<T, Scale>(a.raw * b.raw);

                case fixed_op::SUB:
                    return fixed<T, Scale>(a.raw - b.raw);

                case fixed_op::DIV:
                    return fixed<T, Scale>(a.raw / b.raw);
            }
        }
    }

    template<integral T,
             const std::size_t ScaleTarget,
             const std::size_t ScaleA,
             const std::size_t ScaleB>
    using fixed_operator = std::function<fixed<T, ScaleTarget>(const fixed_expression<T, ScaleA, ScaleB>)>;

    template<integral T,
             const std::size_t ScaleA,
             const std::size_t ScaleB>
    static inline fixed<T, ScaleA>
    as_first(const fixed_expression<T, ScaleA, ScaleB>& expr) {
        fixed a = expr.a;
        fixed b = expr.b;

        constexpr ssize_t shift = ssize_t(ScaleB) - ssize_t(ScaleA);

        fixed<T, ScaleA> b_new;
        if constexpr (shift < 0) {
            b_new = fixed<T, ScaleA>(b.raw << shift);
        } else {
            b_new = fixed<T, ScaleA>(b.raw >> shift);
        }

        return internal::execute(a, b_new, expr.op);
    }
}

#endif // _FIXP_H
