#pragma once
#pragma once
#include "defs.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{


		template<typename T>
		class capableIterator : public std::list<T> {

			using std::list<T>::list;			

		public:
			capableIterator(std::list<T>& other) : std::list<T>(other.begin(), other.end()){};

			virtual std::list<T> prevs(typename std::list<T>::iterator it, size_t cnt=1)=0;
			virtual std::list<T> nexts(typename std::list<T>::iterator it, size_t cnt=1)=0;

			std::vector<T> getWindow(typename const std::list<T>::iterator& it, size_t window_left_size, size_t window_right_size) {
				std::vector<T> window_vec; window_vec.reserve(window_left_size + 1 + window_right_size);

				assert(it != this->end() && "iterator it should not be end iterator!");

				// fill left window side
				std::list<T> left_window = prevs(it, window_left_size);
				window_vec.insert(window_vec.begin(), left_window.begin(), left_window.end());
				// fill ref iterator position
				window_vec.push_back(*it);
				// fill right window side
				std::list<T> right_window = nexts(it, window_right_size);
				window_vec.insert(window_vec.end(), right_window.begin(), right_window.end());

				return window_vec;
			}

		};

		template<typename parent_iterator>
		class unboundedIterator : public parent_iterator {

			typedef typename parent_iterator::value_type T;
			static_assert(std::is_default_constructible_v<T>,
				"Parameter must be default-constructible");

		public:
			T null_val;
			typename parent_iterator::iterator null_iterator() { return this->end(); };
			using parent_iterator::parent_iterator;

		public:

			std::list<T> prevs(typename parent_iterator::iterator it, size_t cnt = 1) {
				assert(it != null_iterator() && "iterator it should not be end iterator!");

				std::list<T> prevs_list;
				while (cnt > 0) {
					if (it == this->begin()) {
						prevs_list.insert(prevs_list.begin(), cnt, null_val); cnt = 0;
					}
					else {
						it = prev(it);
						cnt--;
						prevs_list.push_front(*it);
					}
				}
				return prevs_list;
			}

			std::list<T> nexts(typename parent_iterator::iterator it, size_t cnt = 1) {
				assert(it != null_iterator() && "iterator it should not be end iterator!");

				std::list<T> nexts_list;
				while (cnt > 0) {
					if (next(it) == null_iterator()) {
						nexts_list.insert(nexts_list.end(), cnt, null_val); cnt = 0;
					}
					else {
						it = next(it);
						cnt--;
						nexts_list.push_back(*it);
					}
				}
				return nexts_list;
			}

			typename parent_iterator::iterator prev(typename const parent_iterator::iterator it, size_t cnt = 1) {
				assert(it != null_iterator() && "iterator it should not be end iterator!");

				if (cnt == 0)return it;
				if (it == this->begin())
					return null_iterator();
				else
					return prev(std::prev(it), cnt - 1);
			}

			typename parent_iterator::iterator next(typename const parent_iterator::iterator it, size_t cnt = 1) {
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


		template<typename parent_iterator>
		class circularIterator : public parent_iterator {

			typedef typename parent_iterator::value_type T;

		public:
			using parent_iterator::parent_iterator;
			typename parent_iterator::iterator null_iterator() { return this->end(); }

		public:

			typename parent_iterator::iterator next(typename const parent_iterator::iterator it, size_t cnt = 1) {
				assert(it != null_iterator() && "iterator it should not be end iterator!");

				cnt = cnt % this->size();
				if (cnt == 0)return it;
				auto next_it = std::next(it);
				if (next_it == this->end())
					return next(this->begin(), cnt - 1);
				return next(next_it, cnt - 1);
			}

			typename parent_iterator::iterator prev(typename const parent_iterator::iterator it, size_t cnt = 1) {
				assert(it != null_iterator() && "iterator it should not be end iterator!");

				cnt = cnt % this->size();
				if (cnt == 0)return it;
				if (it == this->begin())
					return prev(std::prev(this->end()), cnt - 1);
				else
					return prev(std::prev(it), cnt - 1);

			}

			std::list<T> prevs(typename parent_iterator::iterator it, size_t cnt = 1) {
				assert(it != null_iterator() && "iterator it should not be end iterator!");

				std::list<T> prevs_list;
				while (cnt > 0) {
					it = prev(it);
					cnt--;
					prevs_list.push_front(*it);
					}
				return prevs_list;
			}

			std::list<T> nexts(typename parent_iterator::iterator it, size_t cnt = 1) {
				assert(it != null_iterator() && "iterator it should not be end iterator!");

				std::list<T> nexts_list;
				while (cnt > 0) {
					it = next(it);
					cnt--;
					nexts_list.push_back(*it);
				}
				return nexts_list;
			}

		};

		
	}
}