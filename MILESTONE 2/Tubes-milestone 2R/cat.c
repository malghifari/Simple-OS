#include "helper.h"

int main()
{
    char curdir;
    char argc;
    char *arg;

    getArgc(&argc);
    getCurdir(&curdir);
    if (argc == 1)
    {
        // cat
        char content[16 * SECTOR_SIZE];
        int success;
        getArgv(0, arg);

        readFile(content, arg, &success, curdir);
        if (success == NOT_FOUND)
        {
            println("file not found");
        }
        else
        {
            printString(content);
        }
    }
    else if (argc == 2)
    {
        //write mode
        getArgv(1, arg);
        if (streq(arg, "-w"))
        {
            char buffer[16 * 512];
            int result;
            int success;
            getArgv(0, arg);
            findFile(arg, &result, curdir, &success);
            if (success == 1)
            {
                printString(arg);
                println("File with the same name already exists");
            }
            else
            {
                readString(buffer);
                writeFile(buffer, arg, &result, curdir);
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
                    println("err");
                }
            }
        }
        else
        {
            println("second argument must be \"-w\"");
        }
    }
    else
    {
        println("Invalid argc");
    }
    terminateProgram(0);
}