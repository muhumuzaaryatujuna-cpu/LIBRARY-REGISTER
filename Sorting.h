// Sorting.h - Task A2: comparator-templated sorts and iterative binary search.
#ifndef SORTING_H
#define SORTING_H

#include <algorithm>
#include <cstddef>
#include <random>
#include <utility>
#include <vector>

// A comparator is a strict "less-than": cmp(a,b) is true iff a must come before b.

// ---- Insertion sort: O(n^2) worst/average, O(n) best, STABLE, in place ----
// Loop invariant: at the start of each iteration with index i, a[0..i-1] holds the
// ORIGINAL first i elements in sorted (non-decreasing) order.
template <typename T, typename Comp>
void insertionSort(std::vector<T>& a, Comp cmp) {
    for (std::size_t i = 1; i < a.size(); ++i) {
        T key = std::move(a[i]);
        std::size_t j = i;
        while (j > 0 && cmp(key, a[j - 1])) {     // strict: equal keys are NOT passed => stable
            a[j] = std::move(a[j - 1]);
            --j;
        }
        a[j] = std::move(key);
    }
}

// ---- Merge sort: O(n log n) all cases, STABLE, O(n) extra space ----
namespace detail {
template <typename T, typename Comp>
void mergeSortRec(std::vector<T>& a, std::vector<T>& tmp,
                  std::size_t lo, std::size_t hi, Comp& cmp) {      // sorts a[lo, hi)
    if (hi - lo < 2) return;
    std::size_t mid = lo + (hi - lo) / 2;
    mergeSortRec(a, tmp, lo, mid, cmp);
    mergeSortRec(a, tmp, mid, hi, cmp);
    std::size_t i = lo, j = mid, k = lo;
    while (i < mid && j < hi) {
        if (cmp(a[j], a[i])) tmp[k++] = std::move(a[j++]);   // right only if STRICTLY smaller
        else                 tmp[k++] = std::move(a[i++]);   // ties go left => stable
    }
    while (i < mid) tmp[k++] = std::move(a[i++]);
    while (j < hi)  tmp[k++] = std::move(a[j++]);
    for (k = lo; k < hi; ++k) a[k] = std::move(tmp[k]);
}
} // namespace detail

template <typename T, typename Comp>
void mergeSort(std::vector<T>& a, Comp cmp) {
    if (a.size() < 2) return;
    std::vector<T> tmp(a.size());
    detail::mergeSortRec(a, tmp, 0, a.size(), cmp);
}

// ---- Quicksort, random pivot, Hoare partition: O(n log n) expected, O(n^2) worst,
//      NOT stable, in place (recurses on the smaller side => O(log n) stack) ----
namespace detail {
template <typename T, typename Comp, typename Rng>
void quickSortRec(std::vector<T>& a, std::ptrdiff_t lo, std::ptrdiff_t hi, Comp& cmp, Rng& rng) {
    while (lo < hi) {                                              // sorts a[lo..hi]
        std::uniform_int_distribution<std::ptrdiff_t> pick(lo, hi);
        T pivot = a[pick(rng)];                                    // random pivot (copy)
        std::ptrdiff_t i = lo, j = hi;
        while (i <= j) {
            while (cmp(a[i], pivot)) ++i;
            while (cmp(pivot, a[j])) --j;
            if (i <= j) { std::swap(a[i], a[j]); ++i; --j; }
        }
        if (j - lo < hi - i) { quickSortRec(a, lo, j, cmp, rng); lo = i; }
        else                 { quickSortRec(a, i, hi, cmp, rng); hi = j; }
    }
}
} // namespace detail

template <typename T, typename Comp>
void quickSort(std::vector<T>& a, Comp cmp) {
    if (a.size() < 2) return;
    static std::mt19937 rng(std::random_device{}());
    detail::quickSortRec(a, 0, static_cast<std::ptrdiff_t>(a.size()) - 1, cmp, rng);
}

// ---- Choose the O(n log n) sort by the last digit of the registration number ----
//   even -> merge sort (1)      odd -> quicksort with random pivot (0)
#ifndef USE_MERGE_SORT
#define USE_MERGE_SORT 1
#endif
template <typename T, typename Comp>
void fastSort(std::vector<T>& a, Comp cmp) {
#if USE_MERGE_SORT
    mergeSort(a, cmp);
#else
    quickSort(a, cmp);
#endif
}

// ---- Iterative binary search: O(log n) ----
// cmp3(element, key) returns <0 if element<key, 0 if equal, >0 if element>key.
// Returns the index of the LEFTMOST match (so duplicates are found from their
// first occurrence) or -1 if absent.  Requires a sorted by the same key.
template <typename T, typename K, typename Cmp3>
std::ptrdiff_t binarySearch(const std::vector<T>& a, const K& key, Cmp3 cmp3) {
    std::ptrdiff_t lo = 0, hi = static_cast<std::ptrdiff_t>(a.size()) - 1, found = -1;
    while (lo <= hi) {
        std::ptrdiff_t mid = lo + (hi - lo) / 2;
        int c = cmp3(a[mid], key);
        if (c == 0)      { found = mid; hi = mid - 1; }   // keep looking left for first match
        else if (c < 0)  lo = mid + 1;
        else             hi = mid - 1;
    }
    return found;
}

#endif // SORTING_H
