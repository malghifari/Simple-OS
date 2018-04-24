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

    // first half
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

    for (i = 0; i < MAX_FILES - 0xA; i++)
    {
        printCharval(i);
        printString("   ");
        for (j = 0; j < 16; j++)
        {
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

void findDirectory(char *filename, char *result, char parentIndex, int *success)
{
    // digunakan untuk mencari index dari directory name, yang memiliki
    // parentIndex dimana index disiman di result dan success
    char dir[SECTOR_SIZE];
    char nama[20];
    int j, fname_len;
    int same = TRUE;
    int i = 0;
    int found = FALSE;

    readSector(dir, DIR_SECTOR);
    i = 0;
    fname_len = 0;
    while (filename[i] != 0)
    {
        i++;
        fname_len++;
    }
    // jika fname_len == 0, berarti masukan string kosong
    if (fname_len == 0)
    {
        *success = -1;
        return;
    }

    // cari karakter / pertama
    i = 0;
    while ((filename[i] != 0) && (filename[i] != '/'))
    {
        nama[i] = filename[i];
        i++;
    }
    nama[i] = 0;

    if (filename[i] == '/')
    {
        // find directory that under parentIndex named nama
        findDirectory(nama, result, parentIndex, success);
        if (success == NOT_FOUND)
        {
            return;
        }
        parentIndex = *result;
        findDirectory(filename + i + 1, result, parentIndex, success);
    }
    else
    {
        // untuk setiap entry di dir
        for (i = 0; i < MAX_FILES; i++)
        {
            // cek parentIndex
            if (dir[i * DIR_ENTRY_LENGTH] != parentIndex)
                continue;

            // nama dir ada di dir[i * DIR_ENTRY_LENGTH + 1]
            same = TRUE;
            for (j = 0; (j < MAX_FILENAME) && (same) && ((filename[j] != 0) || (dir[i * DIR_ENTRY_LENGTH + 1 + j])); j++)
                same = same && (filename[j] == dir[i * DIR_ENTRY_LENGTH + 1 + j]);

            // jika same TRUE, maka ditemukan file
            if (same == TRUE)
            {
                *result = i;
                *success = 0;
                return;
            }
            // jika same FALSE, lanjut ke entry selanjutnya
        }
        *success = NOT_FOUND;
    }
}

void findFile(char *name, char *result, char parentIndex, int *success)
{
    // digunakan untuk mencari index dari directory name, yang memiliki
    // parentIndex dimana index disiman di result dan success
    char files[SECTOR_SIZE];
    char nama[20];
    int i = 0;
    int j, fname_len;
    int same = TRUE;

    readSector(files, FILE_SECTOR);

    i = 0;
    fname_len = 0;
    while (name[i] != 0)
    {
        i++;
        fname_len++;
    }
    // jika fname_len == 0, berarti masukan string kosong
    if (fname_len == 0)
    {
        *success = -1;
        return;
    }

    // cari karakter / pertama
    i = 0;
    while ((name[i] != 0) && (name[i] != '/'))
    {
        nama[i] = name[i];
        i++;
    }
    nama[i] = 0;

    if (name[i] == '/')
    {
        // find directory that under parentIndex named nama
        findDirectory(nama, result, parentIndex, success);
        if (success == NOT_FOUND)
        {
            return;
        }
        parentIndex = *result;
        findFile(name + i + 1, result, parentIndex, success);
    }
    else
    {
        // untuk setiap entry di dir
        for (i = 0; i < MAX_FILES; i++)
        {
            if (files[i * DIR_ENTRY_LENGTH + 1] == 0)
                continue;
            // cek parentIndex
            if (files[i * DIR_ENTRY_LENGTH] != parentIndex)
                continue;

            // nama dir ada di dir[i * DIR_ENTRY_LENGTH + 1]
            same = TRUE;
            for (j = 0; (j < MAX_FILENAME) && (same) && ((name[j] != 0) || (files[i * DIR_ENTRY_LENGTH + 1 + j] != 0)); j++)
                same = same && (name[j] == files[i * DIR_ENTRY_LENGTH + 1 + j]);

            // jika same TRUE, maka ditemukan file
            if (same == TRUE)
            {
                *result = i;
                *success = 0;
                return;
            }
            // jika same FALSE, lanjut ke entry selanjutnya
        }
        *success = NOT_FOUND;
    }
}

int strlen(char *string)
{
    int i;
    while (string[i] != 0)
        i++;
    return i;
}