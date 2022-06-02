/*
 * RCU0.1v.h
 *
 *  Created on: Mar 18, 2020
 *      Author: FMC
 */


#ifndef RCU0_1V_H_
#define RCU0_1V_H_

#include "stdint.h"

#define BMP_COMM_TIMEOUT 	(1U)

#define BMP_I2C_ADDR		(0b1110111)
#define BMP_I2C_ADDR_W		(0b11101110)
#define BMP_I2C_ADDR_R		(0b11101110)

#define BMP_MEM_CHIP_ID		(0xD0)
#define BMP_DATA_CHIP_ID	(0x55)

#define BMP_CALIB_BASE		(0xAA)
#define BMP_CALIB_BYTE		(22U)

#define BMP_MEM_CTRL		(0xF4)

#define BMP_DATA_REQ_TEMP	(0x2E)
#define BMP_DATA_REQ_PRES8	(0xF4)
#define BMP_OSS_ULTRA_HIGH_RESOLUTION (3U)

#define BMP_MEM_GET_ADC		(0xF6)
#define BMP_TEMP_LENGTH		(2U)
#define BMP_PRES_LENGTH		(3U)

#define BMP_TEMP_CONV_TIME	(4500U)//us
#define BMP_PRES8_CONV_TIME	(25500U)//us


#define BMP_MEM_SOFT_RESET	(0xD0)
#define BMP_DATA_SOFT_RESET	(0xE0)

#define BMP_TEMP_MIN 		(-400)//0.1cel unit
#define BMP_TEMP_MAX 		(850U)//0.1cel unit
#define BMP_ALT_MIN 		(-500)
#define BMP_ALT_MAX			(9000U)

//sysdef
#define BMP_HEAP_SIZE		(11U)//word

typedef enum
{
  RCU_OK       = 0x00,
  RCU_ERROR    = 0x01,
  RCU_BUSY     = 0x02,
  RCU_TIMEOUT  = 0x03
} RCU_StatusTypeDef;

typedef struct body_handle_t{
	union{
		struct{
			uint8_t RCU:1;
			uint8_t CCU:1;
			uint8_t DLU:1;
			uint8_t FCU:1;
		};
		uint8_t data;
	}available;

	 //seo-sun;

	union{
		struct{
			union{
				struct{
					uint8_t sensor:1;
					uint8_t report:1;
					uint8_t cmd:1;
				};
				uint8_t data;
			}comm;
			union{
				struct{
					uint8_t sensor:1;
					uint8_t report:1;
					uint8_t cmd:1;
					uint8_t reovery:1;
					uint8_t emr:1;
				};
				uint8_t data;
			}process;
			union{
				struct{
					uint8_t recovery:1;
					uint8_t broadcast:1;
					uint8_t downlink:1;
				};
				uint8_t data;
			}action;
			union{
				struct{
					uint8_t rmrbf:1;
					uint8_t ignit:1;
					uint8_t launch:1;
					uint8_t enrecovery:1;
				};
				uint8_t data;
			}event;
		};
		uint32_t data;
	}flag;
}TYPE_BODY_HANDLER;
TYPE_BODY_HANDLER hbody;

typedef union bmp_handle_t{
	struct{
		struct{
			union{
				struct{
					uint8_t connection:1;
					uint8_t dataready:1;
					uint8_t update:1;
					uint8_t fail:2;

				};
				uint8_t state;
			};
			union{
				struct{
					uint8_t busy:1;
					uint8_t step:3;
					uint8_t oss:3;
				};
				uint8_t control;
			};
		}inst;
		union{
			struct{
				int16_t AC1;
				int16_t AC2;
				int16_t AC3;
				uint16_t AC4;
				uint16_t AC5;
				uint16_t AC6;
				int16_t B1;
				int16_t B2;
				int16_t MB;
				int16_t MC;
				int16_t MD;
			};
			uint8_t data[22];
			uint16_t data16[11];
		}calib;
		union{
			uint8_t ut[4];
			int32_t UT;
		};
		union{
			uint8_t up[4];
			int32_t UP;
		};
		int32_t temp;
		int32_t pressure[2];
		uint16_t dt;
		uint16_t elapsedtime;
	};
	uint32_t data[BMP_HEAP_SIZE];
}TYPE_BMP_HANDLER;
TYPE_BMP_HANDLER hbmp;

void Comm();
void Process();
void RCU_bmp_init(TYPE_BMP_HANDLER* bmp);
void RCU_bmp_comm(TYPE_BMP_HANDLER* bmp);
void RCU_bmp_process(TYPE_BMP_HANDLER* bmp);

void bmp_delay(TYPE_BMP_HANDLER* bmp, int count);
void bmp_call(TYPE_BMP_HANDLER* bmp);
void prh_com(void);
void cmd_com(void);
#endif /* RCU0_1V_H_ */
