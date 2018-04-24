#include "helper.h"

int main()
{
    char argc;
    char pwd;
    char input[20];
    int i, j, result;

    getArgc(&argc);
    if (argc != 1)
    {
        println("invalid number of args");
    }
    else
    {
        getCurdir(&pwd);
        getArgv(0, input);
        makeDirectory(input, &result, pwd);
        if (result == INSUFFICIENT_ENTRIES)
        {
            println("directory sector is full");
        }
    }
    //hexSector(DIR_SECTOR);
    terminateProgram(0);
}