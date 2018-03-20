#define MAX_BYTE 256
#define SECTOR_SIZE 512
#define MAX_FILES 16
#define MAX_FILENAME 12
#define MAX_SECTORS 20
#define DIR_ENTRY_LENGTH 32
#define MAP_SECTOR 1
#define DIR_SECTOR 2
#define TRUE 1
#define FALSE 0
#define INSUFFICIENT_SECTORS 0
#define NOT_FOUND -1
#define INSUFFICIENT_DIR_ENTRIES -1
#define EMPTY 0x00
#define USED 0xFF

void handleInterrupt21 (int AX, int BX, int CX, int DX);
void printString(char *string);
void readString(char **string);
int mod(int a, int b);
int div(int a, int b);
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
void readFile(char *buffer, char *filename, int *success);
void clear(char *buffer, int length);
void writeFile(char *buffer, char *filename, int *sectors);
void executeProgram(char *filename, int segment, int *success);

int main() {
	int found;
	char buffer[SECTOR_SIZE*MAX_SECTORS];
	makeInterrupt21();
	interrupt(0x21, 0x4, buffer, "key.txt", &found);
	if(found == TRUE){
		interrupt(0x21, 0x0, buffer, 0, 0);
	}
	else{
		interrupt(0x21, 0x6, "keyproc", 0x2000, &found);
	}
	while (1);
}
void handleInterrupt21 (int AX, int BX, int CX, int DX){
	switch (AX) {
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
			readFile(BX, CX, DX);
			break;
		case 0x5:
			writeFile(BX, CX, DX);
			break;
		case 0x6:
			executeProgram(BX, CX, DX);
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
			// interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
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

void writeFile(char *buffer, char *filename, int *sectors){
	char map[SECTOR_SIZE];
	char dir[SECTOR_SIZE];
	char sectorBuffer[SECTOR_SIZE];
	int dirIndex;
	readSector(map, MAP_SECTOR);
	readSector(dir, DIR_SECTOR);

	for (dirIndex = 0; dirIndex < MAX_FILES; ++dirIndex) {
		if (dir[dirIndex * DIR_ENTRY_LENGTH] == '\0') {
		break;
		}
	}
	if (dirIndex < MAX_FILES) {
		int i, j, sectorCount;
		for (i = 0, sectorCount = 0; i < MAX_BYTE && sectorCount < *sectors; ++i) {
			if (map[i] == EMPTY) {
				++sectorCount;
			}
		}
		if (sectorCount < *sectors) {
			*sectors = INSUFFICIENT_SECTORS;
			return;
		}
		else {
			clear(dir + dirIndex * DIR_ENTRY_LENGTH, DIR_ENTRY_LENGTH);
			for (i = 0; i < MAX_FILENAME; ++i) {
				if (filename[i] != '\0') {
					dir[dirIndex * DIR_ENTRY_LENGTH + i] = filename[i];
				}
				else {
					break;
				}
			}
			for (i = 0, sectorCount = 0; i < MAX_BYTE && sectorCount < *sectors; ++i) {
				if (map[i] == EMPTY) {
					map[i] = USED;
					dir[dirIndex * DIR_ENTRY_LENGTH + MAX_FILENAME +
					sectorCount] = i;
					clear(sectorBuffer, SECTOR_SIZE);
					for (j = 0; j < SECTOR_SIZE; ++j) {
						sectorBuffer[j] = buffer[sectorCount * SECTOR_SIZE + j];
					}
					writeSector(sectorBuffer, i);
					++sectorCount;
				}
			}
		}
	}
	else {
		*sectors = INSUFFICIENT_DIR_ENTRIES;
		return;
	}
	writeSector(map, MAP_SECTOR);
	writeSector(dir, DIR_SECTOR);
}

void readFile(char* buffer, char* filename, int *success){
	char dir[SECTOR_SIZE];
	int found = FALSE;
	int bener;
	int idx, i, j;
	readSector(dir, DIR_SECTOR);
	
	for(i=0; i<SECTOR_SIZE; i+=DIR_ENTRY_LENGTH){
		bener = TRUE;
		for(j=i; j<MAX_FILENAME+i; j++){
			if(dir[j] != filename[j-i]){
				bener = FALSE;
			}
			if(filename[j-i] == 0){
				break;
			}
		}
		if(bener == TRUE){
			found = TRUE;
			idx = i;
			break;
		}
	}
	if(found == FALSE){
		*success = FALSE;
		return;
	}
	else if(found == TRUE){
		for(i=0; i< MAX_SECTORS; i++){
			if(dir[idx+i+MAX_FILENAME] == 0){
				*success = TRUE;
				return;
			}
			readSector(buffer + i*SECTOR_SIZE, dir[idx+i+MAX_FILENAME]);
		}
		*success = TRUE;
	}

}

void executeProgram(char *filename, int segment, int *success){ //not tested but probably work
	char buffer[MAX_SECTORS*SECTOR_SIZE];
	int i;
	int success2;
	readFile(buffer, filename, &success2);
	*success = success2;
	if(*success == TRUE){
		for(i=0; i<MAX_SECTORS*SECTOR_SIZE; i++){
			putInMemory(segment, i, buffer[i]);
		}
		launchProgram(segment);
	}
	
}


