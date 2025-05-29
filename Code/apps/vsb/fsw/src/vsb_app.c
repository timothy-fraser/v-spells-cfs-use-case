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

/*
 * This file defines the VSB App's main entry point, its
 * initialization rountines and runloop.
 */

#include "cfe.h"
#include "common_types.h"
#include "osapi.h"

#include "vs_msgstruct.h"
#include "vs_ground.h"

#include "vsb_version.h"
#include "vsb_msgstruct.h"
#include "vs_eventids.h"
#include "vsb_eventids.h"
#include "vsb_fcncodes.h"
#include "vsb_table.h"

#define VSB_APP_CMD_PIPE_DEPTH  16  /* Max pending messages before overflow */
#define VSB_APP_CMD_PIPE_NAME   "VSB_APP_CMD_PIPE"


/* ---------------- Module local state and functions ---------------- */

/* cFS/cFE Apps traditionally bundle the variables that store their
 * runtime state into a struct.  Here's the state struct for this app.
 *
 * cFS/cFE Apps traditionally keep counts of command messages (not)
 * processed correctly so that they can be reported in housekeeping
 * telemetry.  Rather than declaring separate variables to keep these
 * counts and copying their values into a housekeeping telemetry
 * message when it's time to send, we'll simply declare a housekeeping
 * telemetry message structure and keep the counts directly in its
 * fields.
 */
   
static struct {
	CFE_SB_PipeId_t  cmd_pipe;   /* we read commands from this pipe */
	CFE_TBL_Handle_t h_table;    /* handle to TBL-managed table */
	VSB_tlm_hk_t     msg_tlm_hk; /* housekeeping telemetry message */
} VSB_state;
	

static void
vsb_reset_diagnostic_counters(void) {
	VSB_state.msg_tlm_hk.payload.ctr_cmd_ok    = 0;
	VSB_state.msg_tlm_hk.payload.ctr_cmd_error = 0;
} /* vsb_reset_diagnostic_counters() */	


/* VSB_init()
 *
 * in:     nothing
 * out:    VSB_state - updated to reflect initialization described below
 * return: CFE_SUCCESS if initialization goes well, otherwise CFE
 *         error codes.
 *
 * This function follows the traditional CFS App initialization
 * pattern: it prepares the app to receive ground and housekeeping
 * commands, initializes its table, and fills in the constant fields
 * of its template telemetry message.
 *
 */

static CFE_Status_t
VSB_init(void) {

	CFE_Status_t result;  /* holds error codes returned by functions */

	/* Initialize our housekeeping telemetry message.  This clears
	 * the diagnostic counters in its payload area to zero.
	 *
	 * Note that we don't need to clear our counters by calling
	 * our vsa_reset_diagnostic_counters() function here because
	 * CFE_MSG_Init() zeroes the entire message.
	 */
	
	CFE_MSG_Init(CFE_MSG_PTR(VSB_state.msg_tlm_hk.header),
		CFE_SB_ValueToMsgId(VSB_TLM_HK_MID),
		sizeof(VSB_tlm_hk_t));

	/* Register with EVS.  Specify no event filter.  Even though
	 * we're not specifying a filter, we must specify the
	 * CFE_EVS_EventFilter_BINARY type or the present EVS
	 * implementation will report CFE_EVS_UNKNOWN_FILTER.
	 */
	if (CFE_SUCCESS != (result = CFE_EVS_Register(NULL, 0,
		CFE_EVS_EventFilter_BINARY))) {
		CFE_ES_WriteToSysLog("%s: CFE_EVS_Register() returned 0x%08X"
			"; %s will shutdown.\n", VSB_APP_NAME, result,
			VSB_APP_NAME);
		return result;
	}

	/* Create SB pipe for receiving commands. */
	if (CFE_SUCCESS != (result = CFE_SB_CreatePipe(&VSB_state.cmd_pipe,
		VSB_APP_CMD_PIPE_DEPTH, VSB_APP_CMD_PIPE_NAME))) {
		CFE_ES_WriteToSysLog("%s: CFE_SB_CreatePipe() returned 0x%08X"
			"; %s will shutdown.\n", VSB_APP_NAME, result,
			VSB_APP_NAME);
		return result;
	}

	/* Subscribe to ground command messages. */
	if (CFE_SUCCESS != (result = CFE_SB_Subscribe(
		CFE_SB_ValueToMsgId(VSB_CMD_MID), VSB_state.cmd_pipe))) {
		CFE_ES_WriteToSysLog("%s: CFE_SB_Subscribe() returned 0x%08X"
			"; %s App will shutdown.\n", VSB_APP_NAME, result,
			VSB_APP_NAME);
		return result;
	}

	/* Subscribe to housekeeping command messages. */
	if (CFE_SUCCESS != (result = CFE_SB_Subscribe(
		CFE_SB_ValueToMsgId(VSB_SEND_HK_MID), VSB_state.cmd_pipe))) {
		CFE_ES_WriteToSysLog("%s: CFE_SB_Subscribe() returned 0x%08X"
			"; %s will shutdown.\n", VSB_APP_NAME, result,
			VSB_APP_NAME);
		return result;
	}

	if (CFE_SUCCESS != (result = VSB_table_init(&(VSB_state.h_table)))) {
		/* No need to CFE_ES_WriteToSysLog() an error message
		 * here; VSB_table_init() will already have written a
		 * specific error message to the log.
		 */
		return result;
	}
	
	/* Report our successfull initialization. */
	CFE_EVS_SendEvent(VSB_STARTUP_OK_INF_EID,
		CFE_EVS_EventType_INFORMATION,
		"%s initialized, awaiting enable command",
		VSB_APP_VERSION_STRING);
	
	return CFE_SUCCESS;

} /* VSB_init() */


