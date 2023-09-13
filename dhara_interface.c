#include <stdbool.h>

#include <zephyr/kernel.h>
#include <errno.h>
#include <zephyr/init.h>

#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>

#include <dhara/error.h>
#include <dhara/nand.h>
#include <dhara/map.h>

#include <dhara_stats.h>

#define DHARA_BLOCK_MARK_GOOD 0
#define DHARA_BLOCK_MARK_BAD 1
#define DHARA_PAGE_NOT_ERASED 0
#define DHARA_PAGE_ERASED 1

extern const struct flash_area *dhara_fap;
extern const struct device *dhara_flash_dev;

struct dhara_stats dhara_stats;

int dhara_nand_is_bad(const struct dhara_nand *n, dhara_block_t b)
{
	int ret;
	bool is_bad_block;
	int block_size = BIT(n->log2_ppb) * BIT(n->log2_page_size);
	uint32_t offset = b * block_size ;

	/* because we are not using flash map interface for ex_op */
	offset += dhara_fap->fa_off;

	dhara_stats.is_bad++;

	ret = flash_ex_op(dhara_flash_dev, FLASH_EX_OP_IS_BAD_BLOCK, offset,
			  &is_bad_block);

	if (ret < 0 || is_bad_block) {
		return DHARA_BLOCK_MARK_BAD;
	}

	return DHARA_BLOCK_MARK_GOOD;
}

void dhara_nand_mark_bad(const struct dhara_nand *n, dhara_block_t b)
{
	int block_size = BIT(n->log2_ppb) * BIT(n->log2_page_size);
	uint32_t offset = b * block_size;

	offset += dhara_fap->fa_off;

	dhara_stats.mark_bad++;

	(void)flash_ex_op(dhara_flash_dev, FLASH_EX_OP_MARK_BAD_BLOCK, offset, NULL);
}

int dhara_nand_erase(const struct dhara_nand *n, dhara_block_t b,
		     dhara_error_t *err)
{
	int ret;
	int block_size = BIT(n->log2_ppb) * BIT(n->log2_page_size);
	uint32_t offset = b * block_size;

	dhara_stats.erase++;
	
	ret = flash_area_erase(dhara_fap, offset, block_size);
	if (ret < 0){
		*err = DHARA_E_BAD_BLOCK;	
		return -1;
	}

	return 0;
}

int dhara_nand_prog(const struct dhara_nand *n, dhara_page_t p,
		    const uint8_t *data,
		    dhara_error_t *err)
{
	int ret;
	uint32_t page_size = BIT(n->log2_page_size);
	uint32_t offset = page_size * p;

	dhara_stats.prog++;
	dhara_stats.prog_bytes += page_size;

	ret = flash_area_write(dhara_fap, offset, data, page_size);
	if (ret < 0) {
		dhara_stats.prog_fail++;
		*err = DHARA_E_BAD_BLOCK;
		return -1;
	}

	return 0;
}

int dhara_nand_is_free(const struct dhara_nand *n, dhara_page_t p)
{
	int ret;
	bool is_erased;
	uint32_t offset = p * BIT(n->log2_page_size);

	/* because we are not using flash map interface for ex_op */
	offset += dhara_fap->fa_off;

	dhara_stats.is_erased++;

	ret = flash_ex_op(dhara_flash_dev, FLASH_EX_OP_IS_PAGE_ERASED, offset,
			  &is_erased);

	if (ret < 0 || !is_erased) {
		return DHARA_PAGE_NOT_ERASED;
	}

	return DHARA_PAGE_ERASED;

}

int dhara_nand_read(const struct dhara_nand *n, dhara_page_t p,
		    size_t offset, size_t length,
		    uint8_t *data,
		    dhara_error_t *err)
{
	int ret;
	uint32_t page_size = BIT(n->log2_page_size);
	uint32_t offset_flash = page_size * p + offset;

	dhara_stats.read++;
	dhara_stats.read_bytes += length;

	ret = flash_area_read(dhara_fap, offset_flash, data, length);
	if (ret < 0) {
		*err = DHARA_E_ECC;
		dhara_stats.read_fail++;
		return -1;
	}

	return 0;
}

int dhara_nand_copy(const struct dhara_nand *n,
		    dhara_page_t src, dhara_page_t dst,
		    dhara_error_t *err)
{
	int ret;
	uint32_t offset_in = src * BIT(n->log2_page_size);
	uint32_t offset_out = dst * BIT(n->log2_page_size);

	/* because we are not using flash map interface for ex_op */
	offset_in += dhara_fap->fa_off;
	offset_out += dhara_fap->fa_off;

	dhara_stats.copy++;

	ret = flash_ex_op(dhara_flash_dev, FLASH_EX_OP_INTERNAL_MOVE_PARTIAL_PAGE,
			  offset_in, &offset_out);
	if (ret < 0) {
		*err = DHARA_E_ECC;
		dhara_stats.copy_fail++;
		return -1;
	}

	return 0;
}
