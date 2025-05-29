#ifndef _GRUNT_OUTPUT_H_
#define _GRUNT_OUTPUT_H_

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

void grunt_output_init(const char **, grunt_string_t);
int  grunt_output_enqueue_boolean(grunt_boolean_t);
int  grunt_output_enqueue_number(grunt_number_t);
int  grunt_output_enqueue_string(grunt_string_t);
void grunt_output_flush(grunt_number_t, grunt_number_t);

#endif
