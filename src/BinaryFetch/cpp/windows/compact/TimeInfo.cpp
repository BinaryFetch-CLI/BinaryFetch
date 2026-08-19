#include "TimeInfo.h"
#include <windows.h>

TimeInfo::TimeInfo()
{
    updateTime();
}

// Grab the current local time when the object is created.
void TimeInfo::updateTime()
{
    SYSTEMTIME currentTime;
    GetLocalTime(&currentTime);

    // Store the current time.
    second = currentTime.wSecond;
    minute = currentTime.wMinute;
    hour = currentTime.wHour;

    // Store the current date.
    day = currentTime.wDay;
    monthNumber = currentTime.wMonth;
    year = currentTime.wYear;

    // Windows gives us the day as a number,
    // so we convert it to a readable name here.
    const char* days[] =
    {
        "Sunday",
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday"
    };

    dayName = days[currentTime.wDayOfWeek];

    // Same thing for the month.
    const char* months[] =
    {
        "",
        "January",
        "February",
        "March",
        "April",
        "May",
        "June",
        "July",
        "August",
        "September",
        "October",
        "November",
        "December"
    };

    monthName = months[currentTime.wMonth];

    // A simple Yes/No value is easier for the output layer to use.
    leapYear = isLeapYear(year) ? "Yes" : "No";

    // Calculate the week after getting the current date.
    weekNumber = calculateWeekNumber();
}


// Check whether the current year is a leap year.
bool TimeInfo::isLeapYear(int yearValue) const
{
    return
        (yearValue % 4 == 0 && yearValue % 100 != 0) ||
        (yearValue % 400 == 0);
}


// Calculate the week number using January 1st as the starting point.
int TimeInfo::calculateWeekNumber() const
{
    SYSTEMTIME currentTime;
    GetLocalTime(&currentTime);

    // Start from the first day of the current year.
    SYSTEMTIME jan1 = {};
    jan1.wYear = currentTime.wYear;
    jan1.wMonth = 1;
    jan1.wDay = 1;

    FILETIME ft1;
    FILETIME ft2;

    SystemTimeToFileTime(&jan1, &ft1);
    SystemTimeToFileTime(&currentTime, &ft2);

    // FILETIME is stored as a 64-bit value,
    // so combine the high and low parts before doing the calculation.
    ULARGE_INTEGER start;
    ULARGE_INTEGER current;

    start.LowPart = ft1.dwLowDateTime;
    start.HighPart = ft1.dwHighDateTime;

    current.LowPart = ft2.dwLowDateTime;
    current.HighPart = ft2.dwHighDateTime;

    // FILETIME uses 100-nanosecond intervals.
    // Convert the difference into days.
    ULONGLONG difference = current.QuadPart - start.QuadPart;

    int dayOfYear =
        static_cast<int>(
            difference / 10000000ULL / 86400ULL
        ) + 1;

    // Convert the day of the year into a simple week number.
    return (dayOfYear + 6) / 7;
}


// Return the stored time values.
int TimeInfo::getSecond() const
{
    return second;
}

int TimeInfo::getMinute() const
{
    return minute;
}

int TimeInfo::getHour() const
{
    return hour;
}


// Return the stored date values.
int TimeInfo::getDay() const
{
    return day;
}

int TimeInfo::getWeekNumber() const
{
    return weekNumber;
}

std::string TimeInfo::getDayName() const
{
    return dayName;
}

int TimeInfo::getMonthNumber() const
{
    return monthNumber;
}

std::string TimeInfo::getMonthName() const
{
    return monthName;
}

int TimeInfo::getYearNumber() const
{
    return year;
}

std::string TimeInfo::getLeapYear() const
{
    return leapYear;
}


// Refresh the values in case the object has been alive for a while.
void TimeInfo::refresh()
{
    updateTime();
}