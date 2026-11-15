// main.cpp - Library Books Returns Register: interactive menu + entry point.
//   Build:  g++ -std=c++17 -O2 -Wall -Wextra main.cpp tests.cpp bench.cpp -o library_register
//   Run:    ./library_register            interactive menu (creates returns.txt if missing)
//           ./library_register test       runs the A3 test table
//           ./library_register bench      runs the A4 timing experiment
//           ./library_register file.txt   menu using another data file
#include <cstdlib>
#include <iostream>
#include <string>

#include "ReturnRegister.h"
#include "Sorting.h"

namespace tests     { int runTests(); }
namespace benchmark { int runBench(); }

namespace menu_app {

// ---- comparators (strict less-than). Ties are broken by id = arrival order, so even the
//      unstable quicksort gives a deterministic, arrival-preserving result. ----
static bool byReturnDate(const ReturnRecord& a, const ReturnRecord& b) {
    if (a.returnDate != b.returnDate) return a.returnDate < b.returnDate;   // ISO dates sort as text
    return false;                                                           // pure key (stability test)
}
static bool byReturnDateThenId(const ReturnRecord& a, const ReturnRecord& b) {
    if (a.returnDate != b.returnDate) return a.returnDate < b.returnDate;
    return a.id < b.id;
}
static bool byMember(const ReturnRecord& a, const ReturnRecord& b) { return a.memberName < b.memberName; }
static bool byTitle (const ReturnRecord& a, const ReturnRecord& b) { return a.bookTitle  < b.bookTitle; }
static bool byFineDesc(const ReturnRecord& a, const ReturnRecord& b) { return a.fineUGX() > b.fineUGX(); }
static bool byId    (const ReturnRecord& a, const ReturnRecord& b) { return a.id < b.id; }

static std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string s;
    if (!std::getline(std::cin, s)) { std::cout << "\n(input closed)\n"; std::exit(0); }
    return s;
}
static int readInt(const std::string& prompt) {      // returns -1 on bad input
    std::string s = readLine(prompt);
    try { std::size_t p; int v = std::stoi(s, &p); return p == s.size() ? v : -1; }
    catch (...) { return -1; }
}

static void sortMenu(ReturnList& reg) {
    std::cout << " 1 return date (stable)  2 member  3 title  4 fine (desc)  5 id\n";
    int k = readInt("Sort by: ");
    int alg = readInt(" Algorithm: 1 insertion  2 O(n log n) sort: ");
    if (k < 1 || k > 5 || (alg != 1 && alg != 2)) { std::cout << "Invalid choice.\n"; return; }
    auto v = reg.toVector();
    auto run = [&](auto cmp) { if (alg == 1) insertionSort(v, cmp); else fastSort(v, cmp); };
    switch (k) {
        case 1: run(byReturnDate);  break;
        case 2: run(byMember);      break;
        case 3: run(byTitle);       break;
        case 4: run(byFineDesc);    break;
        case 5: run(byId);          break;
    }
    if (reg.replaceWith(v)) { std::cout << "Sorted.\n"; printForward(reg, std::cout); }
    else std::cout << "Sort failed; register unchanged.\n";
}

static void searchMenu(ReturnList& reg) {
    if (reg.empty()) { std::cout << "Register is empty.\n"; return; }
    std::string d = readLine("Return date to search (YYYY-MM-DD): ");
    if (!isValidDate(d)) { std::cout << "Invalid date.\n"; return; }
    auto v = reg.toVector();
    mergeSort(v, byReturnDateThenId);                 // binary search needs sorted-by-key data
    auto idx = binarySearch(v, d, [](const ReturnRecord& r, const std::string& key) {
        return r.returnDate < key ? -1 : (r.returnDate > key ? 1 : 0); });
    if (idx < 0) { std::cout << "No returns on " << d << ".\n"; return; }
    std::cout << "Returns on " << d << ":\n";
    for (std::size_t i = idx; i < v.size() && v[i].returnDate == d; ++i)
        std::cout << "  #" << v[i].id << "  " << v[i].memberName << "  -  " << v[i].bookTitle
                  << "  (fine " << v[i].fineUGX() << " UGX)\n";
}

int runMenu(const std::string& path) {
    ReturnList reg;
    int skipped = 0, n = loadReturns(reg, path, &skipped);
    if (n < 0) std::cout << "Could not open '" << path << "' - starting with an empty register.\n";
    else       std::cout << "Loaded " << n << " record(s) from '" << path << "' (" << skipped << " skipped).\n";

    for (;;) {
        std::cout << "\n=== Library Returns Register (" << reg.size() << " records) ===\n"
                     " 1 Add return (end)   2 Add return (front)  3 Remove by id\n"
                     " 4 Remove first       5 Remove last         6 Find by id\n"
                     " 7 Show forward       8 Show backward       9 Sort\n"
                     "10 Search by return date (binary)  11 Save  12 Reload  0 Save & quit\n";
        int c = readInt("Choice: ");
        switch (c) {
            case 1: case 2: {
                ReturnRecord r;
                r.id = readInt("Slip id (positive int): ");
                r.memberName = readLine("Member name: ");
                r.bookTitle  = readLine("Book title: ");
                r.dueDate    = readLine("Due date (YYYY-MM-DD): ");
                r.returnDate = readLine("Return date (YYYY-MM-DD): ");
                bool ok = (c == 1) ? addBack(reg, r) : addFront(reg, r);
                std::cout << (ok ? "Added.\n" : "Rejected: invalid field or duplicate id.\n");
                break; }
            case 3: { int id = readInt("Id to remove: ");
                      std::cout << (removeById(reg, id) ? "Removed.\n" : "No such id.\n"); break; }
            case 4: std::cout << (reg.removeFirst() ? "Removed first.\n" : "Register is empty.\n"); break;
            case 5: std::cout << (reg.removeLast()  ? "Removed last.\n"  : "Register is empty.\n"); break;
            case 6: { int id = readInt("Id: "); const ReturnRecord* r = findById(reg, id);
                      if (r) std::cout << r->memberName << " returned '" << r->bookTitle << "' on "
                                       << r->returnDate << ", fine " << r->fineUGX() << " UGX\n";
                      else   std::cout << "Not found.\n";
                      break; }
            case 7: printForward(reg, std::cout);  break;
            case 8: printBackward(reg, std::cout); break;
            case 9: sortMenu(reg);   break;
            case 10: searchMenu(reg); break;
            case 11: std::cout << (saveReturns(reg, path) ? "Saved.\n" : "Save FAILED.\n"); break;
            case 12: { int sk = 0; int m = loadReturns(reg, path, &sk);
                       if (m < 0) std::cout << "Cannot open file.\n";
                       else std::cout << "Reloaded " << m << " (" << sk << " skipped).\n";
                       break; }
            case 0: std::cout << (saveReturns(reg, path) ? "Saved. Goodbye.\n" : "Save FAILED. Goodbye.\n"); return 0;
            default: std::cout << "Invalid choice.\n";
        }
    }
}

} // namespace menu_app

int main(int argc, char** argv) {
    std::string arg = argc > 1 ? argv[1] : "";
    if (arg == "test")  return tests::runTests();
    if (arg == "bench") return benchmark::runBench();
    std::string path = arg.empty() ? "returns.txt" : arg;
    if (!fileExists(path)) {                       // first run: create the sample file
        writeSampleFile(path);
        std::cout << "Created sample data file '" << path << "'.\n";
    }
    return menu_app::runMenu(path);
}