/* VSB_process_housekeeping()
 *
 * in:     p_cmd_msg - housekeeping command message to process
 * out:    nothing
 * return: CFE_Success on success, otherwise CFE error codes.
 *
 * Handle housekeeping commands, including table validation.
 *
 * This function handles all housekeeping commands, including table
 * validation requests from the CFE Table Service (TBL).  Table
 * validation requests have an interesting control flow:
 *
 * (1) The ground station operator asks TBL to validate a table image.
 * (2) TBL asks the app to validate the image on its next housekeeping
 *     cycle.
 * (3) This function receives TBL's request and uses TBL's
 *     CFE_TBL_Manage() convenience function to handle it.
 * (4) TBL's convenience function ultimately invokes the app's table
 *     validation function to do the actual validation.
 *
 * Side effect: emits a housekeeping telemetry message.
 */

static CFE_Status_t
VSB_process_housekeeping(CFE_MSG_Message_t *p_cmd_msg) {

	/* Housekeeping command messages should have length equal to
	 * sizeof(CFE_MSG_Message_t).  Their command code field isn't
	 * meaningful; the SCH_LAB app I'm building against sets it to
	 * 0.  Rather than check for these proper values, I'm
	 * generously accepting any message with the proper
	 * send-housekeeping MID.
	 */

	/* Ask TBL to perform any requested table loads (aka
	 * "updates") or validations.  Don't bother examining the
	 * result.  Instead, rely on the error reporting done by
	 * CFE_TBL_Manage() and our own validation procedure.
	 */
	CFE_TBL_Manage(VSB_state.h_table);

	/* Emit a housekeeping telemetry message. */
	CFE_SB_TimeStampMsg(CFE_MSG_PTR(VSB_state.msg_tlm_hk.header));
	CFE_SB_TransmitMsg(CFE_MSG_PTR(VSB_state.msg_tlm_hk.header), true);

	return CFE_SUCCESS;
	
} /* VSB_process_housekeeping() */


/* VSB_process_ground_command()
 *
 * in:     p_cmd_msg - ground command message to handle
 * out:    VSB_state - may zero command processing statistics counters
 * return: CFE_SUCCESS on success, otherwise VSB_MSG_BAD_CC_ERR_EID.
 *
 * This function handles all commands from the ground station.
 *
 * Side effect: will emit telemetry messages specific to the type of
 * command processed or an error telemetry message for command codes
 * it doesn't support.
 */

static CFE_Status_t
VSB_process_ground_command(CFE_MSG_Message_t *p_cmd_msg) {

	CFE_MSG_FcnCode_t msg_cc;  /* command code from message header */

	/* None of the presently-supported ground command messages
	 * have payloads; all should have length equal to
	 * sizeof(CFE_MSG_Message_t).  However, I'm generously
	 * accepting ground command messages with any length.
	 */
	CFE_MSG_GetFcnCode(p_cmd_msg, &msg_cc);

	switch (msg_cc) {

	case VSB_NOOP_CC:
		/* Per cFS/cFE App tradition, the NOOP command causes
		 * the app to send telemetry indicating its version.
		 */
		CFE_EVS_SendEvent(VSB_CMD_NOOP_INF_EID,
			CFE_EVS_EventType_INFORMATION,
			"%s received no-op command.", VSB_APP_VERSION_STRING);
		return CFE_SUCCESS;

	case VSB_RESET_COUNTERS_CC:
		vsb_reset_diagnostic_counters();
		CFE_EVS_SendEvent(VSB_CMD_RESET_INF_EID,
			CFE_EVS_EventType_INFORMATION,
			"%s: reset diagnostic counters.", VSB_APP_NAME);
		return CFE_SUCCESS;

	default:
		CFE_EVS_SendEvent(VSB_MSG_BAD_CC_ERR_EID,
			CFE_EVS_EventType_ERROR,
			"%s: received ground command message "
			"with invalid command code 0x%02X.",
			VSB_APP_NAME, msg_cc);
		return VSB_MSG_BAD_CC_ERR_EID;
	}
	
} /* VSB_process_ground_command() */


