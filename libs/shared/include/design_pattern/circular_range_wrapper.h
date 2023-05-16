#pragma once
#include <iterator>
#include "defs.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{

        class RangeUtils {
        public:
            template <typename Iterator>
            static std::vector<Iterator> getRangeIterators(Iterator iter, int leftWindowSize, int rightWindowSize) {

                std::vector<Iterator> centeredRange;
                centeredRange.reserve(leftWindowSize + rightWindowSize + 1);

                for (int i = -leftWindowSize; i <= rightWindowSize; ++i) {
                    Iterator current = iter;
                    std::advance(current, i);
                    centeredRange.emplace_back(current);
                }

                return centeredRange;
            }
        };


        template <typename Iterator>
        class CircularIterator {
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = typename std::iterator_traits<Iterator>::value_type;
            using difference_type = typename std::iterator_traits<Iterator>::difference_type;
            using pointer = typename std::iterator_traits<Iterator>::pointer;
            using reference = typename std::iterator_traits<Iterator>::reference;

            CircularIterator(Iterator current, Iterator begin, Iterator end)
                : current(current), begin(begin), end(end) {}

            reference operator*() const { return *current; }
            pointer operator->() const { return &(*current); }

            Iterator parent_iterator() { return current; }

            CircularIterator& operator++() {
                ++current;
                if (current == end)
                    current = begin;
                return *this;
            }

            CircularIterator operator++(int) {
                CircularIterator tmp = *this;
                ++(*this);
                return tmp;
            }

            CircularIterator& operator--() {
                if (current == begin)
                    current = end;
                --current;
                return *this;
            }

            CircularIterator operator--(int) {
                CircularIterator tmp = *this;
                --(*this);
                return tmp;
            }

            bool operator==(const CircularIterator& other) const { return current == other.current; }
            bool operator!=(const CircularIterator& other) const { return !(*this == other); }

        private:
            Iterator current;
            Iterator begin;
            Iterator end;
        };

        template <typename Container>
        class CircularWrapper : public RangeUtils {
        public:
            template <typename Range>
            CircularWrapper(Range& range)
                : container(range) {}

            using iterator = CircularIterator<typename Container::iterator>;
            using const_iterator = CircularIterator<typename Container::const_iterator>;
            using size_type = typename Container::size_type;

            const_iterator begin() const { return const_iterator(container.begin(), container.begin(), container.end()); }
            const_iterator end() const { return const_iterator(container.end(), container.begin(), container.end()); }
            iterator begin() { return iterator(container.begin(), container.begin(), container.end()); }
            iterator end() { return iterator(container.end(), container.begin(), container.end()); }

            size_type size() const { return container.size(); }

        private:
            Container& container;
        };

        template <typename Container>
        class CopyCircularWrapper : public RangeUtils {
        public:
            template <typename Range>
            CopyCircularWrapper(const Range& range)
                : container(range.begin(), range.end()) {}

            template <typename Iterator>
            CopyCircularWrapper(Iterator begin, Iterator end)
                : container(begin, end) {}

            using iterator = CircularIterator<typename Container::iterator>;
            using const_iterator = CircularIterator<typename Container::const_iterator>;
            using size_type = typename Container::size_type;

            const_iterator begin() const { return const_iterator(container.begin(), container.begin(), container.end()); }
            const_iterator end() const { return const_iterator(container.end(), container.begin(), container.end()); }
            iterator begin() { return iterator(container.begin(), container.begin(), container.end()); }
            iterator end() { return iterator(container.end(), container.begin(), container.end()); }

            size_type size() const { return container.size(); }

            iterator erase(iterator it) {
                auto new_it = container.erase(it.parent_iterator());
                if (new_it == container.end())
                    new_it = container.begin();
                return iterator(new_it, container.begin(), container.end());
            }

        private:
            Container container;
        };


	}
}