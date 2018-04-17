#include "helper.h"

int main()
{
    // menerima satu input
    char curdir;
    char argc;
    char argv[8][20];
    char *x[8];

    getCurdir(&curdir);

    while (1)
    {
        char input[100];
        char command[20];
        int result;
        char c;
        int i, argstart, j, success;

        for (i = 0; i < 100; i++)
            input[i] = 0;

        //printCharval(curdir);
        printString("$ ");
        readString(input);

        if (input[0] == 0)
            continue;

        //println(input);
        for (i = 0; (input[i] != 0) && (input[i] != ' '); ++i)
            command[i] = input[i];
        command[i] = 0;
        // args
        ++i;
        argstart = i;

        argc = 0;
        while (input[argstart] != 0)
        {
            for (j = 0; j < 20; j++)
                argv[argc][j] = 0;

            j = 0;
            while ((input[argstart + j] != 0) && (input[argstart + j] != ' '))
            {
                argv[argc][j] = input[argstart + j];
                ++j;
            }
            argv[argc][j] = 0;
            ++argc;
            argstart += j + 1;
        }

        for (i = 0; i < 8; i++)
        {
            x[i] = (argv[i]);
        }

        putArgs(curdir, argc, x);

        if (streq("cd", command))
        {
            // run cd
            if (argc == 0)
            {
                println("args can't be empty");
                continue;
            }
            if (streq(x[0], '.'))
            {
                println("stay");
            }
            else if (streq(x[0], ".."))
            {
                if (curdir == 0xFF)
                    println("already at root");
                else
                {
                    char dir[SECTOR_SIZE];
                    readSector(dir, DIR_SECTOR);
                    curdir = dir[curdir * DIR_ENTRY_LENGTH];
                    printString("cd up to ");
                    println(curdir == 0xFF ? "0xFF" : dir + curdir * DIR_ENTRY_LENGTH + 1);
                }
            }
            else
            {
                char dir[SECTOR_SIZE];
                char res;
                int s;

                readSector(dir, DIR_SECTOR);

                // find directory that have parent curdir and same name
                findDirectory(x[0], &res, curdir, &s);
                if (s == NOT_FOUND)
                {
                    println("directory not found");
                }
                else
                {
                    curdir = res;
                    printCharval(curdir);
                    println("");
                }
            }
        }
        else if (streq(command, "listdir"))
        {
            char dir[SECTOR_SIZE];

            readSector(dir, DIR_SECTOR);
            for (i = 0; i < MAX_FILES; i++)
                if (dir[i * DIR_ENTRY_LENGTH + 1] != 0)
                {
                    printCharval(i);
                    printChar(' ');
                    printCharval(dir[i * DIR_ENTRY_LENGTH]);
                    printChar(' ');
                    println(dir + i * DIR_ENTRY_LENGTH + 1);
                }
        }

        else
        {
            // putArgs(curdir, argc, x);
            executeProgram(command, 0x2000, &success, 0xFF);
        }
    }
}