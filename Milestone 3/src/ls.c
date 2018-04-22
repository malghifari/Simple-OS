#include "helper.h"

int main()
{
    char files[SECTOR_SIZE];
    char dir[SECTOR_SIZE];
    char pwd;
    char argc;
    int i, j;

    getArgc(&argc);
    if (argc != 0){
        println("too much args");
    }else{
        getCurdir(&pwd);
        readSector(files, FILE_SECTOR);
        for (i = 0; i < MAX_FILES; i++)
            if (files[i * DIR_ENTRY_LENGTH + 1] != 0)
                if (files[i * DIR_ENTRY_LENGTH] == pwd){
                    printString("Files = ");
                    println(files + i * DIR_ENTRY_LENGTH + 1);
                }
        readSector(dir, DIR_SECTOR);
        for (i = 0; i < MAX_FILES; i++)
            if (dir[i * DIR_ENTRY_LENGTH + 1] != 0)
                if (dir[i * DIR_ENTRY_LENGTH] == pwd){
                    printString("Directory = ");
                    println(dir + i * DIR_ENTRY_LENGTH + 1);
                }
    }
    terminateProgram(0);
}
