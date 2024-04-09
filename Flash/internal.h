//
// Created by independent-variable on 3/27/2024.
//

#pragma once

namespace flash {
	enum State {
		IDLE,
		READ_COMMAND, READ_DATA,
		WRITE_COMMAND, WRITE_DATA,
		BUFFER_MEMORY_COMMAND, BUFFER_MEMORY_EXEC, BUFFER_MEMORY_CHECK,
		ERASE_AND_PROGRAM_COMMAND, ERASE_AND_PROGRAM_EXEC, ERASE_AND_PROGRAM_CHECK
	};

	void dma_isr();
}