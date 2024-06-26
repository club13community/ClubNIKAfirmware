//
// Created by independent-variable on 4/21/2024.
//
#include "./sd_init.h"
#include "./periph.h"
#include "timing.h"
#include "./cmd_execution.h"
#include "sd_errors.h"
#include "./config.h"
#include "./sd_info.h"

using namespace sd;
static uint8_t cid_cds_buff[16];

#define MAX_ATTEMPTS	2
/** 1ms between attempts, ACMD41 will be executed not longer than 1 sec(req. by spec) */
#define MAX_ACMD41_ATTEMPTS	1000
static volatile uint16_t attempts;
static void (* callback)(Error);

static inline bool is_retryable(Error error) {
	return error == Error::CMD_CRC_ERROR || error == Error::RESP_CRC_ERROR || error == Error::GENERAL_ERROR
		|| error == Error::NONE;
}

static void cmd0_done(Error error);
static void cmd8_done(Error error);
static void exe_acmd41_in_1ms();
static void acmd41_done(OCR_t ocr, Error error);
static void cmd2_done(Error error);
static void cmd3_done(uint16_t rca, CSR_t csr, Error error);
static void cmd9_done(Error error);
static void cmd7_done(CSR_t csr, Error error);
static void acmd42_done(CSR_t csr, Error error);
static void acmd6_done(CSR_t csr, Error error);
static void init_done();
static void init_failed(Error error);

void sd::init_card(void (* callback)(Error)) {
	::callback = callback;
	uint32_t freq = set_slow_clk();
	uint16_t pwr_up_delay = get_power_up_time_ms(freq);
	use_1bit_dat();
	en_sdio_clk();
	constexpr timing::Callback invoke_cmd0 = []() {
		exe_cmd0(cmd0_done);
	};
	TIMER.invoke_in_ms(pwr_up_delay, invoke_cmd0);
}

/** Card reset, now it is in 'idle' state */
static void cmd0_done(Error error) {
	if (error == Error::NONE) {
		// send interface conditions
		attempts = MAX_ATTEMPTS;
		exe_cmd8(cmd8_done);
	} else {
		// should never happen
		init_failed(error);
	}
}

/** Interface conditions sent */
static void cmd8_done(Error error) {
	if (error == Error::NONE) {
		// card is HC or XC
		hcs = CapacitySupport::HC_XC;
		// send operational conditions
		attempts = MAX_ACMD41_ATTEMPTS;
		exe_acmd41(CapacitySupport::HC_XC, MIN_VDD, MAX_VDD, acmd41_done);	
	} else if (error == Error::NO_RESPONSE) {
		// card is SC
		hcs = CapacitySupport::SC;
		// send operational conditions
		attempts = MAX_ACMD41_ATTEMPTS;
		exe_acmd41(CapacitySupport::SC, MIN_VDD, MAX_VDD, acmd41_done);
	} else if (--attempts > 0 && is_retryable(error)) {
		exe_cmd8(cmd8_done);
	} else {
		init_failed(error);
	}
}

/** Send operational cond. in 1ms */
static void exe_acmd41_in_1ms() {
	constexpr timing::Callback invoke_acmd41 = []() {
		exe_acmd41(hcs, MIN_VDD, MAX_VDD, acmd41_done);
	};
	TIMER.invoke_in_ms(1, invoke_acmd41);
}

/** Operational cond. sent */
static void acmd41_done(OCR_t ocr, Error error) {
	if (error == Error::NONE && ocr.power_up_done()) {
		// request CID
		attempts = MAX_ATTEMPTS;
		exe_cmd2(cid_cds_buff, cmd2_done);
	} else if (--attempts > 0 && is_retryable(error)) {
		exe_acmd41_in_1ms();
	} else {
		init_failed(Error::NOT_SUPPORTED_CARD);
	}
}

/** CID received */
static void cmd2_done(Error error) {
	if (error == Error::NONE) {
		parse_CID(cid_cds_buff);
		// request RCA
		attempts = MAX_ATTEMPTS;
		exe_cmd3(cmd3_done);
	} else if (--attempts > 0 && is_retryable(error)) {
		exe_cmd2(cid_cds_buff, cmd2_done);
	} else {
		init_failed(error);
	}
}

/** RCA received */
static void cmd3_done(uint16_t rca, CSR_t csr, Error error) {
	if (error == Error::NONE) {
		RCA = rca;
		// request card specific data
		attempts = MAX_ATTEMPTS;
		exe_cmd9(rca, cid_cds_buff, cmd9_done);
	}else if (--attempts > 0 && is_retryable(error)) {
		exe_cmd3(cmd3_done);
	} else {
		init_failed(error);
	}
}

/** Card specific data received */
static void cmd9_done(Error error) {
	if (error == Error::NONE) {
		if (!parse_CSD(cid_cds_buff)) {
			// not supported card
			init_failed(Error::NOT_SUPPORTED_CARD);
		} else {
			// select card(to disable pull-up on DAT3 and set bus width)
			attempts = MAX_ATTEMPTS;
			exe_cmd7(RCA, cmd7_done);
		}
	} else if (--attempts > 0 && is_retryable(error)) {
		exe_cmd9(RCA, cid_cds_buff, cmd9_done);
	} else {
		init_failed(error);
	}
}

/** Card selected, now it is in 'tran' state */
static void cmd7_done(CSR_t csr, Error error) {
	if (error == Error::NONE) {
		// disable pull-up on DAT3
		attempts = MAX_ATTEMPTS;
		exe_acmd42(DAT3_PullUp::DISABLE, acmd42_done);
	} else if (--attempts > 0 && is_retryable(error)) {
		exe_cmd7(RCA, cmd7_done);
	} else {
		init_failed(error);
	}
}

/** Pull-up on DAT3 is disabled */
static void acmd42_done(CSR_t csr, Error error) {
	if (error == Error::NONE) {
		// change bus width
		attempts = MAX_ATTEMPTS;
		exe_acmd6(BusWidth::FOUR_BITS, acmd6_done);
	} else if (--attempts > 0 && is_retryable(error)) {
		exe_acmd42(DAT3_PullUp::DISABLE, acmd42_done);
	} else {
		init_failed(error);
	}
}

/** Bus width changed */
static void acmd6_done(CSR_t csr, Error error) {
	if (error == Error::NONE) {
		use_4bits_dat();
		init_done();
	} else if (--attempts > 0 && is_retryable(error)) {
		exe_acmd6(BusWidth::FOUR_BITS, acmd6_done);
	} else {
		init_failed(error);
	}
}

static void init_done() {
	uint32_t freq = set_fast_clk();
	set_read_write_timeout(freq);
	callback(Error::NONE);
}

static void init_failed(Error error) {
	dis_sdio_clk();
	callback(error);
}