/* Copyright (c) 2024 Timothy Jon Fraser Consulting LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.  See the License for the specific language governing
 * permissions and limitations under the License.
 */

/* This file defines functions for creating table image files in the
 * simulated spacecraft's filesystem that the ground station command
 * the cFE Table Service (TBL) to load for later validation and
 * activation.  Test programs can change the values of in-memory table
 * fields by creating table files and commanding TBL to load them.
 */

#include <arpa/inet.h>    /* for htonl() */
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include "cfe.h"
#include "cfe_fs_extern_typedefs.h"   /* for CFE_FS_Header_t type */
#include "cfe_tbl_extern_typedefs.h"  /* for CFE_TBL_File_hdr_t type */
#include "vs_tablestruct.h"           /* for vs_table_t and constants */

#include "file.h"

/* ----------------- module private functions and state ------------- */

/* A VSA Parm table file consists of three parts in series: an FS file
 * header, a TBL table header, and then the parm table data.  We'll
 * use the following three variables to collect what we want to put
 * into each part of the file.
 */
static CFE_FS_Header_t    file_header;
static CFE_TBL_File_Hdr_t table_header;
static vs_table_t         table_data;

/* parm_id_to_string()
 *
 * in:     id - table entry numeric parm ID to translate
 * out:    nothing
 * return: a string version of id if id is valid, otherwise "Invalid".
 *
 * Converts a numeric parm ID to a string, useful for making
 * human-readable output.
 *
 */

static const char *
parm_id_to_string(uint8 id) {

	switch (id) {
	case VS_PARM_UNUSED:  return "Unused";
	case VS_PARM_APE:     return "Ape";
	case VS_PARM_BAT:     return "Bat";
	case VS_PARM_CAT:     return "Cat";
	case VS_PARM_DOG:     return "Dog";
	case VS_PARM_NORTH:   return "North";
	case VS_PARM_SOUTH:   return "South";
	case VS_PARM_EAST:    return "East";
	case VS_PARM_WEST:    return "West";
	default:               return "Invalid";
	}

} /* parm_id_to_string() */


/* print_entry()
 *
 * in:     p_entry - pointer of table entry to print
 * out:    nothing
 * return nothing
 *
 * Prints a table entry to the console.
 *
 */

static void
print_entry(const vs_entry_t *p_entry) {

	printf("      Parm: %7s Pad: 0x%02x%02x%02x "
		"Low: 0x%08X  High: 0x%08X\n",
		parm_id_to_string(p_entry->parm_id),
		p_entry->pad[0], p_entry->pad[1], p_entry->pad[2],
		p_entry->bound_low, p_entry->bound_high);
	
} /* print_entry() */


/* --------------------- module exported functions ---------------- */


/* file_init()
 *
 * in:     tablename_s   - the full table name, for example "VSA_APP.Prm"
 *         description_s - a description of what the image is; a comment
 * out:    file_header, table_header, table_data - see below
 * return: nothing
 *
 * Initializes file_header, table_header, and table_data to describe
 * table image with the following properties:
 *
 *   - the table name and description as specified by the input parms,
 *   - all entries unused, and
 *   - the table header offset and numbytes fields set to make the CFE
 *     Table Service (TBL) load the entire image.
 *
 */

void
file_init(const char *tablename_s, const char *description_s) {

	assert(tablename_s);
	assert(description_s);
	
	/* Clear everything to zeros. For VSA, all-zero table data is
	 * a valid empty table.
	 */
	memset(&file_header,  '\0', sizeof(CFE_FS_Header_t));
	memset(&table_header, '\0', sizeof(CFE_TBL_File_Hdr_t));
	memset(&table_data,   '\0', sizeof(vs_table_t));

	/* Set the numeric header values in network (that is,
	 * big-endian) byte order.  Leave the file header spacecraft
	 * ID, processor ID, and timestamps cleared to zero.  Set the
	 * table header offset and byte count to cause TBL to load the
	 * entire file.
	 */
	file_header.ContentType = htonl(CFE_FS_FILE_CONTENT_ID);
	file_header.SubType     = htonl(CFE_FS_SubType_TBL_IMG);
	file_header.Length      = htonl(sizeof(CFE_FS_Header_t));
	table_header.Offset     = htonl(0);
	table_header.NumBytes   = htonl(sizeof(vs_table_t));
	
	/* Copy in the strings taking care to leave the last byte in
	 * the destination fields as the above zeroization left them
	 * to ensure proper NUL string termination.
	 */
	strncpy(table_header.TableName, tablename_s,
		(CFE_MISSION_TBL_MAX_FULL_NAME_LEN - 1));
	strncpy(file_header.Description, description_s,
		(CFE_FS_HDR_DESC_MAX_LEN - 1));
	
} /* file_init() */


