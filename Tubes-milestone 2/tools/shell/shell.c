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

int strcmp1(char* s1, char* s2);
void clear(char *buffer, int length);
void parseParam(char* buffer, char* filename, char* argc, char* argv);
int find(int sector, char *str, char parentIndex);
char changeDirectory(char* dir, char* parentIndex);

main(){
	char curdir; 
	char cmd[80];
	char buffer[512*20];  
	char argc;   
	char* argv;
	char filename[30];
	int succ;

	interrupt(0x21, 0x21, &curdir, 0, 0);

	while (1){
		clear(cmd, 80);
		clear(filename, 30);
		interrupt(0x21, 0, "$/", 0, 0);
		interrupt(0x21, 1, cmd, 0, 0);

		if (strcmp1(cmd, "cd")){
			changeDirectory(cmd+3, &curdir);
			interrupt(0x21, 0, cmd+3, 0, 0);
		}
		else if (strcmp1(cmd,"./")){
			parseParam(cmd+2, filename, &argc, argv);
			interrupt(0x21, 6, filename, 0x2000, &succ);
		}
		else {
			parseParam(cmd, filename, &argc, &argv);
			interrupt(0x21, 0x20, curdir, argc, argv);
			interrupt(0x21, 0xFF06, filename, 0x2000, &succ);
			if (succ != 0){
				interrupt(0x21, 0, "Wrong Command\n\r", 0, 0);
			}
		}
	}

	interrupt(0x21, 0x7, 0, 0, 0);
}

int strcmp1(char* s1, char* s2){
	int j=0;

	while ((s1[j] == s2[j]) && (s2[j] != '\0') && (s1[j] != 0x20)) {
		j++;
	}
	if ((s1[j] == s2[j]) || (s1[j] == 0x20))
		return TRUE;
	else
		return FALSE;
}

void clear(char *buffer, int length){
	int i;
	for (i=0; i<length; ++i){
		buffer[i] = 0;
	}
}

void printChar(char buff){
	interrupt(0x10, 0xE00+buff, 0, 0, 0);
}

void parseParam(char* buffer, char* filename, char* argc, char** argv){
	int i, j;
	char temp[512];
	clear(filename, 30);
	clear(argv, 16*4);
	*argc = 0;

	i = 0;
	j = 0;

	while (buffer[i] != 0 && buffer[i] != 0x20){
		filename[j++] = buffer[i++];
	}

	if (buffer[i] == 0x20){
		*argv = buffer+i+1;
		i++;
		j = 0;
		while (buffer[i] != 0){
			if (buffer[i] == 0x20){
				*argc += 1;
			}
			i++;
		}
		*argc += 1;
	}
}

char changeDirectory(char* dir, char* parentIndex){
	int idx;

	idx = find(DIRS_SECTOR, dir, *parentIndex);	
	if (idx == -1){
		interrupt(0x21, 0x0, "Directory not found\n\r");
	} else {
		*parentIndex = idx;
	}
}

int find(int sector, char *str, char parentIndex){
	int i;
	char temp[512];

	i = 0;
	interrupt(0x21, 0x2, temp, sector, 0);

	for (i=0; i<0x20; i++){
		if (temp[i*0x10] != parentIndex)
			continue;
		if (strcmp1(str, temp+i*0x10+1, 15)) {
			return i;
		}
	}

	return NOT_FOUND;
}