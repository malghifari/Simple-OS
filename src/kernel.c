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
void printString(char *string);
void readString(char *string);
int mod(int a, int b);
int div(int a, int b);
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
void readFile(char *buffer, char *filename, int *success);
void clear(char *buffer, int length);
void writeFile(char *buffer, char *path, int *sectors, char parentIndex);
void executeProgram(char *filename, int segment, int *success);
void printStringAt(int y, int x, char* s);

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
	// printString("Press any key to continue...\n\r");
	// interrupt(0x16, 0, 0, 0, 0);


	makeInterrupt21();
	interrupt(0x21, 0xFF << 8 | 0x6, "shell", 0x2000, &success);
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
		default:
			printString("Invalid interrupt");
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



// void splitString(char** result, char* str, char splitter){
// 	int istr = 0;
// 	int ires1 = 0;
// 	int ires2 = 0;

// 	while(str[istr] != '\0'){
// 		if(str[istr] == '/' && istr == 0){
// 			istr++;
// 		}
// 		else if (str[istr] == '/'){
// 			result[ires1][ires2] = '\0';
// 			istr++;
// 			ires1++;
// 		}
// 		else{
// 			result[ires1][ires2] = str[istr]
// 			istr++;
// 			ires2++;
// 		}
// 	}
// }

// void readFile(char* buffer, char* path, int *result, char parentIndex){
// 	char dirs[SECTOR_SIZE];
// 	char files[SECTOR_SIZE];
// 	char sectors[SECTOR_SIZE];
// 	char splitted_path[15];
// 	int path_idx = 0;
// 	int splitted_path_idx = 0;
// 	int i, j, found;
// 	// int found = FALSE;
// 	// int bener;
// 	// int idx, i, j;
// 	readSector(dirs, DIRS_SECTOR);
// 	readSector(files, FILES_SECTOR);
// 	readSector(sectors, SECTORS_SECTOR);
// 	while(TRUE){
// 		if(path[path_idx] == '/' && path_idx == 0){
// 			path_idx++;
// 		}
// 		// when splitted_path is a directory
// 		else if(path[path_idx] == '/'){
// 			splitted_path[splitted_path_idx] = '\0';
// 			splitted_path_idx = 0;
// 			for(i=0; i<SECTOR_SIZE; i+=16){
// 				found = TRUE;
// 				if(dirs[i] == parentIndex){
// 					j = 0;
// 					while(splitted_path[j] != '\0' && found){
// 						if(splitted_path[j] != dirs[j+i+1]){
// 							found = FALSE;
// 						}
// 						else{
// 							j++;
// 						}
// 					}
// 				}
// 				else{
// 					found = FALSE;
// 				}
// 				if(found == TRUE){
// 					parentIndex = dirs[i];
// 					break;
// 				}
// 			}
// 			if(found == FALSE){
// 				*result = -1;
// 				return;
// 			}
// 		}

// 		//when splitted_path is a files
// 		else if(path[path_idx] == '\0'){
// 			splitted_path[splitted_path_idx] = '\0';
// 			splitted_path_idx = 0;
// 			for(i=0; i<SECTOR_SIZE; i+=16){
// 				found = TRUE;
// 				if(files[i] == parentIndex){
// 					j = 0;
// 					while(splitted_path[j] != '\0' && found){
// 						if(splitted_path[j] != files[j+i+1]){
// 							found = FALSE;
// 						}
// 						else{
// 							j++;
// 						}
// 					}
// 				}
// 				else{
// 					found = FALSE;
// 				}
// 				if(found == TRUE){
// 					for(j=0; j<16; j++){
// 						readSector(buffer + j*SECTOR_SIZE, sectors[j]);
// 					}
// 					*result = 0;
// 					return;
// 				}
// 			}
// 			if(found == FALSE){
// 				*result = -1;
// 				return;
// 			}
// 		}
// 		else{
// 			splitted_path[splitted_path_idx] = path[path_idx];
// 			splitted_path_idx++;
// 			path_idx++;
// 		} 
// 	}
// }

