#ifndef _VS_EVENTIDS_H_
#define _VS_EVENTIDS_H_

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

/* These are the Event IDs common to all V-SPELLS (VS) apps.  Each cFS
 * App and cFE Service has its own Event ID namespace.  It's OK if
 * this app uses the same EID numbers as another for its own purposes.
 * There's no need to define these EIDs to avoid collisions with those
 * defined by another App or cFE Service.
 *
 * Numbering convention:
 *   0x0XYZ is for information events,
 *   0x1XYZ is for general application error events, and
 *   0x2XYZ is for table validation error events.
 */

/* Information event IDs */
#define VS_CMD_NOOP_INF_EID     0x0001 /* received no-op command */
#define VS_CMD_RESET_INF_EID    0x0002 /* received counter reset command */
#define VS_STARTUP_OK_INF_EID   0x0004 /* app started, intialized OK */
#define VS_VALIDATION_INF_EID   0x0008 /* table validation statistics */

/* Application error IDs not related to table validation */
#define VS_MSG_BAD_CC_ERR_EID   0x1001 /* received message with invalid CC */
#define VS_MSG_BAD_MID_ERR_EID  0x1002 /* received message with invalid MID */
#define VS_PIPE_ERR_EID         0x1004 /* command pipe read error */

/* Error IDs related to table validation. */
#define VS_TBL_ZERO_ERR_EID     0x2001 /* unused entry not zeroed */
#define VS_TBL_PARM_ERR_EID     0x2002 /* entry has invalid parm id value */
#define VS_TBL_PAD_ERR_EID      0x2004 /* entry has nonzero padding value */
#define VS_TBL_LBND_ERR_EID     0x2008 /* low bound out of range */
#define VS_TBL_HBND_ERR_EID     0x2010 /* high bound out of range */
#define VS_TBL_ORDER_ERR_EID    0x2020 /* high bound is smaller than low */
#define VS_TBL_EXTRA_ERR_EID    0x2040 /* in-use entry follows unused entry */
#define VS_TBL_REDEF_ERR_EID    0x2080 /* parm ID used by earlier entry */


#endif
