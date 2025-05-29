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

cmake_minimum_required(VERSION 3.5)
project(CFS_VSB C)

include_directories(fsw/inc fsw/src ../vs/fsw/inc)

add_cfe_app(vsb fsw/src/vsb_app.c fsw/src/vsb_table.c)

add_cfe_tables(VSB_Prm_default fsw/tables/VSB_Prm_default.c)

