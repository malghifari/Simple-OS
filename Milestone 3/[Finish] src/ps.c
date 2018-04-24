#include "proc.h"

void main() {
    char curdir;
    char argc;
    char buff[2];
    char argv[4][16];
    int i;
    int stat;
    int succ;
    
    enableInterrupts();
    interrupt(0x21, 0x21, &curdir, 0, 0);
    interrupt(0x21, 0x22, &argc, 0, 0);
    for (i = 0; i < argc; i++) {interrupt(0x21, 0x23, i, argv[i], 0);}
    buff[1] = '\0';
    for (i = 0; i < MAX_SEGMENTS; i++) {
        int m;
        interrupt(0x21, (i << 8) | 0x93, &m, &stat, 0);
        if (m == SEGMENT_USED) {
            interrupt(0x21, 0x0, "PID: ");
            buff[0] = '0' + i;
            interrupt(0x21, 0x0, buff);
            switch (stat) {
                case DEFUNCT:
                    interrupt(0x21, 0x0, " DEFUNCT");
                    break;
                case RUNNING:
                    interrupt(0x21, 0x0, " RUNNING");
                    break;
                case STARTING:
                    interrupt(0x21, 0x0, " STARTING");
                    break;
                case READY:
                    interrupt(0x21, 0x0, " READY");
                    break;
                case PAUSED:
                    interrupt(0x21, 0x0, " PAUSED");
                    break;
                default:
                    interrupt(0x21, 0x0, " UN0KNOWN");
            }
            interrupt(0x21, 0x0, "\n");
        }
    }
    interrupt(0x21, (0x00 << 8) | 0x07, &succ, 0, 0);
}