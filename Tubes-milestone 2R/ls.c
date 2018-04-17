#include "helper.h"

int main()
{
    char files[SECTOR_SIZE];
    char dir[SECTOR_SIZE];
    char pwd;
    char argc;
    int i, j;

    getArgc(&argc);
    if (argc != 0)
    {
        println("excess args");
    }
    else
    {
        getCurdir(&pwd);

        // ambil sektor files
        readSector(files, FILE_SECTOR);
        for (i = 0; i < MAX_FILES; i++)
            if (files[i * DIR_ENTRY_LENGTH + 1] != 0)
                if (files[i * DIR_ENTRY_LENGTH] == pwd)
                {
                    printString("f ");
                    println(files + i * DIR_ENTRY_LENGTH + 1);
                    //println(" [file]");
                }

        // ambil sektor files
        readSector(dir, DIR_SECTOR);
        for (i = 0; i < MAX_FILES; i++)
            if (dir[i * DIR_ENTRY_LENGTH + 1] != 0)
                if (dir[i * DIR_ENTRY_LENGTH] == pwd)
                {
                    printString("d ");
                    println(dir + i * DIR_ENTRY_LENGTH + 1);
                    //println(" [dir]");
                }
    }
    // hexSector(DIR_SECTOR);
    terminateProgram(0);
}