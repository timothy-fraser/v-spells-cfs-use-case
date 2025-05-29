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

/* This file defines functions for receiving telemety messages from
 * the simulated spacecraft, focusing on those specific messages we
 * need to test cFS app table validation functions.
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "cfe.h"                             /* for CCSDS_PrimaryHeader_t */
#include "cfe_mission_cfg.h"    /* for cFE service HK TLM topic IDs, etc. */
#include "ci_lab_msgids.h"                  /* for CI_LAB HK TLM topic ID */
#include "cfe_evs_extern_typedefs.h"      /* for CFE_EVS_EventType_* enum */
#include "cfe_evs_msg.h"            /* for CFE_EVS_LongEventTlm_Payload_t */
#include "cfe_tbl_eventids.h"                        /* for TBL event IDs */
#include "cfe_time_eventids.h"                      /* for TIME event IDs */
#include "cfe_evs_topicids.h" /* for EVS LONG and SHORT message topic IDs */
#include "cfe_evs_msgids.h"                       /* for EVS HKTLM MSG ID */ 
#include "cfe_es_msgids.h"                        /* for ES HK TLM MSG ID */
#include "cfe_sb_msgids.h"                        /* for SB HK TLM MSG ID */
#include "cfe_tbl_msgids.h"                      /* for TBL HK TLM MSG ID */
#include "cfe_time_msgids.h"                    /* for TIME HK TLM MSG ID */
#include "to_lab_msgids.h"                  /* for TO_LAB HK TLM topic ID */
#include "to_lab_events.h"                        /* for TO_LAB event IDs */
#include "sample_app_msgids.h"          /* for SAMPLE_APP HK TLM topic ID */
#include "vs_eventids.h"                       /* for common VS event IDs */
#include "vs_ground.h"          /* for app names, topic IDs, and perf IDs */

#include "common_constants.h"
#include "tlm.h"

#define TLM_ADDR "127.0.0.1"  /* Send telemetry to this address */
#define TLM_PORT 1235         /* cFS default telemetry port */

/* This is my best guess at the maximum telemetry message size.  The
 * largest size I saw during development was 172 bytes.  The cFS
 * cmdUtil tool uses 1024 as well.
 */
#define TLM_MSG_MAX_SIZE 1024

/* CCSDS primary headers begin with a 16-bit field in network byte
 * order.  The lower TLM_TOPICID_MASK bits of that field describe the
 * "application ID" of the app or service that sent the event
 * described in the telemetry message, or alternately the "topic ID"
 * of the event.
 */
#define TLM_TOPICID_MASK 0x07FF

/* This program handles only certain kinds of messages.  We use the
 * CHECK macro to perform a series of checks on messages received to
 * confirm they are of a kind we handle.  The CHECK macro considers a
 * particular byte of the message, masks it with a particular mask and
 * compares the result to a particular value.  It returns false on
 * match (pass) and true on no match (fail, bad message).  CHECK and
 * the mask, value pairs for each check follow:
 */
#define CHECK(i,m,v) (((i) & (m)) != (v))
#define VER_MASK  0xF0 /* Upper nibble indicates protocol version */
#define VER_VAL   0x00 /* This value indicates CCSDS version 1 */
#define TLM_MASK  0x10 /* This bit is the command/telemetry falg */
#define TLM_VAL   0x00 /* This value indicates telemetry packet */
#define HDR_MASK  0x08 /* This bit is secondary header flag */
#define HDR_VAL   0x08 /* This value indicates secondary header follows */
#define FRG_MASK  0xC0 /* Mask for fragment flag */
#define FRG_VAL   0xC0 /* Flag value indicating complete packet */

/* If we encounter a message whose format doesn't meet our
 * expectations, we'll print a table-like debug dump of its bytes with
 * this many columns.
 */
#define DEBUG_DUMP_COLUMNS 8


/*
 *  -------- Module local state and local functions ---------
 */

static struct sockaddr_in tlm_addr;     /* receive telemetry from here */
static int tlmfd;                       /* socket for receiving telemetry */

/* This union is the buffer into which we will read telemetry
 * messages.  These messages come in many flavors with many sizes.
 * The byte array member exists solely to make sure the buffer is
 * large enough to contain the largest telemetry message we expect to
 * receive.  The ccsds member enables us to examine the fields of the
 * CCSDS header that is common to all message flavors without doing
 * array index math.  The other cFS/cFE structure type members are
 * there so that, once our examination of the CCSDS header has
 * revealed a message's specific flavor, we can examine its
 * flavor-specific fields.
 */
