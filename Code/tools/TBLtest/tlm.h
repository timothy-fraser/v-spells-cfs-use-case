#ifndef _TLM_H_
#define _TLM_H_

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

/* Long-form telemetry ("TLM") messages have an App Name field that
 * uses a NUL-terminated C string to identify the App that asked EVS
 * to send the telemetry message.  This name field is the only way
 * telemetry messages provide to identify the App.  Apps do not define
 * their own names in headers; instead, they cfe_es_startup.scr
 * configuration file that describes the apps and services that will
 * run on a particular CPU defines them.  We can't include headers to
 * get these names, so here are defines for some common ones.
 */
#define TLM_NAME_TBL  "CFE_TBL"    /* TBL, the Table Services service */
#define TLM_NAME_TIME "CFE_TIME"   /* TIME, the Time Service */
#define TLM_NAME_TO   "TO_LAB_APP" /* TO/TO_LAB, Telemetry Output Service */

/* Typedefs for various kinds of telemetry ("TLM") message fields */
typedef unsigned short tlm_topicid_t;          /* TLM msg topic or "flavor" */
typedef unsigned short tlm_length_t;                  /* length of TLM msgs */
typedef unsigned short tlm_eventid_t;              /* app-specific event ID */
typedef enum CFE_EVS_EventType tlm_eventtype_t;  /* INFO, ERROR, DEBUG, etc */

void            tlm_init(void);
void            tlm_receive(void);
tlm_topicid_t   tlm_topicid(void);
tlm_length_t    tlm_length(void);
const char *    tlm_evs_appname(void);
tlm_eventid_t   tlm_evs_eventid(void);
tlm_eventtype_t tlm_evs_eventtype(void);
const char *    tlm_evs_message(void);

const char *    tlm_topicid_to_string(tlm_topicid_t);
const char *    tlm_eventid_to_string(const char *, tlm_eventid_t);
const char *    tlm_eventtype_to_string(tlm_eventtype_t);

#endif
