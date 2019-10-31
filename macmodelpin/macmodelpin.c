/* ========= MACMODELPIN: ========= */
/* gcc -o macmodelpin macmodelpin.c */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <linux/ioctl.h>
#include <linux/types.h>

#define SETUP_OFFSET 0x1FC00
#define   MAC_OFFSET   0x0 // = 0x1FC00 = 0x1FC00 +   0x0
#define MODEL_OFFSET 0x100 // = 0x1FD00 = 0x1FC00 + 0x100
#define   PIN_OFFSET 0x200 // = 0x1FE00 = 0x1FC00 + 0x200
#define FILE_MINSIZE 0x20000

#define   MAC_TYPE 1
#define MODEL_TYPE 2
#define   PIN_TYPE 3

#define IS_DEC 0
#define IS_HEX 1

#define   MAC_SIZE 0x6 // = 12:2
#define MODEL_SIZE 0x8 // = 16:2
#define   PIN_SIZE 0x8 // = 8

#define   MAC_CHAR_SIZE 0x0E // = 14 = 2 ('0x') + 12
#define MODEL_CHAR_SIZE 0x12 // = 18 = 2 ('0x') + 16
#define   PIN_CHAR_SIZE 0x08 // =  8

static unsigned long int str2uint(unsigned const char *str) {
    char *endptr = NULL;
    unsigned long int value = 0xdeadc0de;
    value = strtoul(str, &endptr, 0);
    return value;
}

static long int str2int(unsigned const char *str) {
    char *endptr = NULL;
    long int value = 0xdeadc0de;
    value = strtol(str, &endptr, 0);
    return value;
}

static unsigned long int checksize(FILE *pFile, unsigned const char *str) {
    fseek(pFile, 0L, SEEK_END);
    unsigned long int filesize = ftell(pFile);
    if (filesize < FILE_MINSIZE) {
        fprintf(stdout, "ERROR: %s size is 0x%x, should be >= 0x20000 = 128KB\n", str, filesize);
          fflush(stdout);
            sleep(1);
        return 1;
    }
    return 0;
}

static unsigned long int checklen(unsigned const char *str,
                                    unsigned long int size, unsigned long int type) {
    if (strlen(str) != size) {
        fprintf(stdout, "ERROR: length of a ");
            if (type == 0) fprintf(stdout, "MAC");
            if (type == 1) fprintf(stdout, "MODEL");
            if (type == 2) fprintf(stdout, "PIN");
        fprintf(stdout, " should be %d chars", size);
            if (type == 0) fprintf(stdout, ": 2 ('0x') + %d\n",   MAC_CHAR_SIZE - 2);
            if (type == 1) fprintf(stdout, ": 2 ('0x') + %d\n", MODEL_CHAR_SIZE - 2);
            if (type == 2) fprintf(stdout, "\n");
        fflush(stdout);
          sleep(1);
        return 1;
    }
    return 0;
}

static unsigned long int checkval(unsigned const char *str,
                                    unsigned long int size, unsigned long int ishex) {
    unsigned long int i;
    if (ishex && ((str[0] != '0')||(str[1] != 'x'))) {
        fprintf(stdout, "ERROR: please put '0x' before a hex value at %s\n", str);
          fflush(stdout);
            sleep(1);
        return 1;
    }
    for (i = 2; i < size; i++) {
        if ((str[i] >= '0') && (str[i] <= '9'))
            continue;
        if (ishex && (((str[i] >= 'a') && (str[i] <= 'f')) ||
                      ((str[i] >= 'A') && (str[i] <= 'F'))))
            continue;
        fprintf(stdout, "ERROR: invalid char '%c' found at %d pos of %s\n", str[i], i, str);
          fflush(stdout);
            sleep(1);
        return 1;
    }
    return 0;
}

static unsigned long int rotate(unsigned long int value, unsigned long int size) {
    unsigned long int i, j;
    unsigned char *c_in, *c_out;
    unsigned long int retval = 0x0;

    c_in = (unsigned char *)&value;
    c_out = (unsigned char *)&retval;

    for (i = 0, j = size - 1; i < size; i++, j--)
        c_out[i] = c_in[j];

    return retval;
}

static void print_ascii(unsigned long int value, unsigned long int size) {
    unsigned long int i;
    unsigned char *c;
    c = (unsigned char *)&value;
    for (i = 0; i < size; i++)
        fprintf(stdout, "%c", c[i]);
}

