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
#include <numbers>

namespace fixp {
    
    template<typename T>
    concept is_integral = std::is_integral<T>::value;

    // template<const std::size_t FracBits, is_integral Storage = std::int16_t, is_integral Intermediate = std::int32_t>
    // requires (sizeof(Storage) * 8 > FracBits && sizeof(Intermediate) > sizeof(Storage))
    // class fixed;

    // namespace constants {
    //     #define DEFCONST \
    //         template<const std::size_t FracBits, is_integral Storage = std::int16_t, is_integral Intermediate = std::int32_t> \
    //         static constexpr fixed<FracBits, Storage, Intermediate>

    //     DEFCONST PI  = std::numbers::pi_v<float>;
    //     DEFCONST TAU = 2.0f * std::numbers::pi_v<float>;
    //     DEFCONST E   = std::numbers::e_v<float>;

    //     #undef DEFCONST
    // }
    
    template<const std::size_t FracBits,
             is_integral Storage = std::int16_t,
             is_integral Intermediate = std::int32_t>
    requires (sizeof(Storage) * 8 > FracBits && sizeof(Intermediate) > sizeof(Storage))
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
            constexpr fixed() { }
            
            constexpr fixed(Storage raw_value) : raw(raw_value) { }

            constexpr fixed(float value) {
                raw = static_cast<Storage>(value * Scale);
            }

            inline const fixed<FracBits, Storage, Intermediate>&
            operator=(float value) {
                raw = static_cast<Storage>(value * Scale);
                return *this;
            }

            template<is_integral T>
            inline const fixed<FracBits, Storage, Intermediate>&
            operator=(T value) {
                *this = static_cast<float>(value);
                return *this;
            }

            inline fixed<FracBits, Storage, Intermediate>
            operator+(const fixed<FracBits, Storage, Intermediate>& other) const {
                return fixed(static_cast<Storage>(raw + other.raw));
            }

            inline fixed<FracBits, Storage, Intermediate>
            operator-(const fixed<FracBits, Storage, Intermediate>& other) const {
                return fixed(static_cast<Storage>(raw - other.raw));
            }

            inline fixed<FracBits, Storage, Intermediate>
            operator*(const fixed<FracBits, Storage, Intermediate>& other) const {
                Intermediate im = raw * other.raw;
                return fixed(static_cast<Storage>(im >> FracBits));
            }

            inline fixed<FracBits, Storage, Intermediate>
            operator/(const fixed<FracBits, Storage, Intermediate>& other) const {
                Intermediate im = (static_cast<Intermediate>(raw) << static_cast<Intermediate>(FracBits)) / static_cast<Intermediate>(other.raw);
                return fixed(static_cast<Storage>(im));
            }

            inline fixed<FracBits, Storage, Intermediate>&
            operator+=(const fixed<FracBits, Storage, Intermediate>& other) {
                raw += other.raw;
                return *this;
            }

            inline fixed<FracBits, Storage, Intermediate>&
            operator-=(const fixed<FracBits, Storage, Intermediate>& other) {
                raw -= other.raw;
                return *this;
            }

            inline fixed<FracBits, Storage, Intermediate>&
            operator*=(const fixed<FracBits, Storage, Intermediate>& other) {
                const auto result = *this * other;
                raw = result.raw;
                return *this;
            }

            inline fixed<FracBits, Storage, Intermediate>&
            operator/=(const fixed<FracBits, Storage, Intermediate>& other) {
                const auto result = *this / other;
                raw = result.raw;
                return *this;
            }

            constexpr operator
            Storage() const {
                return raw;
            }

            inline float to_float() const {
                return static_cast<float>(raw) / static_cast<float>(Scale);
            }

            inline int truncate() const {
                return (raw & IntMask) >> FracBits;
            }

            static inline fixed<FracBits, Storage, Intermediate>
            remap_trig_parameter(const fixed<FracBits, Storage, Intermediate>& value, int quadrant) {
                constexpr fixed inverse_tau = 1.0f / (2.0f * std::numbers::pi_v<float>);
                constexpr fixed half_pi = 0.5f * std::numbers::pi_v<float>;
                constexpr fixed inverse_half_pi = 2.0f / std::numbers::pi_v<float>;
                constexpr fixed pi = std::numbers::pi_v<float>;
                constexpr fixed two_pi = 2.0f * std::numbers::pi_v<float>;

                const Storage cycle = (value * inverse_tau).truncate();

                fixed remapped;

                switch (quadrant) {
                    case 0:
                        remapped = value;
                        break;

                    case 1:
                        remapped = half_pi - value;
                        break;

                    case 2:
                        remapped = value - pi;
                        break;

                    case 3:
                        remapped = two_pi - value;
                        break;
                }

                return fixed(static_cast<Storage>(value.raw - cycle * two_pi));
            }

            static inline Storage get_trig_quadrant(const fixed<FracBits, Storage, Intermediate>& value) {
                constexpr fixed inverse_half_pi = 2.0f / std::numbers::pi_v<float>;
                const Storage quadrant = (value * inverse_half_pi).truncate();
                return quadrant;
            }

            static inline fixed<FracBits, Storage, Intermediate>
            sin_quadrant(const fixed<FracBits, Storage, Intermediate>& value, Storage quadrant) {
                // We use a Taylor expansion using coefficients stolen from here:
                // http://www.sahraid.com/FFT/FixedPointArithmatic
                static constexpr fixed a1 = -0.16605f;
                static constexpr fixed a2 = 0.00761f;

                const fixed x_pow2 = value * value;
                const fixed x_pow4 = x_pow2 * x_pow2;
                const fixed<FracBits, Storage, Intermediate> intermediate =
                    static_cast<Storage>(Scale + (a1 * x_pow2 + a2 * x_pow4).raw);

                const Storage sign = (quadrant / 2) * -1 + 1 - (quadrant / 2)) * 1;
                
                return fixed(static_cast<Storage>(sign * intermediate.raw * value.raw));
            }

            static inline fixed<FracBits, Storage, Intermediate>
            cos_quadrant(const fixed<FracBits, Storage, Intermediate>& value, Storage quadrant) {
                static constexpr fixed a1 = -0.49670f;
                static constexpr fixed a2 = 0.03705f;

                const fixed x_pow2 = value * value;
                const fixed x_pow4 = x_pow2 * x_pow2;

                const Storage sign = (quadrant / 2) * -1 + (1 - (quadrant % 2)) * 1;

                return fixed(sign * static_cast<Storage>(Scale + (a1 * x_pow2 + a2 * x_pow4).raw));
            }

            static inline fixed<FracBits, Storage, Intermediate> sin(const fixed<FracBits, Storage, Intermediate>& value) {
                const Storage quadrant = get_trig_quadrant(value);
                const fixed remapped = remap_trig_parameter(value, quadrant);
                return sin_quadrant(remapped, quadrant);
            }

            static inline fixed<FracBits, Storage, Intermediate> cos(const fixed<FracBits, Storage, Intermediate>& value) {
                const int quadrant = get_trig_quadrant(value);
                const fixed remapped = remap_trig_parameter(value, quadrant);
                return cos_quadrant(remapped, quadrant);
            }
    };    
}
