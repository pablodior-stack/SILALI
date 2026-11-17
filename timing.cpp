// A4 timing experiment. Build WITHOUT sanitizers:  g++ -std=c++17 -O2 timing.cpp -o timing
#include <iostream>
#include <iomanip>
#include <fstream>
#include <chrono>
#include <random>
#include "Config.h"
#include "DynamicArray.h"
#include "StudentRecord.h"
#include "Sorting.h"

int main()
{
    using clk = std::chrono::steady_clock;
    const std::string fastName = fastSortName();
    std::ofstream csv("timing.csv");
    csv << "n,insertion_ms," << fastName << "_ms,insertion_ratio," << fastName << "_ratio\n";
    std::cout << std::setw(8) << "n" << std::setw(16) << "insertion(ms)" << std::setw(18) << (fastName + "(ms)")
              << std::setw(14) << "ins ratio" << std::setw(14) << "fast ratio" << "\n";

    std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist(0.0, 100.0);
    double prevIns = 0, prevFast = 0;
    for (int n = 1000; n <= 64000; n *= 2)
    {
        double insTotal = 0, fastTotal = 0;
        for (int run = 0; run < 3; ++run)
        {
            DynamicArray<StudentRecord> a, b;
            for (int i = 0; i < n; ++i)
            {
                StudentRecord r{i, "S", "C", dist(rng)};
                a.push_back(r);
                b.push_back(r); // identical data for both sorts
            }
            auto t0 = clk::now();
            insertionSort(a.raw(), a.size(), ByScore{});
            auto t1 = clk::now();
            fastSort(b.raw(), b.size(), ByScore{});
            auto t2 = clk::now();
            insTotal += std::chrono::duration<double, std::milli>(t1 - t0).count();
            fastTotal += std::chrono::duration<double, std::milli>(t2 - t1).count();
        }
        double insAvg = insTotal / 3.0, fastAvg = fastTotal / 3.0;
        double insRatio = prevIns > 0 ? insAvg / prevIns : 0, fastRatio = prevFast > 0 ? fastAvg / prevFast : 0;
        std::cout << std::setw(8) << n << std::fixed << std::setprecision(3) << std::setw(16) << insAvg
                  << std::setw(18) << fastAvg << std::setw(14) << insRatio << std::setw(14) << fastRatio << "\n";
        csv << n << ',' << insAvg << ',' << fastAvg << ',' << insRatio << ',' << fastRatio << '\n';
        prevIns = insAvg; prevFast = fastAvg;
    }
    std::cout << "Wrote timing.csv (plot n vs time in Excel for the A4 chart)\n";
    return 0;
}
