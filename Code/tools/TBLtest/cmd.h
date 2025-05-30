#ifndef _CMD_H_
#define _CMD_H_

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

void cmd_init(void);
void cmd_to_tlmon(void);
void cmd_tbl_load(const char *);
void cmd_tbl_validate(const char *, unsigned short);
void cmd_tbl_activate(const char *);
void cmd_es_setperffilter(uint32, uint32);
void cmd_es_setperftrigger(uint32, uint32);
void cmd_es_perfstart(void);
void cmd_es_perfstop(void);

#endif
