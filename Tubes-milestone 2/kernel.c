#define MAX_BYTE 256
#define SECTOR_SIZE 512
#define MAX_FILES 16
#define MAX_FILENAME 12
#define MAX_SECTORS 20
#define DIR_ENTRY_LENGTH 32
#define MAP_SECTOR 0x100
#define DIRS_SECTOR 0x101
#define FILES_SECTOR 0x102
#define SECTORS_SECTOR 0x103
#define TRUE 1
#define FALSE 0
#define INSUFFICIENT_SECTORS 0
#define SUCCESS 0
#define NOT_FOUND -1
#define ALREADY_EXIST -2
#define INSUFFICIENT_ENTRIES -3
#define EMPTY 0x00
#define USED 0xFF 
#define SCREEN_SIZE 25*80*2 
#define VIDEO_SEGMENT 0xB000
#define BASE 0x8000

#define ARGS_SECTOR 512

int main();
void handleInterrupt21 (int AX, int BX, int CX, int DX);
void printString(char *buff); 
void readString(char *string); 
int mod(int a, int b);
int div(int a, int b); 
void readSector(char *buffer, int sector); 
void writeSector(char *buffer, int sector); 
void readFile(char *buffer, char *filename, int *success, char parentIndex);
void clear(char *buffer, int length);
void writeFile(char *buffer, char *filename, int *sectors, char parentIndex);
void executeProgram(char *filename, int segment, int *success, char parentIndex);
void clearScreen();
void drawLogo();
int strcmp1(char* s1, char* s2, int lenmax);
int find(int sector, char *str, char parentIndex);
void printChar(char ch);
void terminateProgram();

// void makeDirectory();
// void deleteFile();
// void deleteDirectory();
void putArgs(char curdir, char argc, char *argv);
void getCurdir(char *curdir);
void getArgc(char *argc);
void getArgv(char index, char *argv);
void makeDirectory(char *path, int *success, char parentIndex);
void deleteFile(char *path, int *success, char parentIndex);       
void deleteDirectory(char *path, int *success, char parentIndex);       



int main() {
	int succ=0;
	char s[100];
	makeInterrupt21();

	clearScreen();

	drawLogo();

	clearScreen();

	interrupt(0x10, 0, 0, 0, 0);	

	// makeDirectory("test\0", &succ, 0xFF);
	// makeDirectory("test/argv", &succ, 0xFF);
	// deleteDirectory("test", &succ, 0xFF);
	// printChar(succ + '5');
	// deleteFile("echo", &succ, 0xFF);
	interrupt(0x21, 0x20, 0xFF, 0, 0);
	interrupt(0x21, 0xFF06, "shell\0", 0x2000, &succ);
	// terminateProgram();
	

	while (1){
	}
}

void printString(char *buff){
	int i=0;
	char c = buff[i];
	while (c != '\0'){
		interrupt(0x10, 0xE00+c, 0, 0, 0);
		i++;
		c = buff[i];
	}
}

void printChar(char buff){
	interrupt(0x10, 0xE00+buff, 0, 0, 0);
	printString("\n\n\r");
}

void readString(char *string){
	char c;
	int i = 0;

	while (1){
		c = interrupt(0x16, 0, 0, 0, 0);
		if (c == '\r') {
			interrupt(0x10, 0xE00+'\n', 0, 0, 0);
			interrupt(0x10, 0xE00+'\r', 0, 0, 0);
			string[i++] = 0;
			return;
		}
		else if (c == '\b') {
			if (i>0) {
				interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
				interrupt(0x10, 0xE00 + EMPTY, 0, 0, 0);
				interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
				string[--i] = 0;
			}
		}
		else {
			interrupt(0x10, 0xE00+c, 0, 0, 0);
			string[i++] = c;
		}
	}
}

int mod(int a, int b){
	while (a >= b){
		a = a-b;
	}
	return a;
}

int div(int a, int b){
	int q = 0;
	while (q*b <= a){
		q = q+1;
	}
	return q-1;
}

