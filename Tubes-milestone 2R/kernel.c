#define MAX_BYTE 256
#define SECTOR_SIZE 512
#define MAX_FILES 32
#define MAX_FILENAME 15
#define MAX_SECTORS 512
#define DIR_ENTRY_LENGTH 16
#define MAP_SECTOR 0x100
#define DIR_SECTOR 0x101
#define FILE_SECTOR 0x102
#define SECTOR_SECTOR 0x103
#define TRUE 1
#define FALSE 0
#define INSUFFICIENT_SECTORS 0
#define INSUFFICIENT_DIR_ENTRIES -1
#define ALREADY_EXIST -2
#define INSUFFICIENT_ENTRIES -3
#define NOT_FOUND -1
#define EMPTY 0x00
#define USED 0xFF
#define NEWLINE "\r\n"
#define ARGS_SECTOR 512

void handleInterrupt21(int AX, int BX, int CX, int DX);

void printString(char *string);
void readString(char *string);
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
void readFile(char *buffer, char *filename, int *success, char parentIndex);
void writeFile(char *buffer, char *filename, int *sectors, char parentIndex);
void executeProgram(char *filename, int segment, int *success, char p);
void terminateProgram(int *result);
void makeDirectory(char *path, int *result, char parentIndex);
void deleteFile(char *path, int *result, char parentIndex);
void deleteDirectory(char *path, int *result, char parentIndex);
void getArgv(char index, char *argv);
void putArgs(char curdir, char argc, char **argv);
void getCurdir(char *curdir);
void getArgc(char *argc);

// fungsi pembantu
int mod(int a, int b);
int div(int a, int b);
void clear(char *buffer, int length);
void printChar(char c);
void printCharval(char c);
void findDirectory(char *name, char *result, char parentIndex, int *success);
void findFile(char *name, char *result, char parentIndex, int *success);
void listFiles();
void hexSector(int s);
void deleteFileByIndex(char index, int *success);
void deleteDirectoryByIndex(char index, int *success);
int strlen(char *string);

int main()
{
    int success;

    makeInterrupt21();

    putArgs(0xFF, 0, 0);
    executeProgram("shell", 0x2000, &success, 0xFF);
    while (1)
        ;
}

void handleInterrupt21(int AX, int BX, int CX, int DX)
{
    char AL, AH;
    AL = (char)(AX);
    AH = (char)(AX >> 8);
    /*
    printChar('[');
    printCharval(AL);
    printChar(']');
    */
    switch (AL)
    {
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
        readFile(BX, CX, DX, AH);
        break;
    case 0x5:
        writeFile(BX, CX, DX, AH);
        break;
    case 0x6:
        executeProgram(BX, CX, DX, AH);
        break;
    case 0x7:
        terminateProgram(BX);
        break;
    case 0x8:
        makeDirectory(BX, CX, AH);
        break;
    case 0x9:
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
    case 0x23:
        getArgv(BX, CX);
        break;
    default:
        printString("Invalid interrupt");
    }
}

void printChar(char c)
{
    interrupt(0x10, 0xE00 + c, 0, 0, 0);
}

void printString(char *string)
{
    int i = 0;
    while (string[i] != 0x00)
    {
        interrupt(0x10, 0xE00 + string[i], 0, 0, 0);
        i++;
    }
}

void readString(char *string)
{
    int slen = 0;
    char c;

    c = interrupt(0x16, 0, 0, 0, 0);
    while (c != 0xD)
    {
        if (c == '\b')
        {
            if (slen != 0)
            {

                printChar('\b');
                printChar(' ');
                printChar('\b');
                slen -= 1;
            }
        }
        else
        {
            printChar(c);
            string[slen] = c;
            slen += 1;
        }
        c = interrupt(0x16, 0, 0, 0, 0);
    }
    printChar('\r');
    printChar('\n');
    string[slen] = 0;
}

int mod(int a, int b)
{
    while (a >= b)
    {
        a = a - b;
    }
    return a;
}

