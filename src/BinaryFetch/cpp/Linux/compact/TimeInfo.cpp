#include "TimeInfo.h"
#include <chrono>
#include <ctime>

// Constructor initializes the time info
TimeInfo::TimeInfo() {
    updateTime();
}

// Update time attributes using standard C/C++ libraries
void TimeInfo::updateTime() {
    std::time_t now = std::time(nullptr);
    std::tm timeStruct;
    localtime_r(&now, &timeStruct); // Thread-safe POSIX alternative to localtime

    second = timeStruct.tm_sec;
    minute = timeStruct.tm_min;
    hour = timeStruct.tm_hour;
    day = timeStruct.tm_mday;
    monthNumber = timeStruct.tm_mon + 1; // tm_mon is 0-11
    year = timeStruct.tm_year + 1900;    // tm_year is years since 1900

    const char* days[] = {
        "Sunday", "Monday", "Tuesday", "Wednesday",
        "Thursday", "Friday", "Saturday"
    };
    dayName = days[timeStruct.tm_wday];

    const char* months[] = {
        "", "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };
    monthName = months[monthNumber];

    leapYear = isLeapYear(year) ? "Yes" : "No";
    weekNumber = calculateWeekNumber();
}

// Check if a year is a leap year
bool TimeInfo::isLeapYear(int yearValue) const {
    return (yearValue % 4 == 0 && yearValue % 100 != 0) || (yearValue % 400 == 0);
}

// Calculate the week number (1-53) of the year
int TimeInfo::calculateWeekNumber() const {
    std::time_t now = std::time(nullptr);
    std::tm timeStruct;
    localtime_r(&now, &timeStruct);
    
    // tm_yday is 0-365. Adding 1 to make it 1-366.
    int dayOfYear = timeStruct.tm_yday + 1;
    return (dayOfYear + 6) / 7;
}

// Getters
int TimeInfo::getSecond() const { return second; }
int TimeInfo::getMinute() const { return minute; }
int TimeInfo::getHour() const { return hour; }
int TimeInfo::getDay() const { return day; }
int TimeInfo::getWeekNumber() const { return weekNumber; }
std::string TimeInfo::getDayName() const { return dayName; }
int TimeInfo::getMonthNumber() const { return monthNumber; }
std::string TimeInfo::getMonthName() const { return monthName; }
int TimeInfo::getYearNumber() const { return year; }
std::string TimeInfo::getLeapYear() const { return leapYear; }

// Refresh the time information
void TimeInfo::refresh() {
    updateTime();
}