void readSector(char *buffer, int sector){
	interrupt(0x13, 0x201, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void writeSector(char *buffer, int sector){
	interrupt(0x13, 0x301, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100); 
}

int strcmp1(char* s1, char* s2, int lenmax){
	int j=0;

	while ((s1[j] == s2[j]) && (s1[j] != '\0') && j<lenmax-1) {
		j++;
	}
	if (s1[j] == s2[j])
		return TRUE;
	else
		return FALSE;
}

int searchString(char* buffer, char* filename){
	int i,j;

	for (i=0; i<16; i++){
		j = 0;
		while ((buffer[i*32+j] == filename[j]) && (filename[j] != '\0')) {
			j++;
		}
		if (buffer[i*32+j] == filename[j])
			return i;
	}

	return -1;
}

int find(int sector, char *str, char parentIndex){
	char i;
	char temp[512];

	i = 0;
	readSector(temp, sector);

	for (i=0; i<0x20; i++){
		if (temp[i*0x10] != parentIndex)
			continue;
		if (strcmp1(str, temp+i*0x10+1, 15)) {
			return i;
		}
	}

	return NOT_FOUND;
}

void readFile(char *buffer, char *filename, int *success, char parentIndex){
	char sectors[SECTOR_SIZE];
	char dirname[16];
	char curdir;
	int i, j;
	int idx;
	int sector;

	if (filename[0] == '/') {
		readFile(buffer, filename+1, success, 0xFF);
		return;
	}

	clear(dirname, 16);

	i = 0;
	while (1){
		if ((filename[i] == '/') || (filename[i] == '\0')) {
			break;
		}
		i++;
	}

	if ((filename[i] == '/') && (i > 0)) {
		j = 0;
		while (filename[j] != '/') 
			dirname[j++] = filename[j];
		dirname[j] = '\0';

		idx = find(DIRS_SECTOR, dirname, parentIndex);

		if (idx != NOT_FOUND) {
			readFile(buffer, filename+j+1, success, idx);
		} else {
			*success = NOT_FOUND;
		}

		return;
	}

	idx = find(FILES_SECTOR, filename, parentIndex);

	if (idx == NOT_FOUND) {
		*success = NOT_FOUND;
	} else {
		readSector(sectors, SECTORS_SECTOR);
		i = 0;
		sector = sectors[idx*0x10+i];
		while (sector != '\0'){
			readSector(buffer+512*i,sector);
			i++;
			sector = sectors[idx*0x10+i];
		}
		*success = SUCCESS;
	}
}

void clear(char *buffer, int length){
	int i;
	for (i=0; i<length; ++i){
		buffer[i] = EMPTY;
	}
}

void writeFile(char *buffer, char *filename, int *sectors, char parentIndex){
	char map[SECTOR_SIZE];
	char files[SECTOR_SIZE];
	char dirname[16];
	int i, j;
	int idx;
	int mi;

	if (filename[0] == '/') {
		readFile(buffer, filename+1, sectors, 0xFF);
		return;
	}

	readSector(map, MAP_SECTOR);

	for (mi=0; map[mi]!=0xFF; mi++){
		mi++;
	}

	if (mi==SECTOR_SIZE){
		*sectors = INSUFFICIENT_SECTORS;
		return;
	}

	idx = find(FILES_SECTOR, "\0", 0);

	if (idx == NOT_FOUND){
		*sectors = INSUFFICIENT_ENTRIES;
		return;
	}

	clear(dirname, 16);

	i = 0;
	while (1){
		if ((filename[i] == '/') || (filename[i] == '\0')) {
			break;
		}
		i++;
	}

	if ((filename[i] == '/') && (i > 0)) {
		j = 0;
		while (filename[j] != '/') 
			dirname[j++] = filename[j];
		dirname[j] = '\0';

		idx = find(DIRS_SECTOR, dirname, parentIndex);

		if (idx != NOT_FOUND) {
			writeFile(buffer, filename+j+1, sectors, idx);
		} else {
			*sectors = NOT_FOUND;
		}

		return;
	}

	idx = find(FILES_SECTOR, filename, parentIndex);

	if (idx == NOT_FOUND) {
		*sectors = NOT_FOUND;
	} else {
		idx = find(FILES_SECTOR, "\0", 0);
		readSector(files, FILES_SECTOR);
		i = 0;
		files[idx*0x10] = parentIndex;
		for (i=1; filename[i-1]!='\0'; i++){
			files[idx*0x10+i] = filename[i-1];
		}
		if (i<16)
			files[idx*0x10+i] = '\0';
		writeSector(files, FILES_SECTOR);
		writeSector(buffer, mi);
		*sectors = SUCCESS;
	}
}

void executeProgram(char* name, int segment, int *success, char parentIndex){ 
	int i;
	int address;
	char readingBuffer[MAX_SECTORS*SECTOR_SIZE];

	clear(readingBuffer, MAX_SECTORS*SECTOR_SIZE);
	readFile(readingBuffer, name, success, parentIndex);

	if (*success == 0){	
		for (i=0; i<MAX_SECTORS*SECTOR_SIZE; i++){
			putInMemory(segment,i,readingBuffer[i]);
		} 

		launchProgram(segment);
	}
}

void terminateProgram(){
	int succ;
	char shell[6];
	shell[0] = 's';
	shell[1] = 'h';
	shell[2] = 'e';
	shell[3] = 'l';
	shell[4] = 'l';
	shell[5] = '\0';
	// executeProgram("shell\0", 0x2000, &r, 0xFF);
	interrupt(0x21, 0xFF06, shell, 0x2000, &succ);
}

void clearScreen(){
	int i = 0;
   	while (i < SCREEN_SIZE){
    	putInMemory(VIDEO_SEGMENT, BASE + i*2, '\0');
		putInMemory(VIDEO_SEGMENT, BASE + i*2 + 1, 0);
		i++;
	}
	interrupt(0x10, 0, 0, 0, 0);
}


void handleInterrupt21 (int AX, int BX, int CX, int DX){
	char AL, AH;
	AL = (char) (AX);
	AH = (char) (AX >> 8);

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
			terminateProgram();       
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
			putArgs(BX, CX, DX);       
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


void drawLogo(){

	// Reserved for logo OS
	interrupt(0x21, 0, "\n\r", 0, 0);
	interrupt(0x21, 0, "\n\r", 0, 0);
	interrupt(0x21, 0, "\n\r", 0, 0);
	interrupt(0x21, 0, "\n\r", 0, 0);
	interrupt(0x21, 0, "                                        \r", 0, 0);
	interrupt(0x21, 0, "  &&                                    \r", 0, 0);
	interrupt(0x21, 0, "   %&,                                  \r", 0, 0);
	interrupt(0x21, 0, "    //*                           ,(    \r", 0, 0);
	interrupt(0x21, 0, "     *,,,,.                   ,,,***    \r", 0, 0);
	interrupt(0x21, 0, "     *******,,.           ,,,,*****.    \r", 0, 0);
	interrupt(0x21, 0, "      ***********,,,,,,,**********      \r", 0, 0);
	interrupt(0x21, 0, "       ,************************.       \r", 0, 0);
	interrupt(0x21, 0, "          .******************.          \r", 0, 0);
	interrupt(0x21, 0, "           .,*/#%%####%%#(/*,.          \r", 0, 0);
	interrupt(0x21, 0, "                                        \r", 0, 0);
	interrupt(0x21, 0, "  _                                     \r", 0, 0);
	interrupt(0x21, 0, " | |                                    \r", 0, 0);
	interrupt(0x21, 0, " | | _   ____ ____   ____ ____   ____   \r", 0, 0);
	interrupt(0x21, 0, " | || \\ / _  |  _ \\ / _  |  _ \\ / _  |  \r", 0, 0);
	interrupt(0x21, 0, " | |_) | ( | | | | ( ( | | | | ( ( | |  \r", 0, 0);
	interrupt(0x21, 0, " |____/ \\_||_|_| |_|\\_||_|_| |_|\\_||_|  \r", 0, 0);
	interrupt(0x21, 0, "\n\r", 0, 0);
	interrupt(0x21, 0, "\n\r", 0, 0);
	interrupt(0x21, 0, "\n\r", 0, 0);

	interrupt(0x21, 0, "Press any key to Continue ...", 0, 0);	
	interrupt(0x16, 0, 0, 0, 0);
}


void putArgs (char curdir, char argc, char *argv) {   
	char args[SECTOR_SIZE];   
	int i, j, p;   

	clear(args, SECTOR_SIZE); 
   	args[0] = curdir;   
   	args[1] = argc;   
   	i = 0;   
   	j = 0;   

   	if (argc > 0){
   		for (p=2; p<ARGS_SECTOR && argv[i]!='\0'; p++){
	   		if (argv[i] == 0x20){
	   			args[p] = '\0';
	   		} else {
	   			args[p] = argv[i];
	   		}
	   		i++;
	   	}	
   	}
   	
   	writeSector(args, ARGS_SECTOR); 
   	// while (1);
}

void getCurdir(char *curdir){
	char args[SECTOR_SIZE];   
	readSector(args, ARGS_SECTOR);  
	*curdir = args[0];
}

void getArgc(char *argc){
	char args[SECTOR_SIZE];   
	readSector(args, ARGS_SECTOR);   
	*argc = args[1]; 
}

void getArgv(char index, char *argv){
	char args[SECTOR_SIZE];
	int i, j, p;   
	readSector(args, ARGS_SECTOR);     
	i = 0;   
	j = 0;   

	for (p = 2; p < ARGS_SECTOR; ++p) {     
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

void makeDirectory(char *path, int *success, char parentIndex){
	int i, j;
	int idx;
	char dirs[512];
	char dirname[16];

	if (path[0] == '/'){
		makeDirectory(path+1, success, 0xFF);
		return;
	}

	idx = find(DIRS_SECTOR, "\0", 0);
	if (idx == -1){
		*success = INSUFFICIENT_ENTRIES;
		return;
	} else {
		clear(dirname, 16);

		i = 0;
		while (1){
			if ((path[i] == '/') || (path[i] == '\0')) {
				break;
			}
			i++;
		}

		if ((path[i] == '/') && (i > 0)) {
			j = 0;
			while (path[j] != '/') 
				dirname[j++] = path[j];
			dirname[j] = '\0';

			idx = find(DIRS_SECTOR, dirname, parentIndex);

			if (idx != NOT_FOUND) {
				makeDirectory(path+j+1, success, idx);
			} else {
				*success = NOT_FOUND;
			}

			return;
		}

		idx = find(DIRS_SECTOR, path, parentIndex);

		if (idx == NOT_FOUND) {
			idx = find(DIRS_SECTOR, "\0", 0);
			readSector(dirs, DIRS_SECTOR);
			i = 0;
			dirs[idx*0x10] = parentIndex;
			for (i=1; path[i-1]!='\0'; i++){
				dirs[idx*0x10+i] = path[i-1];
			}
			if (i<16)
				dirs[idx*0x10+i] = '\0';
			writeSector(dirs, DIRS_SECTOR);
			*success = SUCCESS;
		} else {
			*success = ALREADY_EXIST;
		}
	}
}

void deleteFile(char *path, int *success, char parentIndex){
	int i, j;
	int idx;
	char files[512];
	char sectors[512];
	char map[512];
	char dirname[16];

	if (path[0] == '/'){
		deleteFile(path+1, success, 0xFF);
		return;
	}

	clear(dirname, 16);

	i = 0;
	while (1){
		if ((path[i] == '/') || (path[i] == '\0')) {
			break;
		}
		i++;
	}

	if ((path[i] == '/') && (i > 0)) {
		j = 0;
		while (path[j] != '/') 
			dirname[j++] = path[j];
		dirname[j] = '\0';

		idx = find(DIRS_SECTOR, dirname, parentIndex);

		if (idx != NOT_FOUND) {
			deleteFile(path+j+1, success, idx);
		} else {
			*success = NOT_FOUND;
		}

		return;
	}

	idx = find(FILES_SECTOR, path, parentIndex);

	if (idx != NOT_FOUND) {
		readSector(files, FILES_SECTOR);
		readSector(sectors, SECTORS_SECTOR);
		readSector(map, MAP_SECTOR);
		files[idx*0x10] = 0;
		files[idx*0x10+1] = 0;
		for (i=idx*0x10; i<idx*0x10+0x10 && sectors[i]!='\0'; i++){
			map[sectors[i]] = '\0';
		}
		writeSector(files, FILES_SECTOR);
		writeSector(map, MAP_SECTOR);
		*success = SUCCESS;
	} else {
		*success = NOT_FOUND;
	}
}

void deleteDirectory(char *path, int *success, char parentIndex){
	int i, j;
	int idx;
	char dirs[512];
	char files[512];
	char dirname[16];
	char z;

	if (path[0] == '/'){
		deleteDirectory(path+1, success, 0xFF);
		return;
	}

	clear(dirname, 16);

	i = 0;
	while (1){
		if ((path[i] == '/') || (path[i] == '\0')) {
			break;
		}
		i++;
	}

	if ((path[i] == '/') && (i > 0)) {
		j = 0;
		while (path[j] != '/') 
			dirname[j++] = path[j];
		dirname[j] = '\0';

		idx = find(DIRS_SECTOR, dirname, parentIndex);

		if (idx != NOT_FOUND) {
			deleteDirectory(path+j+1, success, idx);
		} else {
			*success = NOT_FOUND;
		}

		return;
	}

	idx = find(DIRS_SECTOR, path, parentIndex);

	if (idx != NOT_FOUND) {
		readSector(files, FILES_SECTOR);
		for (i=0; i<0x20; i++){
			deleteFile(files+i*0x10+1, &j, idx);
		}
		writeSector(files, FILES_SECTOR);
		readSector(dirs, DIRS_SECTOR);
		dirs[idx*0x10] = 0;
		dirs[idx*0x10+1] = 0;
		writeSector(dirs, DIRS_SECTOR);
		for (i=0; i<0x20; i++){
			z = idx & 0xFF;
			if (dirs[i*0x10] == z && dirs[i*0x10+1] != '\0'){
				deleteDirectory(dirs+i*0x10+1, &j, idx);
			}
		}
		*success = SUCCESS;
	} else {
		*success = NOT_FOUND;
	}
	
}