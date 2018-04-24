#define MAX_ARGS 8
#define MAX_ARG_LENGTH 32
#define BUFF_SIZE 64

char compare(char * arr1, char * arr2, int length);
int lengthOf(char * arr);

int main() {
    char dest;
    char succ = 0;
    char curdir;
    char argc = 0;
    char buffer[BUFF_SIZE];
    char command[BUFF_SIZE];
    char filename[36];
    char * argv[MAX_ARGS];
    char running = 1;
    int off;
    int i = 0;
    int segments[8];
    segments[0] = 0x2000; segments[1] = 0x3000;
    segments[2] = 0x4000; segments[3] = 0x5000;
    segments[4] = 0x6000; segments[5] = 0x7000;
    segments[6] = 0x8000; segments[7] = 0x9000;
    enableInterrupts();
    interrupt(0x21, 0x21, &curdir, 0, 0);
    while (running) {
        for (i = 0; i < BUFF_SIZE; i++) {buffer[i] = '\0';}
        interrupt(0x21, 0x0, "$ ", 0, 0);
        interrupt(0x21, 0x1, buffer, 1, 0);
        i = 0;
        while ((buffer[i] != ' ') && (buffer[i] != '\0')) {command[i] = buffer[i]; i++;}
        command[i] = '\0';
        if (i > 0) {
            if ((compare(command, "cd", 2)) && (buffer[i] != '\0')) {
                i++; off = i;
                while ((buffer[i] != ' ') && (buffer[i] != '\0')) {filename[i - off] = buffer[i]; i++;}
                filename[i - off] = '\0';

                if ((i - off == 1) && (compare(filename, "~", 1))) {
                    interrupt(0x21, 0x20, 0xFF, argc, argv);
                    curdir = 0xFF;
                } else {
                    dest = -1;
                    interrupt(0x21, (curdir << 8) | 0x90, filename, &succ, &dest);
                    if (succ) {
                        interrupt(0x21, 0x20, dest, argc, argv);
                        curdir = dest;
                    } else {
                        interrupt(0x21, 0x0, "Directory NOT Found ", 0, 0);
                        interrupt(0x21, 0x0, filename, 0, 0);
                        interrupt(0x21, 0x0, "\n", 0, 0);
                    }
                }
            } else if (compare(command, "pause", 5)) {
                i++;
                off = i;
                while ((buffer[i] != ' ') && (buffer[i] != '\0')) {
                    filename[i - off] = buffer[i]; i++;
                }
                filename[i - off] = '\0';
                if (lengthOf(filename) == 1) {
                    int index = filename[0] - '0';
                    if ((index >= 0) && (index <= 7)) {
                        int segment = segments[index];
                        interrupt(0x21, 0x32, segment, &succ, 0);
                        if (succ == SUCCESS) {interrupt(0x21, 0x0, "Process Paused\n", 0, 0);} 
                        else {interrupt(0x21, 0x0, "Process NOT Found\n", 0, 0);}
                    } else {interrupt(0x21, 0x0, "INVALID PID\n", 0, 0);}
                } else {interrupt(0x21, 0x0, "INVALID PID\n", 0, 0);}
            } else if (compare(command, "resume", 6)) {
                i++; off = i;
                while ((buffer[i] != ' ') && (buffer[i] != '\0')) {
                    filename[i - off] = buffer[i]; i++;
                }
                filename[i - off] = '\0';
                if (lengthOf(filename) == 1) {
                    int index = filename[0] - '0';
                    if ((index >= 0) && (index <= 7)) {
                        int segment = segments[index];
                        interrupt(0x21, 0x33, segment, &succ, 0);
                        if (succ == SUCCESS) {interrupt(0x21, 0x0, "Process Resumed\n", 0, 0);} 
                        else {interrupt(0x21, 0x0, "Process NOT Found\n", 0, 0);}
                    } else {interrupt(0x21, 0x0, "INVALID PID\n", 0, 0);}
                } else {interrupt(0x21, 0x0, "INVALID PID\n", 0, 0);}
            } else if (compare(command, "continue", 8)) {
                i++; off = i;
                while ((buffer[i] != ' ') && (buffer[i] != '\0')) {
                    filename[i - off] = buffer[i]; i++;
                }
                filename[i - off] = '\0';
                if (lengthOf(filename) == 1) {
                    int index = filename[0] - '0';
                    if ((index >= 0) && (index <= 7)) {
                        int segment = segments[index];
                        interrupt(0x21, 0x94, segment, &succ, 0);
                        if (succ == SUCCESS) {interrupt(0x21, 0x0, "Process Resumed\n", 0, 0);} 
                        else {interrupt(0x21, 0x0, "Process NOT Found\n", 0, 0);}
                    } else {interrupt(0x21, 0x0, "INVALID PID\n", 0, 0);}
                } else {interrupt(0x21, 0x0, "INVALID PID\n", 0, 0);}
            } else if (compare(command, "kill", 4)) {
                i++; off = i;
                while ((buffer[i] != ' ') && (buffer[i] != '\0')) {
                    filename[i - off] = buffer[i]; i++;
                }
                filename[i - off] = '\0';
                if (lengthOf(filename) == 1) {
                    int index = filename[0] - '0';
                    if ((index >= 0) && (index <= 7)) {
                        int segment = segments[index];
                        interrupt(0x21, 0x34, segment, &succ, 0);
                        if (succ == SUCCESS){interrupt(0x21, 0x0, "Process Killed\n", 0, 0);} 
                        else{interrupt(0x21, 0x0, "Process NOT Found\n", 0, 0);}
                    } else {interrupt(0x21, 0x0, "INVALID PID\n", 0, 0);}
                } else {interrupt(0x21, 0x0, "INVALID PID\n", 0, 0);}
            } else {
                char buff[MAX_ARGS][MAX_ARG_LENGTH];
                char async = 0;
                argc = 0;
                while (buffer[i] != '\0') {
                    i++; off = i;
                    while ((buffer[i] != ' ') && (buffer[i] != '\0')) {
                        buff[argc][i - off] = buffer[i]; i++;
                    }
                    buff[argc][i - off] = '\0';
                    if (compare(buff[argc], "&", 1)){async = 1;} 
                    else{
                        argv[argc] = buff[argc]; argc++;
                        if (argc >= MAX_ARGS){break;}
                    }
                }
                interrupt(0x21, 0x20, curdir, argc, argv);

                interrupt(0x21, 0xFF91, command, &succ, &dest);
                if (succ) {
                    if (async) {interrupt(0x21, 0xFF92, command, &succ, 0);} 
                    else {interrupt(0x21, 0xFF06, command, &succ, 0);}
                } else {
                    interrupt(0x21, 0x0, "Command NOT Registered ", 0, 0);
                    interrupt(0x21, 0x0, command, 0, 0);
                    interrupt(0x21, 0x0, "\n", 0, 0);
                }
            }
        }
    }
    interrupt(0x21, (0x00 << 8) | 0x07, &succ, 0, 0);
}

char compare(char * arr1, char * arr2, int length) {
    char equal = TRUE;
    int i = 0;
    while ((equal) && (i < length)) {
        equal = arr1[i] == arr2[i];
        if (equal) {
            if (arr1[i] == '\0') {i = length;}
        }
        i++;
    }
    return equal;
}

int lengthOf(char * arr) {
    int l = 0;
    while (arr[l] != '\0'){l++;}
    return l;
}