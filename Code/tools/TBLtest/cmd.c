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

/* This file defines functions for sending ground commands to the
 * simulated spacecraft, focusing on the specific commands we need to
 * test cFS app table validation functions.
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "cfe.h"
#include "cfe_msgids.h"                /* for TBL and ES CMD_MIDs */
#include "cfe_tbl_msg.h"               /* for TBL command message structs */
#include "to_lab_msgids.h"             /* for TO_LAB MID */
#include "to_lab_msg.h"                /* for TO_LAB enable TLM msg struct */
#include "cfe_es_msg.h"                /* for ES command message structs */
#include "cfe_es_perf.h"               /* for ES CFE_ES_PERF_TRIGGER_START */

#include "common_constants.h"
#include "cmd.h"

#define CMD_ADDR "127.0.0.1"           /* Send commands here */
#define CMD_PORT 1234                  /* Send commands here */
#define CMD_TO_TLM_ADDR "127.0.0.1"    /* Tell TO/TO_LAB to send TLM here */

/* This 16-bit value in the Sequence field of the primary CCSDS header
 * indicates a message that is not fragmented and has sequence number
 * 0.  Nobody seems to check sequence numbers.  Be sure to convert
 * this value to network byte order (big-endian) before you put it in
 * a header.
 */
#define CCSDS_MSG_FRAG_SEQ     0xC000

/* Pause for at least this many usecs before sending a command to
 * avoid overflowing the cFE command pipe.
 */
#define PAUSE (1000 * 250)  /* 250 ms */


/*
 * -------- Module local state and functions --------
 */

static struct sockaddr_in cmd_addr;         /* send commands to here */
static int cmdfd;                     /* socket for sending commands */

/* We construct commands in this buffer; a union with a type for every
 * kind of command that we plan on sending plus a
 * CFE_MSG_CommandHeader_t type for manipulating the common primary
 * and secondary headers that begin command messages of all kinds.
 */
static union {
	CFE_MSG_CommandHeader_t        header;
	CFE_TBL_LoadCmd_t              tbl_load;
	CFE_TBL_ValidateCmd_t          tbl_validate;
	CFE_TBL_ActivateCmd_t          tbl_activate;
	TO_LAB_EnableOutputCmd_t       to_tlmon;
	CFE_ES_SetPerfFilterMaskCmd_t  es_filter;
	CFE_ES_SetPerfTriggerMaskCmd_t es_trigger;
	CFE_ES_StartPerfDataCmd_t      es_start;
	CFE_ES_StopPerfDataCmd_t       es_stop;
} cmd_msg;


/* cmd_set_header()
 *
 * in:     mid_hbo - 16-bit message ID in host byte order
 *         len_hbo - 16-bit true message length in host byte order.
 *         command - 8-bit command code
 * out:    cmd_msg - header fields set
 * return: nothing
 *
 * Utility function for setting CCSDS and command header fields.  Note
 * that len_hbo is the true size of the message in bytes without the
 * traditional CCSDS length adjustment.
 *
 * This function takes header field numbers in host byte order and
 * coverts them to network byte order (that is, big-endian) before
 * sending.
 */

static void
cmd_set_header(unsigned short mid_hbo, unsigned short len_hbo,
	unsigned char command) {

	*((unsigned short *)cmd_msg.header.Msg.CCSDS.Pri.StreamId) =
		htons(mid_hbo);
	*((unsigned short *)cmd_msg.header.Msg.CCSDS.Pri.Sequence) =
		htons(CCSDS_MSG_FRAG_SEQ);
	*((unsigned short *)cmd_msg.header.Msg.CCSDS.Pri.Length) =
		htons(len_hbo - CCSDS_MSG_LENGTH_DELTA);

	cmd_msg.header.Sec.FunctionCode = command;
	cmd_msg.header.Sec.Checksum     = 0x00;   /* nobody checks this */

} /* cmd_set_header() */


/* cmd_send()
 *
 * in:      cmdfd - open socket for sending command messages
 *          msg   - command message to send
 *          len   - length of command message
 * out:     cmdfd - command message written to socket
 * return:  nothing
 *
 * Sends command messages to the simulated spacecraft.
 *
 */
 
