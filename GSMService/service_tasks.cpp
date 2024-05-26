//
// Created by independent-variable on 5/21/2024.
//
#include "./state.h"
#include "./service_tasks.h"
#include "./call_tasks.h"
#include "./tasks.h"

#include "FreeRTOS.h"
#include "timers.h"
#include "./config.h"
#include <string.h>
#include "logging.h"

/** Used to poll card status, network registration, signal strength, etc.*/
static volatile TimerHandle_t module_state_timer;
static void module_status_timeout(TimerHandle_t timer);

/** If card fails, network connection lost(and SIM900 is not trying to establish it), etc. - starts
 * a delay before rebooting(if issue stays). */
static volatile TimerHandle_t connection_recovery_timer;
static void connection_recovery_timeout(TimerHandle_t timer);

static inline void schedule_module_state_update() {
	xTimerReset(module_state_timer, portMAX_DELAY);
}

static inline void stop_module_state_update() {
	xTimerStop(module_state_timer, portMAX_DELAY);
	gsm::clear_pending(gsm::Task::CHECK_MODULE_STATE);
}

static inline void schedule_connection_recovery() {
	xTimerReset(connection_recovery_timer, portMAX_DELAY);
}

static inline void stop_connection_recovery() {
	xTimerStop(connection_recovery_timer, portMAX_DELAY);
}

void gsm::init_service_tasks() {
	static StaticTimer_t module_status_timer_ctrl;
	module_state_timer = xTimerCreateStatic("gsm status", pdMS_TO_TICKS(MODULE_STATUS_UPDATE_PERIOD_ms),
											pdFALSE, (void *)0, module_status_timeout, &module_status_timer_ctrl);

	static StaticTimer_t timer_ctrl;
	connection_recovery_timer = xTimerCreateStatic("gsm conn", pdMS_TO_TICKS(CONNECTION_RECOVERY_TIME_ms),
												   pdFALSE, (void *) 0, connection_recovery_timeout, &timer_ctrl);
}

void gsm::turn_on() {
	using namespace sim900;

	static constexpr auto turned_on = [](bool ok) {
		if (ok) {
			powered = true;
			schedule_module_state_update();
		} else {
			rec::log("Failed to turn SIM900 on");
			schedule_reboot();
		}

		executed(Task::TURN_ON);
	};

	sim900::turn_on(turned_on);
}

void gsm::turn_off() {
	using namespace sim900;

	static constexpr auto turned_off = []() {
		powered = false;
		stop_module_state_update();
		stop_connection_recovery();
		reset_connection_info();
		terminate_calls();

		executed(Task::TURN_OFF);
	};

	sim900::turn_off(turned_off);
}

void gsm::check_module_state() {
	using namespace sim900;

	static constexpr auto signal_checked = [](uint8_t signal_pct, Result res) {
		if (res == Result::OK) {
			signal_strength = signal_pct;
		}

		if (res == Result::OK || res == Result::ERROR) {
			schedule_module_state_update();
		} else {
			schedule_reboot();
		}

		executed(Task::CHECK_MODULE_STATE);
	};

	static constexpr auto reg_checked = [](Registration new_reg, Result res) {
		bool check_next;

		if (res == Result::OK) {
			Registration prev_reg = registration;
			registration = new_reg;
			bool was_ok = prev_reg != Registration::FAILED;
			bool ok_now = new_reg != Registration::FAILED;
			if (was_ok && !ok_now) {
				schedule_connection_recovery();
			}
			check_next = new_reg == Registration::DONE;
		} else {
			check_next = false;
			if (res != Result::ERROR) {
				schedule_reboot();
			}
		}

		if (check_next) {
			sim900::get_signal_strength(signal_checked);
		} else {
			schedule_module_state_update(); // schedules update even if going to reboot - ok, it's simpler
			executed(Task::CHECK_MODULE_STATE);
		}
	};

	static constexpr auto sim_checked = [](CardStatus new_status, Result res) {
		bool check_next;

		if (res == Result::OK) {
			CardStatus prev_status = card_status;
			card_status = new_status;
			bool was_ok = prev_status == CardStatus::READY;
			bool is_ok = new_status == CardStatus::READY;
			if (was_ok && !is_ok) {
				// card failure
				schedule_connection_recovery();
			}
			check_next = is_ok;
		} else {
			check_next = false;
			if (res != Result::ERROR) {
				schedule_reboot();
			}
		}

		if (check_next) {
			sim900::get_registration(reg_checked);
		} else {
			schedule_module_state_update(); // schedules update even if going to reboot - ok, it's simpler
			executed(Task::CHECK_MODULE_STATE);
		}
	};

	sim900::get_card_status(sim_checked);
}

static void module_status_timeout(TimerHandle_t timer) {
	gsm::check_module_state_asap();
}

static void connection_recovery_timeout(TimerHandle_t timer) {
	using namespace sim900;
	if (gsm::card_status != CardStatus::READY || gsm::registration == Registration::FAILED) {
		gsm::reboot_asap();
	}
}