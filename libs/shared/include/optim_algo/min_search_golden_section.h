#pragma once
#include "defs.h"
#include <limits>

namespace LxGeo
{
    namespace GeometryFactoryShared
    {

        constexpr double sqrt5 = 2.236067977499789696;
        constexpr double lambda = 0.5 * (sqrt5 - 1.0);
        constexpr double mu = 0.5 * (3.0 - sqrt5); // = 1 - lambda

        bool stopping_rule(double x0, double x1, double tolerance) {
            double xm = 0.5 * std::abs(x1 + x0);
            return (xm <= 1.0) ? (std::abs(x1 - x0) < tolerance) : (std::abs(x1 - x0) < tolerance * xm);
        }

        template<typename Func>
        double min_search_golden_section(Func f, double a, double b, double tolerance, int max_iter) {
            double x1 = b - lambda * (b - a);
            double x2 = a + lambda * (b - a);
            double fx1 = f(x1);
            double fx2 = f(x2);

            if (tolerance <= 0.0) tolerance = std::sqrt(std::numeric_limits<double>::epsilon()) * (b - a);

            int iter = 0;
            while (!stopping_rule(a, b, tolerance) && iter < max_iter) {
                ++iter;

                if (fx1 > fx2) {
                    a = x1;
                    x1 = x2;
                    fx1 = fx2;
                    x2 = b - mu * (b - a);
                    fx2 = f(x2);
                }
                else {
                    b = x2;
                    x2 = x1;
                    fx2 = fx1;
                    x1 = a + mu * (b - a);
                    fx1 = f(x1);
                }
            }
            return b;
        }

        template<typename Func>
        double min_ternary_search(Func f, double left, double right, double tolerance, int max_iter) {

            int iter = 0;
            while (std::abs(right - left) >= tolerance && iter < max_iter) {
                iter++;
                double left_third = left + (right - left) / 3;
                double right_third = right - (right - left) / 3;
                if (f(left_third) > f(right_third))
                    left = left_third;
                else
                    right = right_third;
            }
            return (left + right) / 2;

        }

        template<typename Func>
        double min_binary_search(Func f, double left, double right, double tolerance, int max_iter) {
            int iter = 0;
            while (std::abs(right - left) >= tolerance && iter < max_iter) {
                iter++;
                double mid = (left + right) / 2;
                if (f(left) < f(right))
                    right = mid;
                else
                    left = mid;
            }
            return (left + right) / 2;
        }

        template<typename Func>
        double custom_splitting_search(Func f, double left, double right, double tolerance, int max_iter, int nbins=3) {
            
            double bin_size = (right - left) / nbins;

            std::list<double> bins_extrema;
            for (int c_bin_idx = 0; c_bin_idx < nbins-1; c_bin_idx++) {
                double bin_left = left + c_bin_idx * bin_size;
                double bin_right = left + (c_bin_idx + 1) * bin_size;
                bins_extrema.push_back(min_binary_search(f, bin_left, bin_right, tolerance, max_iter / nbins));
            }
            return *std::min_element(bins_extrema.begin(), bins_extrema.end(), [&f](const double& extrema1, const double& extrema2) {return f(extrema1)<f(extrema2); });
        }


        template<typename Func>
        double powells_method_bounded(Func f, double a, double b, double tolerance, int max_iter, double step=0) {
            double x = (a + b) / 2.0;
            if (step==0)
                step = (b - a) / 2.0;
            double fx = f(x);
            double xmin = x;

            for (int iter = 0; iter < max_iter; ++iter) {
                double xplus = std::min(x + step, b);
                double xminus = std::max(x - step, a);

                double fplus = f(xplus);
                double fminus = f(xminus);

                if (fplus < fx) {
                    x = xplus;
                    fx = fplus;
                }
                else if (fminus < fx) {
                    x = xminus;
                    fx = fminus;
                }
                else {
                    step /= 2.0;
                    continue;
                }

                xmin = (fx < f(xmin)) ? x : xmin;

                if (std::abs(fplus - fminus) < tolerance)
                    break;
            }

            return xmin;
        }

        template<typename Func>
        double simulated_annealing_bounded(Func f, double a, double b, double initial_guess, double initial_temperature, double cooling_rate, int max_iter) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dist(20, 10.0);

            double x = initial_guess;
            double fx = f(x);
            double xmin = x;
            double current_temperature = initial_temperature;

            for (int iter = 0; iter < max_iter; ++iter) {
                double candidate = x + (2 * dist(gen) - 1) * current_temperature;
                candidate = std::max(a, std::min(b, candidate)); // Ensure candidate stays within bounds

                double fc = f(candidate);

                double delta = fc - fx;
                if (delta < 0 || dist(gen) < std::exp(-delta / current_temperature)) {
                    x = candidate;
                    fx = fc;
                }

                xmin = (fx < f(xmin)) ? x : xmin;

                current_temperature *= cooling_rate;
            }

            return xmin;
        }

    }
}