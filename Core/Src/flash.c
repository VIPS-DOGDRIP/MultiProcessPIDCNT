/*
 * flash.c
 *
 *  Created on: Apr 3, 2020
 *      Author: FMC
 */


#include "flash.h"

void flash_idx_update(TYPE_RCU_FLASH_HANDLER*);

RCU_StatusTypeDef RCU_flash_init(TYPE_RCU_FLASH_HANDLER* flash){
	flash->state.pagedefault.TypeErase		= FLASH_TYPEERASE_PAGES;
	flash->state.pagedefault.NbPages		= 1U;
	#ifdef __STM32L496xx_H
	flash->state.pagedefault.Banks			= FLASH_BANK_1;
	flash->state.pagedefault.Page			= RCU_FLASH_PAGE;
#endif
#ifdef __STM32F030x6_H
	flash->state.pagedefault.PageAddress	= RCU_FLASH_SRC(0);
#endif
	flash->state.storagemax = RCU_FLASH_PACKET_MAX;
	flash_idx_update(flash);
	if(flash->state.useage+1 >= flash->state.storagemax)
		return RCU_flash_reflash(flash);
	return RCU_OK;
}

RCU_StatusTypeDef RCU_flash_reflash(TYPE_RCU_FLASH_HANDLER* flash){
	int cntupdat = 0, logupdat = 0;
	TYPE_RCU_PACKET_CONTROL cnt;
	TYPE_RCU_PACKET_LOG log;
	HAL_StatusTypeDef stat;
	if(flash->cnt.val){
		cnt = *(flash->cnt.src);
		cntupdat = 1;
	}
	if(flash->log.val){
		log = *(flash->log.src);
		logupdat = 1;
	}
	stat = HAL_FLASHEx_Erase(&(flash->state.pagedefault),&(flash->state.pagestat));
	if(stat != HAL_OK) return stat;
	flash_idx_update(flash);
	if(cntupdat) stat = RCU_flash_write(flash, cnt.data, 0);
	if(stat != HAL_OK) return stat;
	if(logupdat) stat = RCU_flash_write(flash, log.data, 0);
	if(stat != HAL_OK) return stat;
	return RCU_OK;
}

RCU_StatusTypeDef RCU_flash_write(TYPE_RCU_FLASH_HANDLER* flash, uint64_t data, int mode){
	uint32_t src = 0;
	HAL_StatusTypeDef stat;
	switch(mode){
	case 0://cnt
		break;
		src = flash->cnt.idx = flash->state.useage;
	case 1://log
		break;
		src = flash->log.idx = flash->state.useage;
	default:
		return RCU_ERROR;
		break;
	}
	HAL_FLASH_Unlock();
	stat = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, RCU_FLASH_SRC(src), data);
	HAL_FLASH_Lock();
	if(stat != HAL_OK) return stat;
	flash_idx_update(flash);
	return RCU_OK;
}

void flash_idx_update(TYPE_RCU_FLASH_HANDLER* flash){
	TYPE_RCU_PACKET_LOG *logbuff;
	flash->cnt.idx = 0;
	flash->log.idx = 0;
	for(int i = 0; i<=RCU_FLASH_PACKET_MAX; i++){
	logbuff = (TYPE_RCU_PACKET_LOG*)RCU_FLASH_SRC(i);
	switch(logbuff->PGH){
		case (RCU_FLASH_PACKET_PGH_CONTROL):
		flash->cnt.idx = i;
		flash->state.useage = i+1;
		flash->cnt.val = 1;
			break;
		case (RCU_FLASH_PACKET_PGH_LOG):
		flash->log.idx = i;
		flash->state.useage = i+1;
		flash->log.val = 1;
			break;
		default:
			break;
		}
	}
	flash->cnt.src = (TYPE_RCU_PACKET_LOG*)RCU_FLASH_SRC(flash->cnt.idx);
	flash->log.src = (TYPE_RCU_PACKET_LOG*)RCU_FLASH_SRC(flash->log.idx);
}
