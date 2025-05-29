#ifndef _VS_GROUND_H_
#define _VS_GROUND_H_

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

/* This file defines constants that the ground station/test suite and
 * our V-SPELLS apps must share to inter-operate.  cFS apps
 * traditionally do not have this kind of ground-coordination header,
 * but it seems useful for our V-SPELLS experiments.
 */


/* You must keep these app name constants synchronized with the app
 * names you use to start apps in your cpu?_cfe_es_startup.scr files.
 */

#define VSA_APP_NAME "VSA_APP"
#define VSB_APP_NAME "VSB_APP"
#define VSC_APP_NAME "VSC_APP"


/* These are the app-specific "Message IDs" traditionally defined in
 * fsw/inc/???_msgids.h.  By cFS App / cFE Service tradition, I'm
 * defining them as unsigned integer literals that provide values for
 * the first two bytes of the CCSDS primary header that begins every
 * command and telemetry message.  These two bytes encode four logical
 * fields.  The two that are most relevant to us are a one-bit flag
 * field that distinguishes command messages from telemetry messages
 * and an eleven-bit field that encodes an "application ID".  cFS Apps
 * and cFE Services appear to use this application ID field to contain
 * "Topic IDs" and typically define multiple MIDs, each with a
 * distinct Topic ID for a particular purpose, such ground commands,
 * the "send housekeeping" commands sent by SCH/SCH_LAB, or the
 * housekeeping telemetry they emit in response.
 *
 * These Topic IDs must be distinct from those already in use by other
 * cFS Apps and cFE Services.  Although the CCSDS header suggests that
 * the valid range for Topic IDs is 0x000 to 0x07FF, all the ones I've
 * seen in use are between 0x00 and 0xFF.
 */

#define VSA_CMD_MID     (0x1800|0x0090)    /* commands from ground */
#define VSA_SEND_HK_MID (0x1800|0x0091)    /* send housekeeping command */
#define VSA_TLM_HK_MID  (0x0800|0x0091)    /* housekeeping telemetry */

#define VSB_CMD_MID     (0x1800|0x00A0)    /* commands from ground */
#define VSB_SEND_HK_MID (0x1800|0x00A1)    /* send housekeeping command */
#define VSB_TLM_HK_MID  (0x0800|0x00A1)    /* housekeeping telemetry */

#define VSC_CMD_MID     (0x1800|0x00B0)    /* commands from ground */
#define VSC_SEND_HK_MID (0x1800|0x00B1)    /* send housekeeping command */
#define VSC_TLM_HK_MID  (0x0800|0x00B1)    /* housekeeping telemetry */


/* These are the app-specific "performance IDs" we pass to
 * CFE_ES_PerflogEntry/Exit(), traditionally defined in each app's
 * fsw/inc/???_perfids.h header.  They must be less than
 * CFE_MISSION_ES_PERF_MAX_IDS, avoid the values 0-31 that are
 * reserved for CFE Services, and avoid all the values chosen by other
 * apps.  Configuring ES to capture the performance data of just one
 * of the VS? apps will be easier if all the VS? performance IDs are
 * between 32 and 63 inclusive.
 *
 * The VS?_ALL_PERF_IDs below cover all app processing.  The
 * VS?_VF_PERF_ID cover only table validation function processing.
 */
#define VSA_ALL_PERF_ID 40
#define VSA_VF_PERF_ID  41
#define VSB_ALL_PERF_ID 50
#define VSB_VF_PERF_ID  51
#define VSC_ALL_PERF_ID 60
#define VSC_VF_PERF_ID  61

#endif
