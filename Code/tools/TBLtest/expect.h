#ifndef _EXPECT_H_
#define _EXPECT_H_

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

int expect_tlmon_success(void);
int expect_load_success(const char *);
int expect_activate_success(const char *, const char *);
int expect_activate_failure(const char *);
int expect_validate_success(const char *, const char *, unsigned, unsigned,
	unsigned);
int expect_validate_failure(const char *, const char *, unsigned, unsigned,
	unsigned);
int expect_err(const char *, tlm_eventid_t, const char *);

#endif
