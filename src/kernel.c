
#define MAX_BYTE 256
#define SECTOR_SIZE 512
#define MAX_SECTORS 16
#define MAX_FILES 16
#define MAX_FILENAME 15
#define MAX_DIRNAME 15
#define MAP_SECTOR 0x100
#define DIRS_SECTOR 0x101
#define FILES_SECTOR 0x102
#define SECTORS_SECTOR 0x103
#define DIRS_ENTRY_LENGTH 16
#define FILES_ENTRY_LENGTH 16
#define SECTORS_ENTRY_LENGTH 16
#define TRUE 1
#define FALSE 0
#define INSUFFICIENT_SECTORS 0
#define INSUFFICIENT_ENTRIES -3
#define NOT_FOUND -1
#define ALREADY_EXISTS -2
#define INSUFFICIENT_DIR_ENTRIES -1
#define EMPTY 0x00
#define USED 0xFF
#define ARGS_SECTOR 512



void handleInterrupt21 (int AX, int BX, int CX, int DX);
void putArgs (char curdir, char argc, char **argv);
void getCurdir (char *curdir);
void getArgc (char *argc);
void getArgv (char index, char *argv);
void printString(char *string);
void readString(char *string);
int mod(int a, int b);
int div(int a, int b);
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
void readFile(char *buffer, char *filename, int *success);
void clear(char *buffer, int length);
void writeFile(char *buffer, char *path, int *sectors, char parentIndex);
void executeProgram(char *path, int segment, int *result, char parentIndex);
void terminateProgram (int *result);
void getIndexFromPath(char* path, char* index, char parentIndex);
void makeDirectory(char *path, int *result, char parentIndex);
void deleteFile(char *path, int *result, char parentIndex);
void deleteDirectory(char *path, int *success, char parentIndex);

int main() {
	int success, i;
	char buffer[SECTOR_SIZE*MAX_SECTORS];
	//buat logo OS
	//baris 1
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
	printString("Press any key to continue...\n\r");
	// interrupt(0x16, 0, 0, 0, 0);


	makeInterrupt21();
	// interrupt(0x21, 0xFF << 8 | 0x6, "shell", 0x2000, &success);
	// if(success == 0){
		printString("bener");
	// }
	// else{
		printString("salah");
	// }
	
	// if(found == TRUE){
	// 	interrupt(0x21, 0x0, buffer, 0, 0);
	// }
	// else{
	// 	interrupt(0x21, 0x6, "keyproc", 0x2000, &found);
	// }
	while (1);

}

void handleInterrupt21 (int AX, int BX, int CX, int DX){
	char AL, AH;
	AL = (char) AX;
	AH = (char) (AX>>8);
	switch (AL) {
		case 0x00:
			printString(BX);
			break;
		case 0x01:
			readString(BX);
			break;
		case 0x02:
			readSector(BX, CX);
			break;
		case 0x03:
			writeSector(BX, CX);
			break;
		case 0x04:
			readFile(BX, CX, DX, AH);
			break;
		case 0x05:
			writeFile(BX, CX, DX, AH);
			break;
		case 0x06:
			executeProgram(BX, CX, DX, AH);
			break;
		case 0x07:
			terminateProgram(BX);
			break;
		case 0x08:
			makeDirectory(BX, CX, AH);
			break;
		case 0x09:
			deleteFile(BX, CX, AH);
			break;
		case 0x0A:
			deleteDirectory(BX, CX, AH);
			break;
		case 0x20:
			putArgs(BX, CX);
			break;
		case 0x21:
			getCurdir(BX);
			break;
		case 0x22:
			getArgc(BX);
			break;
		case 0X23:
			getArgv(BX, CX);
			break;
		case 0x24:
			getIndexFromPath(BX, CX, DX, AH);
		default:
			printString("Invalid interrupt");
	}
}

void makeDirectory(char *path, int *result, char parentIndex){}
void deleteFile(char *path, int *result, char parentIndex){}
void deleteDirectory(char *path, int *success, char parentIndex){}

void putArgs (char curdir, char argc, char **argv) {
char args[SECTOR_SIZE];
int i, j, p;
clear(args, SECTOR_SIZE);
args[0] = curdir;
args[1] = argc;
i = 0;
j = 0;
for (p = 1; p < ARGS_SECTOR && i < argc; ++p) {
args[p] = argv[i][j];
if (argv[i][j] == '\0') {
++i;
j = 0;
}
else {
++j;
}
}
writeSector(args, ARGS_SECTOR);
}
void getCurdir (char *curdir) {
char args[SECTOR_SIZE];
readSector(args, ARGS_SECTOR);
*curdir = args[0];
}
void getArgc (char *argc) {
char args[SECTOR_SIZE];
readSector(args, ARGS_SECTOR);
*argc = args[1];
}
void getArgv (char index, char *argv) {
char args[SECTOR_SIZE];
int i, j, p;
readSector(args, ARGS_SECTOR);
i = 0;
j = 0;
for (p = 1; p < ARGS_SECTOR; ++p) {
if (i == index) {
argv[j] = args[p];
++j;
}
if (args[p] == '\0') {
if (i == index) {
break;
}
else {
++i;
}
}
}
}

void printString(char* string){ //tested work
	int i = 0;
	while(string[i] != '\0'){
		interrupt(0x10, 0xE00 + string[i], 0, 0, 0);
		i++;
	}
}

