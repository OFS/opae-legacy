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
#include "fpga_errors.h"
#include <sstream>
#include <fstream>
#include <iomanip>

namespace intel
{
namespace fpga
{

const char* SYSFS_DEV                    = "/sys/class/fpga/intel-fpga-dev.";
const char* SYSFS_PORT                   = "/intel-fpga-port.";
const char* ERRORS_ERRORS                = "/errors/errors";
const char* ERRORS_CLEAR                 = "/errors/clear";

struct port_error_bits{
    uint64_t TxCh0Overflow : 1;
    uint64_t TxCh0InvalidReqEncoding : 1;
    uint64_t TxCh0Len3NotSupported : 1;
    uint64_t TxCh0Len2NotAligned : 1;
    uint64_t TxCh0Len4NotAligned : 1;
    uint64_t ReservedPortErr5 : 4;
    uint64_t MMIORdWhileRst : 1;
    uint64_t MMIOWrWhileRst : 1;
    uint64_t ReservedPortErr4 : 5;
    uint64_t TxCh1Overflow : 1;
    uint64_t TxCh1InvalidReqEncoding : 1;
    uint64_t TxCh1Len3NotSupported : 1;
    uint64_t TxCh1Len2NotAligned : 1;
    uint64_t TxCh1Len4NotAligned : 1;
    uint64_t TxCh1InsufficientData : 1;
    uint64_t TxCh1DataPayloadOverrun : 1;
    uint64_t TxCh1IncorrectAddr : 1;
    uint64_t TxCh1NonZeroSOP : 1;
    uint64_t ReservedPortErr3 : 7;
    uint64_t MMIOTimedOut : 1;
    uint64_t TxCh2FifoOverflow : 1;
    uint64_t UnexpMMIOResp : 1;
    uint64_t ReservedPortErr2 : 5;
    uint64_t TxReqCounterOverflow : 1;
    uint64_t L1prSmrrError : 1;
    uint64_t L1prSmrr2Error : 1;
    uint64_t L1prMesegError : 1;
    uint64_t GenProtRangeError : 1;
    uint64_t LegRangeLowError : 1;
    uint64_t LegRangeHighError : 1;
    uint64_t VgaMemRangeError : 1;
    uint64_t PageFault : 1;
    uint64_t PMRError : 1;
    uint64_t Ap6Event : 1;
    uint64_t VfFlrAccessError : 1;
    uint64_t ReservedPortErr1 : 12;
};

union port_errors_u
{
    port_errors_u(uint64_t value) : v(value){}
    port_error_bits f;
    uint64_t        v;
};

port_error::port_error(uint64_t err)
: exception()
, err_(err)
{
}

uint64_t port_error::read(uint8_t socket_id)
{
    uint64_t errors = 0;
    std::stringstream ss;
    ss << SYSFS_DEV << +socket_id << SYSFS_PORT << +socket_id << ERRORS_ERRORS;
    std::string filename = ss.str();
    std::ifstream portfile(filename);
    portfile >> std::hex >> errors;
    return errors;
}

uint64_t port_error::clear(uint8_t socket_id, uint64_t errs)
{
    std::stringstream ss;
    ss << SYSFS_DEV << +socket_id << SYSFS_PORT << +socket_id << ERRORS_CLEAR;
    std::ofstream clearfile(ss.str());
    clearfile << "0x" << std::hex << errs << "\n";
    clearfile.close();
    return port_error::read(socket_id);
}

std::ostream & operator<<(std::ostream & str, const port_error & err)
{
    str << "Raw Value: 0x" << std::hex << err.err_ << std::dec << "\n";
    auto bits = port_errors_u(err.err_);
    if (bits.f.TxCh0Overflow) str << std::setw(24) << "TxCh0Overflow: " << bits.f.TxCh0Overflow << "\n";
    if (bits.f.TxCh0InvalidReqEncoding) str << std::setw(24) << "TxCh0InvalidReqEncoding: " << bits.f.TxCh0InvalidReqEncoding << "\n";
    if (bits.f.TxCh0Len3NotSupported) str << std::setw(24) << "TxCh0Len3NotSupported: " << bits.f.TxCh0Len3NotSupported << "\n";
    if (bits.f.TxCh0Len2NotAligned) str << std::setw(24) << "TxCh0Len2NotAligned: " << bits.f.TxCh0Len2NotAligned << "\n";
    if (bits.f.TxCh0Len4NotAligned) str << std::setw(24) << "TxCh0Len4NotAligned: " << bits.f.TxCh0Len4NotAligned << "\n";
    if (bits.f.MMIORdWhileRst) str << std::setw(24) << "MMIORdWhileRst: " << bits.f.MMIORdWhileRst << "\n";
    if (bits.f.MMIOWrWhileRst) str << std::setw(24) << "MMIOWrWhileRst: " << bits.f.MMIOWrWhileRst << "\n";
    if (bits.f.TxCh1Overflow) str << std::setw(24) << "TxCh1Overflow: " << bits.f.TxCh1Overflow << "\n";
    if (bits.f.TxCh1InvalidReqEncoding) str << std::setw(24) << "TxCh1InvalidReqEncoding: " << bits.f.TxCh1InvalidReqEncoding << "\n";
    if (bits.f.TxCh1Len3NotSupported) str << std::setw(24) << "TxCh1Len3NotSupported: " << bits.f.TxCh1Len3NotSupported << "\n";
    if (bits.f.TxCh1Len2NotAligned) str << std::setw(24) << "TxCh1Len2NotAligned: " << bits.f.TxCh1Len2NotAligned << "\n";
    if (bits.f.TxCh1Len4NotAligned) str << std::setw(24) << "TxCh1Len4NotAligned: " << bits.f.TxCh1Len4NotAligned << "\n";
    if (bits.f.TxCh1InsufficientData) str << std::setw(24) << "TxCh1InsufficientData: " << bits.f.TxCh1InsufficientData << "\n";
    if (bits.f.TxCh1DataPayloadOverrun) str << std::setw(24) << "TxCh1DataPayloadOverrun: " << bits.f.TxCh1DataPayloadOverrun << "\n";
    if (bits.f.TxCh1IncorrectAddr) str << std::setw(24) << "TxCh1IncorrectAddr: " << bits.f.TxCh1IncorrectAddr << "\n";
    if (bits.f.TxCh1NonZeroSOP) str << std::setw(24) << "TxCh1NonZeroSOP: " << bits.f.TxCh1NonZeroSOP << "\n";
    if (bits.f.MMIOTimedOut) str << std::setw(24) << "MMIOTimedOut: " << bits.f.MMIOTimedOut << "\n";
    if (bits.f.TxCh2FifoOverflow) str << std::setw(24) << "TxCh2FifoOverflow: " << bits.f.TxCh2FifoOverflow << "\n";
    if (bits.f.UnexpMMIOResp) str << std::setw(24) << "UnexpMMIOResp: " << bits.f.UnexpMMIOResp << "\n";
    if (bits.f.TxReqCounterOverflow) str << std::setw(24) << "TxReqCounterOverflow: " << bits.f.TxReqCounterOverflow << "\n";
    if (bits.f.L1prSmrrError) str << std::setw(24) << "L1prSmrrError: " << bits.f.L1prSmrrError << "\n";
    if (bits.f.L1prSmrr2Error) str << std::setw(24) << "L1prSmrr2Error: " << bits.f.L1prSmrr2Error << "\n";
    if (bits.f.L1prMesegError) str << std::setw(24) << "L1prMesegError: " << bits.f.L1prMesegError << "\n";
    if (bits.f.GenProtRangeError) str << std::setw(24) << "GenProtRangeError: " << bits.f.GenProtRangeError << "\n";
    if (bits.f.LegRangeLowError) str << std::setw(24) << "LegRangeLowError: " << bits.f.LegRangeLowError << "\n";
    if (bits.f.LegRangeHighError) str << std::setw(24) << "LegRangeHighError: " << bits.f.LegRangeHighError << "\n";
    if (bits.f.VgaMemRangeError) str << std::setw(24) << "VgaMemRangeError: " << bits.f.VgaMemRangeError << "\n";
    if (bits.f.PageFault) str << std::setw(24) << "PageFault: " << bits.f.PageFault << "\n";
    if (bits.f.PMRError) str << std::setw(24) << "PMRError: " << bits.f.PMRError << "\n";
    if (bits.f.Ap6Event) str << std::setw(24) << "Ap6Event: " << bits.f.Ap6Event << "\n";
    if (bits.f.VfFlrAccessError) str << std::setw(24) << "VfFlrAccessError: " << bits.f.VfFlrAccessError << "\n";
    return str;
}

} // end of namespace fpga
} // end of namespace intel
