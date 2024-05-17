// Deadfrog headers
#include "df_common.h"
#include "df_window.h"
#include "fonts/df_mono.h"

// Contrib headers
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Standard headers
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


void DrawChart(DfBitmap* bmp, std::vector<double> &xVals, std::vector<double> &yVals,
               Axis* xAxis, Axis* yAxis, DfColour colour) {
    ReleaseAssert(xVals.size() == yVals.size(), "xVals and yVals must have same size");

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
    for (unsigned i = 0; i < xVals.size(); i++) {
        double x = (xVals[i] - xAxis->low) * xScale + chartX;
        double y = (yVals[i] - yAxis->low) * yScale + chartY;
//        if (i) DrawLine(bmp, prevX, prevY, x, y, colour);
        RectFill(bmp, x-1, y-1, 3, 3, colour);
        prevX = x;
        prevY = y;
    }
}


static void SwapRedAndBlue(DfBitmap* bmp) {
    DfColour* pixel = bmp->pixels;
    for (int y = 0; y < bmp->height; y++) {
        for (int x = 0; x < bmp->width; x++) {
            unsigned char tmp = pixel->r;
            pixel->r = pixel->b;
            pixel->b = tmp;
            pixel++;
        }
    }
}


static void SaveBitmap(DfBitmap *bmp, char const *filename) {
    SwapRedAndBlue(bmp);
    ReleaseAssert(stbi_write_png(filename, bmp->width,
            bmp->height, 4, bmp->pixels, bmp->width * sizeof(DfColour)), 
        "Couldn't save '%s'", filename);
    SwapRedAndBlue(bmp);
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

    //
    // Parse the log file.

    FILE* f = fopen("C:/work/sniffer/src/android/src/log_difference_after.txt", "rb");
    ReleaseAssert(f, "Couldn't open log");

    std::vector <double> phone_time;
    std::vector <double> difference;
    std::vector <double> rtt;

    char line[200];
    while (fgets(line, sizeof(line), f)) {
        char *phoneTimeStr = strstr(line, "phone_time ");
        char* differenceStr = strstr(line, "difference ");
        char* rttStr = strstr(line, "rtt ");
        if (!phoneTimeStr || !differenceStr || !rttStr) continue;

        phoneTimeStr += 11;
        differenceStr += 11;
        rttStr += 4;

        phone_time.push_back(atof(phoneTimeStr) - 1715244812.723);
        difference.push_back(atof(differenceStr));
        rtt.push_back(atof(rttStr));
    }

    // Draw the chart.
    {
       Axis xAxis = { 0.6, 3.0, 0.2, "%.1f", "Phone time - fpga time", "milliseconds" };
       Axis yAxis = { 0.0, 2.2, 0.2, "%.1f", "Round trip time", "milliseconds" };
       DfColour colour = Colour(128, 20, 20);
       DrawChart(g_window->bmp, difference, rtt, &xAxis, &yAxis, colour);
       SaveBitmap(g_window->bmp, "../../second_difference_vs_rtt.png");
    }

    while (!g_window->windowClosed && !g_window->input.keys[KEY_ESC]) {
        InputPoll(g_window);
        UpdateWin(g_window);
        WaitVsync();
    }
}
