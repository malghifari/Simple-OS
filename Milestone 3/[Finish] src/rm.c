void main() {
    char curdir;
    char argc;
    char argv[4][32];
    char buff[MAX_FILENAME + 1];
    int i;
    int succ;

    enableInterrupts();
    interrupt(0x21, 0x21, &curdir, 0, 0);
    interrupt(0x21, 0x22, &argc, 0, 0);
    for (i = 0; i < argc; i++) {interrupt(0x21, 0x23, i, argv[i], 0);}
    if (argc > 0) {
        interrupt(0x21, (curdir << 8) | 0x09, argv[0], &succ, 0);
        if (succ != SUCCESS) {
            interrupt(0x21, (curdir << 8) | 0x0A, argv[0], &succ, 0);
        }
        if (succ == NOT_FOUND) {
            interrupt(0x21, 0x0, "File Not Found.\n", 0, 0);      
        } else if (succ != SUCCESS) {
            interrupt(0x21, 0x0, "Cannot be Deleted\n", 0, 0);      
        }
    }
    terminateProgram(0);
}