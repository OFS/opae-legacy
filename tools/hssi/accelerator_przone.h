// Copyright(c) 2017, Intel Corporation
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of  source code  must retain the  above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name  of Intel Corporation  nor the names of its contributors
//   may be used to  endorse or promote  products derived  from this  software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#pragma once
#include "przone.h"
#include "mmio.h"

namespace intel
{
namespace fpga
{
namespace hssi
{

class accelerator_przone : public przone_interface
{
public:
    typedef std::shared_ptr<accelerator_przone> ptr_t;

    enum class przone_cmd : uint32_t
    {
        write = 1U << 16,
        read  = 1U << 17
    };

    enum class mmio_reg
    {
        afu_ctrl    = 0x0020,
        afu_wr_data = 0x0028,
        afu_rd_data = 0x0030
    };

    accelerator_przone(intel::fpga::mmio::ptr_t acceleratorptr);
    virtual ~accelerator_przone(){}

    virtual bool read(uint32_t address, uint32_t & value);
    virtual bool write(uint32_t address, uint32_t value);

private:
    intel::fpga::mmio::ptr_t mmio_;

};

} // end of namespace hssi
} // end of namespace fpga
} // end of namespace intel


