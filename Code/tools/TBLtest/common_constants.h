#ifndef _COMMON_CONSTANTS_H_
#define _COMMON_CONSTANTS_H_

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

/* The length field embedded in the primary header of CCSDS messages
 * (CCSDS_PrimaryHeader_t) always reports the size of the message
 * minus this constant.
 */
#define CCSDS_MSG_LENGTH_DELTA 7

/* We'll ask ES to write its performance log data to this file. */
#define PERF_FILENAME "/cf/cfe_es_perf.dat"

#endif
