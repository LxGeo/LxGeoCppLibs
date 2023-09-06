#pragma once
#include "defs.h"

namespace LxGeo
{
    namespace GeometryFactoryShared
    {

        template<typename K, typename V>
        class DefaultMap : public std::map<K, V> {
        private:
            V m_default = V();

        public:
            DefaultMap() = default;
            DefaultMap(const V& defaultValue) : m_default(defaultValue) {}

            V& operator[](const K& key) {
                if (this->find(key) == this->end()) {
                    std::map<K, V>::operator[](key) = m_default;
                }
                return std::map<K, V>::operator[](key);
            }

        };

        template<typename K, typename V>
        class DefaultUnorderedMap : public std::unordered_map<K, V> {
        private:
            V m_default;

        public:
            DefaultUnorderedMap() = default;
            DefaultUnorderedMap(const V& defaultValue) : m_default(defaultValue) {}

            V& operator[](const K& key) {
                if (this->find(key) == this->end()) {
                    std::unordered_map<K, V>::operator[](key) = m_default;
                }
                return std::unordered_map<K, V>::operator[](key);
            }

        };

    }
}