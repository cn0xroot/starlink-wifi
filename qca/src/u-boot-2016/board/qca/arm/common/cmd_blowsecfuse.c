/*
 * Copyright (c) 2015-2017 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <common.h>
#include <command.h>
#include <image.h>
#include <nand.h>
#include <errno.h>
#include <asm/arch-qca-common/smem.h>
#include <asm/arch-qca-common/scm.h>
#include <linux/mtd/ubi.h>
#include <part.h>
#include <asm/arch-qca-common/qca_common.h>

#define FUSEPROV_SUCCESS		0x0
#define FUSEPROV_INVALID_HASH		0x09
#define FUSEPROV_SECDAT_LOCK_BLOWN	0xB
#define TZ_BLOW_FUSE_SECDAT		0x20

int do_fuseipq(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret;
	/*
	Production sec.dat, SHA256 303a1a2934c193494c6f2a404523064ba398cba0278fc76f83b5a404a9d9912a
	*/
	char fusearr[] = {
		0xca, 0x51, 0x72, 0x3b, 0x29, 0x6f, 0x12, 0x2a, 0x01, 0x00, 0x00, 0x00,
		0x7c, 0x01, 0x00, 0x00, 0x64, 0x65, 0x66, 0x61, 0x75, 0x6c, 0x74, 0x20,
		0x64, 0x61, 0x74, 0x20, 0x66, 0x69, 0x6c, 0x65, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x10, 0x80, 0x05, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x22, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x08, 0x00, 0x00, 0x00, 0x20, 0x80, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
		0x28, 0x80, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x30, 0x80, 0x05, 0x00,
		0x00, 0x10, 0x00, 0x00, 0x00, 0x0c, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x03, 0x00, 0x00, 0x00, 0x38, 0x80, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
		0x78, 0x80, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x80, 0x80, 0x05, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x02, 0x00, 0x00, 0x00, 0x88, 0x80, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
		0x90, 0x80, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x98, 0x80, 0x05, 0x00,
		0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82, 0x00, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00, 0xa8, 0x80, 0x05, 0x00, 0x30, 0x3a, 0x1a, 0x29,
		0x34, 0xc1, 0x93, 0xf2, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
		0xb0, 0x80, 0x05, 0x00, 0x49, 0x4c, 0x6f, 0x2a, 0x40, 0x45, 0x23, 0x89,
		0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xb8, 0x80, 0x05, 0x00,
		0x06, 0x4b, 0xa3, 0x98, 0xcb, 0xa0, 0x27, 0xab, 0x00, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00, 0xc0, 0x80, 0x05, 0x00, 0x8f, 0xc7, 0x6f, 0x83,
		0xb5, 0xa4, 0x04, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
		0xc8, 0x80, 0x05, 0x00, 0xa9, 0xd9, 0x91, 0x2a, 0x00, 0x00, 0x00, 0xd2,
		0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0xf8, 0x81, 0x05, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x3d, 0xa8, 0x82, 0xda, 0x17, 0xfb, 0xea, 0xcb, 0xf6, 0x26, 0xbe, 0xc9,
		0x51, 0xf3, 0x46, 0xd0, 0x32, 0x26, 0x11, 0xc1, 0x4e, 0x7c, 0xa1, 0x63,
		0x10, 0x4f, 0xce, 0x0a, 0x00, 0xa3, 0x69, 0x81
	};

	uint32_t fuse_status = 0;
	struct fuse_blow {
		uint32_t address;
		uint32_t status;
	} fuseip;

	if (argc != 2) {
		printf("No Arguments provided\n");
		printf("Command format: fuseipq <address>\n");
		return 1;
	}

	fuseip.address = simple_strtoul(argv[1], NULL, 16);
	fuseip.status = (uint32_t)&fuse_status;

	memcpy((void *)fuseip.address, &fusearr, sizeof(fusearr)); // copy the data into memory instead load from file
	ret = qca_scm_fuseipq(SCM_SVC_FUSE, TZ_BLOW_FUSE_SECDAT,
			&fuseip, sizeof(fuseip));

	if (ret || fuse_status)
		printf("%s: Error in QFPROM write (%d, %d)\n",
			__func__, ret, fuse_status);

	if (fuse_status == FUSEPROV_SECDAT_LOCK_BLOWN)
		printf("Fuse already blown\n");
	else if (fuse_status == FUSEPROV_INVALID_HASH)
		printf("Invalid sec.dat\n");
	else if (fuse_status  != FUSEPROV_SUCCESS)
		printf("Failed to Blow fuses");
	else
		printf("Blow Success\n");

	return 0;
}


U_BOOT_CMD(fuseipq, 2, 0, do_fuseipq,
		"fuse QFPROM registers from memory\n",
		"fuseipq [address]  - Load fuse(s) and blows in the qfprom\n");