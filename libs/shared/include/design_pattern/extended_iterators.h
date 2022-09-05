#pragma once
#pragma once
#include "defs.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{


		template<typename T,
			template<typename, typename> class Container,
			template<typename> class Allocator = std::allocator>
		class iteratorCapableWrapper {

		public:
			typedef typename Container<T, Allocator<T>>::iterator container_iterator_t;
			//Container<T, Allocator<T>>& wrapped_list;
			container_iterator_t wrapped_begin_it;
			container_iterator_t wrapped_end_it;
			size_t wrapped_size;

		public:

			iteratorCapableWrapper(Container<T, Allocator<T>>& other) {
				wrapped_begin_it = other.begin();
				wrapped_end_it = other.end();
				wrapped_size = other.size();
			};

			iteratorCapableWrapper(container_iterator_t& other_begin, container_iterator_t& other_end, size_t other_size ) {
				wrapped_begin_it = other_begin;
				wrapped_end_it = other_end;
				wrapped_size = other_size;
			};

			virtual std::list<container_iterator_t> prevs(typename container_iterator_t it, size_t cnt=1)=0;
			virtual std::list<container_iterator_t> nexts(typename container_iterator_t it, size_t cnt=1)=0;

			std::vector<std::reference_wrapper<T>> getWindow(typename const container_iterator_t& it, size_t window_left_size, size_t window_right_size) {
				std::vector<std::reference_wrapper<T>> window_vec; window_vec.reserve(window_left_size + 1 + window_right_size);

				assert(it != wrapped_end_it && "iterator it should not be end iterator!");

				size_t vec_idx = 0;
				// fill left window side
				std::list<container_iterator_t> left_window = prevs(it, window_left_size);
				for (auto& c_elem_it : left_window) {
					//window_vec[vec_idx] = *c_elem_it; vec_idx++;
					window_vec.push_back(*c_elem_it);
				}
				//window_vec.insert(window_vec.begin(), left_window.begin(), left_window.end());
				// fill ref iterator position
				//window_vec[vec_idx] = *it; vec_idx++;
				window_vec.push_back(*it);
				// fill right window side
				std::list<container_iterator_t> right_window = nexts(it, window_right_size);
				for (auto& c_elem_it : right_window) {
					//window_vec[vec_idx] = *c_elem_it; vec_idx++;
					window_vec.push_back(*c_elem_it);
				}
				//window_vec.insert(window_vec.end(), right_window.begin(), right_window.end());

				return window_vec;
			}

		};

		template<typename T, template<typename, typename> class Container>
		class unboundedIterator : public iteratorCapableWrapper<T, Container> {

			typedef typename iteratorCapableWrapper<T, Container> parent_iterator;
			typedef typename parent_iterator::container_iterator_t container_iterator_t;
			static_assert(std::is_default_constructible_v<T>,
				"Parameter must be default-constructible");

		public:
			T null_val;
			typename parent_iterator::container_iterator_t null_iterator() { return this->wrapped_end_it; };
			using parent_iterator::parent_iterator;

		public:

			std::list<container_iterator_t> prevs(typename container_iterator_t it, size_t cnt = 1) override {
				assert(it != null_iterator() && "iterator it should not be end iterator!");

				std::list<container_iterator_t> prevs_list;
				while (cnt > 0) {
					if (it == this->wrapped_begin_it) {
						prevs_list.insert(prevs_list.begin(), cnt, null_val); cnt = 0;
					}
					else {
						it = prev(it);
						cnt--;
						prevs_list.push_front(it);
					}
				}
				return prevs_list;
			}

			std::list<container_iterator_t> nexts(typename container_iterator_t it, size_t cnt = 1) override {
				assert(it != null_iterator() && "iterator it should not be end iterator!");

				std::list<container_iterator_t> nexts_list;
				while (cnt > 0) {
					if (next(it) == null_iterator()) {
						nexts_list.insert(nexts_list.end(), cnt, null_val); cnt = 0;
					}
					else {
						it = next(it);
						cnt--;
						nexts_list.push_back(it);
					}
				}
				return nexts_list;
			}

			typename container_iterator_t prev(typename const container_iterator_t it, size_t cnt = 1) {
				assert(it != null_iterator() && "iterator it should not be end iterator!");

				if (cnt == 0)return it;
				if (it == this->wrapped_begin_it)
					return null_iterator();
				else
					return prev(std::prev(it), cnt - 1);
			}

			typename container_iterator_t next(typename const container_iterator_t it, size_t cnt = 1) {
				assert(it != null_iterator() && "iterator it should not be end iterator!");

				if (cnt == 0)return it;
				auto next_it = std::next(it);
				if (next_it == null_iterator())
					return next_it;
				return next(next_it, cnt - 1);
			}

			/*
				Input [1,2,3,4]
				Tests:

				///
				prevs(it(4),4) = [NULL,1,2,3]
				prevs(it(4),1) = [3]
				prevs(it(4),0) = []
				prevs(it(1),1) = [NULL]
				prevs(it(1),2) = [NULL, NULL]
				prevs(it(2),2) = [NULL, 1]
				///
				nexts(it(1),1) = [2]
				nexts(it(1),2) = [2,3]
				nexts(it(4),4) = [null,null,null,null]
				nexts(it(3),4) = [4,null,null,null]
				nexts(it(end),1) = error
				///
				prev(it(4), 1) = it(3)
				prev(it(4), 2) = it(2)
				prev(it(4), 3) = it(1)
				prev(it(4), 4) = null_itr
				prev(it(end), 1) = error
				prev(it(end), 4) = error
				prev(it(1), 1) = null_itr
				prev(it(1), 2) = null_itr
				///
				next(it(4),1) = null_itr
				next(it(4),2) = null_itr
				next(it(end),1) = error
				next(it(3),1) = it(4)
				next(it(1),1) = it(2)
				*/

		};


		template<typename T, template<typename, typename> class Container>
		class circularIterator : public iteratorCapableWrapper<T, Container> {

			typedef typename iteratorCapableWrapper<T, Container> parent_iterator;
			typedef typename parent_iterator::container_iterator_t container_iterator_t;

		public:
			using parent_iterator::parent_iterator;
			typename container_iterator_t null_iterator() { return this->wrapped_end_it; }

		public:

			typename container_iterator_t next(typename const container_iterator_t it, size_t cnt = 1) {
				assert(it != null_iterator() && "iterator it should not be end iterator!");

				cnt = cnt % this->wrapped_size;
				if (cnt == 0)return it;
				auto next_it = std::next(it);
				if (next_it == this->wrapped_end_it)
					return next(this->wrapped_begin_it, cnt - 1);
				return next(next_it, cnt - 1);
			}

			typename container_iterator_t prev(typename const container_iterator_t it, size_t cnt = 1) {
				assert(it != null_iterator() && "iterator it should not be end iterator!");

				cnt = cnt % this->wrapped_size;
				if (cnt == 0)return it;
				if (it == this->wrapped_begin_it)
					return prev(std::prev(this->wrapped_end_it), cnt - 1);
				else
					return prev(std::prev(it), cnt - 1);

			}

			std::list<container_iterator_t> prevs(typename container_iterator_t it, size_t cnt = 1) override {
				assert(it != null_iterator() && "iterator it should not be end iterator!");

				std::list<container_iterator_t> prevs_list;
				while (cnt > 0) {
					it = prev(it);
					cnt--;
					prevs_list.push_front(it);
					}
				return prevs_list;
			}

			std::list<container_iterator_t> nexts(typename container_iterator_t it, size_t cnt = 1) override {
				assert(it != null_iterator() && "iterator it should not be end iterator!");

				std::list<container_iterator_t> nexts_list;
				while (cnt > 0) {
					it = next(it);
					cnt--;
					nexts_list.push_back(it);
				}
				return nexts_list;
			}

		};

		
	}
}