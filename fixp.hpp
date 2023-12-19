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

#include <cstdlib>
#include <sstream>
#include <string>
#include <array>
#include <type_traits>
#include <cstdint>
#include <numbers>

namespace fixp {
    template<typename T>
    concept is_integral = std::is_integral<T>::value;

    namespace internals {
        template<typename T>
        static constexpr T abs_cexpr(T x) {
            if (x < static_cast<T>(0)) {
                return -x;
            } else {
                return x;
            }
        }

        template<const std::size_t FracBits>
        static constexpr float sqrt_cexpr(float x, float c, float prev) {
            double dc = static_cast<double>(c);
            double dprev = static_cast<double>(prev);

            constexpr double epsilon = 1.0 / static_cast<double>(1 << FracBits);
            
            if (abs_cexpr(dprev - dc) < epsilon) {
                return c;
            } else {
                return sqrt_cexpr<FracBits>(x, 0.5 * (c + x / c), c);
            }
        }
    }

    template<const std::size_t FracBits,
             is_integral Storage = std::int16_t,
             is_integral Intermediate = std::int32_t>
    requires (sizeof(Storage) * 8 > FracBits && sizeof(Intermediate) >= sizeof(Storage))
    class fixed final {
        public:
            using storage_type = Storage;
            using intermediate_type = Intermediate;

        private:
            static constexpr std::size_t TotalBits = sizeof(Storage) * 8;
            static constexpr std::size_t IntegralBits = TotalBits - FracBits;
            static constexpr Storage Scale = 1 << FracBits;
            static constexpr Storage FracMask = ((-1) << FracBits) >> FracBits;
            static constexpr Storage IntMask = ((-1) >> FracBits) << FracBits;

        private:
            Storage raw;

        private:
            static constexpr fixed<FracBits, Storage, Intermediate>
            remap_trig_parameter(const fixed<FracBits, Storage, Intermediate>& x, int quadrant) {
                constexpr fixed pi     = std::numbers::pi_v<float>;
                constexpr fixed two_pi = 2.0f * std::numbers::pi_v<float>;

                const fixed value = fixed::from_raw(x.raw % two_pi.raw);

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

                return fixed::from_raw(static_cast<Storage>(remapped.raw));
            }

            static constexpr Storage get_trig_quadrant(const fixed<FracBits, Storage, Intermediate>& value) {
                constexpr fixed inverse_half_pi = 2.0f / std::numbers::pi_v<float>;
                Storage quadrant = (value * inverse_half_pi).truncate();

                Storage sign = quadrant >> (sizeof(quadrant) * 8 - 1);
                quadrant ^= sign;
                quadrant += sign & 0x01;
                quadrant &= 0x03;

                return quadrant;
            }

            static constexpr fixed<FracBits, Storage, Intermediate>
            sin_quadrant(const fixed<FracBits, Storage, Intermediate>& value, Storage quadrant) {
                // We use a Taylor expansion using coefficients stolen from here:
                // http://www.sahraid.com/FFT/FixedPointArithmatic
                constexpr fixed a1 = -0.16605f;
                constexpr fixed a2 = 0.00761f;

                const fixed result = value * (fixed(1.0f) + value * (a1 + (a2 * value * value)));
                const fixed sign = fixed(static_cast<float>(quadrant < 2)) - fixed(static_cast<float>(quadrant >= 2));

                return sign * result;
            }

            static constexpr fixed
            cos_quadrant(const fixed& value, Storage quadrant) {
                constexpr fixed a1 = -0.49670f;
                constexpr fixed a2 = 0.03705f;

                const bool is_positive = quadrant == 0 || quadrant == 3;
                const fixed sign = is_positive - !is_positive;

                const fixed result = fixed(1.0f) + value * (value * (a1 + value * value * a2));

                return sign * result;
            }

        public:
            constexpr fixed() { }
            
            template <is_integral U>
            constexpr fixed(U value) {
                raw = static_cast<Storage>(value << FracBits);
            }

