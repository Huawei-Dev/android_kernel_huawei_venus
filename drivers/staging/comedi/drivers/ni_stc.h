/*
    module/ni_stc.h
    Register descriptions for NI DAQ-STC chip

    COMEDI - Linux Control and Measurement Device Interface
    Copyright (C) 1998-9 David A. Schleef <ds@schleef.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

/*
	References:
	    DAQ-STC Technical Reference Manual
*/

#ifndef _COMEDI_NI_STC_H
#define _COMEDI_NI_STC_H

#include "ni_tio.h"

#define _bit15		0x8000
#define _bit14		0x4000
#define _bit13		0x2000
#define _bit12		0x1000
#define _bit11		0x0800
#define _bit10		0x0400
#define _bit9		0x0200
#define _bit8		0x0100
#define _bit7		0x0080
#define _bit6		0x0040
#define _bit5		0x0020
#define _bit4		0x0010
#define _bit3		0x0008
#define _bit2		0x0004
#define _bit1		0x0002
#define _bit0		0x0001

#define NUM_PFI_OUTPUT_SELECT_REGS 6

/*
 * Registers in the National Instruments DAQ-STC chip
 */

#define NISTC_INTA_ACK_REG		2
#define NISTC_INTA_ACK_G0_GATE		BIT(15)
#define NISTC_INTA_ACK_G0_TC		BIT(14)
#define NISTC_INTA_ACK_AI_ERR		BIT(13)
#define NISTC_INTA_ACK_AI_STOP		BIT(12)
#define NISTC_INTA_ACK_AI_START		BIT(11)
#define NISTC_INTA_ACK_AI_START2	BIT(10)
#define NISTC_INTA_ACK_AI_START1	BIT(9)
#define NISTC_INTA_ACK_AI_SC_TC		BIT(8)
#define NISTC_INTA_ACK_AI_SC_TC_ERR	BIT(7)
#define NISTC_INTA_ACK_G0_TC_ERR	BIT(6)
#define NISTC_INTA_ACK_G0_GATE_ERR	BIT(5)
#define NISTC_INTA_ACK_AI_ALL		(NISTC_INTA_ACK_AI_ERR |	\
					 NISTC_INTA_ACK_AI_STOP |	\
					 NISTC_INTA_ACK_AI_START |	\
					 NISTC_INTA_ACK_AI_START2 |	\
					 NISTC_INTA_ACK_AI_START1 |	\
					 NISTC_INTA_ACK_AI_SC_TC |	\
					 NISTC_INTA_ACK_AI_SC_TC_ERR)

#define NISTC_INTB_ACK_REG		3
#define NISTC_INTB_ACK_G1_GATE		BIT(15)
#define NISTC_INTB_ACK_G1_TC		BIT(14)
#define NISTC_INTB_ACK_AO_ERR		BIT(13)
#define NISTC_INTB_ACK_AO_STOP		BIT(12)
#define NISTC_INTB_ACK_AO_START		BIT(11)
#define NISTC_INTB_ACK_AO_UPDATE	BIT(10)
#define NISTC_INTB_ACK_AO_START1	BIT(9)
#define NISTC_INTB_ACK_AO_BC_TC		BIT(8)
#define NISTC_INTB_ACK_AO_UC_TC		BIT(7)
#define NISTC_INTB_ACK_AO_UI2_TC	BIT(6)
#define NISTC_INTB_ACK_AO_UI2_TC_ERR	BIT(5)
#define NISTC_INTB_ACK_AO_BC_TC_ERR	BIT(4)
#define NISTC_INTB_ACK_AO_BC_TC_TRIG_ERR BIT(3)
#define NISTC_INTB_ACK_G1_TC_ERR	BIT(2)
#define NISTC_INTB_ACK_G1_GATE_ERR	BIT(1)
#define NISTC_INTB_ACK_AO_ALL		(NISTC_INTB_ACK_AO_ERR |	\
					 NISTC_INTB_ACK_AO_STOP |	\
					 NISTC_INTB_ACK_AO_START |	\
					 NISTC_INTB_ACK_AO_UPDATE |	\
					 NISTC_INTB_ACK_AO_START1 |	\
					 NISTC_INTB_ACK_AO_BC_TC |	\
					 NISTC_INTB_ACK_AO_UC_TC |	\
					 NISTC_INTB_ACK_AO_BC_TC_ERR |	\
					 NISTC_INTB_ACK_AO_BC_TC_TRIG_ERR)

#define NISTC_AI_CMD2_REG		4
#define NISTC_AI_CMD2_END_ON_SC_TC	BIT(15)
#define NISTC_AI_CMD2_END_ON_EOS	BIT(14)
#define NISTC_AI_CMD2_START1_DISABLE	BIT(11)
#define NISTC_AI_CMD2_SC_SAVE_TRACE	BIT(10)
#define NISTC_AI_CMD2_SI_SW_ON_SC_TC	BIT(9)
#define NISTC_AI_CMD2_SI_SW_ON_STOP	BIT(8)
#define NISTC_AI_CMD2_SI_SW_ON_TC	BIT(7)
#define NISTC_AI_CMD2_SC_SW_ON_TC	BIT(4)
#define NISTC_AI_CMD2_STOP_PULSE	BIT(3)
#define NISTC_AI_CMD2_START_PULSE	BIT(2)
#define NISTC_AI_CMD2_START2_PULSE	BIT(1)
#define NISTC_AI_CMD2_START1_PULSE	BIT(0)

#define NISTC_AO_CMD2_REG		5
#define NISTC_AO_CMD2_END_ON_BC_TC(x)	(((x) & 0x3) << 14)
#define NISTC_AO_CMD2_START_STOP_GATE_ENA BIT(13)
#define NISTC_AO_CMD2_UC_SAVE_TRACE	BIT(12)
#define NISTC_AO_CMD2_BC_GATE_ENA	BIT(11)
#define NISTC_AO_CMD2_BC_SAVE_TRACE	BIT(10)
#define NISTC_AO_CMD2_UI_SW_ON_BC_TC	BIT(9)
#define NISTC_AO_CMD2_UI_SW_ON_STOP	BIT(8)
#define NISTC_AO_CMD2_UI_SW_ON_TC	BIT(7)
#define NISTC_AO_CMD2_UC_SW_ON_BC_TC	BIT(6)
#define NISTC_AO_CMD2_UC_SW_ON_TC	BIT(5)
#define NISTC_AO_CMD2_BC_SW_ON_TC	BIT(4)
#define NISTC_AO_CMD2_MUTE_B		BIT(3)
#define NISTC_AO_CMD2_MUTE_A		BIT(2)
#define NISTC_AO_CMD2_UPDATE2_PULSE	BIT(1)
#define NISTC_AO_CMD2_START1_PULSE	BIT(0)

#define NISTC_G0_CMD_REG		6
#define NISTC_G1_CMD_REG		7

#define NISTC_AI_CMD1_REG		8
#define NISTC_AI_CMD1_ATRIG_RESET	BIT(14)
#define NISTC_AI_CMD1_DISARM		BIT(13)
#define NISTC_AI_CMD1_SI2_ARM		BIT(12)
#define NISTC_AI_CMD1_SI2_LOAD		BIT(11)
#define NISTC_AI_CMD1_SI_ARM		BIT(10)
#define NISTC_AI_CMD1_SI_LOAD		BIT(9)
#define NISTC_AI_CMD1_DIV_ARM		BIT(8)
#define NISTC_AI_CMD1_DIV_LOAD		BIT(7)
#define NISTC_AI_CMD1_SC_ARM		BIT(6)
#define NISTC_AI_CMD1_SC_LOAD		BIT(5)
#define NISTC_AI_CMD1_SCAN_IN_PROG_PULSE BIT(4)
#define NISTC_AI_CMD1_EXTMUX_CLK_PULSE	BIT(3)
#define NISTC_AI_CMD1_LOCALMUX_CLK_PULSE BIT(2)
#define NISTC_AI_CMD1_SC_TC_PULSE	BIT(1)
#define NISTC_AI_CMD1_CONVERT_PULSE	BIT(0)

