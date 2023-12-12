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
#include <concepts>
#include <cstdlib>
#include <sstream>
#include <string>
#include <array>
#include <type_traits>
#include <cstdint>
#include <numbers>
#include <iostream>

namespace fixp {
    
    template<typename T>
    concept is_integral = std::is_integral<T>::value;

    namespace internals {
        static constexpr float sqrt_cexpr(float x, float c, float prev) {
            if (c == prev) {
                return c;
            } else {
                return sqrt_cexpr(x, 0.5 * (c + x / c), c);
            }
        }
    }

    template<const std::size_t FracBits,
             is_integral Storage = std::int16_t,
             is_integral Intermediate = std::int32_t>
    requires (sizeof(Storage) * 8 > FracBits && sizeof(Intermediate) >= sizeof(Storage))
    class fixed {
        private:
            static constexpr std::size_t TotalBits = sizeof(Storage) * 8;
            static constexpr std::size_t IntegralBits = TotalBits - FracBits;
            static constexpr Storage Scale = 1 << FracBits;
            static constexpr Storage FracMask = ((-1) << FracBits) >> FracBits;
            static constexpr Storage IntMask = ((-1) >> FracBits) << FracBits;

        private:
            Storage raw;

        public:
            constexpr fixed() { raw = 0; }
            
            template <is_integral U>
            constexpr fixed(U value) {
                raw = static_cast<Storage>(value << FracBits);
            }

            constexpr fixed(float value) {
                raw = static_cast<Storage>(value * Scale);
            }

            static constexpr fixed<FracBits, Storage, Intermediate>
            from_raw(Storage s) {
                fixed f;
                f.raw = s;

                return f;
            }

            inline const fixed<FracBits, Storage, Intermediate>&
            operator=(float value) {
                raw = static_cast<Storage>(value * Scale);
                return *this;
            }

            constexpr fixed<FracBits, Storage, Intermediate>
            operator+(const fixed<FracBits, Storage, Intermediate>& other) const {
                fixed f;
                f.raw = raw + other.raw;

                return f;
            }

            constexpr fixed<FracBits, Storage, Intermediate>
            operator-(const fixed<FracBits, Storage, Intermediate>& other) const {
                fixed f;
                f.raw = raw - other.raw;

                return f;
            }

            constexpr fixed<FracBits, Storage, Intermediate>
            operator-() const {
                fixed f;
                f.raw = -f.raw;

                return f;
            }

            constexpr fixed<FracBits, Storage, Intermediate>
            operator*(const fixed<FracBits, Storage, Intermediate>& other) const {
                Intermediate im = raw * other.raw;
                fixed f;

                f.raw = static_cast<Storage>(im >> FracBits);

                return f;
            }

            constexpr fixed<FracBits, Storage, Intermediate>
            operator/(const fixed<FracBits, Storage, Intermediate>& other) const {
                Intermediate im = (static_cast<Intermediate>(raw) << static_cast<Intermediate>(FracBits)) / static_cast<Intermediate>(other.raw);

                fixed f;
                f.raw = static_cast<Storage>(im);
                
                return f;
            }

            inline fixed<FracBits, Storage, Intermediate>&
            operator*=(const fixed<FracBits, Storage, Intermediate>& other) {
                const auto result = raw * other.raw;
                raw = result.raw;
                return *this;
            }

            inline fixed<FracBits, Storage, Intermediate>&
            operator/=(const fixed<FracBits, Storage, Intermediate>& other) {
                const auto result = raw / other;
                raw = result.raw;
                return *this;
            }

            constexpr bool
            operator>(const fixed<FracBits, Storage, Intermediate>& other) const {
                return raw > other.raw;
            }

            constexpr bool
            operator<(const fixed<FracBits, Storage, Intermediate>& other) const {
                return raw < other.raw;
            }

            constexpr bool
            operator==(const fixed<FracBits, Storage, Intermediate>& other) const {
                return raw == other.raw;
            }

            constexpr bool
            operator>=(const fixed<FracBits, Storage, Intermediate>& other) const {
                return raw >= other.raw;
            }

            constexpr bool
            operator<=(const fixed<FracBits, Storage, Intermediate>& other) const {
                return raw <= other.raw;
            }

