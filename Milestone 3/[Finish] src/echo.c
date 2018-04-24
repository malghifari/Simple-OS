void main() {
    char curdir;
    char argc;   
    char argv[4][16];
    char buff[6];
    int i;
    int succ;

    enableInterrupts();
    interrupt(0x21, 0x21, &curdir, 0, 0);
    interrupt(0x21, 0x22, &argc, 0, 0);
    for (i = 0; i < argc; i++) {interrupt(0x21, 0x23, i, argv[i], 0);}
    if (argc > 0) {
        interrupt(0x21, 0x0, argv[0], 0, 0);
        interrupt(0x21, 0x0, "\n", 0, 0);
    }
    terminateProgram(0);
}