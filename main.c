#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include "aptiofile.h"

#define BUFFERSIZE (1024*1024)
#define MAXFILES (256)

#define READ_OR_FAIL(OBJ) \
    if(fread(&(OBJ), 1, sizeof(OBJ), in) < sizeof(OBJ)) { \
        fprintf(stderr, "Failed to read header.\n"); \
        fclose(in); \
        if(out != NULL) \
            fclose(out); \
        exit(EXIT_FAILURE); \
    }

#define SEEK_OR_FAIL(OFFSET, WHENCE) \
    if(fseeko(in, (OFFSET), (WHENCE)) < 0) { \
        fprintf(stderr, "Failed to seek: %s\n", strerror(errno)); \
        fclose(in); \
        if(out != NULL) \
            fclose(out); \
        exit(EXIT_FAILURE); \
    }

typedef struct {
    uint32_t offset;
    uint32_t size;
} FilePos;

int main(int argc, char **argv) {
    FILE *in;
    FILE *out = NULL;
    AptioHeader hdr;
    AptioUnkRegion unk;
    AptioTableItem item;
    unsigned int i;
    FilePos files[MAXFILES];
    char buffer[BUFFERSIZE];
    uint32_t needRead, haveRead;

    if(argc < 2) {
        fprintf(stderr, "need more arguments\n");
        exit(EXIT_FAILURE);
    }

    in = fopen(argv[1], "rb");
    if(in == NULL) {
        fprintf(stderr, "Failed to open %s for reading.\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    READ_OR_FAIL(hdr)

    fprintf(stderr, "Main Header\n"
                    " Header Size: %u\n"
                    " Flags: %08X\n"
                    " Capsule Size: %u\n"
                    " Volume Offset: %hu\n"
                    " Table Offset: %hu\n"
                    "\n",
                    hdr.headerSize,
                    hdr.flags,
                    hdr.capsuleSize,
                    hdr.volumeOffset,
                    hdr.tableOffset);

    READ_OR_FAIL(unk)

    fprintf(stderr, "Unknown Region\n"
                    " Region Size: %u\n"
                    " Unknown maybe meaningful value: %hu\n"
                    "\n",
                    unk.size,
                    unk.unk0);

    SEEK_OR_FAIL(hdr.tableOffset, SEEK_SET)
    for(i = 0;; i++) {
        READ_OR_FAIL(item)
        if((item.offsetAndValid & 0xFF000000) != 0xFF000000) {
            files[i].size = 0;
            break;
        }

        fprintf(stderr, "%3u: %10u %10u %08X %08X %08X\n",
                i, item.offset, item.size, item.flags0, item.flags1, item.flags2);

        files[i].offset = item.offset + hdr.volumeOffset;
        files[i].size = item.size;
    }

    for(i = 0; files[i].size != 0; i++) {
        SEEK_OR_FAIL(files[i].offset, SEEK_SET)

        sprintf(buffer, "item_%03u.bin", i);
        out = fopen(buffer, "wb");
        if(out == NULL) {
            fprintf(stderr, "Failed to open %s for writing.\n", buffer);
            fclose(in);
            exit(EXIT_FAILURE);
        }

        fprintf(stderr, "Writing %s...\n", buffer);
        for(needRead = files[i].size; needRead > 0; needRead -= haveRead) {
            haveRead = fread(buffer, 1, needRead > BUFFERSIZE ? BUFFERSIZE : needRead, in);
/*            fprintf(stderr, "%u %u\n", needRead, haveRead);*/
            if(haveRead < (needRead > BUFFERSIZE ? BUFFERSIZE : needRead)) {
                fprintf(stderr, "Failed to read.\n");
                fclose(out);
                fclose(in);
                exit(EXIT_FAILURE);
            }
            if(fwrite(buffer, 1, haveRead, out) < haveRead) {
                fprintf(stderr, "Failed to write.\n");
                fclose(out);
                fclose(in);
                exit(EXIT_FAILURE);
            }
        }
        fclose(out);
    }

    fclose(in);
    exit(EXIT_SUCCESS);
}