void readString(char* string){ //tested work
	char c = interrupt(0x16,0,0,0,0);
	int cursor = 0;
	while(c != '\r'){
		if(c == '\b'){
			interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
			interrupt(0x10, 0xE00 + '\0', 0, 0, 0);
			interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
			if(cursor > 0){
				cursor--;	
			}
		}
		else{
			string[cursor] = c;
			interrupt(0x10, 0xE00 + string[cursor], 0, 0, 0);
			cursor++;
		}
		c = interrupt(0x16,0,0,0,0);
	}
	string[cursor] = '\0';
	interrupt(0x10, 0xE00 + '\n', 0, 0, 0);
	interrupt(0x10, 0xE00 + '\r', 0, 0, 0);
}

int mod(int a, int b) {
	while(a >= b){
		a = a - b;
	}
	return a;
}
int div(int a, int b) {
	int q = 0;
	while(q*b <=a) {
		q = q+1;
	}
	return q-1;
}

void readSector(char *buffer, int sector){
	interrupt(0x13, 0x201, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void writeSector(char* buffer, int sector){
	interrupt(0x13, 0x301, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void clear(char *buffer, int length){
	int i;
	for(i = 0; i < length; ++i){
		buffer[i] = EMPTY;
	}
}


void readFile(char *buffer, char *path, int *result, char parentIndex) {
	// Search directory path on dirs sector
	char dirs[SECTOR_SIZE];
	char files[SECTOR_SIZE];
	int i = 0;
	int j,k,dirNameFound, fileNameFound, dirFound, fileFound;
	int done = FALSE;
	int fileIndex;
	char dirParent = parentIndex;
	char dirName[MAX_DIRNAME];
	char sectors[SECTOR_SIZE];
	int dirNameLength = 0;
	readSector(dirs, DIRS_SECTOR);
	readSector(files, FILES_SECTOR);
	while (!done) {
		// Read current path
		j = 0;
		while (path[i] != '/' && path[i] != '\0') {
			dirName[j] = path[i];
			dirNameLength++;
			i++;
		}

		// Determine whether current path is a directory or filename
		if (path[i] == '/') {
			// Search dirs for current path
			dirFound = FALSE;
			for (j = 0; j < SECTOR_SIZE / DIRS_ENTRY_LENGTH; j++) {
				if (dirs[j * DIRS_ENTRY_LENGTH] == parentIndex) {
					dirNameFound = TRUE;
					for (k = 0; k < dirNameLength; k++) {
						if (dirs[j * DIRS_ENTRY_LENGTH + k + 1] != dirName[k]) {
							dirNameFound = FALSE;
							break;
						}
					}
					if (dirNameFound) {
						dirFound = TRUE;
						dirParent = j;
						dirNameLength = 0;
						break;
					}
				}
			}
			if (!dirFound) {
				*result = NOT_FOUND;
				return;
			}
		} else {
			// Search files fot current path
			fileFound = FALSE;
			for (j = 0; j < SECTOR_SIZE / FILES_ENTRY_LENGTH; j++) {
				if (files[j * DIRS_ENTRY_LENGTH] == parentIndex) {
					fileNameFound = TRUE;
					for (k = 0; k < dirNameLength; k++) {
						if (files[j * FILES_ENTRY_LENGTH + k + 1] != dirName[k]) {
							fileNameFound = FALSE;
							break;
						}
					}
					if (fileNameFound) {
						fileIndex = j;
						fileFound = TRUE;
						break;
					}
				}
			}
			if (!fileFound) {
				*result = NOT_FOUND;
				return;
			} else {
				*result = 0;
				done = TRUE;
			}
		}
	}

	// If file found
	readSector(sectors, FILES_SECTOR);
	for (i = 0; i < SECTORS_ENTRY_LENGTH; i++) {
		readSector(buffer + i*SECTOR_SIZE, sectors[fileIndex*0x10+i]);
	}
}

void writeFile(char *buffer, char *path, int *sectors, char
parentIndex){}

void executeProgram(char *path, int segment, int *result, char
parentIndex) {}


void terminateProgram (int *result) {
	char shell[6];
	shell[0] = 's';
	shell[1] = 'h';
	shell[2] = 'e';
	shell[3] = 'l';
	shell[4] = 'l';
	shell[5] = '\0';
	executeProgram(shell, 0x2000, result, 0xFF);
}

void getIndexFromPath(char* path, char* idx, char* result, char parentIndex){
	// Search directory path on dirs sector
	char dirs[SECTOR_SIZE];
	int i = 0;
	int j,k,dirNameFound, fileNameFound, dirFound, fileFound;
	int done = FALSE;
	char dirParent = parentIndex;
	char dirName[MAX_DIRNAME];
	int dirNameLength = 0;
	readSector(dirs, DIRS_SECTOR);
	while (!done) {
		// Read current path
		j = 0;
		while (path[i] != '/' && path[i] != '\0') {
			dirName[j] = path[i];
			dirNameLength++;
			i++;
		}

		// Determine whether current path is a directory or filename
		if (path[i] == '/') {
			// Search dirs for current path
			dirFound = FALSE;
			for (j = 0; j < SECTOR_SIZE / DIRS_ENTRY_LENGTH; j++) {
				if (dirs[j * DIRS_ENTRY_LENGTH] == parentIndex) {
					dirNameFound = TRUE;
					for (k = 0; k < dirNameLength; k++) {
						if (dirs[j * DIRS_ENTRY_LENGTH + k + 1] != dirName[k]) {
							dirNameFound = FALSE;
							break;
						}
					}
					if (dirNameFound) {
						dirFound = TRUE;
						dirParent = j;
						dirNameLength = 0;
						break;
					}
				}
			}
			if (!dirFound) {
				*result = NOT_FOUND;
				return;
			}
		}
		*result = 0;
		*idx = dirParent;
	}
}