// Copyright(c) 2021-2022, Intel Corporation
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

#include "fpgaperf_counter.h"

extern "C" {

#include <fcntl.h>
#include <glob.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libudev.h>
#include <linux/perf_event.h>

#define DFL_PERF_STR_MAX        256

/* Initialize the pthread mutex */
fpga_result fpga_perf_mutex_init(fpga_perf_counter *fpga_perf);

/* Destroy the pthred mutex */
fpga_result fpga_perf_mutex_destroy(fpga_perf_counter *fpga_perf);

}

#include "intel-fpga.h"
#include <linux/ioctl.h>

#include <cstdlib>
#include <string>

#include <opae/fpga.h>
#include <opae/properties.h>
#include <opae/utils.h>

#include "gtest/gtest.h"
#include "mock/test_system.h"


using namespace opae::testing;

class fpgaperf_counter_c_p : public ::testing::TestWithParam<std::string> {
protected:
        fpgaperf_counter_c_p(): tokens_{ {nullptr, nullptr} } {}

        virtual void SetUp() override 
        {
		ASSERT_TRUE(test_platform::exists(GetParam()));
		platform_ = test_platform::get(GetParam());
		system_ = test_system::instance();
		system_->initialize();
		system_->prepare_syfs(platform_);

		filter_ = nullptr;
		fpga_perf = nullptr;
		ASSERT_EQ(fpgaInitialize(NULL), FPGA_OK);
		ASSERT_EQ(fpgaGetProperties(nullptr, &filter_), FPGA_OK);
		ASSERT_EQ(fpgaPropertiesSetObjectType(filter_, FPGA_DEVICE), FPGA_OK);
		num_matches_ = 0;
		ASSERT_EQ(fpgaEnumerate(&filter_, 1, tokens_.data(), tokens_.size(),
			&num_matches_), FPGA_OK);
		EXPECT_GT(num_matches_, 0);
		dev_ = nullptr;
		fpga_perf = new fpga_perf_counter;
		ASSERT_EQ(fpgaOpen(tokens_[0], &dev_, 0), FPGA_OK);
        }

        virtual void TearDown() override {
		EXPECT_EQ(fpgaDestroyProperties(&filter_), FPGA_OK);
		if (dev_) {
			EXPECT_EQ(fpgaClose(dev_), FPGA_OK);
			dev_ = nullptr;
		}
		for (auto &t : tokens_) {
			if (t) {
				EXPECT_EQ(fpgaDestroyToken(&t), FPGA_OK);
				t = nullptr;
			}
		}
		if (fpga_perf) {
			delete fpga_perf;
			fpga_perf = nullptr;
		}
		fpgaFinalize();
		system_->finalize();
	}
		
	std::array<fpga_token, 2> tokens_;
	fpga_properties filter_;
	fpga_handle dev_;
	test_platform platform_;
	uint32_t num_matches_;
	test_system *system_;
	fpga_perf_counter *fpga_perf;
};

/**
* @test       fpgaperf_0
* @brief      Tests: fpgaPerfCounterGet
* @details    Validates the dfl-fme device path based on token
* 		and parse events and formats with valid fpga_perf
* 		and returns FPGA_OK. With invalid param it sould return
* 		FPGA_INVALID_PARAM <br>
*/
TEST_P(fpgaperf_counter_c_p, fpgaperf_0) {
	
	EXPECT_EQ(fpgaPerfCounterGet(tokens_[0], NULL), FPGA_INVALID_PARAM);
	EXPECT_EQ(fpgaPerfCounterGet(tokens_[0], fpga_perf), FPGA_OK);
	EXPECT_EQ(fpga_perf_mutex_destroy(fpga_perf), FPGA_OK);
}

/**
* @test       fpgaperf_1
* @brief      Tests: fpgaPerfCounterStartRecord
* @details    Read the fpga performance counter start value with valid fpga_perf
* 	       and return FPGA_OK. If we pass NULL it should return
* 	       FPGA_INVALID_PARAM<br>
*/
TEST_P(fpgaperf_counter_c_p, fpgaperf_1) {

	EXPECT_EQ(fpga_perf_mutex_init(fpga_perf), FPGA_OK);
	EXPECT_EQ(fpgaPerfCounterStartRecord(fpga_perf), FPGA_OK);
	EXPECT_EQ(fpgaPerfCounterStartRecord(NULL), FPGA_INVALID_PARAM);
	EXPECT_EQ(fpga_perf_mutex_destroy(fpga_perf), FPGA_OK);
}

/**
* @test       fpgaperf_2
* @brief      Tests: fpgaPerfCounterStopRecord
* @details    Read the fpga performance counter stop value with valid fpga_perf
* 	       and return FPGA_OK. If we pass NULL it should return
* 	       FPGA_INVALID_PARAM<br>
*/
TEST_P(fpgaperf_counter_c_p, fpgaperf_2) {

	EXPECT_EQ(fpga_perf_mutex_init(fpga_perf), FPGA_OK);
	EXPECT_EQ(fpgaPerfCounterStopRecord(fpga_perf), FPGA_OK);
	EXPECT_EQ(fpgaPerfCounterStopRecord(NULL), FPGA_INVALID_PARAM);
	EXPECT_EQ(fpga_perf_mutex_destroy(fpga_perf), FPGA_OK);
}

/**
* @test       fpgaperf_3
* @brief      Tests: fpgaPerfCounterPrint
* @details    Calculate the fpga performance counter delta and prints in stdout
*             with valid fpga_perf and return FPGA_OK. If we pass NULL it will
*             return FPGA_INVALID_PARAM  <br>
*/
TEST_P(fpgaperf_counter_c_p, fpgaperf_3) {

	EXPECT_EQ(fpga_perf_mutex_init(fpga_perf), FPGA_OK);
	EXPECT_EQ(fpgaPerfCounterPrint(stdout, fpga_perf), FPGA_OK);
	EXPECT_EQ(fpgaPerfCounterPrint(stdout, NULL), FPGA_INVALID_PARAM);
	EXPECT_EQ(fpga_perf_mutex_destroy(fpga_perf), FPGA_OK);
}

/**
* @test       fpgaperf_4
* @brief      Tests: fpgaPerfCounterDestroy
* @details    It release the memory for substructures allocated with valid
* 	      fpga_perf and return FPGA_OK. With invalid param it should return
* 	      FPGA_VALID_PARAM <br>
*/
TEST_P(fpgaperf_counter_c_p, fpgaperf_4) {

	EXPECT_EQ(fpga_perf_mutex_init(fpga_perf), FPGA_OK);
	EXPECT_EQ(fpgaPerfCounterDestroy(fpga_perf), FPGA_OK);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(fpgaperf_counter_c_p);
INSTANTIATE_TEST_SUITE_P(fpgaperf_counter_c, fpgaperf_counter_c_p,
	::testing::ValuesIn(test_platform::hw_platforms({ "dfl-n3000", "dfl-d5005" })));
