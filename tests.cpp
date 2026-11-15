// tests.cpp - generates the A3 test table (ID | operation | expected | actual | PASS/FAIL).
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "ReturnRegister.h"
#include "Sorting.h"

namespace tests {
// generates the A3 test table (ID | operation | expected | actual | PASS/FAIL)

static const char* FIXTURE = "_returns_test_fixture.txt";
static int passed = 0, failed = 0;
static void row(const char* id, const std::string& op, const std::string& expected, const std::string& actual) {
    bool ok = (expected == actual);
    (ok ? passed : failed)++;
    std::cout << "| " << id << " | " << op << " | " << expected << " | " << actual << " | " << (ok ? "PASS" : "FAIL") << " |\n";
}
static std::string b(bool v) { return v ? "true" : "false"; }
// Braced-init-list elements are evaluated strictly left-to-right (operands of '+' are not),
// so use cat({...}) whenever one operand has a side effect that a later operand observes.
static std::string cat(std::initializer_list<std::string> l) { std::string s; for (auto& x : l) s += x; return s; }

static ReturnRecord mk(int id, const std::string& name = "Member", const std::string& title = "Book",
                       const std::string& due = "2026-10-10", const std::string& ret = "2026-10-12") {
    ReturnRecord r; r.id = id; r.memberName = name; r.bookTitle = title; r.dueDate = due; r.returnDate = ret; return r;
}
static std::string ids(const std::vector<ReturnRecord>& v) {
    std::string s; for (auto& r : v) s += (s.empty() ? "" : ",") + std::to_string(r.id); return s.empty() ? "(none)" : s;
}
static std::string idsFwd(const ReturnList& g) { return ids(g.toVector()); }
static std::string idsBwd(const ReturnList& g) {
    auto v = g.toVector(); std::reverse(v.begin(), v.end()); return ids(v);
}
static bool lessDate(const ReturnRecord& a, const ReturnRecord& b2) { return a.returnDate < b2.returnDate; }
static bool lessId  (const ReturnRecord& a, const ReturnRecord& b2) { return a.id < b2.id; }
static int  cmpDate (const ReturnRecord& r, const std::string& k) { return r.returnDate < k ? -1 : (r.returnDate > k ? 1 : 0); }
static int  cmpId   (const ReturnRecord& r, const int& k) { return r.id < k ? -1 : (r.id > k ? 1 : 0); }

int runTests() {
    writeSampleFile(FIXTURE);
    std::cout << "| Test ID | Input / operation | Expected result | Actual result | Pass/Fail |\n|---|---|---|---|---|\n";

    // ============ A1: list ============
    { ReturnList g;
      row("A1-01", "Empty register: size(), empty(), selfCheck()", "0, true, true", std::to_string(g.size()) + ", " + b(g.empty()) + ", " + b(g.selfCheck()));
      row("A1-02", "removeFirst() on empty", "false", b(g.removeFirst()));
      row("A1-03", "removeLast() on empty", "false", b(g.removeLast()));
      row("A1-04", "removeById(5) on empty", "false", b(removeById(g, 5)));
      std::ostringstream os; printForward(g, os); printBackward(g, os);
      row("A1-05", "Print forward/backward on empty", "\"(register is empty)\" twice", os.str() == "(register is empty)\n(register is empty)\n" ? "\"(register is empty)\" twice" : "other output");
      row("A1-06", "findById(1) on empty", "nullptr", findById(g, 1) ? "found" : "nullptr");
      // one record
      row("A1-07", "pushBack(id 1) into empty", "true, size 1, list 1", cat({b(addBack(g, mk(1))), ", size ", std::to_string(g.size()), ", list ", idsFwd(g)}));
      row("A1-08", "removeFirst() with ONE record (head==tail)", "true, size 0, selfCheck true", cat({b(g.removeFirst()), ", size ", std::to_string(g.size()), ", selfCheck ", b(g.selfCheck())}));
      addBack(g, mk(1));
      row("A1-09", "removeLast() with ONE record", "true, size 0, selfCheck true", cat({b(g.removeLast()), ", size ", std::to_string(g.size()), ", selfCheck ", b(g.selfCheck())}));
      row("A1-10", "pushBack after emptying (reuse)", "true, list 7", cat({b(addBack(g, mk(7))), ", list ", idsFwd(g)}));
    }
    { ReturnList g;
      for (int i = 1; i <= 5; ++i) addBack(g, mk(i));
      row("A1-11", "pushBack 1..5, forward order", "1,2,3,4,5", idsFwd(g));
      row("A1-12", "Duplicate key: pushBack(id 3) again", "false, size 5", cat({b(addBack(g, mk(3))), ", size ", std::to_string(g.size())}));
      row("A1-13", "pushFront(id 0 = invalid id)", "false", b(addFront(g, mk(0))));
      row("A1-14", "pushBack with empty member name", "false", b(addBack(g, mk(9, ""))));
      row("A1-15", "pushBack with impossible date 2026-02-30", "false", b(addBack(g, mk(9, "M", "B", "2026-02-30"))));
      row("A1-16", "pushBack with '|' in title (would corrupt file)", "false", b(addBack(g, mk(9, "M", "A|B"))));
      row("A1-17", "pushFront(id 10); forward order", "10,1,2,3,4,5", addFront(g, mk(10)) ? idsFwd(g) : "rejected");
      ReturnRecord out;
      row("A1-18", "Remove FIRST of 6; removed id; list", "true, 10, 1,2,3,4,5", cat({b(g.removeFirst(&out)), ", ", std::to_string(out.id), ", ", idsFwd(g)}));
      row("A1-19", "Remove LAST of 5; removed id; list", "true, 5, 1,2,3,4", cat({b(g.removeLast(&out)), ", ", std::to_string(out.id), ", ", idsFwd(g)}));
      row("A1-20", "Remove MIDDLE (id 2); forward / backward", "true, 1,3,4 / 4,3,1", cat({b(removeById(g, 2)), ", ", idsFwd(g), " / ", idsBwd(g)}));
      row("A1-21", "removeById(999) absent key", "false, size 3", cat({b(removeById(g, 999)), ", size ", std::to_string(g.size())}));
      row("A1-22", "Links consistent after removals (selfCheck)", "true", b(g.selfCheck()));
    }
    { // deep copy
      ReturnList a; for (int i = 1; i <= 4; ++i) addBack(a, mk(i, "Orig"));
      ReturnList c(a);
      removeById(a, 2); a.removeFirst(); addBack(a, mk(50, "New"));
      row("A1-23", "Copy ctor: mutate ORIGINAL afterwards; copy unchanged", "copy 1,2,3,4 ; orig 3,4,50", "copy " + idsFwd(c) + " ; orig " + idsFwd(a));
      c.removeLast();
      row("A1-24", "Copy ctor: mutate COPY; original unchanged", "orig 3,4,50 ; copy 1,2,3", "orig " + idsFwd(a) + " ; copy " + idsFwd(c));
      row("A1-25", "Copy shares no nodes (findById addresses differ)", "true", b(findById(a, 3) != findById(c, 3)));
      ReturnList e; ReturnList ec(e);
      row("A1-26", "Copy of an EMPTY register", "size 0, selfCheck true", "size " + std::to_string(ec.size()) + ", selfCheck " + b(ec.selfCheck()));
      ReturnList d; addBack(d, mk(99)); d = a;
      row("A1-27", "Copy assignment replaces old contents", "3,4,50", idsFwd(d));
      ReturnList& dr = d; d = dr;
      row("A1-28", "Self-assignment safe", "3,4,50", idsFwd(d));
    }
    { // file I/O
      ReturnList g; int sk = -1;
      int n = loadReturns(g, FIXTURE, &sk);
      row("A1-29", "Load sample file returns.txt", "12 loaded, 0 skipped", std::to_string(n) + " loaded, " + std::to_string(sk) + " skipped");
      row("A1-30", "Loaded list consistent and in file order", "1001..1012, selfCheck true", (idsFwd(g) == "1001,1002,1003,1004,1005,1006,1007,1008,1009,1010,1011,1012" ? "1001..1012" : idsFwd(g)) + ", selfCheck " + b(g.selfCheck()));
      int n2 = loadReturns(g, "no_such_file.txt", &sk);
      row("A1-31", "Load missing file", "-1, list untouched (12)", std::to_string(n2) + ", list untouched (" + std::to_string(g.size()) + ")");
      { std::ofstream f("bad.txt");
        f << "# test\n1|Ann|Book A|2026-10-01|2026-10-02\n"
             "oops line\n2|Bob|Book B|2026-13-01|2026-10-02\n"
             "1|Dup|Book C|2026-10-01|2026-10-02\n"
             "3|Cy||2026-10-01|2026-10-02\n\n4|Di|Book D|2026-10-01|2026-10-05\n"; }
      ReturnList h; int n3 = loadReturns(h, "bad.txt", &sk);
      row("A1-32", "Load file with 4 bad lines (junk, bad date, dup id, empty title) + 2 good", "2 loaded, 4 skipped", std::to_string(n3) + " loaded, " + std::to_string(sk) + " skipped");
      { std::ofstream f("empty.txt"); }
      ReturnList z; int n4 = loadReturns(z, "empty.txt", &sk);
      row("A1-33", "Load an empty file", "0 loaded, empty=true", std::to_string(n4) + " loaded, empty=" + b(z.empty()));
      g.removeFirst(); g.removeLast(); addBack(g, mk(2000, "Zed Z", "Roundtrip", "2026-11-01", "2026-11-05"));
      bool sv = saveReturns(g, "saved.txt");
      ReturnList r2; int n5 = loadReturns(r2, "saved.txt");
      row("A1-34", "Save modified list then reload (round trip)", "true, 11 records, identical order", b(sv) + ", " + std::to_string(n5) + " records, " + (idsFwd(g) == idsFwd(r2) ? "identical order" : "DIFFERENT"));
      row("A1-35", "Save to impossible path /nonexistent/x.txt", "false", b(saveReturns(g, "/nonexistent/x.txt")));
      std::remove("bad.txt"); std::remove("empty.txt"); std::remove("saved.txt");
    }

    // ============ A2: sorting / searching ============
    std::vector<ReturnRecord> e;
    { auto v = e; insertionSort(v, lessDate); mergeSort(e, lessDate); auto q = e; quickSort(q, lessDate);
      row("A2-01", "All three sorts on EMPTY vector", "still empty", (v.empty() && e.empty() && q.empty()) ? "still empty" : "changed"); }
    { std::vector<ReturnRecord> o{mk(1)}, m = o, q = o; insertionSort(o, lessId); mergeSort(m, lessId); quickSort(q, lessId);
      row("A2-02", "All three sorts on ONE record", "1 / 1 / 1", ids(o) + " / " + ids(m) + " / " + ids(q)); }
    auto sortedIn = [] { std::vector<ReturnRecord> v; for (int i = 1; i <= 8; ++i) v.push_back(mk(i)); return v; };
    { auto v = sortedIn(), m = v, q = v; insertionSort(v, lessId); mergeSort(m, lessId); quickSort(q, lessId);
      row("A2-03", "ALREADY SORTED input (1..8), by id", "1..8 for all three", (ids(v) == "1,2,3,4,5,6,7,8" && ids(m) == ids(v) && ids(q) == ids(v)) ? "1..8 for all three" : ids(v) + " | " + ids(m) + " | " + ids(q)); }
    { auto v = sortedIn(); std::reverse(v.begin(), v.end()); auto m = v, q = v;
      insertionSort(v, lessId); mergeSort(m, lessId); quickSort(q, lessId);
      row("A2-04", "REVERSED input (8..1), by id", "1..8 for all three", (ids(v) == "1,2,3,4,5,6,7,8" && ids(m) == ids(v) && ids(q) == ids(v)) ? "1..8 for all three" : ids(v) + " | " + ids(m) + " | " + ids(q)); }
    { ReturnList g; loadReturns(g, FIXTURE);
      auto v = g.toVector(), m = v; insertionSort(v, lessDate); mergeSort(m, lessDate);
      row("A2-05", "Sample file by returnDate; DUPLICATE keys (10-22 x4, 10-25 x5): insertion = merge = original order within ties (stable)",
          "1004,1002,1001,1005,1007,1012,1006,1003,1008,1009,1010,1011", (ids(v) == ids(m) ? ids(v) : "differ: " + ids(v) + " | " + ids(m)));
      auto q = g.toVector(); quickSort(q, lessDate);
      bool sortedOk = std::is_sorted(q.begin(), q.end(), lessDate);
      row("A2-06", "Quicksort by returnDate: keys non-decreasing (tie order NOT guaranteed)", "true", b(sortedOk)); }
    { std::mt19937 rng(42); std::vector<ReturnRecord> base;
      for (int i = 1; i <= 2000; ++i) base.push_back(mk(i, "M", "B", "2026-10-10", "2026-10-" + std::to_string(10 + rng() % 20)));
      auto a = base, m = base, q = base, ref = base;
      insertionSort(a, lessDate); mergeSort(m, lessDate); quickSort(q, lessDate); std::stable_sort(ref.begin(), ref.end(), lessDate);
      row("A2-07", "2000 random records, 20 distinct dates: insertion & merge identical to std::stable_sort", "true, true", b(ids(a) == ids(ref)) + ", " + b(ids(m) == ids(ref)));
      row("A2-08", "Same data: quicksort keys sorted & same multiset of ids", "true", b(std::is_sorted(q.begin(), q.end(), lessDate) && [&]{ auto x=q,y=base; std::sort(x.begin(),x.end(),lessId); std::sort(y.begin(),y.end(),lessId); return ids(x)==ids(y);}()));
      row("A2-09", "Quicksort: ALL keys equal (2000 records) finishes (no O(n^2) blow-up / crash)", "true", [&]{ std::vector<ReturnRecord> s(2000, mk(1)); quickSort(s, lessDate); return b(s.size()==2000);}()); }
    // binary search
    { ReturnList g; loadReturns(g, FIXTURE); auto v = g.toVector(); mergeSort(v, lessDate);
      auto i1 = binarySearch(v, std::string("2026-10-17"), cmpDate);
      row("A2-10", "Binary search present key 2026-10-17 (first element)", "index 0 -> id 1004", "index " + std::to_string(i1) + " -> id " + std::to_string(i1 >= 0 ? v[i1].id : -1));
      auto i2 = binarySearch(v, std::string("2026-10-25"), cmpDate);
      row("A2-11", "Present key 2026-10-25 (LAST value, 5 duplicates) returns LEFTMOST", "index 7 -> id 1003", "index " + std::to_string(i2) + " -> id " + std::to_string(i2 >= 0 ? v[i2].id : -1));
      auto i3 = binarySearch(v, std::string("2026-10-22"), cmpDate);
      row("A2-12", "Duplicate key 2026-10-22 (4 copies) returns first copy", "index 2 -> id 1001", "index " + std::to_string(i3) + " -> id " + std::to_string(i3 >= 0 ? v[i3].id : -1));
      row("A2-13", "ABSENT key 2026-10-23", "-1", std::to_string(binarySearch(v, std::string("2026-10-23"), cmpDate)));
      row("A2-14", "Absent key smaller than all (2026-01-01)", "-1", std::to_string(binarySearch(v, std::string("2026-01-01"), cmpDate)));
      row("A2-15", "Absent key larger than all (2027-01-01)", "-1", std::to_string(binarySearch(v, std::string("2027-01-01"), cmpDate)));
      std::vector<ReturnRecord> none;
      row("A2-16", "Binary search on EMPTY vector", "-1", std::to_string(binarySearch(none, std::string("2026-10-17"), cmpDate)));
      std::vector<ReturnRecord> one{mk(5)};
      row("A2-17", "Binary search ONE record: id 5 present / id 6 absent", "0 / -1", std::to_string(binarySearch(one, 5, cmpId)) + " / " + std::to_string(binarySearch(one, 6, cmpId)));
      std::vector<ReturnRecord> big; for (int i = 1; i <= 1000; ++i) big.push_back(mk(i * 2));
      bool all = true; for (int i = 1; i <= 1000; ++i) all &= (binarySearch(big, i * 2, cmpId) == i - 1);
      bool none2 = true; for (int i = 0; i <= 1000; ++i) none2 &= (binarySearch(big, i * 2 + 1, cmpId) == -1);
      row("A2-18", "1000 sorted ids: every present key found at right index; every absent (odd) key = -1", "true, true", b(all) + ", " + b(none2));
      // round-trip: sort list through register
      ReturnList h; loadReturns(h, FIXTURE); auto hv = h.toVector(); mergeSort(hv, lessDate);
      bool rep = h.replaceWith(hv);
      row("A2-19", "Sort via register.toVector() -> mergeSort -> replaceWith(): list order & links", "true, 1004 first, 1011 last, selfCheck true",
          b(rep) + ", " + std::to_string(h.toVector().front().id) + " first, " + std::to_string(h.toVector().back().id) + " last, selfCheck " + b(h.selfCheck())); }

    std::remove(FIXTURE);
    std::cout << "\nTOTAL: " << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}

} // namespace tests
