// Demo of A1 + A2. Build: g++ -std=c++17 -Wall -Wextra -g -fsanitize=address,undefined main.cpp -o register
#include <iostream>
#include <iomanip>
#include "Config.h"
#include "DynamicArray.h"
#include "StudentRecord.h"
#include "Sorting.h"

static void printAll(const std::string &title, const DynamicArray<StudentRecord> &a)
{
    std::cout << title << " (" << a.size() << " records)\n";
    for (int i = 0; i < a.size(); ++i)
        std::cout << "  " << std::setw(5) << a.at(i).id << "  " << std::left << std::setw(18) << a.at(i).name
                  << std::right << std::setw(9) << a.at(i).course << "  " << a.at(i).score << "\n";
}

int main()
{
    try
    {
        DynamicArray<StudentRecord> reg;
        int loaded = loadRecords(reg, "students.txt");
        std::cout << "Loaded " << loaded << " records (capacity " << reg.getCapacity() << ")\n";
        printAll("Original order", reg);

        // A1: update, insert, remove, then invalid operations are rejected safely
        reg.at(0).score = 91.5;
        reg.insertAt(1, StudentRecord{1099, "New Student", "BCS2204", 70.0});
        reg.removeAt(reg.size() - 1);
        try { reg.removeAt(100); }
        catch (const std::out_of_range &e) { std::cout << "Rejected safely: " << e.what() << "\n"; }

        DynamicArray<StudentRecord> copy(reg); // deep copy check
        copy.at(0).score = -1;
        std::cout << "Deep copy: original[0]=" << reg.at(0).score << " copy[0]=" << copy.at(0).score << "\n\n";

        // A2: each sort runs on its own copy of the SAME unsorted data
        DynamicArray<StudentRecord> ins(reg), fast(reg);
        insertionSort(ins.raw(), ins.size(), ByScore{});
        fastSort(fast.raw(), fast.size(), ByScore{});
        printAll("insertionSort by score (stable: equal scores keep original order)", ins);
        printAll(std::string(fastSortName()) + " by score", fast);

        auto elemLessScore = [](const StudentRecord &r, double k) { return r.score < k; };
        auto scoreLessElem = [](double k, const StudentRecord &r) { return k < r.score; };
        for (double probe : {75.0, 85.5, 999.0})
        {
            int idx = binarySearch(ins.raw(), ins.size(), probe, elemLessScore, scoreLessElem);
            std::cout << "binarySearch(score=" << probe << ") -> "
                      << (idx >= 0 ? "first match at index " + std::to_string(idx) : std::string("absent")) << "\n";
        }

        // Sort by another field (id) and search by id
        fastSort(fast.raw(), fast.size(), ById{});
        auto elemLessId = [](const StudentRecord &r, int k) { return r.id < k; };
        auto idLessElem = [](int k, const StudentRecord &r) { return k < r.id; };
        int idx = binarySearch(fast.raw(), fast.size(), 1005, elemLessId, idLessElem);
        std::cout << "binarySearch(id=1005) -> " << (idx >= 0 ? "index " + std::to_string(idx) : std::string("absent")) << "\n";

        saveRecords(reg, "students_out.txt");
        std::cout << "Saved updated records to students_out.txt\n";
    }
    catch (const std::exception &ex)
    {
        std::cerr << "FATAL: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
