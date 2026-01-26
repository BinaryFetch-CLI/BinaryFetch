#ifndef TIMEINFO_H
#define TIMEINFO_H

#include <string>
#include <windows.h>

class TimeInfo {
private:
    SYSTEMTIME systemTime;

    void updateTime();
    bool isLeapYear(int year) const;
    int calculateWeekNumber() const;

public:
    // Constructor - automatically fetches current system time
    TimeInfo();

    // Time components
    int getSecond() const;
    int getMinute() const;
    int getHour() const;

    // Date components
    int getDay() const;
    int getWeekNumber() const;
    std::string getDayName() const;
    int getMonthNumber() const;
    std::string getMonthName() const;
    int getYearNumber() const;
    std::string getLeapYear() const;  // Returns "Yes" or "No"

    // Refresh system time
    void refresh();
};

#endif