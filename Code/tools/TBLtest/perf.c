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

/* This file defines functions for reading the ES performance log data
 * dump file and outputting useful statistics to the console.
 */
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "cfe.h"
#include "cfe_fs_extern_typedefs.h"    /* for CFE_FS_Header_t */
#include "cfe_es_perfdata_typedef.h"   /* for ES perf struct types */
#include "cfe_platform_cfg.h"          /* for perf buffer size */
#include "cfe_perfids.h"               /* for CFE_MISSION_ES_PERF_EXIT_BIT */

#include "vs_ground.h"                 /* for perf IDs */

#include "common_constants.h"
#include "perf.h"


/* Full relative path to where we expect the ES perf dump file to be. */
#define FULL_PERF_FILENAME "../cpu1" PERF_FILENAME

/* Time to sleep waiting for ES to finish perf log dumps in seconds */
#define PERF_FILE_PAUSE 5  /* seconds */

/* Number of times we'll poll the ES perf log dump file before giving up */
#define MAX_WAITS 8

/* We'll read the performance data entries into this buffer. */
static CFE_ES_PerfDataEntry_t entries[CFE_PLATFORM_ES_PERF_DATA_BUFFER_SIZE];

/* ES ORs the following bits with the Perf ID its stores in the .Data
 * field of each perf log entry to distinguish log entries describing
 * CFE_ES_PerfLogEntry() calls vs. CFE_ES_PerfLogExit() calls.
 */
#define ENTRY_MASK 0x00
#define EXIT_MASK (0x01 << CFE_MISSION_ES_PERF_EXIT_BIT)


/* --------------------- module local functions ---------------------- */

/* perf_read_data()
 *
 * in:     nothing
 * out:    entries - buffer zeroed/filled
 * return: nothing
 *
 * Reads the ES perf log data entries from the ES perf log file.
 *
 * The perf data file format is as follows:
 * First, a CFE file header structure of type CFE_FS_Header_t.
 * Second, an ES perf metatdata structure of type CFE_ES_PerfMetatData_t.
 * Then, CFE_PLATFORM_ES_PERF_DATA_BUFFER_SIZE CFE_ES_PerfDataEntry_t structs.
 */

static void
perf_read_data(void) {

	int fd;   /* file descriptor for ES perf data dump file */

	/* Its not clear how many entries we'll find in the log file,
	 * so clear the entire buffer to zeroes so that unused entries
	 * are easy to distinguish in our buffer.
	 */
	memset(entries, 0x00, sizeof(entries));

	/* To avoid overloading the spacecraft's CPU, ES will use a
	 * background task to dribble its performance log entry data
	 * out to a file in chunks over time.  Our test suite needs to
	 * wait for this background task to create the performance
	 * data dump file and write all of ES's data to it before
	 * attempting to read data from it.
	 *
	 * HACK: this sleep is simple but wasteful and is begging for
	 * a race condition.
	 */
	sleep(PERF_FILE_PAUSE);
	
	/* Using do/while/break as poor man's try/catch */
	do {
	
		if (-1 == (fd = open(FULL_PERF_FILENAME, O_RDONLY)))
			break;

		/* Skip the headers; we just want the log entries. */
		if (-1 == lseek(fd, (sizeof(CFE_FS_Header_t) +
			sizeof(CFE_ES_PerfMetaData_t)), SEEK_SET))
			break;
	
		/* Read all the log entries. */
		if (-1 == read(fd, entries, sizeof(entries)))
			break;

		if (-1 == close(fd))
			break;

		/* If we reach here, we've successfully read the log
		 * entries.
		 */
		return;

	} while (0);

	/* If we reach here, something went wrong. */

	perror("Failed to read ES performance log file.");
	exit(-1);
	
} /* perf_read_data() */


/* perf_dump_data()
 *
 * in:     app_perfid - perf ID whose entries we want to dump
 *         entries    - perf data read from this buffer
 * out:    nothing
 * return: nothing
 *
 * Scans through the entries array of perf log entries looking for a
 * start entry for app_perfid.  If found, it scans until it finds the
 * corresponding stop entry, computes the duration between start and
 * stop, and outputs it to the console.  The duration, like the start
 * and stop times, is in simulated spacecraft clock ticks.
 */

static void
perf_dump_data(uint32 app_perfid) {

	uint64 time_start; /* timestamp of CFE_ES_PerfLogEntry() log entry */
	uint64 time_stop;  /* timestamp of CFE_ES_PerfLogExit() log entry */
	int i;             /* index into entries array */

	/* Process all the perf log entries in the buffer. */
	for (i = 0; i < CFE_PLATFORM_ES_PERF_DATA_BUFFER_SIZE; i++) {

		/* Advance until we find a CFE_ES_PerfLogEntry() entry
		 * for our desired perf ID.  If we reach the end of
		 * the buffer without finding what we want, we're
		 * done.
		 */
		for(; entries[ i ].Data != (app_perfid | ENTRY_MASK);
			i++) {
			if (i >= CFE_PLATFORM_ES_PERF_DATA_BUFFER_SIZE) return;
		}
		
		/* We've found a CFE_ES_PerfLogEntry() entry.
		 * Remember its timestamp.
		 */
		time_start = entries[ i ].TimerUpper32 * UINT32_MAX +
			entries[ i ].TimerLower32;

		/* Advance until we find a CFE_ES_PerfLogExit() entry
		 * for our desired perf ID.  If we reach the end of
		 * the buffer without finding what we want, we're
		 * done.
		 */
		for(; entries[ i ].Data != (app_perfid | EXIT_MASK); i++) {
			if (i >= CFE_PLATFORM_ES_PERF_DATA_BUFFER_SIZE) return;
		}

		/* We've found a CFE_ES_PerfLogExit() entry. Compute
		 * its timestamp and print it to the console.
		 */
		time_stop = entries[ i ].TimerUpper32 * UINT32_MAX +
			entries[ i ].TimerLower32;

		printf("PERF: Verification function execution duration "
		       "in ticks: %lu\n", time_stop - time_start);

	} /* for all entries */
	
} /* perf_dump_data() */


/* ------------------- module exported functions -------------------- */

/* perf_print()
 *
 * in:     app_perfid - PERF ID whose data we want to print
 * out:    nothing
 * return: nothing
 *
 * Tells the simulated spacecraft to dump all of the performance start
 * and stop entries it has collected.  Picks out the start and stop
 * entries corresponding to the PERF ID app_perfid.  Prints the
 * difference between those starts and stops to the console in terms
 * of spacecraft clock ticks.
 *
 */

void
perf_print(uint32 app_perfid) {

	perf_read_data();
	perf_dump_data(app_perfid);

} /* perf_print() */
