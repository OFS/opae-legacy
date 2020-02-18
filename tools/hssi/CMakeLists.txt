## Copyright(c) 2017-2020, Intel Corporation
##
## Redistribution  and  use  in source  and  binary  forms,  with  or  without
## modification, are permitted provided that the following conditions are met:
##
## * Redistributions of  source code  must retain the  above copyright notice,
##   this list of conditions and the following disclaimer.
## * Redistributions in binary form must reproduce the above copyright notice,
##   this list of conditions and the following disclaimer in the documentation
##   and/or other materials provided with the distribution.
## * Neither the name  of Intel Corporation  nor the names of its contributors
##   may be used to  endorse or promote  products derived  from this  software
##   without specific prior written permission.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
## IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
## LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
## CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
## SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
## INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
## CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.

opae_add_shared_library(TARGET hssi-io
    SOURCE
        przone.h
        accelerator_przone.h
        accelerator_przone.cpp
        hssi_przone.h
        hssi_przone.cpp
        i2c.h
        i2c.cpp
        fme.h
        fme.cpp
        eth_ctrl.h
        eth_ctrl.cpp
        mdio.h
        mdio.cpp
        nios.h
        nios.cpp
        xcvr.h
        xcvr.cpp
        pll.h
        pll.cpp
    LIBS
        opae-c
        opae-cxx-core
        opae-c++-utils
    VERSION ${OPAE_VERSION}
    SOVERSION ${OPAE_VERSION_MAJOR}
    COMPONENT hssiiolib
)

target_include_directories(hssi-io
    PRIVATE
        ${OPAE_SDK_SOURCE}/tools/extra/c++utils
)

set_target_properties(hssi-io
    PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)

opae_add_executable(TARGET hssi_loopback
    SOURCE
        loopback_main.cpp
        loopback_app.h
        loopback_app.cpp
        loopback.h
        loopback.cpp
        e10.h
        e10.cpp
        e40.h
        e40.cpp
        e100.h
        e100.cpp
    LIBS
        hssi-io
    COMPONENT hssiprograms
)

target_include_directories(hssi_loopback
    PRIVATE
        ${OPAE_SDK_SOURCE}/tools/extra/c++utils
)

set_target_properties(hssi_loopback
    PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)

opae_add_executable(TARGET hssi_config
    SOURCE
        config_main.cpp
        config_app.h
        config_app.cpp
    LIBS
        hssi-io
    COMPONENT hssiprograms
)

target_include_directories(hssi_config
    PRIVATE
        ${OPAE_SDK_SOURCE}/tools/extra/c++utils
)

set_target_properties(hssi_config
    PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)
