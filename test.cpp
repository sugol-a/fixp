#include <cmath>
#include <cstddef>
#include <functional>
#include <iomanip>
#include <iostream>
#include <fixp.hpp>
#include <utility>
#include <sciplot/sciplot.hpp>

using fixed_q16_16 = fixp::fixed<16, std::int32_t, std::int64_t>;
using fixed_q4_12 = fixp::fixed<12, std::int16_t, std::int32_t>;
using fixed_q8_8 = fixp::fixed<8, std::int16_t, std::int32_t>;

namespace graphs {
    template<fixp::is_fixed T>
    void fixed_sin(sciplot::Plot2D& plot) {
        sciplot::Vec x_values = sciplot::linspace(-2.0 * std::numbers::pi, 2.0 * std::numbers::pi, 200);

        plot.xlabel("x");
        plot.ylabel("sin(x)");
        plot.xrange(-2.0 * std::numbers::pi, 2.0 * std::numbers::pi);
        plot.yrange(-2.0, 2.0);
        plot.legend();

        sciplot::Vec float_sin = x_values.apply([](double x) { return std::sin(x); });
        sciplot::Vec fixed_sin = x_values.apply([](double x) { return static_cast<double>(T::sin(T(x)).to_float()); });

        plot.drawCurve(x_values, float_sin).label("Float sin(x)");
        plot.drawCurve(x_values, fixed_sin).label("Fixed sin(x)");
    }

    template<fixp::is_fixed T>
    void fixed_cos(sciplot::Plot2D& plot) {
        sciplot::Vec x_values = sciplot::linspace(-2.0 * std::numbers::pi, 2.0 * std::numbers::pi, 200);

        plot.xlabel("x");
        plot.ylabel("cos(x)");
        plot.xrange(-2.0 * std::numbers::pi, 2.0 * std::numbers::pi);
        plot.yrange(-2.0, 2.0);
        plot.legend();

        sciplot::Vec float_cos = x_values.apply([](double x) { return std::cos(x); });
        sciplot::Vec fixed_cos = x_values.apply([](double x) { return static_cast<double>(T::cos(T(x)).to_float()); });

        plot.drawCurve(x_values, float_cos).label("Float cos(x)");
        plot.drawCurve(x_values, fixed_cos).label("Fixed cos(x)");
    }

    template<fixp::is_fixed T>
    void fixed_sqrt(sciplot::Plot2D& plot) {
        sciplot::Vec x_values = sciplot::linspace(0.0, 100.0f, 400);

        plot.xlabel("x");
        plot.ylabel("sqrt(x)");
        plot.xrange(0.0, 100.0);
        plot.yrange(0.0, 10.0);
        plot.legend();

        sciplot::Vec float_sqrt = x_values.apply([](double x) { return std::sqrt(x); });
        sciplot::Vec fixed_sqrt = x_values.apply([](double x) { return static_cast<double>(T::sqrt(T(x)).to_float()); });

        plot.drawCurve(x_values, float_sqrt).label("Float cos(x)");
        plot.drawCurve(x_values, fixed_sqrt).label("Fixed cos(x)");
    }

    template<fixp::is_fixed T>
    void fixed_trig_quadrant(sciplot::Plot2D& plot) {
        sciplot::Vec x_values = sciplot::linspace(-2.0f * std::numbers::pi, 2.0f * std::numbers::pi, 200);
        sciplot::Vec quadrants = x_values.apply([](double x) { return static_cast<double>(T::get_trig_quadrant(x)); });

        plot.xlabel("x");
        plot.ylabel("Trig Quadrant");

        plot.xrange(-2.0f * std::numbers::pi, 2.0f * std::numbers::pi);
        plot.yrange(-5.0, 5.0);
        plot.legend();

        plot.drawCurve(x_values, quadrants).label("Quadrant");
    }
}

static const std::pair<std::string, std::function<void(sciplot::Plot2D&)> > GRAPH_FUNCS[] = {
    { "sin_q16.16"  , graphs::fixed_sin<fixed_q16_16> },
    { "sin_q4.12"   , graphs::fixed_sin<fixed_q4_12> },
    { "sin_q8.8"    , graphs::fixed_sin<fixed_q8_8> },
    { "cos_q16.16"  , graphs::fixed_cos<fixed_q16_16> },
    { "cos_q4.12"   , graphs::fixed_cos<fixed_q4_12> },
    { "cos_q8.8"    , graphs::fixed_cos<fixed_q8_8> },
    { "sqrt_q16.16" , graphs::fixed_sqrt<fixed_q16_16> },
    { "sqrt_q4.12"  , graphs::fixed_sqrt<fixed_q4_12> },
    { "sqrt_q8.8"   , graphs::fixed_sqrt<fixed_q8_8> },
    { "quad_q16.16" , graphs::fixed_trig_quadrant<fixed_q16_16> },
    { "quad_q4.12"  , graphs::fixed_trig_quadrant<fixed_q4_12> },
    { "quad_q8.8"   , graphs::fixed_trig_quadrant<fixed_q8_8> },
};

static constexpr std::size_t N_GRAPH_FUNCS = sizeof(GRAPH_FUNCS) / sizeof(GRAPH_FUNCS[0]);

int
plot_graph(int argc, char** argv)
{
    if (argc != 1) {
        std::cerr << "Graph : Wrong number of arguments (expected graph name)" << std::endl;
        return -1;
    }

    std::string graph_name = argv[0];
    int index = -1;

    for (std::size_t i = 0; i < N_GRAPH_FUNCS; i++) {
        const auto& func = GRAPH_FUNCS[i];

        if (func.first == graph_name) {
            index = i;
            break;
        }
    }

    if (index < 0) {
        std::cerr << "Graph : Could not find graph named " << std::quoted(graph_name) << std::endl;
        return -1;
    }

    std::cout << "Graph : Plotting graph " << std::quoted(graph_name) << std::endl;

    sciplot::Plot2D plot;
    GRAPH_FUNCS[index].second(plot);

    sciplot::Figure fig = {{plot}};
    sciplot::Canvas canvas = {{fig}};

    canvas.size(800, 600);
    canvas.show();

    return 0;
}

int
test_print(int argc, char** argv)
{
    fixed_q4_12 x = -4.3382;
    std::cout << x.to_string() << std::endl;

    return 0;
}

int
test_truncate(int argc, char** argv)
{
    using fixed = fixed_q4_12;
    std::cout << fixed(2.074f).truncate() << std::endl;
    std::cout << fixed(-4.99999f).truncate() << std::endl;

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        std::cerr << "Missing command" << std::endl;
        return -1;
    }

    std::string command = argv[1];

    if (command == "graph") {
        return plot_graph(argc - 2, &argv[2]);
    } else if (command == "print") {
        return test_print(argc - 2, &argv[2]);
    } else if (command == "truncate") {
        return test_truncate(argc - 2, &argv[2]);
    } else {
        std::cerr << "Unknown command " << std::quoted(command) << std::endl;
        return -1;
    }

    return 0;
}