static void read_info(FILE *pFile, unsigned long int* mac,
                                    unsigned long int* model, unsigned long int* pin) {
    fseek(pFile, SETUP_OFFSET + MAC_OFFSET, SEEK_SET);
    fread(mac, MAC_SIZE, 1, pFile);
    *mac = rotate(*mac, MAC_SIZE);
    fprintf(stdout, "  MAC = 0x     %012llX     \n", *mac);

    fseek(pFile, SETUP_OFFSET + MODEL_OFFSET, SEEK_SET);
    fread(model, MODEL_SIZE, 1, pFile);
    *model = rotate(*model, MODEL_SIZE);
    fprintf(stdout, "MODEL = 0x   %016llx   \n", *model);

    fseek(pFile, SETUP_OFFSET + PIN_OFFSET, SEEK_SET);
    fread(pin, PIN_SIZE, 1, pFile);
    fprintf(stdout, "  PIN = 0d       ");
    print_ascii(*pin, PIN_SIZE);
    fprintf(stdout, "       \n");
}

static void write_info(FILE *pFile, unsigned long int* mac,
                                    unsigned long int* model, unsigned char* pin) {
        fseek(pFile, SETUP_OFFSET + MAC_OFFSET, SEEK_SET);
        *mac = rotate(*mac, MAC_SIZE);
        fwrite(mac, MAC_SIZE, 1, pFile);

        fseek(pFile, SETUP_OFFSET + MODEL_OFFSET, SEEK_SET);
        *model = rotate(*model, MODEL_SIZE);
        fwrite(model, MODEL_SIZE, 1, pFile);

        fseek(pFile, SETUP_OFFSET + PIN_OFFSET, SEEK_SET);
        fwrite(pin, PIN_SIZE, 1, pFile);
}

static void disable_buffering(FILE *pFile) {
    setvbuf(pFile, NULL, _IONBF, 0);
}

static void usage(unsigned const char *str) {
    fprintf(stdout, "=== USAGE ===\n");
    fprintf(stdout, "    to print:\n");
    fprintf(stdout, "        %s dump.bin\n", str);
    fprintf(stdout, "    to set:\n");
    fprintf(stdout, "        %s dump.bin 0xMAC 0xMODEL PIN\n", str);
    fprintf(stdout, "        %s dump.bin 0x16A2594B37DF 0x0841000900000001 12345678\n", str);
      fflush(stdout);
        sleep(1);
}

int main(int argc, unsigned char *argv[]) {
    FILE * pFile_in, * pFile_out;
    unsigned long int mac_rd, mac_wr, model_rd, model_wr, pin_rd, write;
    unsigned char pin_wr[9];

    if ((argc != 2) && (argc != 5)) {
        usage(argv[0]);
        return 1;
    }
    else {
        if (argc == 2)
            write = 0;
        else
            write = 1;

          mac_rd = 0x0;
        model_rd = 0x0;
          pin_rd = 0x0;

        if (write) {
            if (checklen(argv[2],   MAC_CHAR_SIZE,   MAC_TYPE) ||
                checklen(argv[3], MODEL_CHAR_SIZE, MODEL_TYPE) ||
                checklen(argv[4],   PIN_CHAR_SIZE,   PIN_TYPE))
                return 1;
            if (checkval(argv[2],   MAC_CHAR_SIZE, IS_HEX) ||
                checkval(argv[3], MODEL_CHAR_SIZE, IS_HEX) ||
                checkval(argv[4],   PIN_CHAR_SIZE, IS_DEC))
                return 1;

              mac_wr = str2uint(argv[2]);
            model_wr = str2uint(argv[3]);
              strcpy(pin_wr, argv[4]);
        }

        pFile_in = fopen(argv[1], "r");
        if (pFile_in == NULL) {
            fprintf(stdout, "ERROR: cannot open file %s for reading\n", argv[1]);
              fflush(stdout);
                sleep(1);
            return 1;
        }
        if (checksize(pFile_in, argv[1])) {
            fclose(pFile_in);
            return 1;
        }

        if (write) {
            pFile_out = fopen(argv[1], "r+");
            if (pFile_out == NULL) {
                fprintf(stdout, "ERROR: cannot open file %s for writing\n", argv[1]);
                  fflush(stdout);
                    sleep(1);
                fclose(pFile_in);
                return 1;
            }
        }

        disable_buffering(pFile_in);
        if (write)
            disable_buffering(pFile_out);

        if (write) {
            fprintf(stdout, "\n=== %s - Before:\n\n", argv[1]);
            read_info(pFile_in, &mac_rd, &model_rd, &pin_rd);
            write_info(pFile_out, &mac_wr, &model_wr, (unsigned char *)&pin_wr);
            fprintf(stdout, "\n=== %s - After: \n\n", argv[1]);
            read_info(pFile_in, &mac_rd, &model_rd, &pin_rd);
        }
        else {
            fprintf(stdout, "\n=== %s\n\n", argv[1]);
            read_info(pFile_in, &mac_rd, &model_rd, &pin_rd);
        }
        fprintf(stdout, "\n");
          fflush(stdout);

        fclose(pFile_in);
        if (write)
            fclose(pFile_out);
    }

    sleep(1);
    return 0;
}
