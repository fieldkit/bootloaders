#ifndef _PHYLUM_INTERNALS_H_
#define _PHYLUM_INTERNALS_H_

const int32_t SectorSize = 512;

const sector_index_t SECTOR_HEAD = 1;
const timestamp_t TIMESTAMP_INVALID = ((timestamp_t)-1);
const block_index_t BLOCK_INDEX_INVALID = ((block_index_t)-1);
const block_age_t BLOCK_AGE_INVALID = ((block_age_t)-1);

static const char MagicKey[] = "phylum00";

#endif
