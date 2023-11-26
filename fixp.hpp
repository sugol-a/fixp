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
            using selftype = fixed<FracBits, Storage, Intermediate>;

        private:
            Storage raw;

        public:
            fixed() : raw(0) { }
            fixed(Storage raw_value) : raw(raw_value) { }
            fixed(float value) {
                raw = static_cast<Storage>(value * (1 << FracBits));
            }

            inline selftype operator+(const selftype& other) const {
                return fixed(raw + other.raw);
            }

            inline selftype operator-(const selftype& other) const {
                return fixed(raw - other.raw);
            }

            inline selftype operator*(const selftype& other) const {
                Intermediate im = raw * other.raw;
                return fixed(static_cast<Storage>(im >> FracBits));
            }

            inline selftype operator/(const selftype& other) const {
                Intermediate im = (static_cast<Intermediate>(raw) << static_cast<Intermediate>(FracBits)) / static_cast<Intermediate>(other.raw);
                return fixed(static_cast<Storage>(im));
            }

            operator Storage() const {
                return raw;
            }

            inline float to_float() const {
                return static_cast<float>(raw) / static_cast<float>(1 << FracBits);
            }
    };
}
