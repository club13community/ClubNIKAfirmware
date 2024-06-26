//
// Created by independent-variable on 5/19/2024.
//

#pragma once
#include "./execution.h"

namespace sim900 {
	bool pressed_key_listener(rx_buffer_t & rx);
	bool timestamp_listener(rx_buffer_t & rx);
	bool dst_listener(rx_buffer_t & rx);
	/** @returns true if received message is expected but is not used. */
	bool ignoring_listener(rx_buffer_t & rx);
}