static void
cmd_send(const unsigned char *msg, size_t len) {

	/* By default, cFE will start emitting Message Limit Errors if
	 * the number of commands that have been sent by the ground
	 * station but not yet processed by their recipient exceeds
	 * CFE_PLATFORM_SB_DEFAULT_MSG_LIMIT.  This constant is set to
	 * only 4 in the default configuration of my version of cFE -
	 * a limit that is easily exceeded by this test suite.  Rather
	 * than mess with the cFE configuration, I've decided to use
	 * the simple but wasteful throttle on the rate at wich we
	 * send commands.
	 */
	if (-1 == usleep(PAUSE)) {
		perror("Failed to usleep");
		exit(-1);
	}
	
	if (-1 == send(cmdfd, msg, len,	0x00)) {
		perror("Failed to send command");
		exit(-1);
	}
	
} /* cmd_send() */


/*
 * --------- Functions exported by this module --------
 */


/* cmd_init()
 *
 * in:     nothing
 * out:    cmd_addr - set to address where we want to send commands
 *         cmd_fd   - set to file descriptor of open socket for sending
 * return: nothing
 *
 * Sets up socket for sending commands to the simulated spacecraft.
 * Call this before sending.  Note that, for commands, the simulated
 * spacecraft acts as the "server" and we act as the "client".
 *
 * Call this function before calling any of this module's other
 * functions.
 */

void
cmd_init(void) {
	
	/* Create IP/UDP socket for sending commands. For commands,
	 * the spacecraft is the server and I am the client.
	 */
	cmd_addr.sin_family      = AF_INET;
	cmd_addr.sin_port        = htons(CMD_PORT);
	cmd_addr.sin_addr.s_addr = inet_addr(CMD_ADDR);

	if (-1 == (cmdfd = socket(AF_INET, SOCK_DGRAM, 0))) {
		perror("Failed to create command socket");
		exit(-1);
	}
	if (-1 == connect(cmdfd, (struct sockaddr *)&cmd_addr,
		sizeof(struct sockaddr_in))) {
		perror("Failed to connect to command port");
		exit(-1);
	}

} /* cmd_init() */


/* cmd_to_tlmon()
 *
 * in:     nothing
 * out:    cmd_msg - set to command message
 * return: nothing
 *
 * Send command to TO/TO_LAB asking it to turn telemetry output on.
 *
 */
 
void
cmd_to_tlmon(void) {

	cmd_set_header(TO_LAB_CMD_MID, sizeof(TO_LAB_EnableOutputCmd_t),
		TO_LAB_OUTPUT_ENABLE_CC);
	memset(cmd_msg.to_tlmon.Payload.dest_IP, 0x00,
		sizeof(TO_LAB_EnableOutput_Payload_t));
	strncpy(cmd_msg.to_tlmon.Payload.dest_IP, CMD_TO_TLM_ADDR,
		(sizeof(TO_LAB_EnableOutput_Payload_t) - 1));
	
	cmd_send((const unsigned char *)&cmd_msg.to_tlmon,
		sizeof(TO_LAB_EnableOutputCmd_t));

} /* cmd_to_tlmon() */


/* cmd_tbl_load()
 *
 * in:     filename - table file filename
 * out:    cmd_msg  - set to command message
 * return: nothing
 *
 * Send command to TBL asking it to load the specified table file.
 * filename must be an absolute path starting with the root of the
 * simulated spacecraft's filesystem "/cf".
 *
 */

void
cmd_tbl_load(const char *filename) {

	/* TBL's CFE_TBL_LoadCmd_Payload_t structure allocates
	 * CFE_MISSION_MAX_PATH_LEN bytes to store the name of the
	 * table file we want to load and its terminating NUL. If we
	 * provide a string that is this long or longer, we have a
	 * bug.  Using C99 memchr() because POSIX strnlen() isn't
	 * available.
	 */
	assert(filename);
	assert(memchr(filename, '\0', CFE_MISSION_MAX_PATH_LEN));

	cmd_set_header(CFE_TBL_CMD_MID, sizeof(CFE_TBL_LoadCmd_t),
		CFE_TBL_LOAD_CC);

	memset(cmd_msg.tbl_load.Payload.LoadFilename, 0x00,
		CFE_MISSION_MAX_PATH_LEN);
	strncpy(cmd_msg.tbl_load.Payload.LoadFilename, filename,
		(CFE_MISSION_MAX_PATH_LEN-1));
	
	cmd_send((const unsigned char *)&cmd_msg.tbl_load,
		sizeof(CFE_TBL_LoadCmd_t));

} /* cmd_tbl_load() */