            constexpr fixed(float value) {
                raw = static_cast<Storage>(value * Scale);
            }

            static constexpr fixed<FracBits, Storage, Intermediate>
            from_raw(Storage s) noexcept {
                fixed f;
                f.raw = s;

                return f;
            }

            inline const fixed<FracBits, Storage, Intermediate>&
            operator=(float value) noexcept {
                raw = static_cast<Storage>(value * Scale);
                return *this;
            }

            constexpr fixed<FracBits, Storage, Intermediate>
            operator+(const fixed<FracBits, Storage, Intermediate>& other) const noexcept {
                fixed f;
                f.raw = raw + other.raw;

                return f;
            }

            constexpr fixed<FracBits, Storage, Intermediate>
            operator-(const fixed<FracBits, Storage, Intermediate>& other) const noexcept {
                fixed f;
                f.raw = raw - other.raw;

                return f;
            }

            constexpr fixed<FracBits, Storage, Intermediate>
            operator-() const noexcept {
                fixed f;
                f.raw = raw * -1;

                return f;
            }

            constexpr fixed<FracBits, Storage, Intermediate>
            operator*(const fixed<FracBits, Storage, Intermediate>& other) const noexcept {
                Intermediate im = static_cast<Intermediate>(raw) * static_cast<Intermediate>(other.raw);

                fixed f;
                f.raw = static_cast<Storage>(im >> FracBits);

                return f;
            }

            constexpr fixed<FracBits, Storage, Intermediate>
            operator/(const fixed<FracBits, Storage, Intermediate>& other) const noexcept {
                Intermediate im = (static_cast<Intermediate>(raw) << static_cast<Intermediate>(FracBits)) / static_cast<Intermediate>(other.raw);

                fixed f;
                f.raw = static_cast<Storage>(im);
                
                return f;
            }

            constexpr fixed<FracBits, Storage, Intermediate>
            operator%(const fixed<FracBits, Storage, Intermediate>& other) const noexcept {
                fixed f;
                f.raw = raw % other.raw;

                return f;
            }

            inline fixed<FracBits, Storage, Intermediate>
            operator+=(const fixed<FracBits, Storage, Intermediate>& other) noexcept {
                *this = *this + other;
                return *this;
            }

            inline fixed<FracBits, Storage, Intermediate>
            operator-=(const fixed<FracBits, Storage, Intermediate>& other) noexcept {
                *this = *this - other;
                return *this;
            }
            
            inline fixed<FracBits, Storage, Intermediate>&
            operator*=(const fixed<FracBits, Storage, Intermediate>& other) noexcept {
                *this = *this * other;
                return *this;
            }

            inline fixed<FracBits, Storage, Intermediate>&
            operator/=(const fixed<FracBits, Storage, Intermediate>& other) noexcept {
                *this = *this / other;
                return *this;
            }

            inline fixed<FracBits, Storage, Intermediate>&
            operator%=(const fixed<FracBits, Storage, Intermediate>& other) noexcept {
                *this = *this % other;
                return *this;
            }

            constexpr bool
            operator>(const fixed<FracBits, Storage, Intermediate>& other) const noexcept {
                return raw > other.raw;
            }

            constexpr bool
            operator<(const fixed<FracBits, Storage, Intermediate>& other) const noexcept {
                return raw < other.raw;
            }

            constexpr bool
            operator==(const fixed<FracBits, Storage, Intermediate>& other) const noexcept {
                return raw == other.raw;
            }

            constexpr bool
            operator>=(const fixed<FracBits, Storage, Intermediate>& other) const noexcept {
                return raw >= other.raw;
            }

            constexpr bool
            operator<=(const fixed<FracBits, Storage, Intermediate>& other) const noexcept {
                return raw <= other.raw;
            }

            constexpr float to_float() const noexcept {
                return static_cast<float>(raw) / static_cast<float>(Scale);
            }

            constexpr Storage to_raw() const noexcept {
                return raw;
            }