#define NISTC_AO_CMD1_REG		9
#define NISTC_AO_CMD1_ATRIG_RESET	BIT(15)
#define NISTC_AO_CMD1_START_PULSE	BIT(14)
#define NISTC_AO_CMD1_DISARM		BIT(13)
#define NISTC_AO_CMD1_UI2_ARM_DISARM	BIT(12)
#define NISTC_AO_CMD1_UI2_LOAD		BIT(11)
#define NISTC_AO_CMD1_UI_ARM		BIT(10)
#define NISTC_AO_CMD1_UI_LOAD		BIT(9)
#define NISTC_AO_CMD1_UC_ARM		BIT(8)
#define NISTC_AO_CMD1_UC_LOAD		BIT(7)
#define NISTC_AO_CMD1_BC_ARM		BIT(6)
#define NISTC_AO_CMD1_BC_LOAD		BIT(5)
#define NISTC_AO_CMD1_DAC1_UPDATE_MODE	BIT(4)
#define NISTC_AO_CMD1_LDAC1_SRC_SEL	BIT(3)
#define NISTC_AO_CMD1_DAC0_UPDATE_MODE	BIT(2)
#define NISTC_AO_CMD1_LDAC0_SRC_SEL	BIT(1)
#define NISTC_AO_CMD1_UPDATE_PULSE	BIT(0)

#define NISTC_DIO_OUT_REG		10
#define NISTC_DIO_OUT_SERIAL(x)	(((x) & 0xff) << 8)
#define NISTC_DIO_OUT_SERIAL_MASK	NISTC_DIO_OUT_SERIAL(0xff)
#define NISTC_DIO_OUT_PARALLEL(x)	((x) & 0xff)
#define NISTC_DIO_OUT_PARALLEL_MASK	NISTC_DIO_OUT_PARALLEL(0xff)
#define NISTC_DIO_SDIN			BIT(4)
#define NISTC_DIO_SDOUT			BIT(0)

#define NISTC_DIO_CTRL_REG		11
#define NISTC_DIO_SDCLK			BIT(11)
#define NISTC_DIO_CTRL_HW_SER_TIMEBASE	BIT(10)
#define NISTC_DIO_CTRL_HW_SER_ENA	BIT(9)
#define NISTC_DIO_CTRL_HW_SER_START	BIT(8)
#define NISTC_DIO_CTRL_DIR(x)		((x) & 0xff)
#define NISTC_DIO_CTRL_DIR_MASK		NISTC_DIO_CTRL_DIR(0xff)

#define NISTC_AI_MODE1_REG		12
#define NISTC_AI_MODE1_CONVERT_SRC(x)	(((x) & 0x1f) << 11)
#define NISTC_AI_MODE1_SI_SRC(x)	(((x) & 0x1f) << 6)
#define NISTC_AI_MODE1_CONVERT_POLARITY	BIT(5)
#define NISTC_AI_MODE1_SI_POLARITY	BIT(4)
#define NISTC_AI_MODE1_START_STOP	BIT(3)
#define NISTC_AI_MODE1_RSVD		BIT(2)
#define NISTC_AI_MODE1_CONTINUOUS	BIT(1)
#define NISTC_AI_MODE1_TRIGGER_ONCE	BIT(0)

#define NISTC_AI_MODE2_REG		13
#define NISTC_AI_MODE2_SC_GATE_ENA	BIT(15)
#define NISTC_AI_MODE2_START_STOP_GATE_ENA BIT(14)
#define NISTC_AI_MODE2_PRE_TRIGGER	BIT(13)
#define NISTC_AI_MODE2_EXTMUX_PRESENT	BIT(12)
#define NISTC_AI_MODE2_SI2_INIT_LOAD_SRC BIT(9)
#define NISTC_AI_MODE2_SI2_RELOAD_MODE	BIT(8)
#define NISTC_AI_MODE2_SI_INIT_LOAD_SRC	BIT(7)
#define NISTC_AI_MODE2_SI_RELOAD_MODE(x) (((x) & 0x7) << 4)
#define NISTC_AI_MODE2_SI_WR_SWITCH	BIT(3)
#define NISTC_AI_MODE2_SC_INIT_LOAD_SRC	BIT(2)
#define NISTC_AI_MODE2_SC_RELOAD_MODE	BIT(1)
#define NISTC_AI_MODE2_SC_WR_SWITCH	BIT(0)

#define NISTC_AI_SI_LOADA_REG		14
#define NISTC_AI_SI_LOADB_REG		16
#define NISTC_AI_SC_LOADA_REG		18
#define NISTC_AI_SC_LOADB_REG		20
#define NISTC_AI_SI2_LOADA_REG		23
#define NISTC_AI_SI2_LOADB_REG		25

#define NISTC_G0_MODE_REG		26
#define NISTC_G1_MODE_REG		27
#define NISTC_G0_LOADA_REG		28
#define NISTC_G0_LOADB_REG		30
#define NISTC_G1_LOADA_REG		32
#define NISTC_G1_LOADB_REG		34
#define NISTC_G0_INPUT_SEL_REG		36
#define NISTC_G1_INPUT_SEL_REG		37

#define NISTC_AO_MODE1_REG		38
#define NISTC_AO_MODE1_UPDATE_SRC(x)	(((x) & 0x1f) << 11)
#define NISTC_AO_MODE1_UPDATE_SRC_MASK	NISTC_AO_MODE1_UPDATE_SRC(0x1f)
#define NISTC_AO_MODE1_UI_SRC(x)	(((x) & 0x1f) << 6)
#define NISTC_AO_MODE1_UI_SRC_MASK	NISTC_AO_MODE1_UI_SRC(0x1f)
#define NISTC_AO_MODE1_MULTI_CHAN	BIT(5)
#define NISTC_AO_MODE1_UPDATE_SRC_POLARITY BIT(4)
#define NISTC_AO_MODE1_UI_SRC_POLARITY	BIT(3)
#define NISTC_AO_MODE1_UC_SW_EVERY_TC	BIT(2)
#define NISTC_AO_MODE1_CONTINUOUS	BIT(1)
#define NISTC_AO_MODE1_TRIGGER_ONCE	BIT(0)

#define NISTC_AO_MODE2_REG		39
#define NISTC_AO_MODE2_FIFO_MODE(x)	(((x) & 0x3) << 14)
#define NISTC_AO_MODE2_FIFO_MODE_MASK	NISTC_AO_MODE2_FIFO_MODE(3)
#define NISTC_AO_MODE2_FIFO_MODE_E	NISTC_AO_MODE2_FIFO_MODE(0)
#define NISTC_AO_MODE2_FIFO_MODE_HF	NISTC_AO_MODE2_FIFO_MODE(1)
#define NISTC_AO_MODE2_FIFO_MODE_F	NISTC_AO_MODE2_FIFO_MODE(2)
#define NISTC_AO_MODE2_FIFO_MODE_HF_F	NISTC_AO_MODE2_FIFO_MODE(3)
#define NISTC_AO_MODE2_FIFO_REXMIT_ENA	BIT(13)
#define NISTC_AO_MODE2_START1_DISABLE	BIT(12)
#define NISTC_AO_MODE2_UC_INIT_LOAD_SRC	BIT(11)
#define NISTC_AO_MODE2_UC_WR_SWITCH	BIT(10)
#define NISTC_AO_MODE2_UI2_INIT_LOAD_SRC BIT(9)
#define NISTC_AO_MODE2_UI2_RELOAD_MODE	BIT(8)
#define NISTC_AO_MODE2_UI_INIT_LOAD_SRC	BIT(7)
#define NISTC_AO_MODE2_UI_RELOAD_MODE(x) (((x) & 0x7) << 4)
#define NISTC_AO_MODE2_UI_WR_SWITCH	BIT(3)
#define NISTC_AO_MODE2_BC_INIT_LOAD_SRC	BIT(2)
#define NISTC_AO_MODE2_BC_RELOAD_MODE	BIT(1)
#define NISTC_AO_MODE2_BC_WR_SWITCH	BIT(0)

#define NISTC_AO_UI_LOADA_REG		40
#define NISTC_AO_UI_LOADB_REG		42
#define NISTC_AO_BC_LOADA_REG		44
#define NISTC_AO_BC_LOADB_REG		46
#define NISTC_AO_UC_LOADA_REG		48
#define NISTC_AO_UC_LOADB_REG		50

#define NISTC_CLK_FOUT_REG		56
#define NISTC_CLK_FOUT_ENA		BIT(15)
#define NISTC_CLK_FOUT_TIMEBASE_SEL	BIT(14)
#define NISTC_CLK_FOUT_DIO_SER_OUT_DIV2	BIT(13)
#define NISTC_CLK_FOUT_SLOW_DIV2	BIT(12)
#define NISTC_CLK_FOUT_SLOW_TIMEBASE	BIT(11)
#define NISTC_CLK_FOUT_G_SRC_DIV2	BIT(10)
#define NISTC_CLK_FOUT_TO_BOARD_DIV2	BIT(9)
#define NISTC_CLK_FOUT_TO_BOARD		BIT(8)
#define NISTC_CLK_FOUT_AI_OUT_DIV2	BIT(7)
#define NISTC_CLK_FOUT_AI_SRC_DIV2	BIT(6)
#define NISTC_CLK_FOUT_AO_OUT_DIV2	BIT(5)
#define NISTC_CLK_FOUT_AO_SRC_DIV2	BIT(4)
#define NISTC_CLK_FOUT_DIVIDER(x)	(((x) & 0xf) << 0)
#define NISTC_CLK_FOUT_TO_DIVIDER(x)	(((x) >> 0) & 0xf)
#define NISTC_CLK_FOUT_DIVIDER_MASK	NISTC_CLK_FOUT_DIVIDER(0xf)

