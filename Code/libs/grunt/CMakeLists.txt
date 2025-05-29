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

project(CFE_GRUNT C)

include_directories(fsw/inc fsw/src ../../apps/vs/fsw/inc)
add_cfe_app(grunt fsw/src/grunt.c fsw/src/grunt_input.c fsw/src/grunt_output.c fsw/src/grunt_stack.c fsw/src/grunt_vm_arithmetic.c fsw/src/grunt_vm_control.c fsw/src/grunt_vm_io.c fsw/src/grunt_vm_logic.c fsw/src/grunt_vm_stack.c)


