#pragma once
#include "defs.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		template<typename unit,
			template<typename, typename> class Container = std::list,
			template<typename> class Allocator = std::allocator>
		class Aggregator {
		public:
			typedef typename Container<unit, Allocator<unit>> container_t;
			Aggregator() {};
			virtual cv::Scalar operator()(container_t& container) = 0;
		};

		template<typename unit,
			template<typename, typename> class Container = std::list,
			template<typename> class Allocator = std::allocator>
		class MeanAggregator : public Aggregator<unit, Container, Allocator> {
			using Aggregator::Aggregator;
			typedef typename Container<unit, Allocator<unit>> container_t;

			cv::Scalar operator()(container_t& container) override {
				auto init = cv::Vec<double, unit::channels>::zeros();
				for (const auto& el : container) {
					for (int c_ch = 0; c_ch < unit::channels; c_ch++)
						init[c_ch] += el[c_ch];
				}
				return init / std::max(1.0, double(container.size()));
			}
		};

		template<typename value_type, typename element_type>
		value_type median(std::vector<element_type>& v, std::function<value_type(element_type)> _getter = [](const element_type& el) -> value_type {return el;})
		{
			auto comparator = [&](const element_type& el1, const element_type& el2) ->bool { return _getter(el1) > _getter(el2); };
			size_t n = v.size() / 2;
			std::nth_element(v.begin(), v.begin() + n, v.end(), comparator);
			return _getter(v[n]);
		}


		template<typename unit,
			template<typename, typename> class Container = std::list,
			template<typename> class Allocator = std::allocator>
		class MedianAggregator : public Aggregator<unit, Container, Allocator> {
			using Aggregator::Aggregator;
			typedef typename Container<unit, Allocator<unit>> container_t;

		public:
			cv::Scalar operator()(container_t& container) override {
				auto init = cv::Vec<double, unit::channels>::zeros();
				if (container.size() == 0)
					return init;

				std::vector<unit> v_container(container.begin(), container.end());
				for (int c_ch = 0; c_ch < unit::channels; c_ch++) {
					auto temp_getter = [&](const unit& el)->double {return el[c_ch]; };
					init[c_ch] = median<double, unit>(v_container, temp_getter);
				}
				return init;
			}

		};

	}
}