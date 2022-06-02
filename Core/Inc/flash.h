/*
 * flash.h
 *
 *  Created on: Apr 3, 2020
 *      Author: FMC
 */

#ifndef SRC_FLASH_H_
#define SRC_FLASH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "main.h"
#include "RCU0.1v.h"

#ifdef __STM32L496xx_H

#define RCU_FLASH_BANK 			(1U)
#define RCU_FLASH_PAGE 			(255U)
#define RCU_FLASH_SIZE_PER_PAGE (0x800U)
#define RCU_FLASH_PACKET_LEN 	(1U)

#endif
#ifdef __STM32F030x6_H

#define RCU_FLASH_BANK 			(1U)
#define RCU_FLASH_PAGE 			(15U)
#define RCU_FLASH_SIZE_PER_PAGE (0x400U)
#define RCU_FLASH_PACKET_LEN 	(4U)


#endif

#define RCU_FLASH_NBPAGES		(1U);

#define RCU_FLASH_PACKET_BIT 	(64U)
#define RCU_FLASH_PACKET_SIZE 	(8U) //byte
#define RCU_FLASH_BASE 			((FLASH_BASE)+(RCU_FLASH_SIZE_PER_PAGE)*(RCU_FLASH_PAGE))

#define RCU_FLASH_SRC(X) (RCU_FLASH_BASE+((RCU_FLASH_PACKET_SIZE)*(X)))
#define RCU_FLASH_PACKET_MAX ((RCU_FLASH_SIZE_PER_PAGE)/(RCU_FLASH_PACKET_SIZE))

#define RCU_FLASH_PACKET_PGH_CONTROL	(0b00)
#define RCU_FLASH_PACKET_PGH_LOG		(0b01)
#define RCU_FLASH_PACKET_CONTROL cnt
#define RCU_FLASH_PACKET_LOG log

typedef struct RCU_control_data_t{
	union{
		struct{
			uint32_t slP; //setted sealevel Pressure for EMR and also memo
			uint32_t EMRdt:10;	//~1024sec;
			uint32_t EMRdP:10;	//EMR dP is 0~1024pa where 0~86.4m/s when descent only
			uint32_t reserved:1;
			uint32_t EEMR_en:1;
			uint32_t Donwlink_en:1;
			uint32_t IDLogger_en:1; //logging when EMR or EEMR
			uint32_t Recovery_en:1;
			uint32_t EMR_mode:2; //0:not use, 1:EMR_dt, 2:EMR_dP, 3:EMR_mixed
			uint32_t RCU_MODE:2; //0:ind_adv, 1:ind_adv, 2:backup, 3:manual
			uint32_t PGH:2;		//0b00
		};
		uint64_t data;
		uint8_t packet[8];
	};
}TYPE_RCU_PACKET_CONTROL;

typedef struct RCU_logdata_t{
	union{
		struct{
			int32_t lowestP:31; //for maximum height
			uint8_t dpPositive:1;	//ascent of descent of dp at max
			uint16_t maxdP:13;		//for conv mxdv ~691m/s
			uint32_t PatmxdP:17; 	//from launching to mxdt; for mxdv from mxdP calib -only positive value
			uint8_t PGH:2;			//0b01
		};
		uint64_t data;
		uint8_t packet[8];
	};
}TYPE_RCU_PACKET_LOG;


typedef struct flash_mem_idx_t {
	struct{
		uint16_t storagemax; //FLASH_RCU_PAGE_SIZE per FLASH_PACKET_SIZE
		uint16_t useage; //package count by  FLASH_RCU_PAGE_SIZE per FLASH_PACKET_SIZE (13/20)
		uint32_t pagestat;
		FLASH_EraseInitTypeDef pagedefault;
	}state;
	struct{
		TYPE_RCU_PACKET_LOG* src;
		uint16_t idx:15;
		uint16_t val:1;
	}RCU_FLASH_PACKET_LOG;
	struct{
		TYPE_RCU_PACKET_CONTROL* src;
		uint16_t idx:15;
		uint16_t val:1;
	}RCU_FLASH_PACKET_CONTROL;
}TYPE_RCU_FLASH_HANDLER;
TYPE_RCU_FLASH_HANDLER hflash;


RCU_StatusTypeDef RCU_flash_init(TYPE_RCU_FLASH_HANDLER*);
RCU_StatusTypeDef RCU_flash_reflash(TYPE_RCU_FLASH_HANDLER*);
RCU_StatusTypeDef RCU_flash_write(TYPE_RCU_FLASH_HANDLER*, uint64_t, int);

#ifdef __cplusplus
}
#endif

#endif /* SRC_FLASH_H_ */
