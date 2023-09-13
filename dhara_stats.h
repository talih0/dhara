#ifndef DHARA_STATS_H_
#define DHARA_STATS_H_

#include <stdint.h>

struct dhara_stats {
	uint16_t is_bad;
	uint16_t mark_bad;
	uint16_t erase;
	uint16_t erase_fail;
	uint16_t is_erased;
	uint16_t prog;
	uint32_t prog_bytes;
	uint16_t prog_fail;
	uint16_t read;
	uint32_t read_bytes;
	uint16_t read_fail;
	uint16_t copy;
	uint16_t copy_fail;
};

#endif
