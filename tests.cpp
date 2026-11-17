// A3 test table generator. Build: g++ -std=c++17 -Wall -Wextra -g -fsanitize=address,undefined tests.cpp -o tests
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm> // std::stable_sort used ONLY as a reference in test code
#include <fstream>
#include <functional>
#include "Config.h"
#include "DynamicArray.h"
#include "StudentRecord.h"
#include "Sorting.h"

static int total = 0, passed = 0;

static void report(const std::string &id, const std::string &op, const std::string &expected, const std::string &actual)
{
    bool ok = (expected == actual);
    ++total;
    if (ok) ++passed;
    auto shorten = [](const std::string &x) { return x.size() > 24 ? x.substr(0, 21) + "..." : x; }; // display only
    std::cout << std::left << std::setw(7) << id << std::setw(66) << op << std::setw(26) << shorten(expected)
              << std::setw(26) << shorten(actual) << (ok ? "PASS" : "FAIL") << "\n";
}

static StudentRecord mk(int id, double score) { return StudentRecord{id, "N" + std::to_string(id), "C", score}; }

static std::string ids(const DynamicArray<StudentRecord> &a)
{
    if (a.empty()) return "(empty)";
    std::string s;
    for (int i = 0; i < a.size(); ++i) s += (i ? "," : "") + std::to_string(a.at(i).id);
    return s;
}
static std::string scoresOf(const DynamicArray<StudentRecord> &a)
{
    if (a.empty()) return "(empty)";
    std::string s;
    for (int i = 0; i < a.size(); ++i) s += (i ? "," : "") + std::to_string((int)a.at(i).score);
    return s;
}
static std::string rejects(const std::function<void()> &f)
{
    try { f(); return "no exception"; }
    catch (const std::out_of_range &) { return "out_of_range"; }
    catch (const std::runtime_error &) { return "runtime_error"; }
}

static DynamicArray<StudentRecord> build(const std::vector<double> &scores)
{
    DynamicArray<StudentRecord> a;
    for (size_t i = 0; i < scores.size(); ++i) a.push_back(mk((int)i + 1, scores[i]));
    return a;
}

static void sortCase(const std::string &tid, const std::string &label, const std::vector<double> &scores)
{
    // Reference result (test code only): std::stable_sort on a std::vector
    std::vector<StudentRecord> ref;
    for (size_t i = 0; i < scores.size(); ++i) ref.push_back(mk((int)i + 1, scores[i]));
    std::stable_sort(ref.begin(), ref.end(), ByScore{});
    std::string refIds, refScores;
    for (size_t i = 0; i < ref.size(); ++i)
    {
        refIds += (i ? "," : "") + std::to_string(ref[i].id);
        refScores += (i ? "," : "") + std::to_string((int)ref[i].score);
    }
    if (ref.empty()) { refIds = "(empty)"; refScores = "(empty)"; }

    DynamicArray<StudentRecord> a = build(scores), b = build(scores);
    insertionSort(a.raw(), a.size(), ByScore{});
    fastSort(b.raw(), b.size(), ByScore{});
    report(tid + "a", "insertionSort: " + label + " [ids => stability]", refIds, ids(a));
    if (USE_MERGE)
        report(tid + "b", "mergeSort: " + label + "", refIds, ids(b));
    else
        report(tid + "b", "quickSortRand: " + label + " (scores)", refScores, scoresOf(b));
}

