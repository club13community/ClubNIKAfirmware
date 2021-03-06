/*
 * SupplySystem.h
 *
 *  Created on: 30 ���. 2020 �.
 *      Author: MaxCm
 */

#ifndef _SUPPLYSYSTEM_H_
#define _SUPPLYSYSTEM_H_

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"

extern const char SupplySystem_mainTaskName[];

TaskHandle_t SupplySystem_Launch(MessageBufferHandle_t msgIn, MessageBufferHandle_t msgOut);
void SupplySystem_12VChannelsOvercurrent_IH();
void SupplySystem_12VChannelsOvercurrent_Timer_IH();

#endif /* INC_SUPPLYSYSTEM_H_ */
