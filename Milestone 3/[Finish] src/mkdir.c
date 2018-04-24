char compare(char * arr1, char * arr2, int length);

void main() {
    char curdir;
    char argc;
    char dest;
    char argv[4][32];
    char buff[MAX_FILENAME + 1];
    char mode = 0;
    int i;
    int succ;

    enableInterrupts();
    interrupt(0x21, 0x21, &curdir, 0, 0);
    interrupt(0x21, 0x22, &argc, 0, 0);
    for (i = 0; i < argc; i++) {interrupt(0x21, 0x23, i, argv[i], 0);}
    if (argc > 0) {
        interrupt(0x21, (curdir << 8) | 0x08, argv[0], &succ, 0);
        if (succ == ALREADY_EXISTS) {interrupt(0x21, 0x0, "Directory Found.\n", 0, 0);} 
        else if (succ == NOT_FOUND) {interrupt(0x21, 0x0, "Directory Not Found.\n", 0, 0);}
    }
    terminateProgram(0);
}