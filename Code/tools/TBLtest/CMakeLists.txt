# Copyright (c) 2024 Timothy Jon Fraser Consulting LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied.  See the License for the specific language governing
# permissions and limitations under the License.

# CMake snippet for building Tbltest

cmake_minimum_required(VERSION 2.6.4)
project(CFS_TBLTEST C)

# include_directories(${MISSION_SOURCE_DIR}/apps/to_lab/fsw/platform_inc)
# include_directories(${MISSION_SOURCE_DIR}/apps/to_lab/fsw/src)
# include_directories(${osal_MISSION_DIR}/src/os/inc)
# include_directories(${cfe-core_MISSION_DIR}/src/inc)
# include_directories(${msglib_MISSION_DIR}/inc)

# for cfe_es_perfdata_typedef.h, for CFE_ES_PerfData_t.
include_directories(${MISSION_SOURCE_DIR}/cfe/modules/core_private/fsw/inc)

include_directories(${MISSION_BINARY_DIR}/inc)
include_directories(${MISSION_BINARY_DIR}/osal_public_api/inc)
include_directories(${MISSION_BINARY_DIR}/native/default_cpu1/inc)
include_directories(${msg_MISSION_DIR}/fsw/inc)
include_directories(${psp_MISSION_DIR}/fsw/inc)

include_directories(${es_MISSION_DIR}/fsw/inc)
include_directories(${es_MISSION_DIR}/fsw/src)
include_directories(${evs_MISSION_DIR}/fsw/inc)
include_directories(${tbl_MISSION_DIR}/fsw/inc)
include_directories(${time_MISSION_DIR}/fsw/inc)

include_directories(${ci_lab_MISSION_DIR}/fsw/platform_inc)
include_directories(${sample_app_MISSION_DIR}/fsw/platform_inc)

include_directories(${to_lab_MISSION_DIR}/fsw/platform_inc)
include_directories(${to_lab_MISSION_DIR}/fsw/src)

include_directories(${MISSION_SOURCE_DIR}/apps/vs/fsw/inc)
# include_directories(${vsa_MISSION_DIR}/fsw/inc)
# include_directories(${vsa_MISSION_DIR}/fsw/src)


add_executable(tbltest tbltest.c cmd.c tlm.c file.c send.c expect.c deterministic.c mqueue.c perf.c)
install (TARGETS tbltest DESTINATION host)
