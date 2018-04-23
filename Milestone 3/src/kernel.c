#define MAIN
#include "proc.h"


#define MAX_BYTE 256
#define SECTOR_SIZE 512
#define MAX_FILES 32
#define MAX_FILENAME 15
#define MAX_SECTORS 512
#define DIR_ENTRY_LENGTH 16
#define MAP_SECTOR 0x100
#define TRUE 1
#define FALSE 0
#define NOT_FOUND -1
#define INSUFFICIENT_SEGMENTS -1
#define ARGS_SECTOR 512
#define EMPTY 0x00
#define USED 0xFF
#define SECTOR_SECTOR 0x103
#define FILE_SECTOR 0x102
#define DIR_SECTOR 0x101
#define INSUFFICIENT_SECTORS 0
#define INSUFFICIENT_DIR_ENTRIES -1
#define ALREADY_EXIST -2
#define INSUFFICIENT_ENTRIES -3
#define NEWLINE "\r\n"
#define SUCCESS 0
#define NOT_FOUND -1

void handleInterrupt21(int AX, int BX, int CX, int DX);

void printString(char *string);
void readString(char *string);
int mod(int a, int b);
int div(int a, int b);
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
void hexSector(int s);
void readFile(char *buffer, char *filename, int *attempt, char parentIndex);
void writeFile(char *buffer, char *filename, int *sectors, char parentIndex);
void findFile(char *id, char *result, char parentIndex, int *attempt);
void deleteFile(char *path, int *result, char parentIndex);
void deleteFileByIndex(char index, int *attempt);
void makeDirectory(char *path, int *result, char parentIndex);
void findDirectory(char *name, char *result, char parentIndex, int *attempt);
void deleteDirectory(char *path, int *result, char parentIndex);
void deleteDirectoryByIndex(char index, int *attempt);
// void executeProgram(char *filename, int segment, int *attempt, char p);
void executeProgram (char *path, int *result, char parentIndex);
void terminateProgram(int *result);
void getArgc(char *argc);
void putArgs(char curdir, char argc, char **argv);
void getArgv(char index, char *argv);
void getCurdir(char *curdir);
void clear(char *buffer, int length);
void listFiles();
void printChar(char c);
void printCharval(char c);
int strlen(char *string);
void printLogo();
void handleTimerInterrupt(int segment, int stackPointer);
void yieldControl();
void sleep();
void pauseProcess (int segment, int *result);
void resumeProcess (int segment, int *result);
void killProcess (int segment, int *result);

int main()
{
    int attempt;

    initializeProcStructures();
    makeInterrupt21();
    makeTimerInterrupt();
    printLogo();
    putArgs(0xFF, 0, 0);
    executeProgram("shell", &attempt, 0xFF);
    while (1);
}

void handleInterrupt21(int AX, int BX, int CX, int DX)
{
    char AL, AH;
    AL = (char)(AX);
    AH = (char)(AX >> 8);
    /*
    printChar('[');
    printCharval(AL);
    printChar(']');
    */
    switch (AL)
    {
    case 0x0:
        printString(BX);
        break;
    case 0x1:
        readString(BX);
        break;
    case 0x2:
        readSector(BX, CX);
        break;
    case 0x3:
        writeSector(BX, CX);
        break;
    case 0x4:
        readFile(BX, CX, DX, AH);
        break;
    case 0x5:
        writeFile(BX, CX, DX, AH);
        break;
    case 0x6:
        executeProgram(BX, CX, AH);
        break;
    case 0x7:
        terminateProgram(BX);
        break;
    case 0x8:
        makeDirectory(BX, CX, AH);
        break;
    case 0x9:
        deleteFile(BX, CX, AH);
        break;
    case 0x0A:
        deleteDirectory(BX, CX, AH);
        break;
    case 0x20:
        putArgs(BX, CX, DX);
        break;
    case 0x21:
        getCurdir(BX);
        break;
    case 0x22:
        getArgc(BX);
        break;
    case 0x23:
        getArgv(BX, CX);
        break;
    case 0x30:
        yieldControl();
        break;
    case 0x31:
        sleep();
        break;
    case 0x32:
        pauseProcess(BX, CX);
        break;
    case 0x33:
        resumeProcess(BX, CX);
        break;
    case 0x34:
        killProcess(BX, CX);
        break;
    default:
        printString("Invalid interrupt");
    }
}