int div(int a, int b)
{
    int q = 0;
    while (q * b <= a)
    {
        q = q + 1;
    }
    return q - 1;
}

void readSector(char *buffer, int sector)
{
    //printString("readsector\r\n");
    interrupt(0x13, 0x201, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void writeSector(char *buffer, int sector)
{
    //printString("writesector\r\n");
    interrupt(0x13, 0x301, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void readFile(char *buffer, char *filename, int *success, char parentIndex)
{
    char dir[SECTOR_SIZE];
    char files[SECTOR_SIZE];
    char sector[SECTOR_SECTOR];
    char same;
    char exist = FALSE;
    char nama[MAX_FILENAME];
    int dir_idx, f_idx, i, fname_len, count;
    int s = -1;
    char res;

    /*
    printString(filename);
    printCharval(parentIndex);
    */
    // hitung panjang filename
    i = 0;
    fname_len = 0;
    while (filename[i] != 0)
    {
        i++;
        fname_len++;
    }
    // jika fname_len == 0, berarti masukan string kosong
    if (fname_len == 0)
    {
        *success = -1;
        return;
    }

    // cari karakter / pertama
    i = 0;
    while ((filename[i] != 0) && (filename[i] != '/'))
    {
        nama[i] = filename[i];
        i++;
    }
    nama[i] = 0;

    if (filename[i] == '/')
    {
        // mencari directory, lalu cari anak rekursif
        // cari name di dalam dirsector
        findDirectory(nama, &res, parentIndex, &s);
        if (s == -1)
        {
            *success = -1;
            return;
        }

        readFile(buffer, filename + i + 1, success, res);
    }
    else
    {
        readSector(sector, SECTOR_SECTOR);
        // mencari file name

        findFile(nama, &res, parentIndex, &s);
        if (s == -1)
        {
            *success = NOT_FOUND;
            return;
        }

        // indeks file ada di res
        // load data sector ke buffer
        for (i = 0; (i < 0x10) && (sector[res * DIR_ENTRY_LENGTH + i] != 0); i++)
        {
            // load data di sector s
            readSector(buffer + i * SECTOR_SIZE, sector[res * DIR_ENTRY_LENGTH + i]);
            //printCharval(i);
        }
        *success = 0;
        //printCharval('f');
    }
}

void clear(char *buffer, int length)
{
    int i;

    for (i = 0; i < length; ++i)
        buffer[i] = EMPTY;
}

void writeFile(char *buffer, char *path, int *sectors, char parentIndex)
{
    // menulis data dari buffer ke file path
    char map[SECTOR_SIZE];
    char file[SECTOR_SIZE];
    char nama[MAX_FILENAME];
    char sector[SECTOR_SIZE];
    int i, count, fname_len, s;
    char f_idx, res, sec_idx;
    int found = FALSE;

    // memeriksa apakah masih ada 16 sektor kosong dari sektor MAP
    count = 0;
    readSector(map, MAP_SECTOR);
    for (i = 0; i < SECTOR_SIZE; i++)
    {
        if (map[i] == USED)
        {
            continue;
        }
        count++;
        if (count >= 16)
            break;
    }
    if (count < 16)
    {
        *sectors = INSUFFICIENT_SECTORS;
        return;
    }

    // memeriksa apakah masih ada entry kosong di sektor FILES
    readSector(file, FILE_SECTOR);
    for (i = 0; i < MAX_FILES; i++)
    {
        // cek apakah entry ke-i kosong
        if (file[i * DIR_ENTRY_LENGTH + 1] == 0)
        {
            found = TRUE;
            f_idx = i;
            break;
        }
    }
    if (found == FALSE)
    {
        *sectors = INSUFFICIENT_ENTRIES;
        return;
    }

    // memeriksa apakah path masih ada folder atau sudah file
    // hitung panjang filename
    i = 0;
    fname_len = 0;
    while (path[i] != 0)
    {
        i++;
        fname_len++;
    }
    // jika fname_len == 0, berarti masukan string kosong
    if (fname_len == 0)
    {
        *sectors = -1;
        return;
    }

    // cari karakter / pertama
    i = 0;
    while ((path[i] != 0) && (path[i] != '/'))
    {
        nama[i] = path[i];
        i++;
    }
    nama[i] = 0;

    if (path[i] == '/')
    {
        // mencari directory, lalu cari anak rekursif
        // cari name di dalam dirsector
        findDirectory(nama, &res, parentIndex, &s);
        if (s == -1)
        {
            *sectors = NOT_FOUND;
            return;
        }

        writeFile(buffer, path + i + 1, sectors, res);
    }
    else
    {
        int buflen;
        // cek apakah file sudah ada
        // mencari file name
        findFile(nama, &res, parentIndex, &s);
        if (s == 0)
        {
            *sectors = ALREADY_EXIST;
            return;
        }

        // entry kosong pertama ada di f_idx
        // set parent index
        file[f_idx * DIR_ENTRY_LENGTH] = parentIndex;
        // write nama
        for (i = 0; i < fname_len; i++)
            file[f_idx * DIR_ENTRY_LENGTH + 1 + i] = path[i];

        readSector(sector, SECTOR_SECTOR);

        // tulis data
        buflen = strlen(buffer) / SECTOR_SIZE + 1;
        sec_idx = 0;
        for (i = 0; i < buflen; i++)
        {
            // cari sektor pertama yang kosong dari map
            for (; sec_idx < MAX_SECTORS; sec_idx++)
                if (map[sec_idx] == EMPTY)
                    break;

            // tulis data ke sector sec_idx
            writeSector(buffer + i * SECTOR_SIZE, sec_idx);
            sector[f_idx * DIR_ENTRY_LENGTH + i] = sec_idx;
            map[sec_idx] == USED;
            sec_idx++;
        }

        writeSector(sector, SECTOR_SECTOR);
        writeSector(file, FILE_SECTOR);
        writeSector(map, MAP_SECTOR);
        *sectors = 1;
    }
}

void executeProgram(char *filename, int segment, int *success, char parentIndex)
{
    char buffer[16 * SECTOR_SIZE];
    clear(buffer, 16 * SECTOR_SIZE);

    readFile(buffer, filename, success, parentIndex);
    if (*success == 0)
    {
        int i;
        //found

        for (i = 0; i < 16 * SECTOR_SIZE; ++i)
            putInMemory(segment, i, buffer[i]);
        launchProgram(segment);
        *success = TRUE;
    }
    else
    {
        *success = FALSE;
    }
}

void terminateProgram(int *result)
{
    char shell[6];
    char curdir;
    shell[0] = 's';
    shell[1] = 'h';
    shell[2] = 'e';
    shell[3] = 'l';
    shell[4] = 'l';
    shell[5] = 0;
    getCurdir(&curdir);
    putArgs(curdir, 0, 0);
    executeProgram(shell, 0x2000, result, 0xFF);
}

void makeDirectory(char *path, int *result, char parentIndex)
{
    char dir[SECTOR_SIZE];
    char same;
    char exist = FALSE;
    char nama[MAX_FILENAME];
    int dir_idx, i, fname_len, count;
    int s = -1;
    char res;
    int found = FALSE;

    // memeriksa apakah masih ada entry kosong di sektor DIR
    readSector(dir, DIR_SECTOR);
    for (i = 0; i < MAX_FILES; i++)
    {
        // cek apakah entry ke-i kosong
        if (dir[i * DIR_ENTRY_LENGTH + 1] == 0)
        {
            found = TRUE;
            dir_idx = i;
            break;
        }
    }
    if (found == FALSE)
    {
        *result = INSUFFICIENT_ENTRIES;
        return;
    }
    // hitung panjang filename
    i = 0;
    fname_len = 0;
    while (path[i] != 0)
    {
        i++;
        fname_len++;
    }
    // jika fname_len == 0, berarti masukan string kosong
    if (fname_len == 0)
    {
        *result = NOT_FOUND;
        return;
    }

    // cari karakter / pertama
    i = 0;
    while ((path[i] != 0) && (path[i] != '/'))
    {
        nama[i] = path[i];
        i++;
    }
    nama[i] = 0;

    if (path[i] == '/')
    {
        // mencari directory, lalu cari anak rekursif
        // cari name di dalam dirsector
        findDirectory(nama, &res, parentIndex, &s);
        if (s == -1)
        {
            *result = NOT_FOUND;
            return;
        }

        makeDirectory(path + i + 1, result, res);
    }
    else
    {
        // mencari dir name
        // cek apakah sudah ada dir
        findDirectory(nama, &res, parentIndex, &s);
        if (s != -1)
        {
            *result = ALREADY_EXIST;
            return;
        }
        dir[dir_idx * DIR_ENTRY_LENGTH] = parentIndex;
        for (i = 0; (i < MAX_FILENAME) && (path[i] != 0); i++)
            dir[dir_idx * DIR_ENTRY_LENGTH + 1 + i] = path[i];

        writeSector(dir, DIR_SECTOR);
        *result = 0;
    }
}

void deleteFile(char *path, int *result, char parentIndex)
{
    char same;
    char exist = FALSE;
    char nama[MAX_FILENAME];
    int f_idx, i, fname_len, count;
    int s = -1;
    char res;
    int found = FALSE;

    printString("deletefile ");
    printString(path);
    printChar(' ');
    printCharval(parentIndex);
    printString("\r\n");

    // hitung panjang filename
    i = 0;
    fname_len = 0;
    while (path[i] != 0)
    {
        i++;
        fname_len++;
    }
    // jika fname_len == 0, berarti masukan string kosong
    if (fname_len == 0)
    {
        // printString("String kosong");
        *result = -1;
        return;
    }

    // cari karakter / pertama
    i = 0;
    while ((path[i] != 0) && (path[i] != '/'))
    {
        nama[i] = path[i];
        i++;
    }
    nama[i] = 0;

    if (path[i] == '/')
    {
        // mencari directory, lalu cari anak rekursif
        // cari name di dalam dirsector
        findDirectory(nama, &res, parentIndex, &s);
        if (s == -1)
        {
            *result = -1;
            return;
        }

        deleteFile(path + i + 1, result, res);
    }
    else
    {
        // mencari file name
        // cek apakah tidak ada file
        findFile(nama, &res, parentIndex, &s);
        if (s == -1)
        {
            *result = -1;
            return;
        }

        deleteFileByIndex(res, result);
    }
}
void deleteFileByIndex(char index, int *success)
{
    char file[SECTOR_SIZE];
    char map[SECTOR_SIZE];
    char sector[SECTOR_SIZE];
    int i, j;

    readSector(file, FILE_SECTOR);

    if (file[index * DIR_ENTRY_LENGTH + 1] == 0)
    {
        *success = 0;
        return;
    }

    readSector(map, MAP_SECTOR);
    readSector(sector, SECTOR_SECTOR);

    // delete file
    // ubah byte pertama nama file
    file[index * DIR_ENTRY_LENGTH + 1] = 0;

    // mengosongkan sector
    for (i = 0; i < 16; i++)
    {
        if (sector[index * DIR_ENTRY_LENGTH + i] == 0)
            break;
        map[sector[index * DIR_ENTRY_LENGTH + i]] = EMPTY;
    }
    writeSector(file, FILE_SECTOR);
    writeSector(map, MAP_SECTOR);

    *success = 0;
}

void deleteDirectoryByIndex(char index, int *success)
{
    char file[SECTOR_SIZE];
    char dir[SECTOR_SIZE];
    char map[SECTOR_SIZE];
    char sector[SECTOR_SIZE];
    char i, j;

    readSector(dir, DIR_SECTOR);
    readSector(map, MAP_SECTOR);
    readSector(sector, SECTOR_SECTOR);
    readSector(file, FILE_SECTOR);

    if (dir[index * DIR_ENTRY_LENGTH + 1] == 0)
    {
        *success = 0;
        return;
    }

    // hapus nama directory di index
    dir[index * DIR_ENTRY_LENGTH + 1] = 0;

    // hapus semua file yang mempunyai parent index
    for (i = 0; i < MAX_FILES; i++)
        if (file[i * DIR_ENTRY_LENGTH] == index)
            deleteFileByIndex(i, success);

    // hapus semua directory yang mempunyai parent index
    for (i = 0; i < MAX_FILES; i++)
        if (dir[i * DIR_ENTRY_LENGTH] == index)
            deleteDirectoryByIndex(i, success);

    writeSector(file, FILE_SECTOR);
    writeSector(map, MAP_SECTOR);
    writeSector(dir, DIR_SECTOR);

    *success = 0;
}

void deleteDirectory(char *path, int *result, char parentIndex)
{
    char same, curdir;
    char exist = FALSE;
    char nama[MAX_FILENAME];
    int dir_idx, i, fname_len, count;
    int s = -1;
    char res;
    int found = FALSE;

    // hitung panjang filename
    i = 0;
    fname_len = 0;
    while (path[i] != 0)
    {
        i++;
        fname_len++;
    }

    // jika fname_len == 0, berarti masukan string kosong
    if (fname_len == 0)
    {
        *result = -1;
        return;
    }

    // cari karakter / pertama
    i = 0;
    while ((path[i] != 0) && (path[i] != '/'))
    {
        nama[i] = path[i];
        i++;
    }
    nama[i] = 0;

    if (path[i] == '/')
    {
        // mencari directory, lalu cari anak rekursif
        // cari name di dalam dirsector
        findDirectory(nama, &res, parentIndex, &s);
        if (s == -1)
        {
            *result = -1;
            return;
        }

        deleteDirectory(path + i + 1, result, res);
    }
    else
    {
        // mencari file name
        // cek apakah tidak ada directory
        findFile(nama, &res, parentIndex, &s);
        if (s == -1)
        {
            *result = -1;
            return;
        }

        deleteDirectoryByIndex(res, result);
    }
}

void putArgs(char curdir, char argc, char **argv)
{
    char args[SECTOR_SIZE];
    int i, j, p;

    clear(args, SECTOR_SIZE);
    args[0] = curdir;
    args[1] = argc;
    i = 0;
    j = 0;
    for (p = 2; p < ARGS_SECTOR && i < argc; ++p)
    {
        args[p] = argv[i][j];
        if (argv[i][j] == 0)
        {
            ++i;
            j = 0;
        }
        else
        {
            ++j;
        }
    }

    writeSector(args, ARGS_SECTOR);
}

void getCurdir(char *curdir)
{
    char args[SECTOR_SIZE];
    readSector(args, ARGS_SECTOR);
    *curdir = args[0];
}

void getArgc(char *argc)
{
    char args[SECTOR_SIZE];
    readSector(args, ARGS_SECTOR);
    *argc = args[1];
}

void getArgv(char index, char *argv)
{
    char args[SECTOR_SIZE];
    int i, j, p;
    readSector(args, ARGS_SECTOR);

    i = 0;
    j = 0;
    for (p = 2; p < ARGS_SECTOR; ++p)
    {
        if (i == index)
        {
            argv[j] = args[p];
            ++j;
        }
        if (args[p] == 0)
        {
            if (i == index)
                break;
            else
                ++i;
        }
    }

    writeSector(args, ARGS_SECTOR);
}
/*===============================================*/

void printCharval(char c)
{
    char f;

    // first half
    f = c >> 4;
    if ((0 <= f) && (f < 0xA))
        printChar('0' + f);
    else
        printChar('A' + f - 0xA);

    f = c % (0x10);
    if ((0 <= f) && (f < 0xA))
        printChar('0' + f);
    else
        printChar('A' + f - 0xA);
}

void findDirectory(char *filename, char *result, char parentIndex, int *success)
{
    // digunakan untuk mencari index dari directory name, yang memiliki
    // parentIndex dimana index disiman di result dan success
    char dir[SECTOR_SIZE];
    char nama[20];
    int j, fname_len;
    int same = TRUE;
    int i = 0;
    int found = FALSE;

    readSector(dir, DIR_SECTOR);
    i = 0;
    fname_len = 0;
    while (filename[i] != 0)
    {
        i++;
        fname_len++;
    }
    // jika fname_len == 0, berarti masukan string kosong
    if (fname_len == 0)
    {
        *success = -1;
        return;
    }

    // cari karakter / pertama
    i = 0;
    while ((filename[i] != 0) && (filename[i] != '/'))
    {
        nama[i] = filename[i];
        i++;
    }
    nama[i] = 0;

    if (filename[i] == '/')
    {
        // find directory that under parentIndex named nama
        findDirectory(nama, result, parentIndex, success);
        if (success == NOT_FOUND)
        {
            return;
        }
        parentIndex = *result;
        findDirectory(filename + i + 1, result, parentIndex, success);
    }
    else
    {
        // untuk setiap entry di dir
        for (i = 0; i < MAX_FILES; i++)
        {
            // cek parentIndex
            if (dir[i * DIR_ENTRY_LENGTH] != parentIndex)
                continue;

            // nama dir ada di dir[i * DIR_ENTRY_LENGTH + 1]
            same = TRUE;
            for (j = 0; (j < MAX_FILENAME) && (same) && ((filename[j] != 0) || (dir[i * DIR_ENTRY_LENGTH + 1 + j])); j++)
                same = same && (filename[j] == dir[i * DIR_ENTRY_LENGTH + 1 + j]);

            // jika same TRUE, maka ditemukan file
            if (same == TRUE)
            {
                *result = i;
                *success = 0;
                return;
            }
            // jika same FALSE, lanjut ke entry selanjutnya
        }
        *success = NOT_FOUND;
    }
}

void findFile(char *name, char *result, char parentIndex, int *success)
{
    // digunakan untuk mencari index dari directory name, yang memiliki
    // parentIndex dimana index disiman di result dan success
    char files[SECTOR_SIZE];
    char nama[20];
    int i = 0;
    int j, fname_len;
    int same = TRUE;

    readSector(files, FILE_SECTOR);

    i = 0;
    fname_len = 0;
    while (name[i] != 0)
    {
        i++;
        fname_len++;
    }
    // jika fname_len == 0, berarti masukan string kosong
    if (fname_len == 0)
    {
        *success = -1;
        return;
    }

    // cari karakter / pertama
    i = 0;
    while ((name[i] != 0) && (name[i] != '/'))
    {
        nama[i] = name[i];
        i++;
    }
    nama[i] = 0;

    if (name[i] == '/')
    {
        // find directory that under parentIndex named nama
        findDirectory(nama, result, parentIndex, success);
        if (success == NOT_FOUND)
        {
            return;
        }
        parentIndex = *result;
        findFile(name + i + 1, result, parentIndex, success);
    }
    else
    {
        // untuk setiap entry di dir
        for (i = 0; i < MAX_FILES; i++)
        {
            if (files[i * DIR_ENTRY_LENGTH + 1] == 0)
                continue;
            // cek parentIndex
            if (files[i * DIR_ENTRY_LENGTH] != parentIndex)
                continue;

            // nama dir ada di dir[i * DIR_ENTRY_LENGTH + 1]
            same = TRUE;
            for (j = 0; (j < MAX_FILENAME) && (same) && ((name[j] != 0) || (files[i * DIR_ENTRY_LENGTH + 1 + j] != 0)); j++)
                same = same && (name[j] == files[i * DIR_ENTRY_LENGTH + 1 + j]);

            // jika same TRUE, maka ditemukan file
            if (same == TRUE)
            {
                *result = i;
                *success = 0;
                return;
            }
            // jika same FALSE, lanjut ke entry selanjutnya
        }
        *success = NOT_FOUND;
    }
}
int strlen(char *string)
{
    int i = 0;
    while (string[i] != 0)
        i++;
    return i;
}