#define NISTC_IO_BIDIR_PIN_REG		57

#define NISTC_RTSI_TRIG_DIR_REG		58
#define NISTC_RTSI_TRIG_OLD_CLK_CHAN	7
#define NISTC_RTSI_TRIG_NUM_CHAN(_m)	((_m) ? 8 : 7)
#define NISTC_RTSI_TRIG_DIR(_c, _m)	((_m) ? BIT(8 + (_c)) : BIT(7 + (_c)))
#define NISTC_RTSI_TRIG_USE_CLK		BIT(1)
#define NISTC_RTSI_TRIG_DRV_CLK		BIT(0)

#define NISTC_INT_CTRL_REG		59
#define NISTC_INT_CTRL_INTB_ENA		BIT(15)
#define NISTC_INT_CTRL_INTB_SEL(x)	(((x) & 0x7) << 12)
#define NISTC_INT_CTRL_INTA_ENA		BIT(11)
#define NISTC_INT_CTRL_INTA_SEL(x)	(((x) & 0x7) << 8)
#define NISTC_INT_CTRL_PASSTHRU0_POL	BIT(3)
#define NISTC_INT_CTRL_PASSTHRU1_POL	BIT(2)
#define NISTC_INT_CTRL_3PIN_INT		BIT(1)
#define NISTC_INT_CTRL_INT_POL		BIT(0)

#define NISTC_AI_OUT_CTRL_REG		60
#define NISTC_AI_OUT_CTRL_START_SEL	BIT(10)
#define NISTC_AI_OUT_CTRL_SCAN_IN_PROG_SEL(x)	(((x) & 0x3) << 8)
#define NISTC_AI_OUT_CTRL_EXTMUX_CLK_SEL(x)	(((x) & 0x3) << 6)
#define NISTC_AI_OUT_CTRL_LOCALMUX_CLK_SEL(x)	(((x) & 0x3) << 4)
#define NISTC_AI_OUT_CTRL_SC_TC_SEL(x)		(((x) & 0x3) << 2)
#define NISTC_AI_OUT_CTRL_CONVERT_SEL(x)	(((x) & 0x3) << 0)
#define NISTC_AI_OUT_CTRL_CONVERT_HIGH_Z	NISTC_AI_OUT_CTRL_CONVERT_SEL(0)
#define NISTC_AI_OUT_CTRL_CONVERT_GND		NISTC_AI_OUT_CTRL_CONVERT_SEL(1)
#define NISTC_AI_OUT_CTRL_CONVERT_LOW		NISTC_AI_OUT_CTRL_CONVERT_SEL(2)
#define NISTC_AI_OUT_CTRL_CONVERT_HIGH		NISTC_AI_OUT_CTRL_CONVERT_SEL(3)

#define NISTC_ATRIG_ETC_REG		61
#define NISTC_ATRIG_ETC_GPFO_1_ENA	BIT(15)
#define NISTC_ATRIG_ETC_GPFO_0_ENA	BIT(14)
#define NISTC_ATRIG_ETC_GPFO_0_SEL(x)	(((x) & 0x3) << 11)
#define NISTC_ATRIG_ETC_GPFO_1_SEL	BIT(7)
#define NISTC_ATRIG_ETC_DRV		BIT(4)
#define NISTC_ATRIG_ETC_ENA		BIT(3)
#define NISTC_ATRIG_ETC_MODE(x)		(((x) & 0x7) << 0)

#define AI_Status_1_Register		2
#define Interrupt_A_St				0x8000
#define AI_FIFO_Full_St				0x4000
#define AI_FIFO_Half_Full_St			0x2000
#define AI_FIFO_Empty_St			0x1000
#define AI_Overrun_St				0x0800
#define AI_Overflow_St				0x0400
#define AI_SC_TC_Error_St			0x0200
#define AI_START2_St				0x0100
#define AI_START1_St				0x0080
#define AI_SC_TC_St				0x0040
#define AI_START_St				0x0020
#define AI_STOP_St				0x0010
#define G0_TC_St				0x0008
#define G0_Gate_Interrupt_St			0x0004
#define AI_FIFO_Request_St			0x0002
#define Pass_Thru_0_Interrupt_St		0x0001

#define AI_Status_2_Register		5

#define AO_Status_1_Register		3
#define Interrupt_B_St				_bit15
#define AO_FIFO_Full_St				_bit14
#define AO_FIFO_Half_Full_St			_bit13
#define AO_FIFO_Empty_St			_bit12
#define AO_BC_TC_Error_St			_bit11
#define AO_START_St				_bit10
#define AO_Overrun_St				_bit9
#define AO_START1_St				_bit8
#define AO_BC_TC_St				_bit7
#define AO_UC_TC_St				_bit6
#define AO_UPDATE_St				_bit5
#define AO_UI2_TC_St				_bit4
#define G1_TC_St				_bit3
#define G1_Gate_Interrupt_St			_bit2
#define AO_FIFO_Request_St			_bit1
#define Pass_Thru_1_Interrupt_St		_bit0

#define AO_Status_2_Register		6

#define DIO_Parallel_Input_Register	7

#define AI_SI_Save_Registers		64
#define AI_SC_Save_Registers		66

#define Joint_Status_1_Register         27
#define DIO_Serial_IO_In_Progress_St            _bit12

#define DIO_Serial_Input_Register       28
#define Joint_Status_2_Register         29
enum Joint_Status_2_Bits {
	AO_TMRDACWRs_In_Progress_St = 0x20,
};

#define AO_UI_Save_Registers		16
#define AO_BC_Save_Registers		18
#define AO_UC_Save_Registers		20

#define AI_START_STOP_Select_Register	62
#define AI_START_Polarity			_bit15
#define AI_STOP_Polarity			_bit14
#define AI_STOP_Sync				_bit13
#define AI_STOP_Edge				_bit12
#define AI_STOP_Select(a)			(((a) & 0x1f)<<7)
#define AI_START_Sync				_bit6
#define AI_START_Edge				_bit5
#define AI_START_Select(a)			((a) & 0x1f)

#define AI_Trigger_Select_Register	63
#define AI_START1_Polarity			_bit15
#define AI_START2_Polarity			_bit14
#define AI_START2_Sync				_bit13
#define AI_START2_Edge				_bit12
#define AI_START2_Select(a)			(((a) & 0x1f) << 7)
#define AI_START1_Sync				_bit6
#define AI_START1_Edge				_bit5
#define AI_START1_Select(a)			((a) & 0x1f)

#define AI_DIV_Load_A_Register	64

#define AO_Start_Select_Register	66
#define AO_UI2_Software_Gate			_bit15
#define AO_UI2_External_Gate_Polarity		_bit14
#define AO_START_Polarity			_bit13
#define AO_AOFREQ_Enable			_bit12
#define AO_UI2_External_Gate_Select(a)		(((a) & 0x1f) << 7)
#define AO_START_Sync				_bit6
#define AO_START_Edge				_bit5
#define AO_START_Select(a)			((a) & 0x1f)

#define AO_Trigger_Select_Register	67
#define AO_UI2_External_Gate_Enable		_bit15
#define AO_Delayed_START1			_bit14
#define AO_START1_Polarity			_bit13
#define AO_UI2_Source_Polarity			_bit12
#define AO_UI2_Source_Select(x)			(((x)&0x1f)<<7)
#define AO_START1_Sync				_bit6
#define AO_START1_Edge				_bit5
#define AO_START1_Select(x)			(((x)&0x1f)<<0)

#define AO_Mode_3_Register		70
#define AO_UI2_Switch_Load_Next_TC		_bit13
#define AO_UC_Switch_Load_Every_BC_TC		_bit12
#define AO_Trigger_Length			_bit11
#define AO_Stop_On_Overrun_Error		_bit5
#define AO_Stop_On_BC_TC_Trigger_Error		_bit4
#define AO_Stop_On_BC_TC_Error			_bit3
#define AO_Not_An_UPDATE			_bit2
#define AO_Software_Gate			_bit1
#define AO_Last_Gate_Disable		_bit0	/* M Series only */

#define Joint_Reset_Register		72
#define Software_Reset			_bit11
#define AO_Configuration_End			_bit9
#define AI_Configuration_End			_bit8
#define AO_Configuration_Start			_bit5
#define AI_Configuration_Start			_bit4
#define G1_Reset				_bit3
#define G0_Reset				_bit2
#define AO_Reset				_bit1
#define AI_Reset				_bit0

