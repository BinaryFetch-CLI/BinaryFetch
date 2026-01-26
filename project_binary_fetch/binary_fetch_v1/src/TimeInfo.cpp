#include "TimeInfo.h"

// Constructor - gets current system time
TimeInfo::TimeInfo() {
    updateTime();
}

// Private method to fetch current system time
void TimeInfo::updateTime() {
    GetLocalTime(&systemTime);
}

// Check if a year is a leap year
bool TimeInfo::isLeapYear(int year) const {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// Calculate ISO week number
int TimeInfo::calculateWeekNumber() const {
    // Get January 1st of current year
    SYSTEMTIME jan1;
    jan1.wYear = systemTime.wYear;
    jan1.wMonth = 1;
    jan1.wDay = 1;
    jan1.wHour = 0;
    jan1.wMinute = 0;
    jan1.wSecond = 0;
    jan1.wMilliseconds = 0;

    FILETIME ft1, ft2;
    SystemTimeToFileTime(&jan1, &ft1);
    SystemTimeToFileTime(&systemTime, &ft2);

    ULARGE_INTEGER uli1, uli2;
    uli1.LowPart = ft1.dwLowDateTime;
    uli1.HighPart = ft1.dwHighDateTime;
    uli2.LowPart = ft2.dwLowDateTime;
    uli2.HighPart = ft2.dwHighDateTime;

    // Calculate day of year
    ULONGLONG diff = uli2.QuadPart - uli1.QuadPart;
    int dayOfYear = (int)(diff / 10000000ULL / 86400ULL) + 1;

    // Calculate week number (simple calculation)
    int weekNumber = (dayOfYear + 6) / 7;

    return weekNumber;
}

// Get second
int TimeInfo::getSecond() const {
    return systemTime.wSecond;
}

// Get minute
int TimeInfo::getMinute() const {
    return systemTime.wMinute;
}

// Get hour
int TimeInfo::getHour() const {
    return systemTime.wHour;
}

// Get day
int TimeInfo::getDay() const {
    return systemTime.wDay;
}

// Get week number
int TimeInfo::getWeekNumber() const {
    return calculateWeekNumber();
}

// Get day name
std::string TimeInfo::getDayName() const {
    const char* days[] = { "Sunday", "Monday", "Tuesday", "Wednesday",
                          "Thursday", "Friday", "Saturday" };
    return days[systemTime.wDayOfWeek];
}

// Get month number
int TimeInfo::getMonthNumber() const {
    return systemTime.wMonth;
}

// Get month name
std::string TimeInfo::getMonthName() const {
    const char* months[] = { "", "January", "February", "March", "April",
                            "May", "June", "July", "August", "September",
                            "October", "November", "December" };
    return months[systemTime.wMonth];
}

// Get year number
int TimeInfo::getYearNumber() const {
    return systemTime.wYear;
}

// Get leap year status
std::string TimeInfo::getLeapYear() const {
    return isLeapYear(systemTime.wYear) ? "Yes" : "No";
}

// Refresh the system time
void TimeInfo::refresh() {
    updateTime();
}