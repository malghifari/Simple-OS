void main() {
    char curdir;
    char argc;
    char argv[4][16];
    char buff[MAX_FILENAME + 1];
    char files[SECTOR_SIZE];
    char dirs[SECTOR_SIZE];
    int i, j;
    int succ;

    enableInterrupts();
    interrupt(0x21, 0x21, &curdir, 0, 0);
    interrupt(0x21, 0x22, &argc, 0, 0);
    for (i = 0; i < argc; i++) {interrupt(0x21, 0x23, i, argv[i], 0);}
    interrupt(0x21, 0x02, dirs, DIRS_SECTOR);
    interrupt(0x21, 0x02, files, FILES_SECTOR);
    for (i = 0; i < MAX_DIRS; i++) {
        if ((dirs[i * ENTRY_LENGTH + 1] != '\0') && (dirs[i * ENTRY_LENGTH] == curdir)) {
            j = 0;
            while ((j < MAX_FILENAME) && (dirs[i * ENTRY_LENGTH + 1 + j] != '\0')) {
                buff[j] = dirs[i * ENTRY_LENGTH + 1 + j]; j++;
            }
            buff[j] = '\0';
            interrupt(0x21, 0x0, " Directory = ", 0, 0);
            interrupt(0x21, 0x0, buff, 0, 0);
            interrupt(0x21, 0x0, "\n", 0, 0);
        }
    }
    for (i = 0; i < MAX_FILES; i++) {
        if ((files[i * ENTRY_LENGTH + 1] != '\0') && (files[i * ENTRY_LENGTH] == curdir)) {
            j = 0;
            while ((j < MAX_FILENAME) && (files[i * ENTRY_LENGTH + 1 + j] != '\0')) {
                buff[j] = files[i * ENTRY_LENGTH + 1 + j]; j++;
            }
            buff[j] = '\0';
            interrupt(0x21, 0x0, " Files = ", 0, 0);
            interrupt(0x21, 0x0, buff, 0, 0);
            interrupt(0x21, 0x0, "\n", 0, 0);
        }
    }
    terminateProgram(0);
}