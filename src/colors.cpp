#include "colors.h"

void ColoredText(WORD wAttributes)
{
    HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hCon, wAttributes);
}