#include <iostream>
#include <cmath>
#include <windows.h>

using namespace std;

const int   WIDTH  = 80;
const int   HEIGHT = 28;
const float PI     = 3.14159265f;
const float DEPTH  = 9.0f;
const float CAMERA = 50.0f;
const float ZOOM   = 32.0f;
const float CENTER_SHIFT = 2.5f;
const float ROUNDNESS = 9.0f;

const char SHADES[] = ".,-~:;=!*#$@";

const int COLORS[12] = { 169, 169, 169, 205, 205, 205, 212, 212, 212, 218, 218, 218 };

const int FRAME_SIZE = 40000;
char frame[FRAME_SIZE];
int  framePos = 0;

void addChar(char c)
{
    frame[framePos] = c;
    framePos++;
}

void addText(const char text[])
{
    for (int i = 0; text[i] != '\0'; i++)
    {
        addChar(text[i]);
    }
}

void addNumber(int n)
{
    if (n >= 100) addChar('0' + n / 100);
    if (n >= 10)  addChar('0' + (n / 10) % 10);
    addChar('0' + n % 10);
}

void enableColorSupport()
{
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(console, &mode);
    SetConsoleMode(console, mode | 0x0004);
}

float heartX(float u)
{
    float sinU = sin(u);

    float low = (1.0f - cos(u)) / 2.0f;
    float lowCubed = low * low * low;

    return 16.0f * sinU * sinU * sinU + ROUNDNESS * sinU * lowCubed;
}

float heartY(float u)
{
    return 13.0f * cos(u) - 5.0f * cos(2.0f * u)
         - 2.0f * cos(3.0f * u) - cos(4.0f * u)
         + CENTER_SHIFT;
}

int main()
{
    enableColorSupport();

    cout << "\x1b[2J";
    cout << "\x1b[?25l";

    float lx = -0.35f, ly = 0.5f, lz = -0.8f;
    float lightLength = sqrt(lx * lx + ly * ly + lz * lz);
    lx = lx / lightLength;
    ly = ly / lightLength;
    lz = lz / lightLength;

    float depthBuffer[WIDTH * HEIGHT];
    int   shade[WIDTH * HEIGHT];

    float angle = 0.0f;

    while (true)
    {
        for (int i = 0; i < WIDTH * HEIGHT; i++)
        {
            depthBuffer[i] = 0.0f;
            shade[i] = -1;
        }

        float cosA = cos(angle);
        float sinA = sin(angle);

        for (float u = 0.0f; u < 2.0f * PI; u += 0.015f)
        {
            float X = heartX(u);
            float Y = heartY(u);

            float X1 = heartX(u - 0.01f);
            float Y1 = heartY(u - 0.01f);
            float X2 = heartX(u + 0.01f);
            float Y2 = heartY(u + 0.01f);
            float dX = (X2 - X1) / 0.02f;
            float dY = (Y2 - Y1) / 0.02f;

            for (float v = 0.0f; v <= PI; v += 0.025f)
            {
                float sinV = sin(v);
                float cosV = cos(v);

                float px = X * sinV;
                float py = Y * sinV;
                float pz = DEPTH * cosV;

                float nx = -DEPTH * dY * sinV * sinV;
                float ny =  DEPTH * dX * sinV * sinV;
                float nz = sinV * cosV * (dX * Y - dY * X);

                float nLength = sqrt(nx * nx + ny * ny + nz * nz);
                if (nLength < 0.000001f) continue;
                nx = nx / nLength;
                ny = ny / nLength;
                nz = nz / nLength;

                float rx =  px * cosA + pz * sinA;
                float ry =  py;
                float rz = -px * sinA + pz * cosA;

                float rnx =  nx * cosA + nz * sinA;
                float rny =  ny;
                float rnz = -nx * sinA + nz * cosA;

                float zc  = rz + CAMERA;
                float ooz = 1.0f / zc;

                int sx = (int)(WIDTH  / 2.0f + 2.0f * ZOOM * rx * ooz);
                int sy = (int)(HEIGHT / 2.0f - ZOOM * ry * ooz);

                if (sx < 0 || sx >= WIDTH || sy < 0 || sy >= HEIGHT) continue;

                int cell = sy * WIDTH + sx;

                if (ooz <= depthBuffer[cell]) continue;
                depthBuffer[cell] = ooz;

                float brightness = rnx * lx + rny * ly + rnz * lz;

                if (brightness < 0.0f) brightness = 0.0f;
                brightness = 0.12f + 0.88f * brightness;
                if (brightness > 1.0f) brightness = 1.0f;

                int level = (int)(brightness * 11.99f);
                shade[cell] = level;
            }
        }

        framePos = 0;
        addText("\x1b[H");

        for (int row = 0; row < HEIGHT; row++)
        {
            for (int col = 0; col < WIDTH; col++)
            {
                int level = shade[row * WIDTH + col];
                if (level < 0)
                {
                    addChar(' ');
                }
                else
                {
                    addText("\x1b[38;5;");
                    addNumber(COLORS[level]);
                    addChar('m');
                    addChar(SHADES[level]);
                }
            }
            addText("\x1b[0m");
            addChar('\n');
        }

        frame[framePos] = '\0';

        cout << frame << flush;

        angle = angle + 0.04f;
        Sleep(30);
    }

    return 0;
}