static union {
	char raw_bytes[TLM_MSG_MAX_SIZE];
	CCSDS_PrimaryHeader_t  ccsds;
	CFE_EVS_LongEventTlm_t evs_long;
} tlm_msg;


/* tlm_check_msg_generic()
 *
 * in:     rec_len - number of bytes read from socket into tlm_msg
 *         tlm_msg - message to check, latest message received
 * out:    nothing
 * return: 0 if message passes all checks, else -1.
 *
 * This function implements sanity checks on fields common to all
 * telemetry messages.
 */

int
tlm_check_msg_generic(size_t rec_len) {
	
	unsigned short length;  /* message length */

	/* Fail immediately if we didn't receive enough bytes to cover
	 * the common CCSDS header.
	 */
	if (rec_len < sizeof(CCSDS_PrimaryHeader_t)) {
		fprintf(stderr,	"Received an incomplete CCSDS header.\n");
		return -1;
	}

	/* Fail if the message's length field indicates a length that
	 * doesn't match the number of bytes we received.
	 */
	length = tlm_length();
	if (length != rec_len) {
		fprintf(stderr, "Received message with length field %u "
			"that doesn't match bytes received %lu\n",
			length, rec_len);
		return -1;
	}
		
	/* We support only CCSDS version 1 messages.  Fail if the
	 * received bytes don't look like one.
	 */
	if (CHECK(tlm_msg.ccsds.StreamId[0], VER_MASK, VER_VAL)) { 
		fprintf(stderr,	"Received a non-CCSDS-ver-1 message.\n");
		return -1;
	}

	/* Fail if this isn't a telemetry message. */
	if (CHECK(tlm_msg.ccsds.StreamId[0], TLM_MASK, TLM_VAL)) {
		fprintf(stderr, "Received a non-telemetry message.\n");
		return -1;
	}

	/* Fail if message doesn't have a secondary header. */
	if (CHECK(tlm_msg.ccsds.StreamId[0], HDR_MASK, HDR_VAL)) {
		fprintf(stderr, "Received message without "
			"a secondary header.\n");
		return -1;
	}
	
	/* Fail if the message's fragmentation flag says it is
	 * incomplete.
	 */
	if (CHECK(tlm_msg.ccsds.Sequence[0], FRG_MASK, FRG_VAL)) {
		fprintf(stderr, "Received message "
			"without complete flag set.\n");
		return -1;
	}

	/* All checks passed */
	return 0;
	
} /* tlm_check_msg_generic() */


/* tlm_check_msg_evs_long()
 *
 * in:     tlm_msg - message to check, latest message received
 * out:    nothing
 * return: 0 if message passes all checks, else -1.
 *
 * This function implements sanity checks on fields specific to EVS
 * long-form telemetry messages (CFE_MISSION_EVS_LONG_EVENT_MSG_MSG).
 */

int
tlm_check_msg_evs_long(void) {

	const char *appname = tlm_evs_appname();
	const char *message = tlm_evs_message();
	
	/* The payloads of topic ID CFE_MISSION_EVS_LONG_EVENT_MSG_MSG
	 * messages contain two strings that *ought* to be
	 * NUL-terminated.  It looks like
	 * cFE/modules/evs/fsw/src/cfe_evs_utils.c:EVS_GenerateEventTelemetry()
	 * takes care to make sure this NUL termination always
	 * happens, but it can't hurt to double-check here.
	 */

	if (!memchr(appname, '\0', CFE_MISSION_MAX_API_LEN))
		return -1;

	if (!memchr(message, '\0', CFE_MISSION_EVS_MAX_MESSAGE_LENGTH))
		return -1;

	return 0;

} /* tlm_check_msg_evs_long() */


/* tlm_check_msg_generic()
 *
 * in:     rec_len - number of bytes read from socket into tlm_msg
 *         tlm_msg - message to check, latest message received
 * out:    nothing
 * return: 0 if message passes all checks, else -1.
 *
 * This function attempts to confirm that the structure of a received
 * telemetry message meets our expectations before we do any further
 * processing on it.  Any deviation is likely a bug in our
 * expectations - specifically, there's probably a flavor of telemetry
 * we don't yet understand.  In this case, this function dumps the
 * message to the console to help debugging.
 */