/* file_set_entry()
 *
 * in:     entry - index number of entry to set
 *         parm_id, bound_low, bound_high - values to set in entry's fields
 *         pad   - byte value to put in all of the entry's padding fields,
 *                 useful for testing the "pad is zero" checks of table
 *                 validation functions
 * out:    table_data - entry'th entry fields set
 * return: nothing
 *
 * Use this function to set the values of all fields in a given entry.
 *
 */

void
file_set_entry(unsigned int entry, uint8 parm_id, uint8 pad, uint32 bound_low,
	uint32 bound_high) {

	/* Attempting to set an entry outside of the range of the
	 * table is a bug in our test program.
	 */
	assert((0 <= entry) && (entry < VS_TABLE_NUM_ENTRIES));

	table_data.entries[entry].parm_id    = parm_id;
	table_data.entries[entry].pad[0]     = pad;
	table_data.entries[entry].pad[1]     = pad;
	table_data.entries[entry].pad[2]     = pad;
	table_data.entries[entry].bound_low  = bound_low; /* host byte order */
	table_data.entries[entry].bound_high = bound_high;/* host byte order */
	
} /* file_set_entry() */


/* file_output()
 *
 * in:     filename_s - name of file to save table image in, see below
 *         file_header, table_header, table_data - table image to output
 * out:    nothing
 *
 * Creates a .tbl CFE table image file based on the contents of
 * file_header, table_header, and table data.  Saves in the file
 * indicated by filename_s.
 *
 * An instance of the CFE Table Service (TBL) running on the simulated
 * spacecraft's cpu1 will expect to find table image files in its
 * simulated on-board filesystem.  The simulation represents this
 * on-board filesystem with the directory "build/exe/cpu1/cf".
 *
 * Choose filename_s to place the output file in this directory.  If
 * you expect to run this program from the "build/exe/host" directory,
 * then a filename like "../cpu1/cf/VS_Prm_test.tbl" would work.
 *
 */

void
file_output(const char *filename_s) {

	int fd;  /* file descriptor to output file */
	
	assert(filename_s);

	/* Using do/while/break as a poor man's try/catch */
	do {
	
		if (-1 == (fd = open(filename_s, (O_CREAT|O_WRONLY),
			(S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH))))
			break;

		if (sizeof(CFE_FS_Header_t) != write(fd, &file_header,
			sizeof(CFE_FS_Header_t)))
			break;
		
		if (sizeof(CFE_TBL_File_Hdr_t) != write(fd, &table_header,
			sizeof(CFE_TBL_File_Hdr_t)))
			break;
			
		if (sizeof(vs_table_t) != write(fd, &table_data,
			sizeof(vs_table_t)))
			break;
		
		close(fd);
		return;    /* file written OK! */

	} while (0);

	perror("Failed to output table file.");
	exit(-1);
		
} /* file_output() */


/* file_print()
 *
 * in:     table_data - table image data
 * out:    nothing
 * return: nothing
 *
 * Prints the current table image data to the console.
 *
 */

void
file_print(void) {

	unsigned int i;  /* indexes table entries */

	for (i = 0; i < VS_TABLE_NUM_ENTRIES; i++ ) {
		print_entry(&(table_data.entries[i]));
	}
	
} /* file_print() */
