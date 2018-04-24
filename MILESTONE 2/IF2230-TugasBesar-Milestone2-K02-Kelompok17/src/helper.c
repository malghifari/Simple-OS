#include "helper.h"

void printString(char *string)
{
    interrupt(0x21, 0x0, string, 0, 0);
}

void readString(char *string)
{
    interrupt(0x21, 0x1, string, 0, 0);
}

void readSector(char *buffer, int sector)
{
    interrupt(0x21, 0x2, buffer, sector, 0);
}

void writeSector(char *buffer, int sector)
{
    interrupt(0x21, 0x3, buffer, sector, 0);
}

void readFile(char *buffer, char *filename, int *success, char parentIndex)
{
    interrupt(0x21, parentIndex << 8 | 0x4, buffer, filename, success);
}

void writeFile(char *buffer, char *filename, int *sectors, char parentIndex)
{
    interrupt(0x21, parentIndex << 8 | 0x5, buffer, filename, sectors);
}

void deleteFile(char *path, int *result, char parentIndex){
    interrupt(0x21, parentIndex << 8 | 0x9, path, result, 0);
}

void executeProgram(char *filename, int segment, int *success, char p)
{
    interrupt(0x21, p << 8 | 0x6, filename, segment, success);
}

void terminateProgram(int *result)
{
    interrupt(0x21, 0x7, result, 0, 0);
}

void makeDirectory(char *path, int *result, char parentIndex)
{
    interrupt(0x21, parentIndex << 8 | 0x8, path, result, 0);
}

void putArgs(char curdir, char argc, char **argv)
{
    interrupt(0x21, 0x20, curdir, argc, argv);
}

void getCurdir(char *curdir)
{
    interrupt(0x21, 0x21, curdir, 0, 0);
}

void getArgc(char *argc)
{
    interrupt(0x21, 0x22, argc, 0, 0);
}

void getArgv(char index, char *argv)
{
    interrupt(0x21, 0x23, index, argv, 0);
}

void printChar(char c)
{
    interrupt(0x10, 0xE00 + c, 0, 0, 0);
}

void printCharval(char c)
{
    char f;
    f = c >> 4;
    if ((0 <= f) && (f < 0xA))
        printChar('0' + f);
    else
        printChar('A' + f - 0xA);
    f = c % (0x10);
    if ((0 <= f) && (f < 0xA))
        printChar('0' + f);
    else
        printChar('A' + f - 0xA);
}

void println(char *s)
{
    printString(s);
    printString("\r\n");
}

void hexSector(int sector)
{
    char files[SECTOR_SIZE];
    char i, j;
    printString("hexfile\r\n");
    readSector(files, sector);
    for (i = 0; i < MAX_FILES - 0xA; i++){
        printCharval(i);
        printString("   ");
        for (j = 0; j < 16; j++){
            printChar(' ');
            printCharval(files[i * DIR_ENTRY_LENGTH + j]);
        }
        printString("\r\n");
    }
}

int streq(char *a, char *b)
{
    int same = TRUE;
    int i;
    for (i = 0; (a[i] != 0) && (b[i] != 0) && same; i++)
        same = same && (a[i] == b[i]);
    return same;
}

void findDirectory(char *filename, char *result, char parentIndex, int *attempt)
{
    char dir[SECTOR_SIZE];
    char name[20];
    int j, l;
    int equal = TRUE;
    int i = 0;
    int found = FALSE;
    readSector(dir, DIR_SECTOR);
    // calculate the length of dir_name
    i = 0; l = 0;
    while (filename[i] != 0){
        i++; l++;
    }
    // l=0, empty string
    if (l == 0){
        *attempt = -1;
        return;
    }
    // find the first '/'
    i = 0;
    while ((filename[i] != 0) && (filename[i] != '/')){
        name[i] = filename[i];
        i++;
    }
    name[i] = 0;
    // Recursive, Find directory with parentIndex name
    if (filename[i] == '/')
    {
        findDirectory(name, result, parentIndex, attempt);
        if (attempt == NOT_FOUND)
            return;
        parentIndex = *result;
        findDirectory(filename + i + 1, result, parentIndex, attempt);
    }else{ //every entry in dir
        for (i = 0; i < MAX_FILES; i++){
            // check parentIndex
            if (dir[i * DIR_ENTRY_LENGTH] != parentIndex)
                continue;
            equal = TRUE;
            for (j = 0; (j < MAX_FILENAME) && (equal) && ((filename[j] != 0) || (dir[i * DIR_ENTRY_LENGTH + 1 + j])); j++)
                equal = equal && (filename[j] == dir[i * DIR_ENTRY_LENGTH + 1 + j]);
            // equal=TRUE, file found
            if (equal == TRUE){
                *result = i; *attempt = 0;
                return;
            }
        }
        *attempt = NOT_FOUND;
    }
}

void findFile(char *id, char *result, char parentIndex, int *attempt)
{
    char files[SECTOR_SIZE];
    char name[20];
    int j, l;
    int i = 0;
    int equal = TRUE;
    readSector(files, FILE_SECTOR);
    // calculate the length of dir_name
    i = 0;
    l = 0;
    while (id[i] != 0){
        i++; l++;
    }
    // l=0, empty string
    if (l == 0){
        *attempt = -1;
        return;
    }
    // find the first '/'
    i = 0;
    while ((id[i] != 0) && (id[i] != '/')){
        name[i] = id[i];
        i++;
    }
    name[i] = 0;
    // Recursive, Find directory with parentIndex name
    if (id[i] == '/'){
        findDirectory(name, result, parentIndex, attempt);
        if (attempt == NOT_FOUND)
            return;
        parentIndex = *result;
        findFile(id + i + 1, result, parentIndex, attempt);
    }else{ // every entry dir
        for (i = 0; i < MAX_FILES; i++){
            if (files[i * DIR_ENTRY_LENGTH + 1] == 0)
                continue;
            // check parentIndex
            if (files[i * DIR_ENTRY_LENGTH] != parentIndex)
                continue;
            equal = TRUE;
            for (j = 0; (j < MAX_FILENAME) && (equal) && ((id[j] != 0) || (files[i * DIR_ENTRY_LENGTH + 1 + j] != 0)); j++)
                equal = equal && (id[j] == files[i * DIR_ENTRY_LENGTH + 1 + j]);
            // equal=TRUE, file found
            if (equal == TRUE){
                *result = i;
                *attempt = 0;
                return;
            }
        }
        *attempt = NOT_FOUND;
    }
}

int strlen(char *string)
{
    int i;
    while (string[i] != 0)
        i++;
    return i;
}