int
tlm_check_msg(size_t rec_len) {

	unsigned short i;       /* message byte index for debug dump */
	
	do {
		/* Do generic checks that apply to all flavors of
		 * telemetry message first.
		 */
		if (tlm_check_msg_generic(rec_len)) break;

		/* Do further flavor-specific checks as needed. */
		if (tlm_topicid() == CFE_MISSION_EVS_LONG_EVENT_MSG_MSG) {
			if (tlm_check_msg_evs_long()) break;
		}

		/* All checks passed. */
		return 0;
		
	} while (0);  /* No loop, using do-while to avoid gotos. */

	/* We wind up here if a check failed.  Print a debug dump of
	 * the message to the console.
	 */
	for(i = 0; i < rec_len; i++) {
		if (!(i % DEBUG_DUMP_COLUMNS)) printf("\n");
		printf("%02X ", tlm_msg.raw_bytes[ i ]);
	}
	printf("\n");

	return -1;
	
} /* tlm_check_msg() */
	

/*
 *  -------- Functions exported by this module ---------
 */

/* tlm_init()
 *
 * in:     nothing
 * out:    tlm_addr - set to address at which we will receive telemetry
 *         tlmfd   - set to port at which we will receive telemetry
 * return: nothing
 *
 * Open the socket for receiving telemetry.  For telemetry, we act as
 * the "server" and the spacecraft acts as the "client".
 *
 * Call this function before calling any of this module's other
 * functions.
 */

void
tlm_init(void) {

	/* Create IP/UDP socket for receiving telemetry.  For
	 * telemetry, I am the server and the spacecraft is the
	 * client.
	 */
	tlm_addr.sin_family      = AF_INET;
	tlm_addr.sin_port        = htons(TLM_PORT);
	tlm_addr.sin_addr.s_addr = inet_addr(TLM_ADDR);

	if (-1 == (tlmfd = socket(AF_INET, SOCK_DGRAM, 0))) {
		perror("Failed to create telemetry socket");
		exit(-1);
	}
	if (-1 == bind(tlmfd, (struct sockaddr *)&tlm_addr,
		sizeof(struct sockaddr_in))) {
		perror("Failed to bind to telemetry port");
		exit(-1);
	}
	
} /* tlm_init() */


/* tlm_receive()
 *
 * in:     tlmfd   - file descriptor to open socket for receiving telemetry
 * out:    tlmfd   - will read next message from socket, consuming it
 *         tlm_msg - will write message to this buffer
 * return: nothing
 *
 * Receives next telemetry message and stores it.  Runs some basic
 * sanity checks on the received message to make sure its structure
 * meets our expectations and forces the program to exit if anything
 * seems surprising.
 *
 */
   
void
tlm_receive(void) {

	size_t rec_len;      /* msg size according to recv() */
	
	/* Receive byes from spacecraft. */
	if (-1 == (rec_len = recv(tlmfd, tlm_msg.raw_bytes, TLM_MSG_MAX_SIZE,
		0x00))) {
		perror("Failed to receive telemetry from spacecraft");
		exit(-1);
	}

	/* If we receive a message longer than TLM_MSG_MAX_SIZE, then
	 * that's a bug - we need to increase that constant's value.
	 */
	assert(rec_len <= TLM_MSG_MAX_SIZE);
	
	/* Check to make sure this is a message we can handle. */
	if (tlm_check_msg(rec_len)) exit(-1);

} /* tlm_receive() */


/* tlm_topicid()
 *
 * in:     tlm_msg - latest received telemetry message
 * out:    nothing
 * return: telemetry message topic ID.
 *
 * The CCSDS primary header begins with a field called "application
 * ID".  I see many of the services and apps use this field for "topic
 * IDs" instead, where each app or service will define two topic IDs,
 * one for housekeeping telemetry and one for other telemetry.  I'm
 * going with "topic ID".
 */

tlm_topicid_t
tlm_topicid(void) {

	/* The header has a 16-bit field whose lower TLM_TOPICID_MASK
	 * bits contain the topic ID in network (big endian) order.
	 */
	unsigned short topicid_no =
		*((unsigned short *)tlm_msg.ccsds.StreamId);

	/* CCSDS header numbers are in network order aka big-endian. */
	return (ntohs(topicid_no) & TLM_TOPICID_MASK);

} /* tlm_topicid() */
	

/* tlm_length()
 *
 * in:     tlm_msg - latest received telemetry message
 * out:    nothing
 * return: telemetry message length
 *
 * This function returns the true full length of the message, not the
 * shorter fake length reported in the message header length field.
 */
 
tlm_length_t
tlm_length(void) {

	/* The header has a 16-bit length field.  Its value is in
	 * network order (big-endian) and under-counts by
	 * CCSDS_MSG_LENGTH_DELTA bytes.
	 */
	unsigned short length_no = *((unsigned short *)tlm_msg.ccsds.Length);

	/* CCSDS header numbers are in network order aka big-endian. */
	/* Report the true length in host order. */
	return (ntohs(length_no) + CCSDS_MSG_LENGTH_DELTA);
	
} /* tlm_length() */