void printString(char *string)
{
    int i = 0;
    while (string[i] != 0x00) {
        interrupt(0x10, 0xE00 + string[i], 0, 0, 0);
        i++;
    }
}

void readString(char *string)
{
    int temp = 0;
    char c;

    c = interrupt(0x16, 0, 0, 0, 0);
    while (c != 0xD) {
        if (c == '\b') {
            if (temp != 0) {
                printChar('\b'); printChar(' '); printChar('\b');
                temp -= 1;
            }
        }else{
            printChar(c); string[temp] = c;
            temp += 1;
        }
        c = interrupt(0x16, 0, 0, 0, 0);
    }
    printChar('\r'); printChar('\n');
    string[temp] = 0;
}

int mod(int a, int b)
{
    while (a >= b) {
        a = a - b;
    }
    return a;
}

int div(int a, int b)
{
    int q = 0;
    while (q * b <= a){
        q = q + 1;
    }
    return (q - 1);
}

void readSector(char *buffer, int sector)
{
    interrupt(0x13, 0x201, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void writeSector(char *buffer, int sector)
{

    interrupt(0x13, 0x301, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void readFile(char *buffer, char *filename, int *attempt, char parentIndex)
{
    char dir[SECTOR_SIZE];
    char files[SECTOR_SIZE];
    char sector[SECTOR_SECTOR];
    char exist = FALSE;
    int temp = -1;
    int index_directory, index_file, i, l, count;
    char name[MAX_FILENAME];
    char c;
    char equal;

    //calculate the length of filename
    i = 0; l = 0; //length of filename = l
    while (filename[i] != 0){
        i++; l++;
    }
    // l=0, string empty
    if (l == 0){
        *attempt = -1;
        return;
    }
    // Find the first '/'
    i = 0;
    while ((filename[i] != 0) && (filename[i] != '/')){
        name[i] = filename[i];
        i++;
    }
    name[i] = 0;
    // Recursive, Find directory in dirsector
    if (filename[i] == '/'){
        findDirectory(name, &c, parentIndex, &temp);
        if (temp == -1){
            *attempt = -1;
            return;
        }
        readFile(buffer, filename + i + 1, attempt, c);
    }else{
        readSector(sector, SECTOR_SECTOR);
        findFile(name, &c, parentIndex, &temp);
        if (temp == -1){
            *attempt = NOT_FOUND;
            return;
        }
        // load data sector to buffer
        for (i = 0; (i < 0x10) && (sector[c * DIR_ENTRY_LENGTH + i] != 0); i++){
            readSector(buffer + i * SECTOR_SIZE,
                       sector[c * DIR_ENTRY_LENGTH + i]);
        }
        *attempt = c; // hasil perubahan dari sebelumnya
    }
}

void writeFile(char *buffer, char *path, int *sectors, char parentIndex)
{
    char sector[SECTOR_SIZE];
    char file[SECTOR_SIZE];
    char map[SECTOR_SIZE];
    char name[MAX_FILENAME];
    char index_file, c, index_sector;
    int i, count, l, temp;
    int found = FALSE;
    // check empty sector around map
    count = 0;
    readSector(map, MAP_SECTOR);
    for (i = 0; i < SECTOR_SIZE; i++){
        if (map[i] == USED)
            continue;
        count++;
        if (count >= 16)
            break;
    }
    if (count < 16){
        *sectors = INSUFFICIENT_SECTORS;
        return;
    }
    // check empty sector FILE_SECTOR
    readSector(file, FILE_SECTOR);
    for (i = 0; i < MAX_FILES; i++){
        if (file[i * DIR_ENTRY_LENGTH + 1] == 0){
            found = TRUE; index_file = i;
            break;
        }
    }
    if (found == FALSE){
        *sectors = INSUFFICIENT_ENTRIES;
        return;
    }
    // check file or folder with the length of filename
    i = 0; l = 0;
    while (path[i] != 0){
        i++; l++;
    }
    // l = 0, string empty
    if (l == 0){
        *sectors = -1;
        return;
    }
    // Find the first '/'
    i = 0;
    while ((path[i] != 0) && (path[i] != '/')){
        name[i] = path[i];
        i++;
    }
    name[i] = 0;
    // Recursive, Find directory in dirsector
    if (path[i] == '/'){
        findDirectory(name, &c, parentIndex, &temp);
        if (temp == -1){
            *sectors = NOT_FOUND;
            return;
        }
        writeFile(buffer, path + i + 1, sectors, c);
    }else{ // check the existance of file with searching filename
        int buffer_length;
        findFile(name, &c, parentIndex, &temp);
        if (temp == 0){
            *sectors = ALREADY_EXIST;
            return;
        }
        // set parentIndex
        file[index_file * DIR_ENTRY_LENGTH] = parentIndex;
        for (i = 0; i < l; i++)
            file[index_file * DIR_ENTRY_LENGTH + 1 + i] = path[i];
        readSector(sector, SECTOR_SECTOR);
        buffer_length = strlen(buffer) / SECTOR_SIZE + 1;
        index_sector = 0;
        for (i = 0; i < buffer_length; i++){
            // find the first empty sector
            for (; index_sector < MAX_SECTORS; index_sector++)
                if (map[index_sector] == EMPTY)
                    break;
            writeSector(buffer + i * SECTOR_SIZE, index_sector);
            sector[index_file * DIR_ENTRY_LENGTH + i] = index_sector;
            map[index_sector] == USED; index_sector++;
        }
        writeSector(sector, SECTOR_SECTOR);
        writeSector(file, FILE_SECTOR); writeSector(map, MAP_SECTOR);
        *sectors = 1;
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

void deleteFile(char *path, int *result, char parentIndex)
{
    char name[MAX_FILENAME];
    char equal;
    char c;
    char exist = FALSE;
    int temp = -1;
    int found = FALSE;
    int index_file, i, l, count;
    printString("deletefile ");
    printString(path); printChar(' ');
    printCharval(parentIndex); printString("\r\n");
    // calculate the length of filename
    i = 0; l = 0;
    while (path[i] != 0){
        i++; l++;
    }
    // l=0, empty string
    if (l == 0){
        *result = -1;
        return;
    }
    // find the first '/'
    i = 0;
    while ((path[i] != 0) && (path[i] != '/')){
        name[i] = path[i];
        i++;
    }
    name[i] = 0;
    // Recursive, Find directory in dirsector
    if (path[i] == '/'){
        findDirectory(name, &c, parentIndex, &temp);
        if (temp == -1){
            *result = -1;
            return;
        }
        deleteFile(path + i + 1, result, c);
    }else{// search filename and check the existance
        findFile(name, &c, parentIndex, &temp);
        if (temp == -1){
            *result = -1;
            return;
        }
        deleteFileByIndex(c, result);
    }
}

void deleteFileByIndex(char index, int *attempt)
{
    char file[SECTOR_SIZE];
    char map[SECTOR_SIZE];
    char sector[SECTOR_SIZE];
    int i, j;
    readSector(file, FILE_SECTOR);
    if (file[index * DIR_ENTRY_LENGTH + 1] == 0){
        *attempt = 0;
        return;
    }
    readSector(map, MAP_SECTOR);
    readSector(sector, SECTOR_SECTOR);
    // delete file, change the first byte
    file[index * DIR_ENTRY_LENGTH + 1] = 0;
    // sector empty
    for (i = 0; i < 16; i++){
        if (sector[index * DIR_ENTRY_LENGTH + i] == 0)
            break;
        map[sector[index * DIR_ENTRY_LENGTH + i]] = EMPTY;
    }
    writeSector(file, FILE_SECTOR);
    writeSector(map, MAP_SECTOR);
    *attempt = 0;
}

void makeDirectory(char *path, int *result, char parentIndex)
{
    char dir[SECTOR_SIZE];
    char name[MAX_FILENAME];
    char equal;
    char c;
    char exist = FALSE;
    int temp = -1;
    int index_directory, i, l, count;
    int found = FALSE;
    // check empty sector in DIR_SECTOR
    readSector(dir, DIR_SECTOR);
    for (i = 0; i < MAX_FILES; i++){
        if (dir[i * DIR_ENTRY_LENGTH + 1] == 0){
            found = TRUE; index_directory = i;
            break;
        }
    }
    if (found == FALSE){
        *result = INSUFFICIENT_ENTRIES;
        return;
    }
    // calculate the length of filename
    i = 0; l = 0;
    while (path[i] != 0){
        i++; l++;
    }
    // l=0, empty String
    if (l == 0){
        *result = NOT_FOUND;
        return;
    }
    // find the first '/'
    i = 0;
    while ((path[i] != 0) && (path[i] != '/')){
        name[i] = path[i];
        i++;
    }
    name[i] = 0;
    // Recursive, Find directory in dirsector
    if (path[i] == '/'){
        findDirectory(name, &c, parentIndex, &temp);
        if (temp == -1){
            *result = NOT_FOUND;
            return;
        }
        makeDirectory(path + i + 1, result, c);
    }else{// search dir_name and check the existance
        findDirectory(name, &c, parentIndex, &temp);
        if (temp != -1){
            *result = ALREADY_EXIST;
            return;
        }
        dir[index_directory * DIR_ENTRY_LENGTH] = parentIndex;
        for (i = 0; (i < MAX_FILENAME) && (path[i] != 0); i++)
            dir[index_directory * DIR_ENTRY_LENGTH + 1 + i] = path[i];
        writeSector(dir, DIR_SECTOR);
        *result = 0;
    }
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

void deleteDirectory(char *path, int *result, char parentIndex)
{
    char name[MAX_FILENAME];
    char equal, curdir;
    char c;
    char exist = FALSE;
    int temp = -1;
    int found = FALSE;
    int index_directory, i, l, count;
    // calculate the length of filename
    i = 0; l = 0;
    while (path[i] != 0){
        i++; l++;
    }
    // l=0, empty string
    if (l == 0){
        *result = -1;
        return;
    }
    // find the first '/'
    i = 0;
    while ((path[i] != 0) && (path[i] != '/')){
        name[i] = path[i];
        i++;
    }
    name[i] = 0;
    // Recursive, Find directory in dirsector
    if (path[i] == '/'){
        findDirectory(name, &c, parentIndex, &temp);
        if (temp == -1){
            *result = -1;
            return;
        }
        deleteDirectory(path + i + 1, result, c);
    }else{// search filename and check the existance
        findFile(name, &c, parentIndex, &temp);
        if (temp == -1){
            *result = -1;
            return;
        }
        deleteDirectoryByIndex(c, result);
    }
}

void deleteDirectoryByIndex(char index, int *attempt)
{
    char i, j;
    char sector[SECTOR_SIZE];
    char dir[SECTOR_SIZE];
    char file[SECTOR_SIZE];
    char map[SECTOR_SIZE];
    readSector(dir, DIR_SECTOR);
    readSector(map, MAP_SECTOR);
    readSector(sector, SECTOR_SECTOR);
    readSector(file, FILE_SECTOR);
    if (dir[index * DIR_ENTRY_LENGTH + 1] == 0){
        *attempt = 0;
        return;
    }
    // delete dir_name from index
    dir[index * DIR_ENTRY_LENGTH + 1] = 0;
    // delete file with the same parentIndex
    for (i = 0; i < MAX_FILES; i++)
        if (file[i * DIR_ENTRY_LENGTH] == index)
            deleteFileByIndex(i, attempt);
    // delete directory with the same parentIndex
    for (i = 0; i < MAX_FILES; i++)
        if (dir[i * DIR_ENTRY_LENGTH] == index)
            deleteDirectoryByIndex(i, attempt);
    writeSector(file, FILE_SECTOR);
    writeSector(map, MAP_SECTOR);
    writeSector(dir, DIR_SECTOR);
    *attempt = 0;
}

void executeProgram (char *path, int *result, char parentIndex){  
  struct PCB* pcb;
  int segment;
  int i, fileIndex;
  char buffer[MAX_SECTORS * SECTOR_SIZE];
  readFile(buffer, path, result, parentIndex);
 
  if (*result != NOT_FOUND) {
    setKernelDataSegment();
    segment = getFreeMemorySegment();
    restoreDataSegment();
    
    fileIndex = *result; 
    if (segment != NO_FREE_SEGMENTS) {
      setKernelDataSegment();
      pcb = getFreePCB();
      pcb->index = fileIndex;
      pcb->state = STARTING;
      pcb->segment = segment;
      pcb->stackPointer = 0xFF00;
      pcb->parentSegment = running->segment;
      addToReady(pcb);
      restoreDataSegment();
      for (i = 0; i < SECTOR_SIZE * MAX_SECTORS; i++) {
        putInMemory(segment, i, buffer[i]);
      }
      initializeProgram(segment);
      sleep();
    }
    else {
      *result = INSUFFICIENT_SEGMENTS;
    }
  }
}


void terminateProgram (int *result) {
  int parentSegment;
   
  setKernelDataSegment();
  parentSegment = running->parentSegment;
  releaseMemorySegment(running->segment);
  releasePCB(running);
  restoreDataSegment();
  if (parentSegment != NO_PARENT) {
    resumeProcess(parentSegment, result);
  }
  yieldControl();
}


void getArgc(char *argc)
{
    char args[SECTOR_SIZE];
    readSector(args, ARGS_SECTOR);
    *argc = args[1];
}

void putArgs(char curdir, char argc, char **argv)
{
    char args[SECTOR_SIZE];
    int i, j, p;
    clear(args, SECTOR_SIZE);
    args[0] = curdir; args[1] = argc;
    i = 0; j = 0;
    for (p = 2; p < ARGS_SECTOR && i < argc; ++p){
        args[p] = argv[i][j];
        if (argv[i][j] == 0){
            ++i; j = 0;
        }else
            ++j;
    }
    writeSector(args, ARGS_SECTOR);
}

void getArgv(char index, char *argv)
{
    char args[SECTOR_SIZE];
    int i, j, p;
    readSector(args, ARGS_SECTOR);
    i = 0; j = 0;
    for (p = 2; p < ARGS_SECTOR; ++p){
        if (i == index){
            argv[j] = args[p];
            ++j;
        }
        if (args[p] == 0){
            if (i == index)
                break;
            else
                ++i;
        }
    }
    writeSector(args, ARGS_SECTOR);
}

void getCurdir(char *curdir)
{
    char args[SECTOR_SIZE];
    readSector(args, ARGS_SECTOR);
    *curdir = args[0];
}

void clear(char *buffer, int length)
{
    int i;
    for (i = 0; i < length; ++i)
        buffer[i] = EMPTY;
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

int strlen(char *string)
{
    int i = 0;
    while (string[i] != 0)
        i++;
    return i;
}

void printLogo()
{
	int i;
	for(i=60; i<69; i++){
		putInMemory(0xB000, 0x8000 + (80*2+i)*2, '=');
		putInMemory(0xB000, 0x8001 + (80*2+i)*2, 0xA);
		putInMemory(0xB000, 0x8000 + (80*8+i)*2, '=');
		putInMemory(0xB000, 0x8001 + (80*8+i)*2, 0xA);
		if(i != 4+60){
			putInMemory(0xB000, 0x8000 + (80*3+i)*2, '#');
			putInMemory(0xB000, 0x8001 + (80*3+i)*2, 0xD);
			putInMemory(0xB000, 0x8000 + (80*7+i)*2, '#');
			putInMemory(0xB000, 0x8001 + (80*7+i)*2, 0xD);
		}
		if(i == 0+60 || i == 3+60 || i == 5+60){
			putInMemory(0xB000, 0x8000 + (80*4+i)*2, '#');
			putInMemory(0xB000, 0x8001 + (80*4+i)*2, 0xD);
			putInMemory(0xB000, 0x8000 + (80*5+i)*2, '#');
			putInMemory(0xB000, 0x8001 + (80*5+i)*2, 0xD);
		}
		if(i >= 6+60 && i <= 8+60){
			putInMemory(0xB000, 0x8000 + (80*5+i)*2, '#');
			putInMemory(0xB000, 0x8001 + (80*5+i)*2, 0xD);
		}
		if(i==0+60 || i==3+60 || i==8+60){
			putInMemory(0xB000, 0x8000 + (80*6+i)*2, '#');
			putInMemory(0xB000, 0x8001 + (80*6+i)*2, 0xD);
		}
	}
}

void handleTimerInterrupt(int segment, int stackPointer) {
  struct PCB *currPCB;
  struct PCB *nextPCB;
 
  setKernelDataSegment();
  currPCB = getPCBOfSegment(segment);
  currPCB->stackPointer = stackPointer;
  if (currPCB->state != PAUSED) {
    currPCB->state = READY;
    addToReady(currPCB);
  }
 
  do {
    nextPCB = removeFromReady();
  }  
  while (nextPCB != NULL && (nextPCB->state == DEFUNCT || nextPCB->state == PAUSED));

  if (nextPCB != NULL) {
    nextPCB->state = RUNNING;
    segment = nextPCB->segment;
    stackPointer = nextPCB->stackPointer;
    running = nextPCB;
  }
  else {
    running = &idleProc;
  }
  restoreDataSegment();
 
  returnFromTimer(segment, stackPointer);
}

void yieldControl () {
  interrupt(0x08, 0, 0, 0, 0);
}

void sleep () {
  setKernelDataSegment();
  running->state = PAUSED;
  restoreDataSegment();  
  yieldControl();
}

void pauseProcess (int segment, int *result) {
  struct PCB *pcb;
  int res;
   
  setKernelDataSegment();
  pcb = getPCBOfSegment(segment);
  if (pcb != NULL && pcb->state != PAUSED) {
    pcb->state = PAUSED;
    res = SUCCESS;
  }
  else {
    res = NOT_FOUND;
  }
  restoreDataSegment();
   
  *result = res;
}

void resumeProcess (int segment, int *result) {
  struct PCB *pcb;
  int res;
   
  setKernelDataSegment();
  pcb = getPCBOfSegment(segment);
  if (pcb != NULL && pcb->state == PAUSED) {
    pcb->state = READY;
    addToReady(pcb);
    res = SUCCESS;
  }
  else {
    res = NOT_FOUND;
  }
  restoreDataSegment();
   
  *result = res;
}

void killProcess (int segment, int *result) {
  struct PCB *pcb;
  int res;
   
  setKernelDataSegment();
  pcb = getPCBOfSegment(segment);
  if (pcb != NULL) {
    releaseMemorySegment(pcb->segment);
    releasePCB(pcb);
    res = SUCCESS;
  }
  else {
    res = NOT_FOUND;
  }
  restoreDataSegment();
   
  *result = res;
}




