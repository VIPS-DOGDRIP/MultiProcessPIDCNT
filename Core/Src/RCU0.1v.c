/*
 * RCU0.1v.c
 *
 *  Created on: Mar 18, 2020
 *      Author: FMC
 */

#include "RCU0.1v.h"
#include "flash.h"
#include "tim.h"
#include "i2c.h"

#define RCU_CLOCK htim16
#define BMP_TIM htim14
#define BMP_I2C hi2c1

void RCU_bmp_init(TYPE_BMP_HANDLER* bmp){
	uint8_t buff = 0;
	for(int i = 0; i!=BMP_HEAP_SIZE; i++){
		bmp->data[i]= 0;
	}
	//for to 5times after delay...? not yet.
	if(HAL_I2C_Mem_Read(&BMP_I2C, BMP_I2C_ADDR_R, BMP_MEM_CHIP_ID, 1, &buff, 1, 10)!=HAL_OK)bmp->inst.fail = 1;
	if(bmp->inst.fail != 1){
		if(buff == BMP_DATA_CHIP_ID){
			buff = BMP_DATA_SOFT_RESET;
			HAL_I2C_Mem_Write(&BMP_I2C, BMP_I2C_ADDR_W, BMP_MEM_SOFT_RESET, 1,&buff, 1, 10);
			bmp->inst.connection = 1;
			bmp_delay(bmp, 10000);
		}else bmp->inst.fail = 1;
	}
}

void Comm(void){
	if(hbody.flag.comm.sensor)RCU_bmp_comm(&hbmp);
	if(hbody.flag.comm.report)RCU_prh_comm();
	if(hbody.flag.comm.cmd)RCU_cmd_comm();
}
void Process(void){
	if(hbody.flag.process.sensor)RCU_bmp_process(&hbmp);
}
void RCU_bmp_comm(TYPE_BMP_HANDLER* bmp){
	if(!bmp->inst.busy) {
		uint8_t sendbuff = 0;
		switch(bmp->inst.step){
		case 0:
			{
				//set bmp oss
				bmp->inst.oss = BMP_OSS_ULTRA_HIGH_RESOLUTION;
				//get bmp calib-data
				uint8_t getbuff[BMP_CALIB_BYTE] = {0};
				HAL_I2C_Mem_Read(&BMP_I2C, BMP_I2C_ADDR_R, BMP_CALIB_BASE, 1, getbuff, BMP_CALIB_BYTE, 50);
				for (uint8_t i = 0; i != BMP_CALIB_BYTE / 2; i++){
					bmp->calib.data[i*2] = getbuff[i*2 +1];
					bmp->calib.data[i*2 +1] = getbuff[i*2];
					if ((0 == bmp->calib.data16[i]) | (-1 == bmp->calib.data16[i])){
						bmp->inst.fail = 2;
					}
					else HAL_TIM_Base_Start(&RCU_CLOCK);
				}
			}
			bmp->pressure[0] = 101325;
			bmp->inst.step++;
			break;
		case 1:
			sendbuff = BMP_DATA_REQ_TEMP;
			HAL_I2C_Mem_Write(&BMP_I2C, BMP_I2C_ADDR_W, BMP_MEM_CTRL, 1, &sendbuff, 1, 5);
 			bmp->inst.step++;
 			bmp_delay(bmp, 450);
			break;
		case 2:
			bmp->UT = 0;
			HAL_I2C_Mem_Read(&BMP_I2C, BMP_I2C_ADDR_R, BMP_MEM_GET_ADC, 1, bmp->ut, BMP_TEMP_LENGTH, 5);
			sendbuff = BMP_DATA_REQ_PRES8;
			HAL_I2C_Mem_Write(&BMP_I2C, BMP_I2C_ADDR_W, BMP_MEM_CTRL, 1, &sendbuff, 1, 5);
			bmp->inst.step++;
			bmp_delay(bmp, 2250);
			break;
		case 3:
			bmp->UP = 0;
			HAL_I2C_Mem_Read(&BMP_I2C, BMP_I2C_ADDR_R, BMP_MEM_GET_ADC, 1, bmp->up, BMP_PRES_LENGTH, 5);
			bmp->dt=(RCU_CLOCK.Instance->CNT);
			RCU_CLOCK.Instance->CNT = 0;
			bmp->inst.busy = 0;
			bmp->inst.update = bmp->inst.step = 1;
			break;
		default:
			break;
		}
	}
}

void bmp_delay(TYPE_BMP_HANDLER* bmp, int count){
	bmp->inst.busy = 1;
	BMP_TIM.Instance->ARR = count - 1 ;//qhwjd
	HAL_TIM_Base_Start_IT(&BMP_TIM);
}



void RCU_bmp_process(TYPE_BMP_HANDLER* bmp){
	if(bmp->inst.update){
		int32_t X1, X2, X3, B3, B5, temp, press; uint32_t B4, B7;
		bmp->UT = (int32_t)((bmp->ut[0]<<8) + (bmp->ut[1]));
		bmp->UP = (int32_t)((bmp->up[0]<<16) + (bmp->up[1]<<8) + (bmp->up[2]))>>(8-bmp->inst.oss);
		X1 = ((bmp->UT-bmp->calib.AC6)*bmp->calib.AC5)>>15;
		X2 = (bmp->calib.MC<<11)/(X1+bmp->calib.MD);
		B5 = X1 + X2;
		temp = (B5 + 8)>>4;
		if((BMP_TEMP_MIN > temp || temp > BMP_TEMP_MAX)){
			bmp->inst.fail = 3;
			bmp->inst.dataready = 0;
		}
		else{
			B5 -= 4000;
			X1 = (bmp->calib.B2 * ((B5*B5) >> 12))>>11;
			X2 = (bmp->calib.AC2*B5)>>11;
			X3 = X1 + X2;
			B3 = (((bmp->calib.AC1*4 + X3) << bmp->inst.oss)+2)/4;
			X1 = (bmp->calib.AC3*B5)>>13;
			X2 = (bmp->calib.B1*((B5*B5)>>12))>>16;
			X3 = ((X1+X2)+2)/4;
			B4 = (bmp->calib.AC4*(uint32_t)(X3+32768))>>15;
			B7 = ((uint32_t)bmp->UP - B3)*(50000>>bmp->inst.oss);
			if (B7 < 0x80000000) press = (B7*2)/B4;
			else press = (B7/B4)*2;
			X1 = (press>>8)*(press>>8);
			X1 = (X1*3038)>>16;
			X2 = (-7357*press)>>16;
			bmp->pressure[1] = press + (X1+X2+3791)/16;
			bmp->temp = temp;
			bmp->inst.dataready = 1;
		}
		bmp->inst.update = 0;
	}
}

void RCU_prh_comm(void){

}
void RCU_cmd_comm(void){

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == BMP_TIM.Instance)
		hbmp.inst.busy = 0;
}