#define Interrupt_A_Enable_Register	73
#define Pass_Thru_0_Interrupt_Enable		_bit9
#define G0_Gate_Interrupt_Enable		_bit8
#define AI_FIFO_Interrupt_Enable		_bit7
#define G0_TC_Interrupt_Enable			_bit6
#define AI_Error_Interrupt_Enable		_bit5
#define AI_STOP_Interrupt_Enable		_bit4
#define AI_START_Interrupt_Enable		_bit3
#define AI_START2_Interrupt_Enable		_bit2
#define AI_START1_Interrupt_Enable		_bit1
#define AI_SC_TC_Interrupt_Enable		_bit0

#define Interrupt_B_Enable_Register	75
#define Pass_Thru_1_Interrupt_Enable		_bit11
#define G1_Gate_Interrupt_Enable		_bit10
#define G1_TC_Interrupt_Enable			_bit9
#define AO_FIFO_Interrupt_Enable		_bit8
#define AO_UI2_TC_Interrupt_Enable		_bit7
#define AO_UC_TC_Interrupt_Enable		_bit6
#define AO_Error_Interrupt_Enable		_bit5
#define AO_STOP_Interrupt_Enable		_bit4
#define AO_START_Interrupt_Enable		_bit3
#define AO_UPDATE_Interrupt_Enable		_bit2
#define AO_START1_Interrupt_Enable		_bit1
#define AO_BC_TC_Interrupt_Enable		_bit0

#define Second_IRQ_A_Enable_Register	74
enum Second_IRQ_A_Enable_Bits {
	AI_SC_TC_Second_Irq_Enable = _bit0,
	AI_START1_Second_Irq_Enable = _bit1,
	AI_START2_Second_Irq_Enable = _bit2,
	AI_START_Second_Irq_Enable = _bit3,
	AI_STOP_Second_Irq_Enable = _bit4,
	AI_Error_Second_Irq_Enable = _bit5,
	G0_TC_Second_Irq_Enable = _bit6,
	AI_FIFO_Second_Irq_Enable = _bit7,
	G0_Gate_Second_Irq_Enable = _bit8,
	Pass_Thru_0_Second_Irq_Enable = _bit9
};

#define Second_IRQ_B_Enable_Register	76
enum Second_IRQ_B_Enable_Bits {
	AO_BC_TC_Second_Irq_Enable = _bit0,
	AO_START1_Second_Irq_Enable = _bit1,
	AO_UPDATE_Second_Irq_Enable = _bit2,
	AO_START_Second_Irq_Enable = _bit3,
	AO_STOP_Second_Irq_Enable = _bit4,
	AO_Error_Second_Irq_Enable = _bit5,
	AO_UC_TC_Second_Irq_Enable = _bit6,
	AO_UI2_TC_Second_Irq_Enable = _bit7,
	AO_FIFO_Second_Irq_Enable = _bit8,
	G1_TC_Second_Irq_Enable = _bit9,
	G1_Gate_Second_Irq_Enable = _bit10,
	Pass_Thru_1_Second_Irq_Enable = _bit11
};

#define AI_Personal_Register		77
#define AI_SHIFTIN_Pulse_Width			_bit15
#define AI_EOC_Polarity				_bit14
#define AI_SOC_Polarity				_bit13
#define AI_SHIFTIN_Polarity			_bit12
#define AI_CONVERT_Pulse_Timebase		_bit11
#define AI_CONVERT_Pulse_Width			_bit10
#define AI_CONVERT_Original_Pulse		_bit9
#define AI_FIFO_Flags_Polarity			_bit8
#define AI_Overrun_Mode				_bit7
#define AI_EXTMUX_CLK_Pulse_Width		_bit6
#define AI_LOCALMUX_CLK_Pulse_Width		_bit5
#define AI_AIFREQ_Polarity			_bit4

#define AO_Personal_Register		78
enum AO_Personal_Bits {
	AO_Interval_Buffer_Mode = 1 << 3,
	AO_BC_Source_Select = 1 << 4,
	AO_UPDATE_Pulse_Width = 1 << 5,
	AO_UPDATE_Pulse_Timebase = 1 << 6,
	AO_UPDATE_Original_Pulse = 1 << 7,
	AO_DMA_PIO_Control = 1 << 8,	/* M Series: reserved */
	AO_AOFREQ_Polarity = 1 << 9,	/* M Series: reserved */
	AO_FIFO_Enable = 1 << 10,
	AO_FIFO_Flags_Polarity = 1 << 11,	/* M Series: reserved */
	AO_TMRDACWR_Pulse_Width = 1 << 12,
	AO_Fast_CPU = 1 << 13,	/* M Series: reserved */
	AO_Number_Of_DAC_Packages = 1 << 14,	/*  1 for "single" mode, 0 for "dual" */
	AO_Multiple_DACS_Per_Package = 1 << 15	/*  m-series only */
};
#define	RTSI_Trig_A_Output_Register	79
#define	RTSI_Trig_B_Output_Register	80
enum RTSI_Trig_B_Output_Bits {
	RTSI_Sub_Selection_1_Bit = 0x8000	/*  not for m-series */
};
static inline unsigned RTSI_Trig_Output_Bits(unsigned rtsi_channel,
					     unsigned source)
{
	return (source & 0xf) << ((rtsi_channel % 4) * 4);
};

static inline unsigned RTSI_Trig_Output_Mask(unsigned rtsi_channel)
{
	return 0xf << ((rtsi_channel % 4) * 4);
};

/* inverse to RTSI_Trig_Output_Bits() */
static inline unsigned RTSI_Trig_Output_Source(unsigned rtsi_channel,
					       unsigned bits)
{
	return (bits >> ((rtsi_channel % 4) * 4)) & 0xf;
};

#define	RTSI_Board_Register		81
#define Write_Strobe_0_Register		82
#define Write_Strobe_1_Register		83
#define Write_Strobe_2_Register		84
#define Write_Strobe_3_Register		85

#define AO_Output_Control_Register	86
#define AO_External_Gate_Enable			_bit15
#define AO_External_Gate_Select(x)		(((x)&0x1f)<<10)
#define AO_Number_Of_Channels(x)		(((x)&0xf)<<6)
#define AO_UPDATE2_Output_Select(x)		(((x)&0x3)<<4)
#define AO_External_Gate_Polarity		_bit3
#define AO_UPDATE2_Output_Toggle		_bit2
enum ao_update_output_selection {
	AO_Update_Output_High_Z = 0,
	AO_Update_Output_Ground = 1,
	AO_Update_Output_Enable_Low = 2,
	AO_Update_Output_Enable_High = 3
};
static unsigned AO_UPDATE_Output_Select(enum ao_update_output_selection
					selection)
{
	return selection & 0x3;
}

#define AI_Mode_3_Register		87
#define AI_Trigger_Length			_bit15
#define AI_Delay_START				_bit14
#define AI_Software_Gate			_bit13
#define AI_SI_Special_Trigger_Delay		_bit12
#define AI_SI2_Source_Select			_bit11
#define AI_Delayed_START2			_bit10
#define AI_Delayed_START1			_bit9
#define AI_External_Gate_Mode			_bit8
#define AI_FIFO_Mode_HF_to_E			(3<<6)
#define AI_FIFO_Mode_F				(2<<6)
#define AI_FIFO_Mode_HF				(1<<6)
#define AI_FIFO_Mode_NE				(0<<6)
#define AI_External_Gate_Polarity		_bit5
#define AI_External_Gate_Select(a)		((a) & 0x1f)

#define G_Autoincrement_Register(a)	(68+(a))
#define G_HW_Save_Register(a)		(8+(a)*2)
#define G_HW_Save_Register_High(a)	(8+(a)*2)
#define G_HW_Save_Register_Low(a)	(9+(a)*2)
#define G_Save_Register(a)		(12+(a)*2)
#define G_Save_Register_High(a)		(12+(a)*2)
#define G_Save_Register_Low(a)		(13+(a)*2)
#define G_Status_Register		4

/* command register */
#define G_Disarm_Copy			_bit15	/* strobe */
#define G_Save_Trace_Copy		_bit14
#define G_Arm_Copy			_bit13	/* strobe */
#define G_Bank_Switch_Start		_bit10	/* strobe */
#define G_Little_Big_Endian		_bit9
#define G_Synchronized_Gate		_bit8
#define G_Write_Switch			_bit7
#define G_Up_Down(a)			(((a)&0x03)<<5)
#define G_Disarm			_bit4	/* strobe */
#define G_Analog_Trigger_Reset		_bit3	/* strobe */
#define G_Save_Trace			_bit1
#define G_Arm				_bit0	/* strobe */

