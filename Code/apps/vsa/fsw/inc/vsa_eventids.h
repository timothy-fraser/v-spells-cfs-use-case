#ifndef _VSA_EVENTIDS_H_
#define _VSA_EVENTIDS_H_

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

/* These are the Event IDs for this app defined in terms of the common
 * V-SPELLS (VS) event IDs in vs_eventids.h.
 */

/* Information event IDs */
#define VSA_CMD_NOOP_INF_EID     VS_CMD_NOOP_INF_EID
#define VSA_CMD_RESET_INF_EID    VS_CMD_RESET_INF_EID
#define VSA_STARTUP_OK_INF_EID   VS_STARTUP_OK_INF_EID
#define VSA_VALIDATION_INF_EID   VS_VALIDATION_INF_EID

/* Application error IDs not related to table validation */
#define VSA_MSG_BAD_CC_ERR_EID   VS_MSG_BAD_CC_ERR_EID
#define VSA_MSG_BAD_MID_ERR_EID  VS_MSG_BAD_MID_ERR_EID
#define VSA_PIPE_ERR_EID         VS_PIPE_ERR_EID

/* Error IDs related to table validation. */
#define VSA_TBL_ZERO_ERR_EID     VS_TBL_ZERO_ERR_EID
#define VSA_TBL_PARM_ERR_EID     VS_TBL_PARM_ERR_EID
#define VSA_TBL_PAD_ERR_EID      VS_TBL_PAD_ERR_EID
#define VSA_TBL_LBND_ERR_EID     VS_TBL_LBND_ERR_EID
#define VSA_TBL_HBND_ERR_EID     VS_TBL_HBND_ERR_EID
#define VSA_TBL_ORDER_ERR_EID    VS_TBL_ORDER_ERR_EID
#define VSA_TBL_EXTRA_ERR_EID    VS_TBL_EXTRA_ERR_EID
#define VSA_TBL_REDEF_ERR_EID    VS_TBL_REDEF_ERR_EID


#endif
