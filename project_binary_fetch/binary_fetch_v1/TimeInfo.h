#ifndef TIMEINFO_H
#define TIMEINFO_H

#include <string>
#include <ctime>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

class TimeInfo {
private:
#if defined(_WIN32) || defined(_WIN64)
    SYSTEMTIME systemTime;
#else
    struct tm timeInfo;
#endif

    void updateTime();
    bool isLeapYear(int year) const;
    int calculateWeekNumber() const;

public:
    TimeInfo();

    int getSecond() const;
    int getMinute() const;
    int getHour() const;

    int getDay() const;
    int getWeekNumber() const;
    std::string getDayName() const;
    int getMonthNumber() const;
    std::string getMonthName() const;
    int getYearNumber() const;
    std::string getLeapYear() const;

    void refresh();
};

#endif