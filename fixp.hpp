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

#include <cmath>
#include <type_traits>
#include <cstdint>

namespace fixp {
    template<typename T>
    concept is_integral = std::is_integral<T>::value;

    template<const std::size_t FracBits,
             is_integral Storage = std::int16_t,
             is_integral Intermediate = std::int32_t>
    requires (sizeof(Storage) * 8 > FracBits && sizeof(Intermediate) > sizeof(Storage))
    class fixed {
        private:
            static constexpr std::size_t TotalBits = sizeof(Storage) * 8;
            static constexpr std::size_t IntegralBits = TotalBits - FracBits;
            static constexpr Storage Scale = 1 << FracBits;
            using self_type = fixed<FracBits, Storage, Intermediate>;

        private:
            Storage raw;
        public:
            using underlying_type = Storage;
            
            static constexpr self_type PI  = static_cast<Storage>(3.1415926535f * Scale);
            static constexpr self_type TAU = static_cast<Storage>(6.2831853071f * Scale);
            static constexpr self_type E   = static_cast<Storage>(2.7182818284f * Scale);

        public:
            fixed() : raw(0) { }

            constexpr fixed(Storage raw_value) : raw(raw_value) { }

            constexpr fixed(float value) {
                raw = static_cast<Storage>(value * (1 << FracBits));
            }

            inline self_type operator+(const self_type& other) const {
                return fixed(static_cast<Storage>(raw + other.raw));
            }

            inline self_type operator-(const self_type& other) const {
                return fixed(static_cast<Storage>(raw - other.raw));
            }

            inline self_type operator*(const self_type& other) const {
                Intermediate im = raw * other.raw;
                return fixed(static_cast<Storage>(im >> FracBits));
            }

            inline self_type operator/(const self_type& other) const {
                Intermediate im = (static_cast<Intermediate>(raw) << static_cast<Intermediate>(FracBits)) / static_cast<Intermediate>(other.raw);
                return fixed(static_cast<Storage>(im));
            }

            inline self_type& operator+=(const self_type& other) {
                raw += other.raw;
                return *this;
            }

            inline self_type& operator-=(const self_type& other) {
                raw -= other.raw;
                return *this;
            }

            inline self_type& operator*=(const self_type& other) {
                const self_type result = *this * other;
                raw = result.raw;
                return *this;
            }

            inline self_type& operator/=(const self_type& other) {
                const self_type result = *this / other;
                raw = result.raw;
                return *this;
            }

            constexpr operator Storage() const {
                return raw;
            }

            inline float to_float() const {
                return static_cast<float>(raw) / static_cast<float>(Scale);
            }

            static inline self_type sin(const self_type& value) {
                // We use a Taylor expansion using coefficients stolen from here:
                // http://www.sahraid.com/FFT/FixedPointArithmatic
                static constexpr self_type a1 = static_cast<Storage>(-0.16605f * static_cast<float>(Scale));
                static constexpr self_type a2 = static_cast<Storage>(0.00761f * static_cast<float>(Scale));

                const self_type x_pow2 = value * value;
                const self_type x_pow4 = x_pow2 * x_pow2;
                const self_type intermediate = static_cast<Storage>(Scale + (a1 * x_pow2 + a2 * x_pow4).raw);

                return intermediate * value;
            }

            static inline self_type cos(const self_type& value) {
                static constexpr self_type a1 = static_cast<Storage>(-0.49670f * static_cast<float>(Scale));
                static constexpr self_type a2 = static_cast<Storage>(0.03705f * static_cast<float>(Scale));

                const self_type x_pow2 = value * value;
                const self_type x_pow4 = x_pow2 * x_pow2;

                return fixed(static_cast<Storage>(Scale + (a1 * x_pow2 + a2 * x_pow4).raw));
            }
    };
}