/*channel agnostic names for the command register #defines */
#define G_Bank_Switch_Enable		_bit12
#define G_Bank_Switch_Mode		_bit11
#define G_Load				_bit2	/* strobe */

/* input select register */
#define G_Gate_Select(a)		(((a)&0x1f)<<7)
#define G_Source_Select(a)		(((a)&0x1f)<<2)
#define G_Write_Acknowledges_Irq	_bit1
#define G_Read_Acknowledges_Irq		_bit0

/* same input select register, but with channel agnostic names */
#define G_Source_Polarity		_bit15
#define G_Output_Polarity		_bit14
#define G_OR_Gate			_bit13
#define G_Gate_Select_Load_Source	_bit12

/* mode register */
#define G_Loading_On_TC			_bit12
#define G_Output_Mode(a)		(((a)&0x03)<<8)
#define G_Trigger_Mode_For_Edge_Gate(a)	(((a)&0x03)<<3)
#define G_Gating_Mode(a)		(((a)&0x03)<<0)

/* same input mode register, but with channel agnostic names */
#define G_Load_Source_Select		_bit7
#define G_Reload_Source_Switching	_bit15
#define G_Loading_On_Gate		_bit14
#define G_Gate_Polarity 		_bit13

#define G_Counting_Once(a)		(((a)&0x03)<<10)
#define G_Stop_Mode(a)			(((a)&0x03)<<5)
#define G_Gate_On_Both_Edges		_bit2

/* G_Status_Register */
#define G1_Gate_Error_St		_bit15
#define G0_Gate_Error_St		_bit14
#define G1_TC_Error_St			_bit13
#define G0_TC_Error_St			_bit12
#define G1_No_Load_Between_Gates_St	_bit11
#define G0_No_Load_Between_Gates_St	_bit10
#define G1_Armed_St			_bit9
#define G0_Armed_St			_bit8
#define G1_Stale_Data_St		_bit7
#define G0_Stale_Data_St		_bit6
#define G1_Next_Load_Source_St		_bit5
#define G0_Next_Load_Source_St		_bit4
#define G1_Counting_St			_bit3
#define G0_Counting_St			_bit2
#define G1_Save_St			_bit1
#define G0_Save_St			_bit0

/* general purpose counter timer */
#define G_Autoincrement(a)              ((a)<<0)

/* Additional windowed registers unique to E series */

/* 16 bit registers shadowed from DAQ-STC */
#define Window_Address			0x00
#define Window_Data			0x02

#define Configuration_Memory_Clear	82
#define ADC_FIFO_Clear			83
#define DAC_FIFO_Clear			84

/* i/o port offsets */

/* 8 bit registers */
#define XXX_Status			0x01
enum XXX_Status_Bits {
	PROMOUT = 0x1,
	AI_FIFO_LOWER_NOT_EMPTY = 0x8,
};
#define Serial_Command			0x0d
#define Misc_Command			0x0f
#define Port_A				0x19
#define Port_B				0x1b
#define Port_C				0x1d
#define Configuration			0x1f
#define Strobes				0x01
#define Channel_A_Mode			0x03
#define Channel_B_Mode			0x05
#define Channel_C_Mode			0x07
#define AI_AO_Select			0x09
enum AI_AO_Select_Bits {
	AI_DMA_Select_Shift = 0,
	AI_DMA_Select_Mask = 0xf,
	AO_DMA_Select_Shift = 4,
	AO_DMA_Select_Mask = 0xf << AO_DMA_Select_Shift
};
#define G0_G1_Select			0x0b
static inline unsigned ni_stc_dma_channel_select_bitfield(unsigned channel)
{
	if (channel < 4)
		return 1 << channel;
	if (channel == 4)
		return 0x3;
	if (channel == 5)
		return 0x5;
	BUG();
	return 0;
}

static inline unsigned GPCT_DMA_Select_Bits(unsigned gpct_index,
					    unsigned mite_channel)
{
	BUG_ON(gpct_index > 1);
	return ni_stc_dma_channel_select_bitfield(mite_channel) << (4 *
								    gpct_index);
}

static inline unsigned GPCT_DMA_Select_Mask(unsigned gpct_index)
{
	BUG_ON(gpct_index > 1);
	return 0xf << (4 * gpct_index);
}

/* 16 bit registers */

#define Configuration_Memory_Low	0x10
enum Configuration_Memory_Low_Bits {
	AI_DITHER = 0x200,
	AI_LAST_CHANNEL = 0x8000,
};
#define Configuration_Memory_High	0x12
enum Configuration_Memory_High_Bits {
	AI_AC_COUPLE = 0x800,
	AI_DIFFERENTIAL = 0x1000,
	AI_COMMON = 0x2000,
	AI_GROUND = 0x3000,
};
static inline unsigned int AI_CONFIG_CHANNEL(unsigned int channel)
{
	return channel & 0x3f;
}

#define ADC_FIFO_Data_Register		0x1c

#define AO_Configuration		0x16
#define AO_Bipolar		_bit0
#define AO_Deglitch		_bit1
#define AO_Ext_Ref		_bit2
#define AO_Ground_Ref		_bit3
#define AO_Channel(x)		((x) << 8)

#define DAC_FIFO_Data			0x1e
#define DAC0_Direct_Data		0x18
#define DAC1_Direct_Data		0x1a

/* 611x registers (these boards differ from the e-series) */

#define Magic_611x			0x19	/* w8 (new) */
#define Calibration_Channel_Select_611x	0x1a	/* w16 (new) */
#define ADC_FIFO_Data_611x		0x1c	/* r32 (incompatible) */
#define AI_FIFO_Offset_Load_611x	0x05	/* r8 (new) */
#define DAC_FIFO_Data_611x		0x14	/* w32 (incompatible) */
#define Cal_Gain_Select_611x		0x05	/* w8 (new) */

#define AO_Window_Address_611x		0x18
#define AO_Window_Data_611x		0x1e

/* 6143 registers */
#define Magic_6143			0x19	/* w8 */
#define G0G1_DMA_Select_6143		0x0B	/* w8 */
#define PipelineDelay_6143		0x1f	/* w8 */
#define EOC_Set_6143			0x1D	/* w8 */
#define AIDMA_Select_6143		0x09	/* w8 */
#define AIFIFO_Data_6143		0x8C	/* w32 */
#define AIFIFO_Flag_6143		0x84	/* w32 */
#define AIFIFO_Control_6143		0x88	/* w32 */
#define AIFIFO_Status_6143		0x88	/* w32 */
#define AIFIFO_DMAThreshold_6143	0x90	/* w32 */
#define AIFIFO_Words_Available_6143	0x94	/* w32 */

#define Calibration_Channel_6143	0x42	/* w16 */
#define Calibration_LowTime_6143	0x20	/* w16 */
#define Calibration_HighTime_6143	0x22	/* w16 */
#define Relay_Counter_Load_Val__6143	0x4C	/* w32 */
#define Signature_6143			0x50	/* w32 */
#define Release_Date_6143		0x54	/* w32 */
#define Release_Oldest_Date_6143	0x58	/* w32 */

#define Calibration_Channel_6143_RelayOn	0x8000	/* Calibration relay switch On */
#define Calibration_Channel_6143_RelayOff	0x4000	/* Calibration relay switch Off */
#define Calibration_Channel_Gnd_Gnd	0x00	/* Offset Calibration */
#define Calibration_Channel_2v5_Gnd	0x02	/* 2.5V Reference */
#define Calibration_Channel_Pwm_Gnd	0x05	/* +/- 5V Self Cal */
#define Calibration_Channel_2v5_Pwm	0x0a	/* PWM Calibration */
#define Calibration_Channel_Pwm_Pwm	0x0d	/* CMRR */
#define Calibration_Channel_Gnd_Pwm	0x0e	/* PWM Calibration */

/* 671x, 611x registers */

/* 671xi, 611x windowed ao registers */
enum windowed_regs_67xx_61xx {
	AO_Immediate_671x = 0x11,	/* W 16 */
	AO_Timed_611x = 0x10,	/* W 16 */
	AO_FIFO_Offset_Load_611x = 0x13,	/* W32 */
	AO_Later_Single_Point_Updates = 0x14,	/* W 16 */
	AO_Waveform_Generation_611x = 0x15,	/* W 16 */
	AO_Misc_611x = 0x16,	/* W 16 */
	AO_Calibration_Channel_Select_67xx = 0x17,	/* W 16 */
	AO_Configuration_2_67xx = 0x18,	/* W 16 */
	CAL_ADC_Command_67xx = 0x19,	/* W 8 */
	CAL_ADC_Status_67xx = 0x1a,	/* R 8 */
	CAL_ADC_Data_67xx = 0x1b,	/* R 16 */
	CAL_ADC_Config_Data_High_Word_67xx = 0x1c,	/* RW 16 */
	CAL_ADC_Config_Data_Low_Word_67xx = 0x1d,	/* RW 16 */
};
static inline unsigned int DACx_Direct_Data_671x(int channel)
{
	return channel;
}

