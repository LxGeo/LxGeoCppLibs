#pragma once
#include <iostream>
#include <vector>
#include <list>
#include <functional>

template <typename T>
std::list<std::vector<T>> get_all_combinations(std::vector<T> elements, size_t s) {

    std::list<std::vector<T>> comb_result;
    if (elements.size() < s)
        return comb_result;
    std::vector<T> combination;
    combination.reserve(s);

    std::function<void(int, size_t)> dfs;
    dfs = [&](int offset, size_t k) -> void {
        if (k == 0) {
            comb_result.push_back(combination);
            return;
        }
        for (size_t i = offset; i <= elements.size() - k; ++i) {
            combination.push_back(elements[i]);
            dfs(i + 1, k - 1);
            combination.pop_back();
        }
    };
    dfs(0, s);
    return comb_result;
}