int main()
{
    std::cout << std::left << std::setw(7) << "ID" << std::setw(66) << "Input / operation" << std::setw(26) << "Expected"
              << std::setw(26) << "Actual" << "Result\n" << std::string(125, '-') << "\n";

    // ---------------- A1 ----------------
    DynamicArray<StudentRecord> e;
    report("A1-01", "new array: size()", "0", std::to_string(e.size()));
    report("A1-02", "empty: removeAt(0)", "out_of_range", rejects([&] { e.removeAt(0); }));
    report("A1-03", "empty: at(0)", "out_of_range", rejects([&] { e.at(0); }));
    e.push_back(mk(1, 50));
    report("A1-04", "one record: size() after push_back", "1", std::to_string(e.size()));
    report("A1-05", "one record: capacity", "1", std::to_string(e.getCapacity()));
    e.removeAt(0);
    report("A1-06", "one record: removeAt(0) -> empty", "(empty)", ids(e));

    DynamicArray<StudentRecord> a;
    for (int i = 1; i <= 5; ++i) a.push_back(mk(i, i * 10));
    report("A1-07", "push_back x5: capacity doubles 1,2,4,8", "8", std::to_string(a.getCapacity()));
    report("A1-08", "removeAt(-1)", "out_of_range", rejects([&] { a.removeAt(-1); }));
    report("A1-09", "removeAt(size())", "out_of_range", rejects([&] { a.removeAt(a.size()); }));
    a.removeAt(0);
    report("A1-10", "remove FIRST of [1..5]", "2,3,4,5", ids(a));
    a.removeAt(a.size() - 1);
    report("A1-11", "remove LAST of [2..5]", "2,3,4", ids(a));
    a.insertAt(1, mk(9, 0));
    report("A1-12", "insertAt(1, id 9)", "2,9,3,4", ids(a));
    report("A1-13", "insertAt(size()+1)", "out_of_range", rejects([&] { a.insertAt(a.size() + 1, mk(8, 0)); }));

    DynamicArray<StudentRecord> c(a);
    c.at(0).score = -5;
    report("A1-14", "copy ctor is deep: change copy, original intact", "20", std::to_string((int)a.at(0).score));
    DynamicArray<StudentRecord> d;
    d = a;
    d.at(1).id = 777;
    report("A1-15", "copy assignment is deep", "2,9,3,4", ids(a));
    d = d;
    report("A1-16", "self-assignment safe", "2,777,3,4", ids(d));

    DynamicArray<StudentRecord> f; // full array, then push_back of its own element (aliasing)
    f.push_back(mk(1, 1)); f.push_back(mk(2, 2)); // capacity 2, full
    f.push_back(f.at(0));
    report("A1-17", "push_back(own element) while full", "1,2,1", ids(f));

    DynamicArray<StudentRecord> s;
    for (int i = 1; i <= 16; ++i) s.push_back(mk(i, i));
    while (s.size() > 1) s.removeAt(s.size() - 1);
    report("A1-18", "shrink: 16 -> 1 records, capacity", "4", std::to_string(s.getCapacity()));
    report("A1-19", "shrink: surviving record intact", "1", ids(s));

    { std::ofstream t("test_tmp.txt");
      for (int i = 1; i <= 10; ++i) t << 2000 + i << ",Student" << i << ",BCS2204," << 50 + i << "\n";
      t << "\n" << "abc,Bad,BCS2204,50\n" << "2003,Dup,BCS2204,60\n" << "2050,Range,BCS2204,150\n" << "2051,Short\n"; }
    DynamicArray<StudentRecord> L;
    std::cerr << "(expected skip messages from A1-20:)\n";
    report("A1-20", "load file: 10 good + 4 bad + 1 duplicate id", "10", std::to_string(loadRecords(L, "test_tmp.txt")));
    saveRecords(L, "test_tmp_out.txt");
    DynamicArray<StudentRecord> L2;
    loadRecords(L2, "test_tmp_out.txt");
    report("A1-21", "save then reload gives same records", ids(L), ids(L2));
    report("A1-22", "load missing file", "runtime_error", rejects([&] { DynamicArray<StudentRecord> z; loadRecords(z, "no_such_file.txt"); }));

    // ---------------- A2 ----------------
    sortCase("A2-01", "empty", {});
    sortCase("A2-02", "one record", {42});
    sortCase("A2-03", "duplicate keys 70,60,70,60,70", {70, 60, 70, 60, 70});
    sortCase("A2-04", "already sorted", {1, 2, 3, 4, 5});
    sortCase("A2-05", "reversed", {5, 4, 3, 2, 1});
    sortCase("A2-06", "all equal", {50, 50, 50, 50});
    std::vector<double> rnd;
    for (int i = 0; i < 200; ++i) rnd.push_back((i * 7919) % 101);
    sortCase("A2-07", "200 pseudo-random values", rnd);

    DynamicArray<StudentRecord> sorted = build({10, 20, 30, 30, 30, 40, 50});
    auto eL = [](const StudentRecord &r, double k) { return r.score < k; };
    auto kL = [](double k, const StudentRecord &r) { return k < r.score; };
    report("A2-08", "binarySearch empty array", "-1", std::to_string(binarySearch(e.raw(), 0, 5.0, eL, kL)));
    DynamicArray<StudentRecord> one = build({10});
    report("A2-09", "binarySearch one record, present", "0", std::to_string(binarySearch(one.raw(), 1, 10.0, eL, kL)));
    report("A2-10", "binarySearch one record, absent", "-1", std::to_string(binarySearch(one.raw(), 1, 11.0, eL, kL)));
    report("A2-11", "binarySearch first element (10)", "0", std::to_string(binarySearch(sorted.raw(), sorted.size(), 10.0, eL, kL)));
    report("A2-12", "binarySearch last element (50)", "6", std::to_string(binarySearch(sorted.raw(), sorted.size(), 50.0, eL, kL)));
    report("A2-13", "binarySearch duplicate key 30 -> FIRST index", "2", std::to_string(binarySearch(sorted.raw(), sorted.size(), 30.0, eL, kL)));
    report("A2-14", "binarySearch absent key 35", "-1", std::to_string(binarySearch(sorted.raw(), sorted.size(), 35.0, eL, kL)));
    report("A2-15", "binarySearch absent key below min (5)", "-1", std::to_string(binarySearch(sorted.raw(), sorted.size(), 5.0, eL, kL)));
    report("A2-16", "binarySearch absent key above max (99)", "-1", std::to_string(binarySearch(sorted.raw(), sorted.size(), 99.0, eL, kL)));

    std::cout << std::string(125, '-') << "\nPassed " << passed << " / " << total << "\n";
    std::remove("test_tmp.txt"); std::remove("test_tmp_out.txt");
    return passed == total ? 0 : 1;
}