enum AO_Misc_611x_Bits {
	CLEAR_WG = 1,
};
enum cs5529_configuration_bits {
	CSCFG_CAL_CONTROL_MASK = 0x7,
	CSCFG_SELF_CAL_OFFSET = 0x1,
	CSCFG_SELF_CAL_GAIN = 0x2,
	CSCFG_SELF_CAL_OFFSET_GAIN = 0x3,
	CSCFG_SYSTEM_CAL_OFFSET = 0x5,
	CSCFG_SYSTEM_CAL_GAIN = 0x6,
	CSCFG_DONE = 1 << 3,
	CSCFG_POWER_SAVE_SELECT = 1 << 4,
	CSCFG_PORT_MODE = 1 << 5,
	CSCFG_RESET_VALID = 1 << 6,
	CSCFG_RESET = 1 << 7,
	CSCFG_UNIPOLAR = 1 << 12,
	CSCFG_WORD_RATE_2180_CYCLES = 0x0 << 13,
	CSCFG_WORD_RATE_1092_CYCLES = 0x1 << 13,
	CSCFG_WORD_RATE_532_CYCLES = 0x2 << 13,
	CSCFG_WORD_RATE_388_CYCLES = 0x3 << 13,
	CSCFG_WORD_RATE_324_CYCLES = 0x4 << 13,
	CSCFG_WORD_RATE_17444_CYCLES = 0x5 << 13,
	CSCFG_WORD_RATE_8724_CYCLES = 0x6 << 13,
	CSCFG_WORD_RATE_4364_CYCLES = 0x7 << 13,
	CSCFG_WORD_RATE_MASK = 0x7 << 13,
	CSCFG_LOW_POWER = 1 << 16,
};
static inline unsigned int CS5529_CONFIG_DOUT(int output)
{
	return 1 << (18 + output);
}

static inline unsigned int CS5529_CONFIG_AOUT(int output)
{
	return 1 << (22 + output);
}

enum cs5529_command_bits {
	CSCMD_POWER_SAVE = 0x1,
	CSCMD_REGISTER_SELECT_MASK = 0xe,
	CSCMD_OFFSET_REGISTER = 0x0,
	CSCMD_GAIN_REGISTER = 0x2,
	CSCMD_CONFIG_REGISTER = 0x4,
	CSCMD_READ = 0x10,
	CSCMD_CONTINUOUS_CONVERSIONS = 0x20,
	CSCMD_SINGLE_CONVERSION = 0x40,
	CSCMD_COMMAND = 0x80,
};
enum cs5529_status_bits {
	CSS_ADC_BUSY = 0x1,
	CSS_OSC_DETECT = 0x2,	/* indicates adc error */
	CSS_OVERRANGE = 0x4,
};
#define SerDacLd(x)			(0x08<<(x))

/*
	This is stuff unique to the NI E series drivers,
	but I thought I'd put it here anyway.
*/

enum { ai_gain_16 =
	    0, ai_gain_8, ai_gain_14, ai_gain_4, ai_gain_611x, ai_gain_622x,
	ai_gain_628x, ai_gain_6143
};
enum caldac_enum { caldac_none = 0, mb88341, dac8800, dac8043, ad8522,
	ad8804, ad8842, ad8804_debug
};
enum ni_reg_type {
	ni_reg_normal = 0x0,
	ni_reg_611x = 0x1,
	ni_reg_6711 = 0x2,
	ni_reg_6713 = 0x4,
	ni_reg_67xx_mask = 0x6,
	ni_reg_6xxx_mask = 0x7,
	ni_reg_622x = 0x8,
	ni_reg_625x = 0x10,
	ni_reg_628x = 0x18,
	ni_reg_m_series_mask = 0x18,
	ni_reg_6143 = 0x20
};

static const struct comedi_lrange range_ni_E_ao_ext;

/*
 * M-Series specific registers not handled by the DAQ-STC and GPCT register
 * remapping.
 */
#define NI_M_CDIO_DMA_SEL_REG		0x007
#define NI_M_CDIO_DMA_SEL_CDO(x)	(((x) & 0xf) << 4)
#define NI_M_CDIO_DMA_SEL_CDO_MASK	NI_M_CDIO_DMA_SEL_CDO(0xf)
#define NI_M_CDIO_DMA_SEL_CDI(x)	(((x) & 0xf) << 0)
#define NI_M_CDIO_DMA_SEL_CDI_MASK	NI_M_CDIO_DMA_SEL_CDI(0xf)
#define NI_M_SCXI_STATUS_REG		0x007
#define NI_M_AI_AO_SEL_REG		0x009
#define NI_M_G0_G1_SEL_REG		0x00b
#define NI_M_MISC_CMD_REG		0x00f
#define NI_M_SCXI_SER_DO_REG		0x011
#define NI_M_SCXI_CTRL_REG		0x013
#define NI_M_SCXI_OUT_ENA_REG		0x015
#define NI_M_AI_FIFO_DATA_REG		0x01c
#define NI_M_DIO_REG			0x024
#define NI_M_DIO_DIR_REG		0x028
#define NI_M_CAL_PWM_REG		0x040
#define NI_M_CAL_PWM_HIGH_TIME(x)	(((x) & 0xffff) << 16)
#define NI_M_CAL_PWM_LOW_TIME(x)	(((x) & 0xffff) << 0)
#define NI_M_GEN_PWM_REG(x)		(0x044 + ((x) * 2))
#define NI_M_AI_CFG_FIFO_DATA_REG	0x05e
#define NI_M_AI_CFG_LAST_CHAN		BIT(14)
#define NI_M_AI_CFG_DITHER		BIT(13)
#define NI_M_AI_CFG_POLARITY		BIT(12)
#define NI_M_AI_CFG_GAIN(x)		(((x) & 0x7) << 9)
#define NI_M_AI_CFG_CHAN_TYPE(x)	(((x) & 0x7) << 6)
#define NI_M_AI_CFG_CHAN_TYPE_MASK	NI_M_AI_CFG_CHAN_TYPE(7)
#define NI_M_AI_CFG_CHAN_TYPE_CALIB	NI_M_AI_CFG_CHAN_TYPE(0)
#define NI_M_AI_CFG_CHAN_TYPE_DIFF	NI_M_AI_CFG_CHAN_TYPE(1)
#define NI_M_AI_CFG_CHAN_TYPE_COMMON	NI_M_AI_CFG_CHAN_TYPE(2)
#define NI_M_AI_CFG_CHAN_TYPE_GROUND	NI_M_AI_CFG_CHAN_TYPE(3)
#define NI_M_AI_CFG_CHAN_TYPE_AUX	NI_M_AI_CFG_CHAN_TYPE(5)
#define NI_M_AI_CFG_CHAN_TYPE_GHOST	NI_M_AI_CFG_CHAN_TYPE(7)
#define NI_M_AI_CFG_BANK_SEL(x)		((((x) & 0x40) << 4) | ((x) & 0x30))
#define NI_M_AI_CFG_CHAN_SEL(x)		(((x) & 0xf) << 0)
#define NI_M_INTC_ENA_REG		0x088
#define NI_M_INTC_ENA			BIT(0)
#define NI_M_INTC_STATUS_REG		0x088
#define NI_M_INTC_STATUS		BIT(0)
#define NI_M_ATRIG_CTRL_REG		0x08c
#define NI_M_AO_SER_INT_ENA_REG		0x0a0
#define NI_M_AO_SER_INT_ACK_REG		0x0a1
#define NI_M_AO_SER_INT_STATUS_REG	0x0a1
#define NI_M_AO_CALIB_REG		0x0a3
#define NI_M_AO_FIFO_DATA_REG		0x0a4
#define NI_M_PFI_FILTER_REG		0x0b0
#define NI_M_PFI_FILTER_SEL(_c, _f)	(((_f) & 0x3) << ((_c) * 2))
#define NI_M_PFI_FILTER_SEL_MASK(_c)	NI_M_PFI_FILTER_SEL((_c), 0x3)
#define NI_M_RTSI_FILTER_REG		0x0b4
#define NI_M_SCXI_LEGACY_COMPAT_REG	0x0bc
#define NI_M_DAC_DIRECT_DATA_REG(x)	(0x0c0 + ((x) * 4))
#define NI_M_AO_WAVEFORM_ORDER_REG(x)	(0x0c2 + ((x) * 4))
#define NI_M_AO_CFG_BANK_REG(x)		(0x0c3 + ((x) * 4))
#define NI_M_AO_CFG_BANK_BIPOLAR	BIT(7)
#define NI_M_AO_CFG_BANK_UPDATE_TIMED	BIT(6)
#define NI_M_AO_CFG_BANK_REF(x)		(((x) & 0x7) << 3)
#define NI_M_AO_CFG_BANK_REF_MASK	NI_M_AO_CFG_BANK_REF(7)
#define NI_M_AO_CFG_BANK_REF_INT_10V	NI_M_AO_CFG_BANK_REF(0)
#define NI_M_AO_CFG_BANK_REF_INT_5V	NI_M_AO_CFG_BANK_REF(1)
#define NI_M_AO_CFG_BANK_OFFSET(x)	(((x) & 0x7) << 0)
#define NI_M_AO_CFG_BANK_OFFSET_MASK	NI_M_AO_CFG_BANK_OFFSET(7)
#define NI_M_AO_CFG_BANK_OFFSET_0V	NI_M_AO_CFG_BANK_OFFSET(0)
#define NI_M_AO_CFG_BANK_OFFSET_5V	NI_M_AO_CFG_BANK_OFFSET(1)
#define NI_M_RTSI_SHARED_MUX_REG	0x1a2
#define NI_M_CLK_FOUT2_REG		0x1c4
#define NI_M_CLK_FOUT2_RTSI_10MHZ	BIT(7)
#define NI_M_CLK_FOUT2_TIMEBASE3_PLL	BIT(6)
#define NI_M_CLK_FOUT2_TIMEBASE1_PLL	BIT(5)
#define NI_M_CLK_FOUT2_PLL_SRC(x)	(((x) & 0x1f) << 0)
#define NI_M_CLK_FOUT2_PLL_SRC_MASK	NI_M_CLK_FOUT2_PLL_SRC(0x1f)
#define NI_M_MAX_RTSI_CHAN		7
#define NI_M_CLK_FOUT2_PLL_SRC_RTSI(x)	(((x) == NI_M_MAX_RTSI_CHAN)	\
					 ? NI_M_CLK_FOUT2_PLL_SRC(0x1b)	\
					 : NI_M_CLK_FOUT2_PLL_SRC(0xb + (x)))
