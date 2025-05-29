#ifndef _GRUNT_EVENTIDS_H_
#define _GRUNT_EVENTIDS_H_

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

/* These are the Event IDs for the Grunt library.  When an app calls a
 * function provided by this library and that function calls
 * CFE_EVS_SendEvent(), EVS will present the ground station with an
 * event that says it came from the app, not this library.  I'm
 * synchronizing the eventids with the VS* app eventids since those
 * are the apps that will use this library.
 */

/* Information event IDs */
#define GRUNT_STARTUP_OK_INF_EID   VS_STARTUP_OK_INF_EID


#endif
