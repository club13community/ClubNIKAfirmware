#include "GSMService.h"
#include "sim900.h"
#include "sim900_callbacks.h"
#include "rtc.h"
#include "./state.h"
#include "./service.h"
#include "./callback_handling.h"
#include "./call.h"
#include "./sms.h"

void gsm::init_periph() {
	sim900::init_periph();
}

void gsm::start() {
	init_state();
	init_callback_handling();
	init_service_tasks();

	sim900::start();
}

uint8_t gsm::get_signal_strength() {
	using namespace sim900;
	return card_status == CardStatus::READY && registration == Registration::DONE ? signal_strength : 0;
}

void gsm::set_on_incoming_call(void (* callback)(char *)) {
	on_incoming_call = callback;
}

void gsm::set_on_call_dialed(void (* callback)(Direction direction)) {
	on_call_dialed = callback;
}

void gsm::set_on_key_pressed(void (* callback)(char key)) {
	on_key_pressed = callback;
}

void gsm::set_on_call_ended(void (* callback)()) {
	on_call_ended = callback;
}

gsm::Controls & gsm::get_ctrl() {
	while (xSemaphoreTake(ctrl_mutex, portMAX_DELAY) == pdFALSE);
	return Controls::inst;
}

void sim900::on_timestamp(rtc::DateTime & timestamp, uint8_t dst_shift) {
	rtc::set(timestamp, dst_shift);
}

void sim900::on_dst_update(uint8_t dst_shift) {
	rtc::change_dst(dst_shift);
}

gsm::Controls gsm::Controls::inst;

gsm::Dialing gsm::Controls::call(const char * phone) {
	return gsm::call(phone);
}

void gsm::Controls::end_call() {
	gsm::end_call();
}

bool gsm::Controls::accept_call() {
	return gsm::accept_call();
}

bool gsm::Controls::send_sms(const char * text, const char * phone) {
	return gsm::send_sms(text, phone);
}