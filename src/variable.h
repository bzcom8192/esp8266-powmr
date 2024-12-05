#ifndef __VARIABLE_H
#define __VARIABLE_H

#include "Arduino.h"

typedef union
{
    uint16_t    u16[40];
} MB_U16;

extern MB_U16 modbus_read;
/*																			Addr	Scale		Descriptions										*/
#define     u16_Output				    modbus_read.u16[0]                 // 4501		    :	Output Source Priority (Returns index with offset. I'd prefer to use register 4537) settings
#define     u16_AC_Volt			    	modbus_read.u16[1]                 // 4502		    :	AC Voltage measurement
#define     u16_AC_Freq			    	modbus_read.u16[2]                 // 4503		    :	AC Frequency measurement
#define     u16_PV_Volt			    	modbus_read.u16[3]                 // 4504	(0.1)   :	PV Voltage (?) measurement
#define     u16_Charging			    modbus_read.u16[4]                 // 4505		    :	Charging (right now) settings
#define     u16_Batt_Volt			    modbus_read.u16[5]                 // 4506	(0.1)	:	Battery Voltage measurement
#define     u16_Batt_SoC			    modbus_read.u16[6]                 // 4507		    :	Battery SoC measurement
#define     u16_Charge_Cur			    modbus_read.u16[7]                 // 4508		    :	Battery Charge Current measurement
#define     u16_Discharge_Cur		    modbus_read.u16[8]                 // 4509		    :	Battery Discharge Current measurement
#define     u16_Load_Volt			    modbus_read.u16[9]                 // 4510	(0.1)	:	Load Voltage measurement
#define     u16_Load_Freq			    modbus_read.u16[10]                // 4511	(0.1)	:	Load Frequency measurement
#define     u16_Load_Power			    modbus_read.u16[11]                // 4512		    :	Load Power measurement
#define     u16_Load_VA			    	modbus_read.u16[12]                // 4513		    :	Load VA measurement
#define     u16_Load_Per1			    modbus_read.u16[13]                // 4514		    :	Load Percent measurement
#define     u16_Load_Per2			    modbus_read.u16[14]                // 4515		    :	Load Percent measurement
#define     u16_Bin_Flag			    modbus_read.u16[15]                // 4516		    :	Binary flags binary_flags
#define     u16_Err_Code			    modbus_read.u16[16]                // 4530		    :	Error Code
#define     u16_Set_Flag			    modbus_read.u16[17]                // 4535		    :	Settings binary flags binary_flags
#define     u16_Charge_Pri			    modbus_read.u16[18]                // 4536		    :	Charger Source Priority settings
#define     u16_Output_Pri			    modbus_read.u16[19]                // 4537		    :	Output Source Priority (More correct one) settings
#define     u16_AC_Inrange			    modbus_read.u16[20]                // 4538		    :	AC Input Voltage Range settings
#define     u16_Out_Freq			   	modbus_read.u16[21]                // 4540		    :	Target Output Frequency settings
#define     u16_MaxTotal_Cur		   	modbus_read.u16[22]                // 4541		    :	Max Total Charging Current settings
#define     u16_Out_Volt			   	modbus_read.u16[23]                // 4542		    :	Target Output Voltage settings
#define     u16_MaxUtil_Cur		   		modbus_read.u16[24]                // 4543		    :	Max Utility Charging Current settings
#define     u16_UtilSrc_Volt		   	modbus_read.u16[25]                // 4544		    :	Back To Utility Source Voltage settings
#define     u16_BattSrc_Volt		   	modbus_read.u16[26]                // 4545		    :	Back To Battery Source Voltage settings
#define     u16_Bulk_Volt			    modbus_read.u16[27]                // 4546		    :	Bulk Charging Voltage settings
#define     u16_Float_Volt			    modbus_read.u16[28]                // 4547		    :	Floating Charging Voltage settings
#define     u16_LowCut_Volt		    	modbus_read.u16[29]                // 4548		    :	Low CutOff Voltage settings
#define     u16_BattEq_Volt		    	modbus_read.u16[30]                // 4549		    :	Battery Equalization Voltage settings
#define     u16_BattEq_Time		    	modbus_read.u16[31]                // 4550		    :	Battery Equalized Time settings
#define     u16_BattEq_Timeout 	    	modbus_read.u16[32]                // 4551		    :	Battery Equalized Timeout settings
#define     u16_BattEq_Interval	    	modbus_read.u16[33]                // 4552		    :	Equalization Interval settings
#define     u16_Bin_flags0			    modbus_read.u16[34]                // 4553		    :	Binary flags binary_flags
#define     u16_Bin_flags1			    modbus_read.u16[35]                // 4554		    :	Binary flags binary_flags
#define     u16_Charge_Status			modbus_read.u16[36]                // 4555		    :	Charger Status (0 - Off, 1 - Idle, 2 - Active)
#define     u16_Temp					modbus_read.u16[37]                // 4557		    :	Temperature sensor

#define     EEPROM_SIZE            		1024
#define     ADDR_InWh              		0
#define     ADDR_OutWh             		8

/*
	Reference: https://github.com/odya/esphome-powmr-hybrid-inverter/blob/main/docs/registers-map.md

    4501 : Output Source Priority (Returns index with offset. I'd prefer to use register 4537) settings
    4502 : AC Voltage measurement
    4503 : AC Frequency measurement
    4504 : PV Voltage (?) measurement
    4505 : Charging (right now) settings
    4506 : Battery Voltage measurement
    4507 : Battery SoC measurement
    4508 : Battery Charge Current measurement
    4509 : Battery Discharge Current measurement
    4510 : Load Voltage measurement
    4511 : Load Frequency measurement
    4512 : Load Power measurement
    4513 : Load VA measurement
    4514 : Load Percent measurement
    4515 : Load Percent measurement
    4516 : Binary flags binary_flags
        0x100 Something overload related (?)
    4530 : Error Code
    4535 : Settings binary flags binary_flags
        0x1 Record Fault Code settings
        0x2 Battery Equalization settings
        0x4 Equalization Activated Immediately settings
        0x100 Alarm settings
        0x400 Backlight settings
        0x800 Restart On Overload settings
        0x1000 Restart On Overheat settings
        0x2000 Beep On Primary Source Fail settings
        0x4000 Return To Default Screen settings
        0x8000 Overload Bypass settings
    4536 : Charger Source Priority settings
    4537 : Output Source Priority (More correct one) settings
    4538 : AC Input Voltage Range settings
    4540 : Target Output Frequency settings
    4541 : Max Total Charging Current settings
    4542 : Target Output Voltage settings
    4543 : Max Utility Charging Current settings
    4544 : Back To Utility Source Voltage settings
    4545 : Back To Battery Source Voltage settings
    4546 : Bulk Charging Voltage settings
    4547 : Floating Charging Voltage settings
    4548 : Low CutOff Voltage settings
    4549 : Battery Equalization Voltage settings
    4550 : Battery Equalized Time settings
    4551 : Battery Equalized Timeout settings
    4552 : Equalization Interval settings
    4553 : Binary flags binary_flags
        0x100 On Battery
        0x200 AC Active
        0x1000 Load Off (Inverted "Load Enabled")
        0x2000 AC Active
        0x4000 Load Enabled
    4554 : Binary flags binary_flags
        0x1 On Battery
        0x100 AC Active
        0x8000 AC Active
    4555 : Charger Status (0 - Off, 1 - Idle, 2 - Active)
    4557 : Temperature sensor (HVM3.6M confirmed, HVM2.4H not confirmed)
*/

#endif // __VARIABLE_H