/* cmd_tbl_validate()
 *
 * in:     tablename - name of table to validate
 *         atflag    - CFE_TBL_Bufferselect_[IN]ACTIVE for [in]active table
 * out:    cmd_msg   - set to command message
 * return: nothing
 *
 * Send command to TBL asking it to validate the [in]active image of
 * the named table in its table Registry.
 *
 */

void
cmd_tbl_validate(const char *tablename, unsigned short atflag) {

	/* TBL's CFE_TBL_ValidateCmd_Payload_t structure allocates
	 * CFE_MISSION_TBL_MAX_FULL_NAME_LEN bytes to store the name
	 * of the table we want to validate and its terminating
	 * NUL. If we provide a string that is this long or longer, we
	 * have a bug.  Using C99 memchr() because POSIX strnlen()
	 * isn't available.
	 */
	assert(tablename);
	assert(memchr(tablename, '\0', CFE_MISSION_TBL_MAX_FULL_NAME_LEN));

	/* If atflag isn't one of these two values, we have a bug. */
	assert((atflag == CFE_TBL_BufferSelect_INACTIVE) ||
	       (atflag == CFE_TBL_BufferSelect_ACTIVE));

	cmd_set_header(CFE_TBL_CMD_MID, sizeof(CFE_TBL_ValidateCmd_t),
		CFE_TBL_VALIDATE_CC);

	/* TBL payload numbers are in host byte order */
	cmd_msg.tbl_validate.Payload.ActiveTableFlag = atflag;
	memset(cmd_msg.tbl_validate.Payload.TableName, 0x00,
		CFE_MISSION_TBL_MAX_FULL_NAME_LEN);
	strncpy(cmd_msg.tbl_validate.Payload.TableName, tablename,
		(CFE_MISSION_TBL_MAX_FULL_NAME_LEN-1));
	
	cmd_send((const unsigned char *)&cmd_msg.tbl_validate,
		sizeof(CFE_TBL_ValidateCmd_t));

} /* cmd_tbl_validate() */


/* cmd_tbl_activate()
 *
 * in:     tablename - name of table to validate
 * out:    cmd_msg   - set to command message
 * return: nothing
 *
 * Send command to TBL asking it to activate the inactive image of the
 * named table in its table Registry.
 *
 */

void
cmd_tbl_activate(const char *tablename) {

	/* TBL's CFE_TBL_ActivateCmd_Payload_t structure allocates
	 * CFE_MISSION_TBL_MAX_FULL_NAME_LEN bytes to store the name
	 * of the table we want to validate and its terminating
	 * NUL. If we provide a string that is this long or longer, we
	 * have a bug.  Using C99 memchr() because POSIX strnlen()
	 * isn't available.
	 */
	assert(tablename);
	assert(memchr(tablename, '\0', CFE_MISSION_TBL_MAX_FULL_NAME_LEN));

	cmd_set_header(CFE_TBL_CMD_MID, sizeof(CFE_TBL_ActivateCmd_t),
		CFE_TBL_ACTIVATE_CC);

	memset(cmd_msg.tbl_activate.Payload.TableName, 0x00,
		CFE_MISSION_TBL_MAX_FULL_NAME_LEN);
	strncpy(cmd_msg.tbl_activate.Payload.TableName, tablename,
		(CFE_MISSION_TBL_MAX_FULL_NAME_LEN-1));
	
	cmd_send((const unsigned char *)&cmd_msg.tbl_activate,
		sizeof(CFE_TBL_ActivateCmd_t));

} /* cmd_tbl_activate() */