/* tlm_evs_appname()
 *
 * in:     tlm_msg - latest received *EVS long-form* telemetry message
 * out:    nothing
 * return: application name string
 *
 * This function returns the application name string from EVS
 * long-form telemetry message payloads.
 */

const char *
tlm_evs_appname(void) {

	CFE_EVS_LongEventTlm_Payload_t *p_payload = &tlm_msg.evs_long.Payload;

	/* Callers should have already confirmed that this is a
	 * long-form EVS telemetry messages.  Anything else is a bug.
	 */
	assert(tlm_topicid() == CFE_MISSION_EVS_LONG_EVENT_MSG_MSG);

	/* Return a pointer to the AppName string. */
	return p_payload->PacketID.AppName;
	
} /* tlm_evs_appname() */


/* tlm_evs_eventid()
 *
 * in:     tlm_msg - latest received *EVS long-form* telemetry message
 * out:    nothing
 * return: event ID
 *
 * This function returns the Event ID from EVS long-form telemetry
 * message payloads.
 */

tlm_eventid_t
tlm_evs_eventid(void) {

	CFE_EVS_LongEventTlm_Payload_t *p_payload = &tlm_msg.evs_long.Payload;

	/* Callers should have already confirmed that this is a
	 * long-form EVS telemetry messages.  Anything else is a bug.
	 */
	assert(tlm_topicid() == CFE_MISSION_EVS_LONG_EVENT_MSG_MSG);

	/* CFE_EVS_PacketID_t numbers are in host byte order. */
	return p_payload->PacketID.EventID;

} /* tlm_evs_eventid() */


/* tlm_evs_eventtype()
 *
 * in:     tlm_msg - latest received *EVS long-form* telemetry message
 * out:    nothing
 * return: event type
 *
 * This function returns the Event Type from EVS long-form telemetry
 * message payloads.
 */

tlm_eventtype_t
tlm_evs_eventtype(void) {

	CFE_EVS_LongEventTlm_Payload_t *p_payload = &tlm_msg.evs_long.Payload;

	/* Callers should have already confirmed that this is a
	 * long-form EVS telemetry messages.  Anything else is a bug.
	 */
	assert(tlm_topicid() == CFE_MISSION_EVS_LONG_EVENT_MSG_MSG);

	/* CFE_EVS_PacketID_t numbers are in host byte order. */
	return (tlm_eventtype_t)(p_payload->PacketID.EventType);

} /* tlm_evs_eventtype() */


/* tlm_evs_message()
 *
 * in:     tlm_msg - latest received *EVS long-form* telemetry message
 * out:    nothing
 * return: message string
 *
 * This function returns the message string from EVS long-form
 * telemetry message payloads.
 */

const char *
tlm_evs_message(void) {

	CFE_EVS_LongEventTlm_Payload_t *p_payload = &tlm_msg.evs_long.Payload;

	/* Callers should have already confirmed that this is a
	 * long-form EVS telemetry messages.  Anything else is a bug.
	 */
	assert(tlm_topicid() == CFE_MISSION_EVS_LONG_EVENT_MSG_MSG);

	return (const char *)p_payload->Message;

} /* tlm_evs_message() */


/* tlm_topicid_to_string()
 *
 * in:     topicid - topic ID to translate
 * out:    nothing
 * return: string form of topicid
 *
 * This function provides strings suitable for pretty-printing Topic IDs.
 *
 */

const char *
tlm_topicid_to_string(tlm_topicid_t topicid) {

	switch (topicid) {
	case (CI_LAB_HK_TLM_MID & 0xFF):
		return "Command Ingest Service (CI) housekeeping";
	case CFE_MISSION_ES_HK_TLM_MSG:
		return "Executive Services (ES) housekeeping";
	case CFE_MISSION_EVS_HK_TLM_MSG:
		return "Event Services (EVS) housekeeping";
	case CFE_MISSION_EVS_LONG_EVENT_MSG_MSG:
		return "Event Services (EVS) long message";
	case CFE_MISSION_EVS_SHORT_EVENT_MSG_MSG:
		return "Event Services (EVS) short message";
	case (SAMPLE_APP_HK_TLM_MID & 0xFF):
		return "Sample App housekeeping";
	case CFE_MISSION_SB_HK_TLM_MSG:
		return "Software Bus (SB) housekeeping";
	case CFE_MISSION_TBL_HK_TLM_MSG:
		return "Table Services (TBL) housekeeping";
	case CFE_MISSION_TIME_HK_TLM_MSG:
		return "Time Services (TIME) housekeeping";
	case (TO_LAB_HK_TLM_MID & 0xFF):
		return "Telemetry Output (TO, TO_LAB) housekeeping";
	case (VSA_TLM_HK_MID & 0xFF):
		return "V-SPELLS App Alpha (VSA) housekeeping";
	case (VSB_TLM_HK_MID & 0xFF):
		return "V-SPELLS App Bravo (VSB) housekeeping";
	case (VSC_TLM_HK_MID & 0xFF):
		return "V-SPELLS App Charlie (VSC) housekeeping";
	default:
		return "Unknown topic ID";
	}

} /* tlm_topicid_to_string() */


