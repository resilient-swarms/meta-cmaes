
#ifndef STAT_FUNS_HPP
#define STAT_FUNS_HPP

#include <set>
#include <random>
#include <algorithm>

namespace statfuns
{
    // sampling without replacement (see https://stackoverflow.com/questions/28287138/c-randomly-sample-k-numbers-from-range-0n-1-n-k-without-replacement)
    std::set<size_t> _pickSet(size_t N, size_t k, std::mt19937 &gen)
    {
        std::uniform_int_distribution<> dis(0, N - 1);
        std::set<size_t> elems;
        elems.clear();

        while (elems.size() < k)
        {
            elems.insert(dis(gen));
        }

        return elems;
    }

    std::set<size_t> _fullSet(size_t N)
    {
        std::set<size_t> elems;
        size_t k = 0;
        while (elems.size() < N)
        {
            elems.insert(k);
            ++k;
        }

        return elems;
    }

    std::set<size_t> _take_complement(std::set<size_t> full_set, std::set<size_t> sub_set)
    {
        std::set<size_t> diff;

        std::set_difference(full_set.begin(), full_set.end(), sub_set.begin(), sub_set.end(),
                            std::inserter(diff, diff.begin()));
        return diff;
    }
} // namespace statfuns

#endif