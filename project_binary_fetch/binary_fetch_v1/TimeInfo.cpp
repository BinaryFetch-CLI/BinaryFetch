#include "TimeInfo.h"

TimeInfo::TimeInfo() {
    updateTime();
}

void TimeInfo::updateTime() {
#if defined(_WIN32) || defined(_WIN64)
    GetLocalTime(&systemTime);
#else
    time_t now = time(nullptr);
    localtime_r(&now, &timeInfo);
#endif
}

bool TimeInfo::isLeapYear(int year) const {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int TimeInfo::calculateWeekNumber() const {
#if defined(_WIN32) || defined(_WIN64)
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

    ULONGLONG diff = uli2.QuadPart - uli1.QuadPart;
    int dayOfYear = (int)(diff / 10000000ULL / 86400ULL) + 1;
    int weekNumber = (dayOfYear + 6) / 7;
    return weekNumber;
#else
    int dayOfYear = timeInfo.tm_yday + 1;
    int weekNumber = (dayOfYear + 6) / 7;
    return weekNumber;
#endif
}

int TimeInfo::getSecond() const {
#if defined(_WIN32) || defined(_WIN64)
    return systemTime.wSecond;
#else
    return timeInfo.tm_sec;
#endif
}

int TimeInfo::getMinute() const {
#if defined(_WIN32) || defined(_WIN64)
    return systemTime.wMinute;
#else
    return timeInfo.tm_min;
#endif
}

int TimeInfo::getHour() const {
#if defined(_WIN32) || defined(_WIN64)
    return systemTime.wHour;
#else
    return timeInfo.tm_hour;
#endif
}

int TimeInfo::getDay() const {
#if defined(_WIN32) || defined(_WIN64)
    return systemTime.wDay;
#else
    return timeInfo.tm_mday;
#endif
}

int TimeInfo::getWeekNumber() const {
    return calculateWeekNumber();
}

std::string TimeInfo::getDayName() const {
    const char* days[] = { "Sunday", "Monday", "Tuesday", "Wednesday",
                          "Thursday", "Friday", "Saturday" };
#if defined(_WIN32) || defined(_WIN64)
    return days[systemTime.wDayOfWeek];
#else
    return days[timeInfo.tm_wday];
#endif
}

int TimeInfo::getMonthNumber() const {
#if defined(_WIN32) || defined(_WIN64)
    return systemTime.wMonth;
#else
    return timeInfo.tm_mon + 1;
#endif
}

std::string TimeInfo::getMonthName() const {
    const char* months[] = { "", "January", "February", "March", "April",
                            "May", "June", "July", "August", "September",
                            "October", "November", "December" };
#if defined(_WIN32) || defined(_WIN64)
    return months[systemTime.wMonth];
#else
    return months[timeInfo.tm_mon + 1];
#endif
}

int TimeInfo::getYearNumber() const {
#if defined(_WIN32) || defined(_WIN64)
    return systemTime.wYear;
#else
    return timeInfo.tm_year + 1900;
#endif
}

std::string TimeInfo::getLeapYear() const {
#if defined(_WIN32) || defined(_WIN64)
    return isLeapYear(systemTime.wYear) ? "Yes" : "No";
#else
    return isLeapYear(timeInfo.tm_year + 1900) ? "Yes" : "No";
#endif
}

void TimeInfo::refresh() {
    updateTime();
}