/* VSB_process_command()
 *
 * in:     p_cmd_msg - housekeeping or ground command to handle.
 * out:    VSB_state - increments command processing statistics counters,
 *                     potentially rolling them back to zero on overflow.
 * return: nothing
 *
 * This function distinguishes between housekeeping and ground
 * commands and invokes the appropriate handler function.
 * Housekeeping commands are automatic, sent periodically by the CFE
 * Scheduler service (SCH) per its configuration table.  Ground
 * commands come from the human ground station operator.
 *
 * Side effect: will emit an error telemetry message if it sees a
 * message that is neither a ground or housekeeping command.
 */

static void
VSB_process_command(CFE_MSG_Message_t *p_cmd_msg) {

	CFE_SB_MsgId_t msgid_opaque;  /* MID in some opaque representation */
	CFE_SB_MsgId_Atom_t msgid;    /* MID in integer representation */
	CFE_Status_t result;          /* processed OK vs. error code */
	
	/* Get the message ID from the command in p_cmd_buf and invoke
	 * the proper handler function for that kind of message based
	 * on what we see.
	 *
	 */
	CFE_MSG_GetMsgId(p_cmd_msg, &msgid_opaque);
	msgid = CFE_SB_MsgIdToValue(msgid_opaque);
	
	switch (msgid) {

	case VSB_SEND_HK_MID:
		result = VSB_process_housekeeping(p_cmd_msg);
		break;

	case VSB_CMD_MID:
		result = VSB_process_ground_command(p_cmd_msg);
		break;

	default:
		result = VSB_MSG_BAD_MID_ERR_EID;
		CFE_EVS_SendEvent(result, CFE_EVS_EventType_ERROR,
			"%s: received command message "
			"with invalid MID 0x%03X.", VSB_APP_NAME, msgid);
	}

	/* Update diagnostic count of messages (not) handled correctly. */
	if (result == CFE_SUCCESS) {
		VSB_state.msg_tlm_hk.payload.ctr_cmd_ok++;    /* can roll */
	} else {
		VSB_state.msg_tlm_hk.payload.ctr_cmd_error++; /* can roll */
	}
	
} /* VSB_process_command() */


/* ---------------- Functions exported by this module ---------------- */

/* VSB_Main()
 *
 * in:     nothing
 * out:
 * return: nothing
 *
 * This is the main entry point.  It initializes the app and then
 * executes its run-loop until the CFE Executive Service (ES) tells it
 * to quit.
 *
 */

void
VSB_Main(void) {

	/* This is the "run status" argument we pass to
	 * CFE_ES_RunLoop().  We'll begin with it set to
	 * CFE_ES_RunStatus_APP_RUN, a value that will tell ES we are
	 * happy and healthy.  If we encounter an unrecoverable error
	 * we will set it to ES_RunStatus_APP_ERROR.  This will tell
	 * ES to shut us down the next time we call CFE_ES_RunLoop().
	 */
	CFE_ES_RunStatus_Enum_t run_status = CFE_ES_RunStatus_APP_RUN;
	CFE_SB_Buffer_t *p_cmd_buf; /* SB-provided command buffer */
	uint32 result;  /* holds error codes returned by functions */
	
	/* Mark the start of app-specific processing for performance
	 * monitoring.
	 */
	CFE_ES_PerfLogEntry(VSB_ALL_PERF_ID);

	/* Initialize.  If anything fails, tell CFE_ES_RunLoop() to
	 * shut us down.
	 */
	if (CFE_SUCCESS != VSB_init()) run_status = CFE_ES_RunStatus_APP_ERROR;

	/* Enter the main processing loop and process commands until
	 * ES tells us to stop.
	 */
	while (CFE_ES_RunLoop(&run_status)) {

		/* Mark the start of a pause in app-specific processing
		 * while we block waiting for SB to give us a command
		 * to process.
		 */
		CFE_ES_PerfLogExit(VSB_ALL_PERF_ID);

		/* Block here until we get the next command. */
		result = CFE_SB_ReceiveBuffer(&p_cmd_buf, VSB_state.cmd_pipe,
			CFE_SB_PEND_FOREVER);

		/* Mark the resumption of app-specific processing
		 * now that we're done waiting for a command.
		 */
		CFE_ES_PerfLogEntry(VSB_ALL_PERF_ID);

		/* Process command.  Errors reading from the command
		 * pipe are unrecoverable; ask ES to shut us down if
		 * we see one.  Any errors we find in the command
		 * itself while attempting to process it are
		 * recoverable and don't merit a shutdown - we'll just
		 * ignore a malformed command and await the next one.
		 */
		if (result == CFE_SUCCESS) {
			VSB_process_command(&(p_cmd_buf->Msg));
		} else {
			CFE_EVS_SendEvent(VSB_PIPE_ERR_EID,
				CFE_EVS_EventType_ERROR,
				"VSB: SB pipe read error; "
				"VSB App will shutdown");
			run_status = CFE_ES_RunStatus_APP_ERROR;
		}
		
	} /* while we're in the runloop processing commands */

	/* CFE_ES_RunLoop() has told us to shut down.  Mark the final
	 * end of app-specific processing and exit.
	 */
	CFE_ES_PerfLogExit(VSB_ALL_PERF_ID);
	CFE_ES_ExitApp(run_status);

} /* VSB_Main() */
