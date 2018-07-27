
typedef uint32_t block_age_t;
typedef uint32_t block_index_t;
typedef uint32_t timestamp_t;
typedef uint16_t sector_index_t;

const int32_t SectorSize = 512;

const sector_index_t SECTOR_HEAD = 1;
const timestamp_t TIMESTAMP_INVALID = ((timestamp_t)-1);
const block_index_t BLOCK_INDEX_INVALID = ((block_index_t)-1);
const block_age_t BLOCK_AGE_INVALID = ((block_age_t)-1);

static const char MagicKey[] = "phylum00";

typedef struct {
    char key[8];
} BlockMagic;

typedef struct {
    BlockMagic magic;
    uint8_t type;
    block_age_t age;
    timestamp_t timestamp;
    block_index_t linked_block;
} BlockHead;

typedef struct {
    BlockHead header;
    sector_index_t sector;
    block_index_t chained_block;
} SuperBlockLink;

typedef struct {
    SuperBlockLink link;
} SuperBlock;
