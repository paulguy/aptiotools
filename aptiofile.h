#include <stdint.h>

typedef struct {
    uint8_t magic[16];
    uint32_t headerSize;
    uint32_t flags;
    uint32_t capsuleSize;
    uint16_t volumeOffset;
    uint16_t tableOffset;
} AptioHeader;

typedef struct {
    uint32_t size;
    uint16_t unk0;
} AptioUnkRegion;

typedef struct {
    uint32_t offsetAndValid;
    uint32_t flags0;
    uint32_t offset;
    uint32_t size;
    uint32_t flags1;
    uint32_t flags2;
} AptioTableItem;

const uint8_t APTIO_SIGNED_MAGIC[16];
const uint8_t APTIO_UNSIGNED_MAGIC[16];
