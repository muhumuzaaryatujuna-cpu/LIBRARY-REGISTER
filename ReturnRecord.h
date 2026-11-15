// ReturnRecord.h - the scenario record (a book return slip) plus its validation helpers.
#ifndef RETURN_RECORD_H
#define RETURN_RECORD_H

#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

struct ReturnRecord {
    int         id = 0;          // unique return-slip number (also the arrival sequence)
    std::string memberName;      // borrower
    std::string bookTitle;       // book returned
    std::string dueDate;         // YYYY-MM-DD
    std::string returnDate;      // YYYY-MM-DD

    int  daysLate() const;       // 0 if returned on/before the due date
    long fineUGX()  const;       // 500 UGX per late day
};

// ---------------- date helpers (pure functions, never throw) ----------------
inline bool parseDate(const std::string& s, int& y, int& m, int& d) {
    if (s.size() != 10 || s[4] != '-' || s[7] != '-') return false;
    for (int i : {0, 1, 2, 3, 5, 6, 8, 9})
        if (s[i] < '0' || s[i] > '9') return false;
    y = std::atoi(s.substr(0, 4).c_str());
    m = std::atoi(s.substr(5, 2).c_str());
    d = std::atoi(s.substr(8, 2).c_str());
    if (y < 1900 || y > 2200 || m < 1 || m > 12 || d < 1) return false;
    static const int dim[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
    int maxd = dim[m - 1] + ((m == 2 && leap) ? 1 : 0);
    return d <= maxd;
}
inline bool isValidDate(const std::string& s) { int y, m, d; return parseDate(s, y, m, d); }

// days since 1970-01-01, or -1 if the date is invalid (days-from-civil algorithm)
inline long dayNumber(const std::string& s) {
    int y, m, d;
    if (!parseDate(s, y, m, d)) return -1;
    y -= m <= 2;
    long era = (y >= 0 ? y : y - 399) / 400;
    long yoe = y - era * 400;
    long doy = (153L * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}

inline int ReturnRecord::daysLate() const {
    long due = dayNumber(dueDate), ret = dayNumber(returnDate);
    if (due < 0 || ret < 0 || ret <= due) return 0;
    return static_cast<int>(ret - due);
}
inline long ReturnRecord::fineUGX() const { return 500L * daysLate(); }

// ---------------- validation ----------------
inline bool cleanText(const std::string& s) {      // non-empty, no '|' or line breaks
    if (s.empty()) return false;
    bool anyNonSpace = false;
    for (char c : s) {
        if (c == '|' || c == '\n' || c == '\r') return false;
        if (c != ' ' && c != '\t') anyNonSpace = true;
    }
    return anyNonSpace;
}
inline bool isValidRecord(const ReturnRecord& r) {   // id>0, clean text, real dates
    return r.id > 0 && cleanText(r.memberName) && cleanText(r.bookTitle)
        && isValidDate(r.dueDate) && isValidDate(r.returnDate);
}

// ---------------- text file format: id|member|title|due|returned ----------------
inline bool parseReturnLine(const std::string& line, ReturnRecord& r) {
    std::vector<std::string> f;
    std::string cur;
    std::istringstream ss(line);
    while (std::getline(ss, cur, '|')) f.push_back(cur);
    if (!line.empty() && line.back() == '|') f.push_back("");
    if (f.size() != 5) return false;
    char* end = nullptr;
    long id = std::strtol(f[0].c_str(), &end, 10);
    if (f[0].empty() || *end != '\0' || id <= 0 || id > 2000000000L) return false;
    r.id = static_cast<int>(id);
    r.memberName = f[1]; r.bookTitle = f[2]; r.dueDate = f[3]; r.returnDate = f[4];
    return isValidRecord(r);
}
inline std::string formatReturnLine(const ReturnRecord& r) {
    return std::to_string(r.id) + '|' + r.memberName + '|' + r.bookTitle + '|' + r.dueDate + '|' + r.returnDate;
}

#endif // RETURN_RECORD_H
