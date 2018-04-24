#ifndef __HELPER__
#define __HELPER__

#define MAX_BYTE 256
#define SECTOR_SIZE 512
#define MAX_FILES 32
#define MAX_FILENAME 15
#define MAX_SECTORS 512
#define DIR_ENTRY_LENGTH 16
#define MAP_SECTOR 0x100
#define DIR_SECTOR 0x101
#define FILE_SECTOR 0x102
#define SECTOR_SECTOR 0x103
#define TRUE 1
#define FALSE 0
#define INSUFFICIENT_SECTORS 0
#define INSUFFICIENT_DIR_ENTRIES -1
#define ALREADY_EXIST -2
#define INSUFFICIENT_ENTRIES -3
#define NOT_FOUND -1
#define EMPTY 0x00
#define USED 0xFF
#define NEWLINE "\r\n"
#define ARGS_SECTOR 512

void printString(char *string);
void readString(char *string);
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
void readFile(char *buffer, char *filename, int *success, char parentIndex);
void writeFile(char *buffer, char *filename, int *sectors, char parentIndex);
void executeProgram(char *filename, int segment, int *success, char p);
void terminateProgram(int *result);
void makeDirectory(char *path, int *result, char parentIndex);
void deleteFile(char *path, int *result, char parentIndex);
void deleteDirectory(char *path, int *result, char parentIndex);
void putArgs(char curdir, char argc, char **argv);
void getArgv(char index, char *argv);
void getCurdir(char *curdir);
void getArgc(char *argc);
void printChar(char c);
void printCharval(char c);

void println(char *s);

void hexSector(int sector);

int streq(char *a, char *b);

void findDirectory(char *name, char *result, char parentIndex, int *success);
void findFile(char *name, char *result, char parentIndex, int *success);
int strlen(char *string);

#endif