#ifndef _VSB_VERSION_H_
#define _VSB_VERSION_H_

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

/* These macros describe the version of the app using a combination of
 * vM.m.r version number plus the Git repo tag of the cFS version I
 * built against.
 */

#define VSB_MAJOR_VERSION  1
#define VSB_MINOR_VERSION  0
#define VSB_REVISION       0

#define STRINGME(s) #s
#define EXPAND_AND_STRINGME(s) STRINGME(s)

#define VSB_BUILD_BASELINE "draco-rc5"

#define VSB_APP_VERSION_STRING ( \
	VSB_APP_NAME \
	" v" \
	EXPAND_AND_STRINGME(VSB_MAJOR_VERSION) \
	"." \
	EXPAND_AND_STRINGME(VSB_MINOR_VERSION) \
	"." \
	EXPAND_AND_STRINGME(VSB_REVISION) \
	" for cFS " \
	VSB_BUILD_BASELINE)

#endif
