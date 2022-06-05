#include <sharpen/Console.hpp>

#include <sharpen/SystemMacro.hpp>

#ifdef SHARPEN_IS_WIN
#include <Windows.h>
#else
#include <unistd.h>
#endif

void sharpen::ClearConsole()
{
#ifdef SHARPEN_IS_WIN
    HANDLE hStdOut{::GetStdHandle(STD_OUTPUT_HANDLE)};
    COORD coord = {0,0};
    DWORD count{0};
    CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
    ::GetConsoleScreenBufferInfo(hStdOut, &bufferInfo);
    ::FillConsoleOutputCharacterA(hStdOut, ' ',bufferInfo.dwSize.X * bufferInfo.dwSize.Y,coord, &count);
    ::SetConsoleCursorPosition(hStdOut, coord);
    (void)count;
#else
    ::write(STDOUT_FILENO,"\x1b[1J",4);
#endif
}