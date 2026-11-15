// ReturnRegister.h - glue between the generic list and the library scenario.
// All record-specific rules live here: validation, unique ids, lookup/removal by id,
// printing, and the load/save format.
#ifndef RETURN_REGISTER_H
#define RETURN_REGISTER_H

#include <fstream>
#include <iomanip>
#include <ostream>
#include <string>

#include "DoublyLinkedList.h"
#include "ReturnRecord.h"

using ReturnList = DoublyLinkedList<ReturnRecord>;

inline const ReturnRecord* findById(const ReturnList& l, int id) {
    return l.findIf([id](const ReturnRecord& r) { return r.id == id; });
}
// Reject (return false, change nothing) if the record is invalid, the id exists, or memory runs out.
inline bool addBack(ReturnList& l, const ReturnRecord& r) {
    if (!isValidRecord(r) || findById(l, r.id)) return false;
    return l.pushBack(r);
}
inline bool addFront(ReturnList& l, const ReturnRecord& r) {
    if (!isValidRecord(r) || findById(l, r.id)) return false;
    return l.pushFront(r);
}
inline bool removeById(ReturnList& l, int id, ReturnRecord* removed = nullptr) {
    return l.removeIf([id](const ReturnRecord& r) { return r.id == id; }, removed);
}

// ---------------- printing ----------------
inline void printHeader(std::ostream& os) {
    os << std::left << std::setw(6) << "ID" << std::setw(20) << "Member"
       << std::setw(30) << "Book" << std::setw(12) << "Due"
       << std::setw(12) << "Returned" << std::right << std::setw(6) << "Late"
       << std::setw(10) << "Fine(UGX)" << "\n";
}
inline void printRow(std::ostream& os, const ReturnRecord& r) {
    os << std::left << std::setw(6) << r.id << std::setw(20) << r.memberName.substr(0, 19)
       << std::setw(30) << r.bookTitle.substr(0, 29) << std::setw(12) << r.dueDate
       << std::setw(12) << r.returnDate << std::right << std::setw(6) << r.daysLate()
       << std::setw(10) << r.fineUGX() << "\n";
}
inline void printForward(const ReturnList& l, std::ostream& os) {
    if (l.empty()) { os << "(register is empty)\n"; return; }
    printHeader(os);
    l.forEach([&os](const ReturnRecord& r) { printRow(os, r); });
}
inline void printBackward(const ReturnList& l, std::ostream& os) {
    if (l.empty()) { os << "(register is empty)\n"; return; }
    printHeader(os);
    l.forEachReverse([&os](const ReturnRecord& r) { printRow(os, r); });
}

// ---------------- file I/O ----------------
// load: number of records loaded, or -1 if the file cannot be opened. Bad lines and duplicate ids are skipped.
inline int loadReturns(ReturnList& l, const std::string& path, int* skipped = nullptr) {
    return l.load(path, parseReturnLine,
                  [](const ReturnRecord& r, const ReturnList& soFar) { return findById(soFar, r.id) == nullptr; },
                  skipped);
}
inline bool saveReturns(const ReturnList& l, const std::string& path) {
    return l.save(path, formatReturnLine, "# id|member|title|due|returned");
}

// ---------------- built-in sample data (12 records) so the program runs with no extra file ----------------
inline const char* sampleData() {
    return R"DATA(# Library returns register sample data
# id|member|title|due|returned
1001|Achieng Grace|Things Fall Apart|2026-10-20|2026-10-22
1002|Okello David|Introduction to Algorithms|2026-10-18|2026-10-18
1003|Nakato Sarah|Data Structures in C++|2026-10-21|2026-10-25
1004|Mugisha Peter|The River Between|2026-10-19|2026-10-17
1005|Namukasa Ruth|Operating System Concepts|2026-10-22|2026-10-22
1006|Ssemwogerere Ivan|Clean Code|2026-10-15|2026-10-24
1007|Atim Joan|Weep Not, Child|2026-10-23|2026-10-22
1008|Kato Brian|Computer Networks|2026-10-20|2026-10-25
1009|Nabirye Esther|Database System Concepts|2026-10-24|2026-10-25
1010|Opio Samuel|The Pragmatic Programmer|2026-10-17|2026-10-25
1011|Tumusiime Alice|Discrete Mathematics|2026-10-25|2026-10-25
1012|Byaruhanga Moses|Artificial Intelligence: A Modern Approach|2026-10-16|2026-10-22
)DATA";
}
inline void writeSampleFile(const std::string& path) {
    std::ofstream f(path, std::ios::trunc);
    f << sampleData();
}
inline bool fileExists(const std::string& path) { std::ifstream f(path); return static_cast<bool>(f); }

#endif // RETURN_REGISTER_H