#define NI_M_CLK_FOUT2_PLL_SRC_STAR	NI_M_CLK_FOUT2_PLL_SRC(0x14)
#define NI_M_CLK_FOUT2_PLL_SRC_PXI10	NI_M_CLK_FOUT2_PLL_SRC(0x1d)
#define NI_M_PLL_CTRL_REG		0x1c6
#define NI_M_PLL_CTRL_VCO_MODE(x)	(((x) & 0x3) << 13)
#define NI_M_PLL_CTRL_VCO_MODE_200_325MHZ NI_M_PLL_CTRL_VCO_MODE(0)
#define NI_M_PLL_CTRL_VCO_MODE_175_225MHZ NI_M_PLL_CTRL_VCO_MODE(1)
#define NI_M_PLL_CTRL_VCO_MODE_100_225MHZ NI_M_PLL_CTRL_VCO_MODE(2)
#define NI_M_PLL_CTRL_VCO_MODE_75_150MHZ  NI_M_PLL_CTRL_VCO_MODE(3)
#define NI_M_PLL_CTRL_ENA		BIT(12)
#define NI_M_PLL_MAX_DIVISOR		0x10
#define NI_M_PLL_CTRL_DIVISOR(x)	(((x) & 0xf) << 8)
#define NI_M_PLL_MAX_MULTIPLIER		0x100
#define NI_M_PLL_CTRL_MULTIPLIER(x)	(((x) & 0xff) << 0)
#define NI_M_PLL_STATUS_REG		0x1c8
#define NI_M_PLL_STATUS_LOCKED		BIT(0)
#define NI_M_PFI_OUT_SEL_REG(x)		(0x1d0 + ((x) * 2))
#define NI_M_PFI_CHAN(_c)		(((_c) % 3) * 5)
#define NI_M_PFI_OUT_SEL(_c, _s)	(((_s) & 0x1f) << NI_M_PFI_CHAN(_c))
#define NI_M_PFI_OUT_SEL_MASK(_c)	(0x1f << NI_M_PFI_CHAN(_c))
#define NI_M_PFI_OUT_SEL_TO_SRC(_c, _b)	(((_b) >> NI_M_PFI_CHAN(_c)) & 0x1f)
#define NI_M_PFI_DI_REG			0x1dc
#define NI_M_PFI_DO_REG			0x1de
#define NI_M_CFG_BYPASS_FIFO_REG	0x218
#define NI_M_CFG_BYPASS_FIFO		BIT(31)
#define NI_M_CFG_BYPASS_AI_POLARITY	BIT(22)
#define NI_M_CFG_BYPASS_AI_DITHER	BIT(21)
#define NI_M_CFG_BYPASS_AI_GAIN(x)	(((x) & 0x7) << 18)
#define NI_M_CFG_BYPASS_AO_CAL(x)	(((x) & 0xf) << 15)
#define NI_M_CFG_BYPASS_AO_CAL_MASK	NI_M_CFG_BYPASS_AO_CAL(0xf)
#define NI_M_CFG_BYPASS_AI_MODE_MUX(x)	(((x) & 0x3) << 13)
#define NI_M_CFG_BYPASS_AI_MODE_MUX_MASK NI_M_CFG_BYPASS_AI_MODE_MUX(3)
#define NI_M_CFG_BYPASS_AI_CAL_NEG(x)	(((x) & 0x7) << 10)
#define NI_M_CFG_BYPASS_AI_CAL_NEG_MASK	NI_M_CFG_BYPASS_AI_CAL_NEG(7)
#define NI_M_CFG_BYPASS_AI_CAL_POS(x)	(((x) & 0x7) << 7)
#define NI_M_CFG_BYPASS_AI_CAL_POS_MASK	NI_M_CFG_BYPASS_AI_CAL_POS(7)
#define NI_M_CFG_BYPASS_AI_CAL_MASK	(NI_M_CFG_BYPASS_AI_CAL_POS_MASK | \
					 NI_M_CFG_BYPASS_AI_CAL_NEG_MASK | \
					 NI_M_CFG_BYPASS_AI_MODE_MUX_MASK | \
					 NI_M_CFG_BYPASS_AO_CAL_MASK)
#define NI_M_CFG_BYPASS_AI_BANK(x)	(((x) & 0xf) << 3)
#define NI_M_CFG_BYPASS_AI_BANK_MASK	NI_M_CFG_BYPASS_AI_BANK(0xf)
#define NI_M_CFG_BYPASS_AI_CHAN(x)	(((x) & 0x7) << 0)
#define NI_M_CFG_BYPASS_AI_CHAN_MASK	NI_M_CFG_BYPASS_AI_CHAN(7)
#define NI_M_SCXI_DIO_ENA_REG		0x21c
#define NI_M_CDI_FIFO_DATA_REG		0x220
#define NI_M_CDO_FIFO_DATA_REG		0x220
#define NI_M_CDIO_STATUS_REG		0x224
#define NI_M_CDIO_STATUS_CDI_OVERFLOW	BIT(20)
#define NI_M_CDIO_STATUS_CDI_OVERRUN	BIT(19)
#define NI_M_CDIO_STATUS_CDI_ERROR	(NI_M_CDIO_STATUS_CDI_OVERFLOW | \
					 NI_M_CDIO_STATUS_CDI_OVERRUN)
#define NI_M_CDIO_STATUS_CDI_FIFO_REQ	BIT(18)
#define NI_M_CDIO_STATUS_CDI_FIFO_FULL	BIT(17)
#define NI_M_CDIO_STATUS_CDI_FIFO_EMPTY	BIT(16)
#define NI_M_CDIO_STATUS_CDO_UNDERFLOW	BIT(4)
#define NI_M_CDIO_STATUS_CDO_OVERRUN	BIT(3)
#define NI_M_CDIO_STATUS_CDO_ERROR	(NI_M_CDIO_STATUS_CDO_UNDERFLOW | \
					 NI_M_CDIO_STATUS_CDO_OVERRUN)
