#pragma once

#if false
#include <execution>
#define SEQ std::execution::seq,
#define PAR std::execution::par,
#else
#define SEQ
#define PAR
#endif

#include "defs.h"

namespace LxGeo
{
	namespace numcpp
	{

		namespace statsFn {
			template <typename C_t>
			requires std::ranges::range<C_t>
			double circular_mean(const C_t& container, double min_val = 0, double max_val = 360) {
				size_t len = container.size();
				double polar_x_sum = 0, polar_y_sum = 0;
				for (const auto& c_val : container) {
					double c_rad_val = (c_val - min_val) / (max_val / 2) * M_PI;
					polar_x_sum += std::sin(c_rad_val);
					polar_y_sum += std::cos(c_rad_val);
				}
				double mean_rad = std::atan2(polar_x_sum / len, polar_y_sum / len);
				double mean_deg = mean_rad / M_PI * (max_val / 2) + min_val;
				return mean_deg;
			}

			template <typename C_t>
			requires std::ranges::range<C_t>
			double mean_fn(const C_t& container) {
				using T = typename std::ranges::range_value_t<C_t>;
				T sum = std::reduce(PAR container.cbegin(), container.cend());;
				return sum/container.size();
			}


			template <typename C_t>
			requires std::ranges::range<C_t>
			double variance_fn(const C_t& container) {
				using T = typename std::ranges::range_value_t<C_t>;
				T mean = mean_fn(container);
				size_t count = container.size();
				std::vector<T> diff(count);
				std::transform(container.begin(), container.end(), diff.begin(), [mean](T x) { return x - mean; });
				T sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
				T variance = sq_sum / count;
				return variance;
			}

		}

		/** DetailedStats is applicable for any type as long as it implements the following methods and operators
		/* operator==(value_type1, value_type2)
		/* operator>(value_type1, value_type2)
		/* operator *(double, value_type1)
		/* value_type2.isNull()
		*/

		template <typename value_type>
		class DetailedStats{
			typedef std::optional<value_type> optval_t;
		public:
			DetailedStats(const std::list<value_type>& input_values, value_type _null_value, value_type _zero_value): null_value(_null_value), zero_value(_zero_value){
				non_null_values.reserve(input_values.size());
				std::copy_if(
					input_values.begin(), input_values.end(), std::back_inserter(non_null_values),
					[&_null_value](const value_type& c_value) {return abs(c_value-_null_value) > 1e-1; }
				);
				non_null_values.shrink_to_fit();
				std::sort(PAR non_null_values.begin(), non_null_values.end());
				null_count = input_values.size() - non_null_values.size();
			};

		public:

			const std::vector<value_type>& values_vector() const { return non_null_values; }
			bool empty() { return non_null_values.empty(); }
			// All statistics will be regarding non_null_values
			size_t count_null() { return null_count; }
			size_t count() { return non_null_values.size(); }
			value_type sum() {
				if (!m_sum.has_value())
					m_sum = std::reduce(PAR non_null_values.cbegin(), non_null_values.cend());
				return m_sum.value();
			}
			value_type min() {
				if (!m_min.has_value()) {
					const auto [min_el, max_el] = std::minmax_element(PAR non_null_values.cbegin(), non_null_values.cend());
					m_min = *min_el;
					m_max = *max_el;
				}
				return m_min.value();
			}
			value_type max() {
				if (!m_max.has_value()) {
					const auto [min_el, max_el] = std::minmax_element(PAR non_null_values.cbegin(), non_null_values.cend());
					m_min = *min_el;
					m_max = *max_el;
				}
				return m_max.value();
			}
			value_type mean(){
				if (!m_mean.has_value())
					m_mean = sum() / count();
				return m_mean.value();
			}
			value_type variance() {
				if (!m_variance.has_value()) {
					m_variance = statsFn::variance_fn(non_null_values);
				}
				return m_variance.value();
			}
			value_type percentile(double p){
				if (!std::isfinite(p) || p < 0 || p > 100) {
					throw std::runtime_error("Percentile must be between 0 and 1.");
				}
				
				if (m_percentiles.find(p) == m_percentiles.end())
					m_percentiles[p] = optval_t();
				optval_t& c_percentile = m_percentiles[p];
				if (!c_percentile.has_value()) {
					// Compute the index of the element that corresponds to the percentile
					double index = (p / 100.0) * (count() - 1);

					// Compute the integer and fractional parts of the index
					double int_part;
					double frac_part = std::modf(index, &int_part);

					// If the index is an integer, return the corresponding value
					if (frac_part == 0.0) {
						return non_null_values[static_cast<size_t>(int_part)];
					}

					// Otherwise, interpolate between the values at the integer and integer+1 indices
					value_type lower_value = non_null_values[static_cast<size_t>(int_part)];
					value_type upper_value = non_null_values[static_cast<size_t>(int_part) + 1];
					c_percentile = lower_value + frac_part * (upper_value - lower_value);
				}
				return c_percentile.value();
			}
			value_type stdev() {
					return std::sqrt(variance());
			}
			value_type coeff_var() {
				if (mean() == zero_value)
					return null_value;
				return stdev() / mean();
			}

			value_type gini_coefficient() {
				
				if (!gini_coef.has_value()) {

					double sum_of_absolute_differences = 0;
					int n = count();

					for (int i = 0; i < n; i++) {
						sum_of_absolute_differences += (2 * i - n + 1) * non_null_values[i];
					}

					gini_coef = sum_of_absolute_differences / (n * sum());
				}
				return gini_coef.value();
			}


		private:
			optval_t m_sum;
			optval_t m_min;
			optval_t m_max;
			optval_t m_mean;
			optval_t m_variance;
			optval_t gini_coef;
			std::map<double, optval_t> m_percentiles;


		private:
			std::vector<value_type> non_null_values;
			value_type null_value, zero_value;
			size_t null_count;
		};

			

	}
}