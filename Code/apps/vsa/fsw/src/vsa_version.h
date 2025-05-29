#ifndef _VSA_VERSION_H_
#define _VSA_VERSION_H_

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

#define VSA_MAJOR_VERSION  1
#define VSA_MINOR_VERSION  0
#define VSA_REVISION       0

#define STRINGME(s) #s
#define EXPAND_AND_STRINGME(s) STRINGME(s)

#define VSA_BUILD_BASELINE "draco-rc5"

#define VSA_APP_VERSION_STRING ( \
	VSA_APP_NAME \
	" v" \
	EXPAND_AND_STRINGME(VSA_MAJOR_VERSION) \
	"." \
	EXPAND_AND_STRINGME(VSA_MINOR_VERSION) \
	"." \
	EXPAND_AND_STRINGME(VSA_REVISION) \
	" for cFS " \
	VSA_BUILD_BASELINE)

#endif
