// bench.cpp - A4 timing experiment: ./library_register bench   (build with -O2)
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "ReturnRecord.h"
#include "Sorting.h"

namespace benchmark {
// A4 timing experiment (built into the same program; build with -O2).
// Sorts n random ReturnRecords (unique random ids) by id; 3 repeats per size; prints averages + doubling ratios as CSV.

static bool lessId(const ReturnRecord& a, const ReturnRecord& b) { return a.id < b.id; }

static std::vector<ReturnRecord> randomRecords(std::size_t n, std::mt19937& rng) {
    std::vector<int> perm(n);
    std::iota(perm.begin(), perm.end(), 1);
    std::shuffle(perm.begin(), perm.end(), rng);                 // unique random keys
    std::vector<ReturnRecord> v(n);
    for (std::size_t i = 0; i < n; ++i) {
        v[i].id = perm[i];
        v[i].memberName = "Member " + std::to_string(perm[i] % 977);
        v[i].bookTitle  = "Book "   + std::to_string(perm[i] % 463);
        v[i].dueDate    = "2026-10-10";
        v[i].returnDate = "2026-10-" + std::to_string(10 + perm[i] % 20);
    }
    return v;
}

template <typename Sorter>
double timeOnce(const std::vector<ReturnRecord>& base, Sorter sorter) {
    auto v = base;                                               // copy outside the timed region
    auto t0 = std::chrono::steady_clock::now();
    sorter(v);
    auto t1 = std::chrono::steady_clock::now();
    if (!std::is_sorted(v.begin(), v.end(), lessId)) { std::cerr << "SORT BROKEN\n"; std::exit(1); }
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

int runBench() {
    std::mt19937 rng(12345);
    std::printf("n,insertion_ms,insertion_ratio,merge_ms,merge_ratio,quick_ms,quick_ratio\n");
    double pi = 0, pm = 0, pq = 0;
    for (std::size_t n = 1000; n <= 64000; n *= 2) {
        double si = 0, sm = 0, sq = 0;
        for (int rep = 0; rep < 3; ++rep) {
            auto base = randomRecords(n, rng);                   // fresh random data each repeat
            si += timeOnce(base, [](auto& v) { insertionSort(v, lessId); });
            sm += timeOnce(base, [](auto& v) { mergeSort(v, lessId); });
            sq += timeOnce(base, [](auto& v) { quickSort(v, lessId); });
        }
        si /= 3; sm /= 3; sq /= 3;
        std::printf("%zu,%.3f,%.2f,%.3f,%.2f,%.3f,%.2f\n", n, si, pi ? si / pi : 0.0, sm, pm ? sm / pm : 0.0, sq, pq ? sq / pq : 0.0);
        std::fflush(stdout);
        pi = si; pm = sm; pq = sq;
    }
    return 0;
}

} // namespace benchmark