/* cmd_es_setperffilter()
 *
 * in:     word_num
 *         word_mask
 * out:    cmd_msg   - set to command message
 * return: nothing
 *
 * ES maintains a huge filter bitmask with one bit for every possible
 * perf ID.  It's implemented as an array of uint32s called
 * FilterMask[].  The word_num parm indicates which word to set.  The
 * word_mask parm is the pattern of set and clear bits for that word.
 *
 */

void
cmd_es_setperffilter(uint32 word_num, uint32 word_mask) {

	cmd_set_header(CFE_ES_CMD_MID, sizeof(CFE_ES_SetPerfFilterMaskCmd_t),
		CFE_ES_SET_PERF_FILTER_MASK_CC);

	cmd_msg.es_filter.Payload.FilterMaskNum = word_num;
	cmd_msg.es_filter.Payload.FilterMask    = word_mask;
	
	cmd_send((const unsigned char *)&cmd_msg.es_filter,
		sizeof(CFE_ES_SetPerfFilterMaskCmd_t));

} /* cmd_es_setperffilter() */


/* cmd_es_setperftrigger()
 *
 * in:     word_num
 *         word_mask
 * out:    cmd_msg   - set to command message
 * return: nothing
 *
 * ES maintains a huge trigger bitmask with one bit for every possible
 * perf ID.  It's implemented as an array of uint32s called
 * TriggerMask[].  The word_num parm indicates which word to set.  The
 * word_mask parm is the pattern of set and clear bits for that word.
 *
 */

void
cmd_es_setperftrigger(uint32 word_num, uint32 word_mask) {

	cmd_set_header(CFE_ES_CMD_MID, sizeof(CFE_ES_SetPerfTriggerMaskCmd_t),
		CFE_ES_SET_PERF_TRIGGER_MASK_CC);

	cmd_msg.es_trigger.Payload.TriggerMaskNum = word_num;
	cmd_msg.es_trigger.Payload.TriggerMask    = word_mask;
	
	cmd_send((const unsigned char *)&cmd_msg.es_trigger,
		sizeof(CFE_ES_SetPerfTriggerMaskCmd_t));

} /* cmd_es_setperftrigger */


/* cmd_es_perfstart()
 *
 * in:     nothing
 * out:    cmd_msg   - set to command message
 * return: nothing
 *
 * Ask ES to start storing CFE_ES_PerfLogEntry/Exit() events in its
 * ring buffer.  Tell it to use its CFE_ES_PERF_TRIGGER_START mode,
 * which will cause it to store at most one ring buffer's worth of
 * events before getting tired and ceasing to store.
 */

void
cmd_es_perfstart(void) {

	cmd_set_header(CFE_ES_CMD_MID, sizeof(CFE_ES_StartPerfDataCmd_t),
		CFE_ES_START_PERF_DATA_CC);

	cmd_msg.es_start.Payload.TriggerMode = CFE_ES_PERF_TRIGGER_START;
	
	cmd_send((const unsigned char *)&cmd_msg.es_start,
		sizeof(CFE_ES_StartPerfDataCmd_t));

} /* cmd_es_perfstart() */


/* cmd_es_perfstop()
 *
 * in:     nothing
 * out:    cmd_msg   - set to command message
 * return: nothing
 *
 * Ask ES to stop storing CFE_ES_PerfLogEntry/Exit() events in its
 * ring buffer and write the ring buffer's contents to a file.
 *
 * By default, cFS prefers to save this data to a file in the
 * spacecraft's /ram ramdisk.  However, PSP does not support /ram in
 * its Linux desktop build configuration.  We'll use a file in /cf
 * instead.
 */

void
cmd_es_perfstop(void) {

	cmd_set_header(CFE_ES_CMD_MID, sizeof(CFE_ES_StopPerfDataCmd_t),
		CFE_ES_STOP_PERF_DATA_CC);

	memset(cmd_msg.es_stop.Payload.DataFileName, 0x00,
		CFE_MISSION_MAX_PATH_LEN);
	strncpy(cmd_msg.es_stop.Payload.DataFileName, PERF_FILENAME,
		(CFE_MISSION_MAX_PATH_LEN-1));
	
	cmd_send((const unsigned char *)&cmd_msg.es_stop,
		sizeof(CFE_ES_StopPerfDataCmd_t));

} /* cmd_es_perfstop() */
