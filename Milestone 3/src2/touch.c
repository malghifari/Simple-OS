#include "helper.h"

int main()
{
    char argc;
    char pwd;
    char input[20];
    int i, j, result;

    enableInterrupts();
    getArgc(&argc);
    if (argc != 1)
    {
        println("invalid number of args");
    }
    else
    {
        char b[1];
        b[0] = 0;
        getCurdir(&pwd);
        getArgv(0, input);
        writeFile(b, input, &result, pwd);
        if (result == INSUFFICIENT_SECTORS)
        {
            println("Sector full");
        }
        else if (result == INSUFFICIENT_ENTRIES)
        {
            println("File full");
        }
        else if (result == NOT_FOUND)
        {
            println("Directory not found");
        }
        else if (result == ALREADY_EXIST)
        {
            println("File with the same name already exists");
        }
    }
    //hexSector(FILE_SECTOR);
    terminateProgram(0);
}