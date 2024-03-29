#pragma once
#include "defs.h"

namespace LxGeo
{
    namespace GeometryFactoryShared
    {

        template<typename Key, typename Result, typename Hash = std::hash<Key>>
        class FunctionCache {
        public:
            using FunctionType = std::function<Result(Key)>;

            FunctionCache(FunctionType& func)
                : function(func) {}

            Result operator()(const Key& key) {
                if (cache.find(key) == cache.end()) {
                    cache[key] = function(key);
                }
                return cache[key];
            }

        private:
            std::unordered_map<Key, Result, Hash> cache;
            FunctionType& function;
        };

    }
}