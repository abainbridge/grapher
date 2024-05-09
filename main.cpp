#include "df_bmp.h"
#include "df_common.h"
#include "df_font.h"
#include "df_window.h"
#include "fonts/df_mono.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>


struct Axis {
    double low;
    double high;
    double tickInterval;
    char const *numberFormat;
    char const *label;
    char const *unit;
};


void DrawChart(DfBitmap* bmp, double* xVals, unsigned numXVals, double* yVals, unsigned numYVals,
               Axis* xAxis, Axis* yAxis, DfColour colour) {
    int chartW = bmp->width * 0.7;
    int chartH = bmp->height * 0.8;
    int chartX = bmp->width * 0.2;
    int chartY = bmp->height * 0.9;
    int tickLen = bmp->width * 0.007;

    double xScale = chartW / (xAxis->high - xAxis->low);
    double yScale = -chartH / (yAxis->high - yAxis->low);

    BitmapClear(g_window->bmp, g_colourWhite);

    // Draw X axis
    HLine(bmp, chartX, chartY, chartW, g_colourBlack);
    HLine(bmp, chartX, chartY + 1, chartW, g_colourBlack);
    DrawTextCentre(g_defaultFont, g_colourBlack, bmp, chartX + chartW / 2, chartY + 50, "%s (%s)", xAxis->label, xAxis->unit);
    for (double val = xAxis->low; val <= xAxis->high; val += xAxis->tickInterval) {
        int x = (val - xAxis->low) * xScale + chartX;
        VLine(bmp, x, chartY, tickLen, g_colourBlack);
        DrawTextCentre(g_defaultFont, g_colourBlack, bmp, x, chartY + 20, xAxis->numberFormat, val);
    }

    // Draw Y axis
    VLine(bmp, chartX, chartY-chartH, chartH, g_colourBlack);
    VLine(bmp, chartX - 1, chartY-chartH, chartH, g_colourBlack);
    DrawTextCentre(g_defaultFont, g_colourBlack, bmp, chartX / 2, chartY / 2 - 10, yAxis->label);
    DrawTextCentre(g_defaultFont, g_colourBlack, bmp, chartX / 2, chartY / 2 + 10, "(%s)", yAxis->unit);
    for (double val = yAxis->low; val <= yAxis->high; val += yAxis->tickInterval) {
        int y = (val - yAxis->low) * yScale + chartY;
        DrawTextRight(g_defaultFont, g_colourBlack, bmp, chartX - 15, y - 7, yAxis->numberFormat, val);
        HLine(bmp, chartX - tickLen, y, tickLen, g_colourBlack);
    }

    // Draw data
    int prevX = 0;
    int prevY = 0;
    for (unsigned i = 0; i < numXVals; i++) {
        double x = (xVals[i] - xAxis->low) * xScale + chartX;
        double y = (yVals[i] - yAxis->low) * yScale + chartY;
        if (i) DrawLine(bmp, prevX, prevY, x, y, colour);
        prevX = x;
        prevY = y;
    }
}


#ifdef _MSC_VER
#include <windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#else
int main()
#endif
{
    g_window = CreateWin(1400, 1100, WT_WINDOWED_RESIZEABLE, "Grapher");
    g_defaultFont = LoadFontFromMemory(df_mono_9x18, sizeof(df_mono_9x18));

    FILE* f = fopen("c:/work/sniffer/src/android/src/log_difference_after.txt", "rb");
    ReleaseAssert(f, "Couldn't open log");

    // Log lines look like:
    // 05-09 09:53:26.697  6344  6372 D mini_moreph_capture: wuz phone_time 1715244806.697 difference 0.367 rtt 0.602
    std::vector <double> phone_time;
    std::vector <double> difference;
    std::vector <double> rtt;

    char line[200];
    while (fgets(line, sizeof(line), f)) {
        char *phoneTimeStr = strstr(line, "phone_time");
        char* differenceStr = strstr(line, "difference");
        char* rttStr = strstr(line, "rtt");
        if (!phoneTimeStr || !differenceStr || !rttStr) continue;

        phoneTimeStr += 11;
        differenceStr += 11;
        rttStr += 4;

        phone_time.push_back(atof(phoneTimeStr));
        difference.push_back(atof(differenceStr));
        rtt.push_back(atof(rttStr));
    }

    //{
    //    Axis xAxis = { 1715260890.0, 1715260930.0, 5, "%.0f", "Phone time", "seconds" };
    //    Axis yAxis = { 0.0, 2.0, 0.2, "%.1f", "Round trip time", "milliseconds" };
    //    DfColour colour = Colour(128, 20, 20);
    //    DrawChart(g_window->bmp, phone_time.data(), phone_time.size(), rtt.data(), rtt.size(), &xAxis, &yAxis, colour);
    //    SaveBmp(g_window->bmp, "../../rtt.bmp");
    //}

    {
        Axis xAxis = { 1715260890.0, 1715260930.0, 5, "%.0f", "Phone time", "seconds" };
        Axis yAxis = { 0.0, 3.0, 0.5, "%.1f", "Time delta", "milliseconds" };
        DfColour colour = Colour(20, 20, 170);
        DrawChart(g_window->bmp, phone_time.data(), phone_time.size(), difference.data(), difference.size(), &xAxis, &yAxis, colour);
        SaveBmp(g_window->bmp, "../../difference.bmp");
    }

    while (!g_window->windowClosed && !g_window->input.keys[KEY_ESC]) {
        InputPoll(g_window);
        UpdateWin(g_window);
        WaitVsync();
    }
}