#define NI_M_CDIO_STATUS_CDO_FIFO_REQ	BIT(2)
#define NI_M_CDIO_STATUS_CDO_FIFO_FULL	BIT(1)
#define NI_M_CDIO_STATUS_CDO_FIFO_EMPTY	BIT(0)
#define NI_M_CDIO_CMD_REG		0x224
#define NI_M_CDI_CMD_SW_UPDATE		BIT(20)
#define NI_M_CDO_CMD_SW_UPDATE		BIT(19)
#define NI_M_CDO_CMD_F_E_INT_ENA_CLR	BIT(17)
#define NI_M_CDO_CMD_F_E_INT_ENA_SET	BIT(16)
#define NI_M_CDI_CMD_ERR_INT_CONFIRM	BIT(15)
#define NI_M_CDO_CMD_ERR_INT_CONFIRM	BIT(14)
#define NI_M_CDI_CMD_F_REQ_INT_ENA_CLR	BIT(13)
#define NI_M_CDI_CMD_F_REQ_INT_ENA_SET	BIT(12)
#define NI_M_CDO_CMD_F_REQ_INT_ENA_CLR	BIT(11)
#define NI_M_CDO_CMD_F_REQ_INT_ENA_SET	BIT(10)
#define NI_M_CDI_CMD_ERR_INT_ENA_CLR	BIT(9)
#define NI_M_CDI_CMD_ERR_INT_ENA_SET	BIT(8)
#define NI_M_CDO_CMD_ERR_INT_ENA_CLR	BIT(7)
#define NI_M_CDO_CMD_ERR_INT_ENA_SET	BIT(6)
#define NI_M_CDI_CMD_RESET		BIT(5)
#define NI_M_CDO_CMD_RESET		BIT(4)
#define NI_M_CDI_CMD_ARM		BIT(3)
#define NI_M_CDI_CMD_DISARM		BIT(2)
#define NI_M_CDO_CMD_ARM		BIT(1)
#define NI_M_CDO_CMD_DISARM		BIT(0)
#define NI_M_CDI_MODE_REG		0x228
#define NI_M_CDI_MODE_DATA_LANE(x)	(((x) & 0x3) << 12)
#define NI_M_CDI_MODE_DATA_LANE_MASK	NI_M_CDI_MODE_DATA_LANE(3)
#define NI_M_CDI_MODE_DATA_LANE_0_15	NI_M_CDI_MODE_DATA_LANE(0)
#define NI_M_CDI_MODE_DATA_LANE_16_31	NI_M_CDI_MODE_DATA_LANE(1)
#define NI_M_CDI_MODE_DATA_LANE_0_7	NI_M_CDI_MODE_DATA_LANE(0)
#define NI_M_CDI_MODE_DATA_LANE_8_15	NI_M_CDI_MODE_DATA_LANE(1)
#define NI_M_CDI_MODE_DATA_LANE_16_23	NI_M_CDI_MODE_DATA_LANE(2)
#define NI_M_CDI_MODE_DATA_LANE_24_31	NI_M_CDI_MODE_DATA_LANE(3)
#define NI_M_CDI_MODE_FIFO_MODE		BIT(11)
#define NI_M_CDI_MODE_POLARITY		BIT(10)
#define NI_M_CDI_MODE_HALT_ON_ERROR	BIT(9)
#define NI_M_CDI_MODE_SAMPLE_SRC(x)	(((x) & 0x3f) << 0)
#define NI_M_CDI_MODE_SAMPLE_SRC_MASK	NI_M_CDI_MODE_SAMPLE_SRC(0x3f)
#define NI_M_CDO_MODE_REG		0x22c
#define NI_M_CDO_MODE_DATA_LANE(x)	(((x) & 0x3) << 12)
#define NI_M_CDO_MODE_DATA_LANE_MASK	NI_M_CDO_MODE_DATA_LANE(3)
#define NI_M_CDO_MODE_DATA_LANE_0_15	NI_M_CDO_MODE_DATA_LANE(0)
#define NI_M_CDO_MODE_DATA_LANE_16_31	NI_M_CDO_MODE_DATA_LANE(1)
#define NI_M_CDO_MODE_DATA_LANE_0_7	NI_M_CDO_MODE_DATA_LANE(0)
#define NI_M_CDO_MODE_DATA_LANE_8_15	NI_M_CDO_MODE_DATA_LANE(1)
#define NI_M_CDO_MODE_DATA_LANE_16_23	NI_M_CDO_MODE_DATA_LANE(2)
#define NI_M_CDO_MODE_DATA_LANE_24_31	NI_M_CDO_MODE_DATA_LANE(3)
#define NI_M_CDO_MODE_FIFO_MODE		BIT(11)
#define NI_M_CDO_MODE_POLARITY		BIT(10)
#define NI_M_CDO_MODE_HALT_ON_ERROR	BIT(9)
#define NI_M_CDO_MODE_RETRANSMIT	BIT(8)
#define NI_M_CDO_MODE_SAMPLE_SRC(x)	(((x) & 0x3f) << 0)
#define NI_M_CDO_MODE_SAMPLE_SRC_MASK	NI_M_CDO_MODE_SAMPLE_SRC(0x3f)
#define NI_M_CDI_MASK_ENA_REG		0x230
#define NI_M_CDO_MASK_ENA_REG		0x234
#define NI_M_STATIC_AI_CTRL_REG(x)	((x) ? (0x260 + (x)) : 0x064)
#define NI_M_AO_REF_ATTENUATION_REG(x)	(0x264 + (x))
#define NI_M_AO_REF_ATTENUATION_X5	BIT(0)

#define M_SERIES_EEPROM_SIZE 1024

struct ni_board_struct {
	const char *name;
	int device_id;
	int isapnp_id;

	int n_adchan;
	unsigned int ai_maxdata;

	int ai_fifo_depth;
	unsigned int alwaysdither:1;
	int gainlkup;
	int ai_speed;

	int n_aochan;
	unsigned int ao_maxdata;
	int ao_fifo_depth;
	const struct comedi_lrange *ao_range_table;
	unsigned ao_speed;

	int reg_type;
	unsigned int has_8255:1;
	unsigned int has_32dio_chan:1;

	enum caldac_enum caldac[3];
};

#define MAX_N_CALDACS	34
#define MAX_N_AO_CHAN	8
#define NUM_GPCT	2

struct ni_private {
	unsigned short dio_output;
	unsigned short dio_control;
	int aimode;
	unsigned int ai_calib_source;
	unsigned int ai_calib_source_enabled;
	spinlock_t window_lock;
	spinlock_t soft_reg_copy_lock;
	spinlock_t mite_channel_lock;

	int changain_state;
	unsigned int changain_spec;

	unsigned int caldac_maxdata_list[MAX_N_CALDACS];
	unsigned short caldacs[MAX_N_CALDACS];

	unsigned short ai_cmd2;

	unsigned short ao_conf[MAX_N_AO_CHAN];
	unsigned short ao_mode1;
	unsigned short ao_mode2;
	unsigned short ao_mode3;
	unsigned short ao_cmd1;
	unsigned short ao_cmd2;
	unsigned short ao_trigger_select;

	struct ni_gpct_device *counter_dev;
	unsigned short an_trig_etc_reg;

	unsigned ai_offset[512];

	unsigned long serial_interval_ns;
	unsigned char serial_hw_mode;
	unsigned short clock_and_fout;
	unsigned short clock_and_fout2;

	unsigned short int_a_enable_reg;
	unsigned short int_b_enable_reg;
	unsigned short io_bidirection_pin_reg;
	unsigned short rtsi_trig_direction_reg;
	unsigned short rtsi_trig_a_output_reg;
	unsigned short rtsi_trig_b_output_reg;
	unsigned short pfi_output_select_reg[NUM_PFI_OUTPUT_SELECT_REGS];
	unsigned short ai_ao_select_reg;
	unsigned short g0_g1_select_reg;
	unsigned short cdio_dma_select_reg;

	unsigned clock_ns;
	unsigned clock_source;

	unsigned short pwm_up_count;
	unsigned short pwm_down_count;

	unsigned short ai_fifo_buffer[0x2000];
	uint8_t eeprom_buffer[M_SERIES_EEPROM_SIZE];
	__be32 serial_number;

	struct mite_struct *mite;
	struct mite_channel *ai_mite_chan;
	struct mite_channel *ao_mite_chan;
	struct mite_channel *cdo_mite_chan;
	struct mite_dma_descriptor_ring *ai_mite_ring;
	struct mite_dma_descriptor_ring *ao_mite_ring;
	struct mite_dma_descriptor_ring *cdo_mite_ring;
	struct mite_dma_descriptor_ring *gpct_mite_ring[NUM_GPCT];

	/* ni_pcimio board type flags (based on the boardinfo reg_type) */
	unsigned int is_m_series:1;
	unsigned int is_6xxx:1;
	unsigned int is_611x:1;
	unsigned int is_6143:1;
	unsigned int is_622x:1;
	unsigned int is_625x:1;
	unsigned int is_628x:1;
	unsigned int is_67xx:1;
	unsigned int is_6711:1;
	unsigned int is_6713:1;
};

#endif /* _COMEDI_NI_STC_H */