            inline std::string to_string() const noexcept {
                using fixed_aux = fixed<FracBits, Intermediate, Intermediate>;

                std::stringstream ss;
                fixed_aux value = fixed_aux::from_raw(static_cast<Intermediate>(raw));

                // integral part
                ss << value.truncate();
                value -= value.truncate();

                if (!(value.to_raw() & FracMask)) {
                    // nothing in the fractional part; early exit
                    return ss.str();
                }

                static constexpr std::size_t max_digits = ([]() constexpr {
                    std::size_t log = 0;
                    std::size_t n = 1 << (FracBits - 1);

                    while (n) {
                        n /= 10;
                        log++;
                    }

                    return log + 1;
                })();

                ss << ".";

                if (value < 0) {
                    value *= -1;
                }

                std::size_t i = 0;
                while ((value.to_raw() & FracMask) && i < max_digits) {
                    value *= 10;
                    ss << (value.truncate() % 10);
                    value -= value.truncate();

                    i++;
                }

                return ss.str();
            }

            constexpr int truncate() const noexcept {
                return raw / Scale;
            }

            static constexpr fixed
            sin(const fixed& value) noexcept {
                const bool is_negative = value < 0;
                const fixed coeff = fixed(is_negative ? -1.0f : 1.0f);

                const int quadrant = get_trig_quadrant(value);
                const fixed remapped = remap_trig_parameter(coeff * value, quadrant);

                return coeff * sin_quadrant(remapped, quadrant);
            }

            static constexpr fixed
            cos(const fixed& value) noexcept {
                const bool is_negative = value < 0;
                const fixed coeff = fixed(is_negative ? -1.0f : 1.0f);

                const int quadrant = get_trig_quadrant(value);
                const fixed remapped = remap_trig_parameter(coeff * value, quadrant);

                return cos_quadrant(remapped, quadrant);
            }

            template<const std::size_t Iterations=2, const std::size_t LutLimit=1024>
            static inline constexpr fixed
            sqrt(const fixed& value) noexcept {
                using fixed_aux = fixed<FracBits, Intermediate, Intermediate>;

                if (value < 0) {
                    return 0;
                }

                static constexpr auto SQRT_LUT {[]() constexpr {
                    constexpr std::size_t NElements = std::min(
                        static_cast<std::size_t>(LutLimit),
                        static_cast<std::size_t>(1 << IntegralBits));

                    std::array<fixed_aux, NElements> entries = { };

                    // fudge it for values 0 < x < 1 otherwise we'll
                    // get trapped at a 0 result
                    entries[0] = fixed_aux(0.5f);

                    for (std::size_t i = 1; i < NElements; i++) {
                        float x_float = static_cast<float>(i);
                        entries[i] = fixed_aux(internals::sqrt_cexpr<FracBits>(x_float, x_float, 0.0f));
                    }

                    return entries;
                }()};

                std::size_t truncated = value.truncate();
                fixed_aux x;

                // only include bounds checking if the space of the
                // integer part of our value is too large to fit in
                // the LUT
                if constexpr ((1 << IntegralBits) >= LutLimit) {
                    if (truncated < SQRT_LUT.size()) {
                        x = SQRT_LUT[truncated];
                    } else {
                        // Value's integral part lies outside the LUT,
                        // so just use the value as an initial
                        // guess. Probably good enough.
                        x = fixed_aux::from_raw(value.to_raw());
                    }
                } else {
                    x = SQRT_LUT[truncated];
                }

                const fixed_aux a = fixed_aux::from_raw(
                    static_cast<Intermediate>(value.to_raw()));

                for (std::size_t i = 0; x.to_raw() && i < Iterations; i++) {
                    constexpr fixed_aux one_half = 0.5f;
                    fixed_aux inv_x = fixed_aux(1.0f) / x;

                    x = x - (x * x - a) * inv_x * one_half;
                }

                return fixed::from_raw(static_cast<Storage>(x.to_raw()));
            }
    };

    template<typename T>
    concept is_fixed = requires(T a) {
        { fixed(a) } -> std::same_as<T>;
    };
}
