#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

HANDLE event;

DWORD test(LPVOID lpParam)
{
    UNREFERENCED_PARAMETER(lpParam);

    WaitForSingleObject(event, INFINITE);
    ResetEvent(event);
    Sleep(3000);
    printf("kek\n");
    SetEvent(event);

    return EXIT_SUCCESS;
}

int main(void)
{
    HANDLE h[2];
    event = CreateEvent(NULL, TRUE, FALSE, NULL);

    h[0] = CreateThread(NULL, 0, test, NULL, 0, NULL);
    h[1] = CreateThread(NULL, 0, test, NULL, 0, NULL);

    Sleep(3000);
    SetEvent(event);

    WaitForMultipleObjects(2, h, TRUE, INFINITE);

    return EXIT_SUCCESS;
}

