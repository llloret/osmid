// MIT License

// Copyright (c) 2016 Luis Lloret

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <algorithm>
#include <time.h>
#include "utils.h"
#include "monitorlogger.h"

using namespace std;
void replace_chars(string& str, char from, char to)
{
    replace_if(str.begin(), str.end(), [from, to](char c) {return c == from; }, to);
}

#ifdef _WIN32

// Taken from stack overflow (http://stackoverflow.com/questions/10905892/equivalent-of-gettimeday-for-windows)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdint.h> // portable: uint64_t   MSVC: __int64 

// MSVC defines this in winsock2.h!?
typedef struct timeval {
    long tv_sec;
    long tv_usec;
} timeval;

int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    time = ((uint64_t)file_time.dwLowDateTime);
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec = (long)((time - EPOCH) / 10000000L);
    tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
    return 0;
}
#endif

string timestamp()
{
    struct timeval tv;
    time_t tt;
    struct tm *tm;
    char tmbuf[64], buf[64];

    gettimeofday(&tv, nullptr);
    tt = tv.tv_sec;
    tm = localtime(&tt);
    strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", tm);
    snprintf(buf, sizeof buf, "*%s.%06d*: ", tmbuf, tv.tv_usec);
    return string(buf);
}

void logOSCMessage(const char *data, size_t size)
{
    MonitorLogger::getInstance().trace("{}: sent UDP message: ", timestamp());
    for (int i = 0; i < size; i++) {
        const unsigned char udata = (unsigned char)(data[i]);
        // is it printable?
        if (udata >= 32 && udata <= 127)
            MonitorLogger::getInstance().trace("{}", udata);
        else
            MonitorLogger::getInstance().trace("[{:02x}]", udata);
    }
}