/* tlm_eventid_to_string()
 *
 * in:     app_name - application whose event ID we want translated
 *         eventid  - app-specific event ID we want translated
 * out:    nothing
 * return: string form of eventid
 *
 * This function provides strings suitable for pretty-printing Event
 * IDs.  Note that you need to identify the app that owns the event ID
 * to accomplish this translation since different apps use the same
 * event ID values to mean different app-specific things.
 *
 */

const char *
tlm_eventid_to_string(const char *app_name, tlm_eventid_t eventid) {

	if (!strcmp(app_name, TLM_NAME_TBL)) {
		switch (eventid) {
		case CFE_TBL_UPDATE_SUCCESS_INF_EID: return "ACTOK";
		case CFE_TBL_UPDATE_ERR_EID:         return "ACTER";
		case CFE_TBL_FILE_LOADED_INF_EID:    return "LOAD ";
		case CFE_TBL_UNVALIDATED_ERR_EID:    return "UNVLD";
		case CFE_TBL_VALIDATION_INF_EID:     return "VALOK";
		case CFE_TBL_VALIDATION_ERR_EID:     return "VALER";
		}
	} else if (!strcmp(app_name, TLM_NAME_TIME)) {
		switch (eventid) {
		case CFE_TIME_FLY_ON_EID:  return "FLYON";
		case CFE_TIME_FLY_OFF_EID: return "FLYOF";
		}
	} else if (!strcmp(app_name, TLM_NAME_TO)) {
		switch (eventid) {
		case TO_LAB_TLMOUTENA_INF_EID: return "TLMON";
		}

	} else if ((!strcmp(app_name, VSA_APP_NAME)) ||
		   (!strcmp(app_name, VSB_APP_NAME)) ||
		   (!strcmp(app_name, VSC_APP_NAME))) {
		switch (eventid) {
		case VS_CMD_NOOP_INF_EID:    return "VNOOP";
		case VS_CMD_RESET_INF_EID:   return "RESET";
		case VS_STARTUP_OK_INF_EID:  return "START";
		case VS_VALIDATION_INF_EID:  return "VINFO";
		case VS_MSG_BAD_CC_ERR_EID:  return "BADCC";
		case VS_MSG_BAD_MID_ERR_EID: return "BADMD";
		case VS_PIPE_ERR_EID:        return "PIPER";
		case VS_TBL_ZERO_ERR_EID:    return "ZEROS";
		case VS_TBL_PARM_ERR_EID:    return "EPARM";
		case VS_TBL_PAD_ERR_EID:     return "PADER";
		case VS_TBL_LBND_ERR_EID:    return "LBNDR";
		case VS_TBL_HBND_ERR_EID:    return "HBNDR";
		case VS_TBL_ORDER_ERR_EID:   return "ORDER";
		case VS_TBL_EXTRA_ERR_EID:   return "EXTRA";
		case VS_TBL_REDEF_ERR_EID:   return "REDEF";
		}
	}

	/* default for all event IDs we haven't yet encoded */
	return "UNKWN";

} /* tlm_eventid_to_string() */


/* tlm_eventtype_to_string()
 *
 * in:     eventtype - event type to translate
 * out:    nothing
 * return: string form of event type
 *
 * This function provides strings suitable for pretty-printing event types.
 *
 */

const char *
tlm_eventtype_to_string(tlm_eventtype_t eventtype) {
	
	switch(eventtype) {
	case CFE_EVS_EventType_DEBUG:       return "DEBG";
	case CFE_EVS_EventType_INFORMATION: return "INFO";
	case CFE_EVS_EventType_ERROR:       return "EROR";
	case CFE_EVS_EventType_CRITICAL:    return "CRIT";
	default:                            return "UNKN";
	}

} /* tlm_eventtype_to_string() */


