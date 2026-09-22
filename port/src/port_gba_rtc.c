#include "port_gba_rtc.h"

#include <string.h>
#include <time.h>

#include "gba/types.h"
#include "siirtc.h"
#include "rtc.h"
#include "constants/siirtc.h"

static bool sLocked;
static u8 sStatus = SIIRTCINFO_24HOUR;

static u8 BinToBcd(int value)
{
    value %= 100;
    if (value < 0)
        value += 100;
    return (u8)(((value / 10) << 4) | (value % 10));
}

static void FillHostDateTime(struct SiiRtcInfo *rtc, bool timeOnly)
{
    if (rtc == NULL)
        return;

    time_t now = time(NULL);
    struct tm local;
    memset(&local, 0, sizeof(local));
    localtime_r(&now, &local);

    if (!timeOnly)
    {
        rtc->year = BinToBcd(local.tm_year + 1900 - 2000);
        rtc->month = BinToBcd(local.tm_mon + 1);
        rtc->day = BinToBcd(local.tm_mday);
        rtc->dayOfWeek = BinToBcd(local.tm_wday);
    }

    rtc->hour = BinToBcd(local.tm_hour);
    rtc->minute = BinToBcd(local.tm_min);
    rtc->second = BinToBcd(local.tm_sec);
    rtc->status = sStatus | SIIRTCINFO_24HOUR;
}

void SiiRtcUnprotect(void)
{
    sLocked = false;
}

void SiiRtcProtect(void)
{
    sLocked = true;
}

u8 SiiRtcProbe(void)
{
    return 1;
}

bool8 SiiRtcReset(void)
{
    if (sLocked)
        return FALSE;

    sStatus = SIIRTCINFO_24HOUR;
    return TRUE;
}

bool8 SiiRtcGetStatus(struct SiiRtcInfo *rtc)
{
    if (rtc == NULL)
        return FALSE;

    rtc->status = sStatus | SIIRTCINFO_24HOUR;
    return TRUE;
}

bool8 SiiRtcSetStatus(struct SiiRtcInfo *rtc)
{
    if (rtc == NULL || sLocked)
        return FALSE;

    sStatus = (rtc->status & (SIIRTCINFO_INTFE | SIIRTCINFO_INTME | SIIRTCINFO_INTAE))
        | SIIRTCINFO_24HOUR;
    return TRUE;
}

bool8 SiiRtcGetDateTime(struct SiiRtcInfo *rtc)
{
    if (rtc == NULL)
        return FALSE;

    FillHostDateTime(rtc, false);
    return TRUE;
}

bool8 SiiRtcGetTime(struct SiiRtcInfo *rtc)
{
    if (rtc == NULL)
        return FALSE;

    FillHostDateTime(rtc, true);
    return TRUE;
}


static u32 BcdToBinary(u8 value)
{
    return (u32)(((value >> 4) & 0xFu) * 10u + (value & 0xFu));
}

static bool IsLeapYearHost(u32 year)
{
    return ((year % 4u == 0u && year % 100u != 0u) || year % 400u == 0u);
}

void RtcInit(void)
{
    sLocked = false;
    sStatus = SIIRTCINFO_24HOUR;
}

void RtcGetInfo(struct SiiRtcInfo *rtc)
{
    FillHostDateTime(rtc, false);
}

u16 RtcGetDayCount(struct SiiRtcInfo *rtc)
{
    if (rtc == NULL)
        return 0;

    static const u8 monthDays[12] =
        {31,28,31,30,31,30,31,31,30,31,30,31};

    const u32 year = BcdToBinary(rtc->year);
    const u32 month = BcdToBinary(rtc->month);
    const u32 day = BcdToBinary(rtc->day);
    u32 count = 0;

    for (u32 y = 0; y < year; ++y)
        count += 365u + (IsLeapYearHost(y) ? 1u : 0u);

    for (u32 m = 1; m < month && m <= 12u; ++m)
    {
        count += monthDays[m - 1u];
        if (m == 2u && IsLeapYearHost(year))
            ++count;
    }

    count += day;
    return (u16)count;
}

bool PortGbaRtc_SelfTest(void)
{
    struct SiiRtcInfo rtc;
    memset(&rtc, 0, sizeof(rtc));

    SiiRtcUnprotect();

    if (SiiRtcProbe() != 1)
        return false;
    if (!SiiRtcGetStatus(&rtc))
        return false;
    if ((rtc.status & SIIRTCINFO_24HOUR) == 0)
        return false;
    if (!SiiRtcGetDateTime(&rtc))
        return false;

    const int month = ((rtc.month >> 4) & 0xF) * 10 + (rtc.month & 0xF);
    const int day = ((rtc.day >> 4) & 0xF) * 10 + (rtc.day & 0xF);
    const int hour = ((rtc.hour >> 4) & 0xF) * 10 + (rtc.hour & 0xF);

    return month >= 1 && month <= 12
        && day >= 1 && day <= 31
        && hour >= 0 && hour < 24;
}