void readFile(char *buffer, char *path, int *result, char parentIndex) {
	// Search directory path on dirs sector
	char dirs[SECTOR_SIZE];
	char files[SECTOR_SIZE];
	readSector(dirs, DIRS_SECTOR);
	readSector(files, FILES_SECTOR);
	int i = 0;
	int done = FALSE;
	int fileIndex;
	char dirParent = parentIndex;
	char dirName[MAX_DIRNAME];
	int dirNameLength = 0;
	while (!done) {
		// Read current path
		int j = 0;
		while (path[i] != '/' && path[i] != '\0') {
			dirName[j] = path[i];
			dirNameLength++;
			i++;
		}

		// Determine whether current path is a directory or filename
		if (path[i] == '/') {
			// Search dirs for current path
			dirFound = FALSE;
			for (int j = 0; j < SECTOR_SIZE / DIRS_ENTRY_LENGTH; j++) {
				if (dirs[j * DIRS_ENTRY_LENGTH] == parentIndex) {
					int dirNameFound = TRUE;
					for (int k = 0; k < dirNameLength; k++) {
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
			for (int j = 0; j < SECTOR_SIZE / FILES_ENTRY_LENGTH; j++) {
				if (files[j * DIRS_ENTRY_LENGTH] == parentIndex) {
					int fileNameFound = TRUE;
					for (int k = 0; k < dirNameLength; k++) {
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
	char sectors[SECTOR_SIZE];
	readSector(sectors, FILES_SECTOR);
	for (i = 0; i < SECTORS_ENTRY_LENGTH; i++) {
		buffer[fileIndex * SECTORS_ENTRY_LENGTH + i] = sectors[i];
	}
}
// void writeFile(char *buffer, char *path, int *sectors, char parentIndex) {
// 	char map[SECTOR_SIZE];
// 	char dirs[SECTOR_SIZE];
// 	char files[SECTOR_SIZE];
// 	char sectorBuffer[SECTOR_SIZE];
// 	int filesIndex;

// 	readSector(map, MAP_SECTOR);
// 	readSector(dirs, DIRS_SECTOR);
// 	readSector(files, FILES_SECTOR);

// 	// Check whether map has enough empty sectors
// 	int i, j, sectorCount;
// 	for (i = 0, sectorCount = 0; i < MAX_BYTE && sectorCount < *sectors; ++i) {
// 		if (map[i] == EMPTY) {
// 			++sectorCount;
// 		}
// 	}
// 	if (sectorCount < *sectors) {
// 		*sectors = INSUFFICIENT_SECTORS;
// 		return;
// 	} else {
// 		// Check whether files sector has an empty entry
// 		for (filesIndex = 0; filesIndex < MAX_FILES; ++filesIndex) {
// 			if (files[filesIndex * FILES_ENTRY_LENGTH] == '\0') {
// 				break;
// 			}
// 		}

// 		if (filesIndex < MAX_FILES) {
// 			// Check for dirs sector
// 			int i = 0;
// 			int done = FALSE;
// 			int fileIndex;
// 			int dirFound;
// 			int fileFound;
// 			char dirParent = parentIndex;
// 			char dirName[MAX_DIRNAME];
// 			int dirNameLength = 0;
// 			while (!done) {
// 				// Read current path
// 				int j = 0;
// 				while (path[i] != '/' && path[i] != '\0') {
// 					dirName[j] = path[i];
// 					dirNameLength++;
// 					i++;
// 				}

// 				// Determine whether current path is a directory or filename
// 				if (path[i] == '/') {
// 					// Search dirs for current path
// 					dirFound = FALSE;
// 					for (int j = 0; j < SECTOR_SIZE; j+=DIRS_ENTRY_LENGTH) {
// 						if (dirs[j] == parentIndex) {
// 							int dirNameFound = TRUE;
// 							for (int k = 0; k < dirNameLength; k++) {
// 								if (dirs[j + k + 1] != dirName[k]) {
// 									dirNameFound = FALSE;
// 									break;
// 								}
// 							}
// 							if (dirNameFound) {
// 								dirFound = TRUE;
// 								dirParent = j;
// 								dirNameLength = 0;
// 								break;
// 							}
// 						}
// 					}
// 					if (!dirFound) {
// 						*sectors = NOT_FOUND;
// 						return;
// 					}
// 				} else {
// 					// Search files whether filename exist for current path
// 					for (int j = 0; j < SECTOR_SIZE; j+=FILES_ENTRY_LENGTH) {
// 						if (files[j] == parentIndex) {
// 							int fileNameFound = FALSE;
// 							for (int k = 0; k < dirNameLength; k++) {
// 								if (files[j + k + 1] == dirName[k]) {
// 									fileNameFound = TRUE;
// 									break;
// 								}
// 							}
// 							if (fileNameFound) {
// 								*sectors = ALREADY_EXISTS;
// 								return;
// 							}
// 						}
// 					}
// 					done = TRUE;
// 				}
// 			}

// 			// If filename doesn't exist for current path, write to sectors
			
// 				// Find the first empty entry in files sector

// 				// Write data from buffer to the sector found before

// 		} else {
// 			*sectors = INSUFFICIENT_ENTRIES;
// 			return;
// 		}
// 	}
// 	writeSector(map, MAP_SECTOR);
// 	writeSector(dirs, DIRS_SECTOR);
// 	writeSector(files, FILES_SECTOR);
// }

void executeProgram(char *path, int segment, int *result, char parentIndex){ //not tested but probably work
	char buffer[MAX_SECTORS*SECTOR_SIZE];
	int i;
	readFile(buffer, path, result, parentIndex);
	if(*result == 0){
		for(i=0; i<MAX_SECTORS*SECTOR_SIZE; i++){
			putInMemory(segment, i, buffer[i]);
		}
		launchProgram(segment);
	}
	
}


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



