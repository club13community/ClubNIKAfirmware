//
// Created by independent-variable on 5/26/2024.
//

#pragma once
namespace gsm {
	bool send_sms(const char * text, const char * phone);
	/** Deletes all received SMS */
	void delete_incoming_sms();
}