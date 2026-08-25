#ifndef TIMEINFO_H
#define TIMEINFO_H

#include <string>

class TimeInfo {
private:
    int second;
    int minute;
    int hour;

    int day;
    int weekNumber;
    int monthNumber;
    int year;

    std::string dayName;
    std::string monthName;
    std::string leapYear;

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