            inline float to_float() const {
                return static_cast<float>(raw) / static_cast<float>(Scale);
            }

            inline Storage to_raw() const {
                return raw;
            }

            constexpr int truncate() const {
                return raw >> FracBits;
            }

            static constexpr fixed<FracBits, Storage, Intermediate>
            remap_trig_parameter(const fixed<FracBits, Storage, Intermediate>& value, int quadrant) {
                constexpr fixed pi            = std::numbers::pi_v<float>;
                constexpr fixed two_pi        = 2.0f * std::numbers::pi_v<float>;

                fixed remapped;
                switch (quadrant) {
                    case 0:
                        remapped = value;
                        break;

                    case 1:
                        remapped = pi - value;
                        break;

                    case 2:
                        remapped = value - pi;
                        break;

                    case 3:
                        remapped = two_pi - value;
                        break;
                }

                return fixed(static_cast<Storage>(remapped.raw));
            }

            static constexpr Storage get_trig_quadrant(const fixed<FracBits, Storage, Intermediate>& value) {
                constexpr fixed inverse_half_pi = 2.0f / std::numbers::pi_v<float>;
                const Storage quadrant = (value * inverse_half_pi).truncate();

                return quadrant & 0x03;
            }

            static constexpr fixed<FracBits, Storage, Intermediate>
            sin_quadrant(const fixed<FracBits, Storage, Intermediate>& value, Storage quadrant) {
                // We use a Taylor expansion using coefficients stolen from here:
                // http://www.sahraid.com/FFT/FixedPointArithmatic
                constexpr fixed a1 = -0.16605f;
                constexpr fixed a2 = 0.00761f;

                const fixed result = value * (fixed(1.0f) + value * (a1 + value * value * a2));
                const fixed sign = fixed(static_cast<float>(quadrant < 2)) - fixed(static_cast<float>(quadrant >= 2));

                return sign * result;
            }

            static constexpr fixed<FracBits, Storage, Intermediate>
            cos_quadrant(const fixed<FracBits, Storage, Intermediate>& value, Storage quadrant) {
                constexpr fixed a1 = -0.49670f;
                constexpr fixed a2 = 0.03705f;

                const bool is_positive = quadrant == 0 || quadrant == 3;
                const fixed sign = is_positive - !is_positive;

                const fixed result = fixed(1.0f) + value * (value * (a1 + value * value * a2));

                return sign * result;
            }

            static constexpr fixed<FracBits, Storage, Intermediate>
            sin(const fixed<FracBits, Storage, Intermediate>& value) {
                const Storage quadrant = get_trig_quadrant(value);
                const fixed remapped = remap_trig_parameter(value, quadrant);
                return sin_quadrant(remapped, quadrant);
            }

            static constexpr fixed<FracBits, Storage, Intermediate> cos(const fixed<FracBits, Storage, Intermediate>& value) {
                const int quadrant = get_trig_quadrant(value);
                const fixed remapped = remap_trig_parameter(value, quadrant);
                return cos_quadrant(remapped, quadrant);
            }

            template<const std::size_t Iterations=2>
            static constexpr fixed<FracBits, Storage, Intermediate>
            sqrt(const fixed<FracBits, Storage, Intermediate>& value)
            {
                using fixed_aux = fixed<FracBits, Intermediate, Intermediate>;

                constexpr auto SQRT_LUT {[]() constexpr {
                    constexpr std::size_t NElements = 1 << IntegralBits;
                    std::array<fixed_aux, NElements> entries = { };

                    for (std::size_t i = 0; i < NElements; i++) {
                        float x_float = static_cast<float>(i);
                        entries[i] = fixed_aux(internals::sqrt_cexpr(x_float, x_float, 0.0f));
                    }

                    return entries;
                }()};

                constexpr fixed_aux one_half = 0.5f;
                int truncated = value.truncate();

                fixed_aux x = SQRT_LUT[truncated];
                const fixed_aux a = fixed_aux::from_raw(static_cast<Intermediate>(value.to_raw()));

                for (std::size_t i = 0; i < Iterations; i++) {
                    fixed_aux inv_x = fixed_aux(1.0f) / x;
                    x = x - (x * x - a) * inv_x * one_half;
                }

                return fixed::from_raw(static_cast<Storage>(x.to_raw()));
            }
    };
}
