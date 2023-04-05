#ifndef _SYS_TSC_ADC_SS_H_
#define _SYS_TSC_ADC_SS_H_

/* SYS TSC ADC SS register set */
typedef struct {

	/* SYS_TSC_ADC_SS_REVISION register bit field */
	union {
		volatile uint32_t REVISION;

		volatile struct {
			uint32_t Y_MINOR : 6; // 5:0
			uint32_t CUSTOM : 2; // 7:6
			uint32_t X_MAJOR : 3; // 10:8
			uint32_t R_RTL : 5; // 15:11
			uint32_t FUNC : 12; // 27:16
			uint32_t rsvd28 : 2; // 29:28
			uint32_t SCHEME : 2; // 31:30
		} REVISION_bit;
	}; // 0x0

	uint8_t rsvd4[12]; // 0x4 - 0xf

	/* SYS_TSC_ADC_SS_SYSCONFIG register bit field */
	union {
		volatile uint32_t SYSCONFIG;

		volatile struct {
			uint32_t rsvd0 : 2; // 1:0
			uint32_t IDLEMODE : 2; // 3:2
			uint32_t rsvd4 : 28; // 31:4
		} SYSCONFIG_bit;
	}; // 0x10

	uint8_t rsvd14[16]; // 0x14 - 0x23

	/* SYS_TSC_ADC_SS_IRQSTATUS_RAW register bit field */
	union {
		volatile uint32_t IRQSTATUS_RAW;

		volatile struct {
			uint32_t HW_PEN_EVENT_ASYNCHRONOUS : 1; // 0
			uint32_t END_OF_SEQUENCE : 1; // 1
			uint32_t FIFO0_THRESHOLD : 1; // 2
			uint32_t FIFO0_OVERRUN : 1; // 3
			uint32_t FIFO0_UNDERFLOW : 1; // 4
			uint32_t FIFO1_THRESHOLD : 1; // 5
			uint32_t FIFO1_OVERRUN : 1; // 6
			uint32_t FIFO1_UNDERFLOW : 1; // 7
			uint32_t OUT_OF_RANGE : 1; // 8
			uint32_t PEN_UP_EVENT : 1; // 9
			uint32_t PEN_IRQ_SYNCHRONIZED : 1; // 10
			uint32_t rsvd11 : 21; // 31:11
		} IRQSTATUS_RAW_bit;
	}; // 0x24

	/* SYS_TSC_ADC_SS_IRQSTATUS register bit field */
	union {
		volatile uint32_t IRQSTATUS;

		volatile struct {
			uint32_t HW_PEN_EVENT_ASYNCHRONOUS : 1; // 0
			uint32_t END_OF_SEQUENCE : 1; // 1
			uint32_t FIFO0_THRESHOLD : 1; // 2
			uint32_t FIFO0_OVERRUN : 1; // 3
			uint32_t FIFO0_UNDERFLOW : 1; // 4
			uint32_t FIFO1_THRESHOLD : 1; // 5
			uint32_t FIFO1_OVERRUN : 1; // 6
			uint32_t FIFO1_UNDERFLOW : 1; // 7
			uint32_t OUT_OF_RANGE : 1; // 8
			uint32_t PEN_UP_EVENT : 1; // 9
			uint32_t HW_PEN_EVENT_SYNCHRONOUS : 1; // 10
			uint32_t rsvd11 : 21; // 31:11
		} IRQSTATUS_bit;
	}; // 0x28

	/* SYS_TSC_ADC_SS_IRQENABLE_SET register bit field */
	union {
		volatile uint32_t IRQENABLE_SET;

		volatile struct {
			uint32_t HW_PEN_EVENT_ASYNCHRONOUS : 1; // 0
			uint32_t END_OF_SEQUENCE : 1; // 1
			uint32_t FIFO0_THRESHOLD : 1; // 2
			uint32_t FIFO0_OVERRUN : 1; // 3
			uint32_t FIFO0_UNDERFLOW : 1; // 4
			uint32_t FIFO1_THRESHOLD : 1; // 5
			uint32_t FIFO1_OVERRUN : 1; // 6
			uint32_t FIFO1_UNDERFLOW : 1; // 7
			uint32_t OUT_OF_RANGE : 1; // 8
			uint32_t PEN_UP_EVENT : 1; // 9
			uint32_t HW_PEN_EVENT_SYNCHRONOUS : 1; // 10
			uint32_t rsvd11 : 21; // 31:11
		} IRQENABLE_SET_bit;
	}; // 0x2c

	/* SYS_TSC_ADC_SS_IRQENABLE_CLR register bit field */
	union {
		volatile uint32_t IRQENABLE_CLR;

		volatile struct {
			uint32_t HW_PEN_EVENT_ASYNCHRONOUS : 1; // 0
			uint32_t END_OF_SEQUENCE : 1; // 1
			uint32_t FIFO0_THRESHOLD : 1; // 2
			uint32_t FIFO0_OVERRUN : 1; // 3
			uint32_t FIFO0_UNDERFLOW : 1; // 4
			uint32_t FIFO1_THRESHOLD : 1; // 5
			uint32_t FIFO1_OVERRUN : 1; // 6
			uint32_t FIFO1_UNDERFLOW : 1; // 7
			uint32_t OUT_OF_RANGE : 1; // 8
			uint32_t PEN_UP_EVENT : 1; // 9
			uint32_t HW_PEN_EVENT_SYNCHRONOUS : 1; // 10
			uint32_t rsvd11 : 21; // 31:11
		} IRQENABLE_CLR_bit;
	}; // 0x30

	/* SYS_TSC_ADC_SS_IRQWAKEUP register bit field */
	union {
		volatile uint32_t IRQWAKEUP;

		volatile struct {
			uint32_t WAKEEN0 : 1; // 0
			uint32_t rsvd1 : 31; // 31:1
		} IRQWAKEUP_bit;
	}; // 0x34

	/* SYS_TSC_ADC_SS_DMAENABLE_SET register bit field */
	union {
		volatile uint32_t DMAENABLE_SET;

		volatile struct {
			uint32_t ENABLE_0 : 1; // 0
			uint32_t ENABLE_1 : 1; // 1
			uint32_t rsvd2 : 30; // 31:2
		} DMAENABLE_SET_bit;
	}; // 0x38

	/* SYS_TSC_ADC_SS_DMAENABLE_CLR register bit field */
	union {
		volatile uint32_t DMAENABLE_CLR;

		volatile struct {
			uint32_t ENABLE_0 : 1; // 0
			uint32_t ENABLE_1 : 1; // 1
			uint32_t rsvd2 : 30; // 31:2
		} DMAENABLE_CLR_bit;
	}; // 0x3c

	/* SYS_TSC_ADC_SS_CTRL register bit field */
	union {
		volatile uint32_t CTRL;

		volatile struct {
			uint32_t ENABLE : 1; // 0
			uint32_t STEP_ID_TAG : 1; // 1
			uint32_t STEPCONFIG_WRITEPROTECT_N_ACTIVE_LOW : 1; // 2
			uint32_t ADC_BIAS_SELECT : 1; // 3
			uint32_t POWER_DOWN : 1; // 4
			uint32_t AFE_PEN_CTRL : 2; // 6:5
			uint32_t TOUCH_SCREEN_ENABLE : 1; // 7
			uint32_t HW_EVENT_MAPPING : 1; // 8
			uint32_t HW_PREEMPT : 1; // 9
			uint32_t rsvd10 : 22; // 31:10
		} CTRL_bit;
	}; // 0x40

	/* SYS_TSC_ADC_SS_ADCSTAT register bit field */
	union {
		volatile uint32_t ADCSTAT;

		volatile struct {
			uint32_t STEP_ID : 5; // 4:0
			uint32_t FSM_BUSY : 1; // 5
			uint32_t PEN_IRQ0 : 1; // 6
			uint32_t PEN_IRQ1 : 1; // 7
			uint32_t rsvd8 : 24; // 31:8
		} ADCSTAT_bit;
	}; // 0x44

	/* SYS_TSC_ADC_SS_ADCRANGE register bit field */
	union {
		volatile uint32_t ADCRANGE;

		volatile struct {
			uint32_t LOW_RANGE_DATA : 12; // 11:0
			uint32_t rsvd12 : 4; // 15:12
			uint32_t HIGH_RANGE_DATA : 12; // 27:16
			uint32_t rsvd28 : 4; // 31:28
		} ADCRANGE_bit;
	}; // 0x48

	/* SYS_TSC_ADC_SS_ADC_CLKDIV register bit field */
	union {
		volatile uint32_t ADC_CLKDIV;

		volatile struct {
			uint32_t ADC_CLKDIV : 16; // 15:0
			uint32_t rsvd16 : 16; // 31:16
		} ADC_CLKDIV_bit;
	}; // 0x4c

	/* SYS_TSC_ADC_SS_ADC_MISC register bit field */
	union {
		volatile uint32_t ADC_MISC;

		volatile struct {
			uint32_t AFE_SPARE_INPUT : 4; // 3:0
			uint32_t AFE_SPARE_OUTPUT : 4; // 7:4
			uint32_t rsvd8 : 24; // 31:8
		} ADC_MISC_bit;
	}; // 0x50

	/* SYS_TSC_ADC_SS_STEPENABLE register bit field */
	union {
		volatile uint32_t STEPENABLE;

		volatile struct {
			uint32_t TS_CHARGE : 1; // 0
			uint32_t STEP1 : 1; // 1
			uint32_t STEP2 : 1; // 2
			uint32_t STEP3 : 1; // 3
			uint32_t STEP4 : 1; // 4
			uint32_t STEP5 : 1; // 5
			uint32_t STEP6 : 1; // 6
			uint32_t STEP7 : 1; // 7
			uint32_t STEP8 : 1; // 8
			uint32_t STEP9 : 1; // 9
			uint32_t STEP10 : 1; // 10
			uint32_t STEP11 : 1; // 11
			uint32_t STEP12 : 1; // 12
			uint32_t STEP13 : 1; // 13
			uint32_t STEP14 : 1; // 14
			uint32_t STEP15 : 1; // 15
			uint32_t STEP16 : 1; // 16
			uint32_t rsvd17 : 15; // 31:17
		} STEPENABLE_bit;
	}; // 0x54

	/* SYS_TSC_ADC_SS_IDLECONFIG register bit field */
	union {
		volatile uint32_t IDLECONFIG;

		volatile struct {
			uint32_t rsvd0 : 5; // 4:0
			uint32_t XPPSW_SWC : 1; // 5
			uint32_t XNNSW_SWC : 1; // 6
			uint32_t YPPSW_SWC : 1; // 7
			uint32_t YNNSW_SWC : 1; // 8
			uint32_t XNPSW_SWC : 1; // 9
			uint32_t YPNSW_SWC : 1; // 10
			uint32_t WPNSW_SWC : 1; // 11
			uint32_t SEL_RFP_SWC_2_0 : 3; // 14:12
			uint32_t SEL_INM_SWC_3_0 : 4; // 18:15
			uint32_t SEL_INP_SWC_3_0 : 4; // 22:19
			uint32_t SEL_RFM_SWC_1_0 : 2; // 24:23
			uint32_t DIFF_CNTRL : 1; // 25
			uint32_t rsvd26 : 6; // 31:26
		} IDLECONFIG_bit;
	}; // 0x58

	/* SYS_TSC_ADC_SS_TS_CHARGE_STEPCONFIG register bit field */
	union {
		volatile uint32_t TS_CHARGE_STEPCONFIG;

		volatile struct {
			uint32_t rsvd0 : 5; // 4:0
			uint32_t XPPSW_SWC : 1; // 5
			uint32_t XNNSW_SWC : 1; // 6
			uint32_t YPPSW_SWC : 1; // 7
			uint32_t YNNSW_SWC : 1; // 8
			uint32_t XNPSW_SWC : 1; // 9
			uint32_t YPNSW_SWC : 1; // 10
			uint32_t WPNSW_SWC : 1; // 11
			uint32_t SEL_RFP_SWC_2_0 : 3; // 14:12
			uint32_t SEL_INM_SWM3_0 : 4; // 18:15
			uint32_t SEL_INP_SWC_3_0 : 4; // 22:19
			uint32_t SEL_RFM_SWC_1_0 : 2; // 24:23
			uint32_t DIFF_CNTRL : 1; // 25
			uint32_t rsvd26 : 6; // 31:26
		} TS_CHARGE_STEPCONFIG_bit;
	}; // 0x5c

	/* SYS_TSC_ADC_SS_TS_CHARGE_DELAY register bit field */
	union {
		volatile uint32_t TS_CHARGE_DELAY;

		volatile struct {
			uint32_t OPENDELAY : 18; // 17:0
			uint32_t rsvd18 : 14; // 31:18
		} TS_CHARGE_DELAY_bit;
	}; // 0x60

	/* SYS_TSC_ADC_SS_STEPCONFIG1 register bit field */
	union {
		volatile uint32_t STEPCONFIG1;

		volatile struct {
			uint32_t MODE : 2; // 1:0
			uint32_t AVERAGING : 3; // 4:2
			uint32_t XPPSW_SWC : 1; // 5
			uint32_t XNNSW_SWC : 1; // 6
			uint32_t YPPSW_SWC : 1; // 7
			uint32_t YNNSW_SWC : 1; // 8
			uint32_t XNPSW_SWC : 1; // 9
			uint32_t YPNSW_SWC : 1; // 10
			uint32_t WPNSW_SWC : 1; // 11
			uint32_t SEL_RFP_SWC_2_0 : 3; // 14:12
			uint32_t SEL_INM_SWC_3_0 : 4; // 18:15
			uint32_t SEL_INP_SWC_3_0 : 4; // 22:19
			uint32_t SEL_RFM_SWC_1_0 : 2; // 24:23
			uint32_t DIFF_CNTRL : 1; // 25
			uint32_t FIFO_SELECT : 1; // 26
			uint32_t RANGE_CHECK : 1; // 27
			uint32_t rsvd28 : 4; // 31:28
		} STEPCONFIG1_bit;
	}; // 0x64

	/* SYS_TSC_ADC_SS_STEPDELAY1 register bit field */
	union {
		volatile uint32_t STEPDELAY1;

		volatile struct {
			uint32_t OPENDELAY : 18; // 17:0
			uint32_t rsvd18 : 6; // 23:18
			uint32_t SAMPLEDELAY : 8; // 31:24
		} STEPDELAY1_bit;
	}; // 0x68

	/* SYS_TSC_ADC_SS_STEPCONFIG2 register bit field */
	union {
		volatile uint32_t STEPCONFIG2;

		volatile struct {
			uint32_t MODE : 2; // 1:0
			uint32_t AVERAGING : 3; // 4:2
			uint32_t XPPSW_SWC : 1; // 5
			uint32_t XNNSW_SWC : 1; // 6
			uint32_t YPPSW_SWC : 1; // 7
			uint32_t YNNSW_SWC : 1; // 8
			uint32_t XNPSW_SWC : 1; // 9
			uint32_t YPNSW_SWC : 1; // 10
			uint32_t WPNSW_SWC : 1; // 11
			uint32_t SEL_RFP_SWC_2_0 : 3; // 14:12
			uint32_t SEL_INM_SWC_3_0 : 4; // 18:15
			uint32_t SEL_INP_SWC_3_0 : 4; // 22:19
			uint32_t SEL_RFM_SWC_1_0 : 2; // 24:23
			uint32_t DIFF_CNTRL : 1; // 25
			uint32_t FIFO_SELECT : 1; // 26
			uint32_t RANGE_CHECK : 1; // 27
			uint32_t rsvd28 : 4; // 31:28
		} STEPCONFIG2_bit;
	}; // 0x6c

	/* SYS_TSC_ADC_SS_STEPDELAY2 register bit field */
	union {
		volatile uint32_t STEPDELAY2;

		volatile struct {
			uint32_t OPENDELAY : 18; // 17:0
			uint32_t rsvd18 : 6; // 23:18
			uint32_t SAMPLEDELAY : 8; // 31:24
		} STEPDELAY2_bit;
	}; // 0x70

	/* SYS_TSC_ADC_SS_STEPCONFIG3 register bit field */
	union {
		volatile uint32_t STEPCONFIG3;

		volatile struct {
			uint32_t MODE : 2; // 1:0
			uint32_t AVERAGING : 3; // 4:2
			uint32_t XPPSW_SWC : 1; // 5
			uint32_t XNNSW_SWC : 1; // 6
			uint32_t YPPSW_SWC : 1; // 7
			uint32_t YNNSW_SWC : 1; // 8
			uint32_t XNPSW_SWC : 1; // 9
			uint32_t YPNSW_SWC : 1; // 10
			uint32_t WPNSW_SWC : 1; // 11
			uint32_t SEL_RFP_SWC_2_0 : 3; // 14:12
			uint32_t SEL_INM_SWC_3_0 : 4; // 18:15
			uint32_t SEL_INP_SWC_3_0 : 4; // 22:19
			uint32_t SEL_RFM_SWC_1_0 : 2; // 24:23
			uint32_t DIFF_CNTRL : 1; // 25
			uint32_t FIFO_SELECT : 1; // 26
			uint32_t RANGE_CHECK : 1; // 27
			uint32_t rsvd28 : 4; // 31:28
		} STEPCONFIG3_bit;
	}; // 0x74

	/* SYS_TSC_ADC_SS_STEPDELAY3 register bit field */
	union {
		volatile uint32_t STEPDELAY3;

		volatile struct {
			uint32_t OPENDELAY : 18; // 17:0
			uint32_t rsvd18 : 6; // 23:18
			uint32_t SAMPLEDELAY : 8; // 31:24
		} STEPDELAY3_bit;
	}; // 0x78

	/* SYS_TSC_ADC_SS_STEPCONFIG4 register bit field */
	union {
		volatile uint32_t STEPCONFIG4;

		volatile struct {
			uint32_t MODE : 2; // 1:0
			uint32_t AVERAGING : 3; // 4:2
			uint32_t XPPSW_SWC : 1; // 5
			uint32_t XNNSW_SWC : 1; // 6
			uint32_t YPPSW_SWC : 1; // 7
			uint32_t YNNSW_SWC : 1; // 8
			uint32_t XNPSW_SWC : 1; // 9
			uint32_t YPNSW_SWC : 1; // 10
			uint32_t WPNSW_SWC : 1; // 11
			uint32_t SEL_RFP_SWC_2_0 : 3; // 14:12
			uint32_t SEL_INM_SWC_3_0 : 4; // 18:15
			uint32_t SEL_INP_SWC_3_0 : 4; // 22:19
			uint32_t SEL_RFM_SWC_1_0 : 2; // 24:23
			uint32_t DIFF_CNTRL : 1; // 25
			uint32_t FIFO_SELECT : 1; // 26
			uint32_t RANGE_CHECK : 1; // 27
			uint32_t rsvd28 : 4; // 31:28
		} STEPCONFIG4_bit;
	}; // 0x7c

	/* SYS_TSC_ADC_SS_STEPDELAY4 register bit field */
	union {
		volatile uint32_t STEPDELAY4;

		volatile struct {
			uint32_t OPENDELAY : 18; // 17:0
			uint32_t rsvd18 : 6; // 23:18
			uint32_t SAMPLEDELAY : 8; // 31:24
		} STEPDELAY4_bit;
	}; // 0x80

	/* SYS_TSC_ADC_SS_STEPCONFIG5 register bit field */
	union {
		volatile uint32_t STEPCONFIG5;

		volatile struct {
			uint32_t MODE : 2; // 1:0
			uint32_t AVERAGING : 3; // 4:2
			uint32_t XPPSW_SWC : 1; // 5
			uint32_t XNNSW_SWC : 1; // 6
			uint32_t YPPSW_SWC : 1; // 7
			uint32_t YNNSW_SWC : 1; // 8
			uint32_t XNPSW_SWC : 1; // 9
			uint32_t YPNSW_SWC : 1; // 10
			uint32_t WPNSW_SWC : 1; // 11
			uint32_t SEL_RFP_SWC_2_0 : 3; // 14:12
			uint32_t SEL_INM_SWC_3_0 : 4; // 18:15
			uint32_t SEL_INP_SWC_3_0 : 4; // 22:19
			uint32_t SEL_RFM_SWC_1_0 : 2; // 24:23
			uint32_t DIFF_CNTRL : 1; // 25
			uint32_t FIFO_SELECT : 1; // 26
			uint32_t RANGE_CHECK : 1; // 27
			uint32_t rsvd28 : 4; // 31:28
		} STEPCONFIG5_bit;
	}; // 0x84

	/* SYS_TSC_ADC_SS_STEPDELAY5 register bit field */
	union {
		volatile uint32_t STEPDELAY5;

		volatile struct {
			uint32_t OPENDELAY : 18; // 17:0
			uint32_t rsvd18 : 6; // 23:18
			uint32_t SAMPLEDELAY : 8; // 31:24
		} STEPDELAY5_bit;
	}; // 0x88

	/* SYS_TSC_ADC_SS_STEPCONFIG6 register bit field */
	union {
		volatile uint32_t STEPCONFIG6;

		volatile struct {
			uint32_t MODE : 2; // 1:0
			uint32_t AVERAGING : 3; // 4:2
			uint32_t XPPSW_SWC : 1; // 5
			uint32_t XNNSW_SWC : 1; // 6
			uint32_t YPPSW_SWC : 1; // 7
			uint32_t YNNSW_SWC : 1; // 8
			uint32_t XNPSW_SWC : 1; // 9
			uint32_t YPNSW_SWC : 1; // 10
			uint32_t WPNSW_SWC : 1; // 11
			uint32_t SEL_RFP_SWC_2_0 : 3; // 14:12
			uint32_t SEL_INM_SWC_3_0 : 4; // 18:15
			uint32_t SEL_INP_SWC_3_0 : 4; // 22:19
			uint32_t SEL_RFM_SWC_1_0 : 2; // 24:23
			uint32_t DIFF_CNTRL : 1; // 25
			uint32_t FIFO_SELECT : 1; // 26
			uint32_t RANGE_CHECK : 1; // 27
			uint32_t rsvd28 : 4; // 31:28
		} STEPCONFIG6_bit;
	}; // 0x8c

	/* SYS_TSC_ADC_SS_STEPDELAY6 register bit field */
	union {
		volatile uint32_t STEPDELAY6;

		volatile struct {
			uint32_t OPENDELAY : 18; // 17:0
			uint32_t rsvd18 : 6; // 23:18
			uint32_t SAMPLEDELAY : 8; // 31:24
		} STEPDELAY6_bit;
	}; // 0x90

	/* SYS_TSC_ADC_SS_STEPCONFIG7 register bit field */
	union {
		volatile uint32_t STEPCONFIG7;

		volatile struct {
			uint32_t MODE : 2; // 1:0
			uint32_t AVERAGING : 3; // 4:2
			uint32_t XPPSW_SWC : 1; // 5
			uint32_t XNNSW_SWC : 1; // 6
			uint32_t YPPSW_SWC : 1; // 7
			uint32_t YNNSW_SWC : 1; // 8
			uint32_t XNPSW_SWC : 1; // 9
			uint32_t YPNSW_SWC : 1; // 10
			uint32_t WPNSW_SWC : 1; // 11
			uint32_t SEL_RFP_SWC_2_0 : 3; // 14:12
			uint32_t SEL_INM_SWC_3_0 : 4; // 18:15
			uint32_t SEL_INP_SWC_3_0 : 4; // 22:19
			uint32_t SEL_RFM_SWC_1_0 : 2; // 24:23
			uint32_t DIFF_CNTRL : 1; // 25
			uint32_t FIFO_SELECT : 1; // 26
			uint32_t RANGE_CHECK : 1; // 27
			uint32_t rsvd28 : 4; // 31:28
		} STEPCONFIG7_bit;
	}; // 0x94

	/* SYS_TSC_ADC_SS_STEPDELAY7 register bit field */
	union {
		volatile uint32_t STEPDELAY7;

		volatile struct {
			uint32_t OPENDELAY : 18; // 17:0
			uint32_t rsvd18 : 6; // 23:18
			uint32_t SAMPLEDELAY : 8; // 31:24
		} STEPDELAY7_bit;
	}; // 0x98

	/* SYS_TSC_ADC_SS_STEPCONFIG8 register bit field */
	union {
		volatile uint32_t STEPCONFIG8;

		volatile struct {
			uint32_t MODE : 2; // 1:0
			uint32_t AVERAGING : 3; // 4:2
			uint32_t XPPSW_SWC : 1; // 5
			uint32_t XNNSW_SWC : 1; // 6
			uint32_t YPPSW_SWC : 1; // 7
			uint32_t YNNSW_SWC : 1; // 8
			uint32_t XNPSW_SWC : 1; // 9
			uint32_t YPNSW_SWC : 1; // 10
			uint32_t WPNSW_SWC : 1; // 11
			uint32_t SEL_RFP_SWC_2_0 : 3; // 14:12
			uint32_t SEL_INM_SWC_3_0 : 4; // 18:15
			uint32_t SEL_INP_SWC_3_0 : 4; // 22:19
			uint32_t SEL_RFM_SWC_1_0 : 2; // 24:23
			uint32_t DIFF_CNTRL : 1; // 25
			uint32_t FIFO_SELECT : 1; // 26
			uint32_t RANGE_CHECK : 1; // 27
			uint32_t rsvd28 : 4; // 31:28
		} STEPCONFIG8_bit;
	}; // 0x9c

	/* SYS_TSC_ADC_SS_STEPDELAY8 register bit field */
	union {
		volatile uint32_t STEPDELAY8;

		volatile struct {
			uint32_t OPENDELAY : 18; // 17:0
			uint32_t rsvd18 : 6; // 23:18
			uint32_t SAMPLEDELAY : 8; // 31:24
		} STEPDELAY8_bit;
	}; // 0xa0

	/* SYS_TSC_ADC_SS_STEPCONFIG9 register bit field */
	union {
		volatile uint32_t STEPCONFIG9;

		volatile struct {
			uint32_t MODE : 2; // 1:0
			uint32_t AVERAGING : 3; // 4:2
			uint32_t XPPSW_SWC : 1; // 5
			uint32_t XNNSW_SWC : 1; // 6
			uint32_t YPPSW_SWC : 1; // 7
			uint32_t YNNSW_SWC : 1; // 8
			uint32_t XNPSW_SWC : 1; // 9
			uint32_t YPNSW_SWC : 1; // 10
			uint32_t WPNSW_SWC : 1; // 11
			uint32_t SEL_RFP_SWC_2_0 : 3; // 14:12
			uint32_t SEL_INM_SWC_3_0 : 4; // 18:15
			uint32_t SEL_INP_SWC_3_0 : 4; // 22:19
			uint32_t SEL_RFM_SWC_1_0 : 2; // 24:23
			uint32_t DIFF_CNTRL : 1; // 25
			uint32_t FIFO_SELECT : 1; // 26
			uint32_t RANGE_CHECK : 1; // 27
			uint32_t rsvd28 : 4; // 31:28
		} STEPCONFIG9_bit;
	}; // 0xa4

	/* SYS_TSC_ADC_SS_STEPDELAY9 register bit field */
	union {
		volatile uint32_t STEPDELAY9;

		volatile struct {
			uint32_t OPENDELAY : 18; // 17:0
			uint32_t rsvd18 : 6; // 23:18
			uint32_t SAMPLEDELAY : 8; // 31:24
		} STEPDELAY9_bit;
	}; // 0xa8

	/* SYS_TSC_ADC_SS_STEPCONFIG10 register bit field */
	union {
		volatile uint32_t STEPCONFIG10;

		volatile struct {
			uint32_t MODE : 2; // 1:0
			uint32_t AVERAGING : 3; // 4:2
			uint32_t XPPSW_SWC : 1; // 5
			uint32_t XNNSW_SWC : 1; // 6
			uint32_t YPPSW_SWC : 1; // 7
			uint32_t YNNSW_SWC : 1; // 8
			uint32_t XNPSW_SWC : 1; // 9
			uint32_t YPNSW_SWC : 1; // 10
			uint32_t WPNSW_SWC : 1; // 11
			uint32_t SEL_RFP_SWC_2_0 : 3; // 14:12
			uint32_t SEL_INM_SWC_3_0 : 4; // 18:15
			uint32_t SEL_INP_SWC_3_0 : 4; // 22:19
			uint32_t SEL_RFM_SWC_1_0 : 2; // 24:23
			uint32_t DIFF_CNTRL : 1; // 25
			uint32_t FIFO_SELECT : 1; // 26
			uint32_t RANGE_CHECK : 1; // 27
			uint32_t rsvd28 : 4; // 31:28
		} STEPCONFIG10_bit;
	}; // 0xac

	/* SYS_TSC_ADC_SS_STEPDELAY10 register bit field */
	union {
		volatile uint32_t STEPDELAY10;

		volatile struct {
			uint32_t OPENDELAY : 18; // 17:0
			uint32_t rsvd18 : 6; // 23:18
			uint32_t SAMPLEDELAY : 8; // 31:24
		} STEPDELAY10_bit;
	}; // 0xb0

	/* SYS_TSC_ADC_SS_STEPCONFIG11 register bit field */
	union {
		volatile uint32_t STEPCONFIG11;

		volatile struct {
			uint32_t MODE : 2; // 1:0
			uint32_t AVERAGING : 3; // 4:2
			uint32_t XPPSW_SWC : 1; // 5
			uint32_t XNNSW_SWC : 1; // 6
			uint32_t YPPSW_SWC : 1; // 7
			uint32_t YNNSW_SWC : 1; // 8
			uint32_t XNPSW_SWC : 1; // 9
			uint32_t YPNSW_SWC : 1; // 10
			uint32_t WPNSW_SWC : 1; // 11
			uint32_t SEL_RFP_SWC_2_0 : 3; // 14:12
			uint32_t SEL_INM_SWC_3_0 : 4; // 18:15
			uint32_t SEL_INP_SWC_3_0 : 4; // 22:19
			uint32_t SEL_RFM_SWC_1_0 : 2; // 24:23
			uint32_t DIFF_CNTRL : 1; // 25
			uint32_t FIFO_SELECT : 1; // 26
			uint32_t RANGE_CHECK : 1; // 27
			uint32_t rsvd28 : 4; // 31:28
		} STEPCONFIG11_bit;
	}; // 0xb4

	/* SYS_TSC_ADC_SS_STEPDELAY11 register bit field */
	union {
		volatile uint32_t STEPDELAY11;

		volatile struct {
			uint32_t OPENDELAY : 18; // 17:0
			uint32_t rsvd18 : 6; // 23:18
			uint32_t SAMPLEDELAY : 8; // 31:24
		} STEPDELAY11_bit;
	}; // 0xb8

	/* SYS_TSC_ADC_SS_STEPCONFIG12 register bit field */
	union {
		volatile uint32_t STEPCONFIG12;

		volatile struct {
			uint32_t MODE : 2; // 1:0
			uint32_t AVERAGING : 3; // 4:2
			uint32_t XPPSW_SWC : 1; // 5
			uint32_t XNNSW_SWC : 1; // 6
			uint32_t YPPSW_SWC : 1; // 7
			uint32_t YNNSW_SWC : 1; // 8
			uint32_t XNPSW_SWC : 1; // 9
			uint32_t YPNSW_SWC : 1; // 10
			uint32_t WPNSW_SWC : 1; // 11
			uint32_t SEL_RFP_SWC_2_0 : 3; // 14:12
			uint32_t SEL_INM_SWC_3_0 : 4; // 18:15
			uint32_t SEL_INP_SWC_3_0 : 4; // 22:19
			uint32_t SEL_RFM_SWC_1_0 : 2; // 24:23
			uint32_t DIFF_CNTRL : 1; // 25
			uint32_t FIFO_SELECT : 1; // 26
			uint32_t RANGE_CHECK : 1; // 27
			uint32_t rsvd28 : 4; // 31:28
		} STEPCONFIG12_bit;
	}; // 0xbc

	/* SYS_TSC_ADC_SS_STEPDELAY12 register bit field */
	union {
		volatile uint32_t STEPDELAY12;

		volatile struct {
			uint32_t OPENDELAY : 18; // 17:0
			uint32_t rsvd18 : 6; // 23:18
			uint32_t SAMPLEDELAY : 8; // 31:24
		} STEPDELAY12_bit;
	}; // 0xc0

	/* SYS_TSC_ADC_SS_STEPCONFIG13 register bit field */
	union {
		volatile uint32_t STEPCONFIG13;

		volatile struct {
			uint32_t MODE : 2; // 1:0
			uint32_t AVERAGING : 3; // 4:2
			uint32_t XPPSW_SWC : 1; // 5
			uint32_t XNNSW_SWC : 1; // 6
			uint32_t YPPSW_SWC : 1; // 7
			uint32_t YNNSW_SWC : 1; // 8
			uint32_t XNPSW_SWC : 1; // 9
			uint32_t YPNSW_SWC : 1; // 10
			uint32_t WPNSW_SWC : 1; // 11
			uint32_t SEL_RFP_SWC_2_0 : 3; // 14:12
			uint32_t SEL_INM_SWC_3_0 : 4; // 18:15
			uint32_t SEL_INP_SWC_3_0 : 4; // 22:19
			uint32_t SEL_RFM_SWC_1_0 : 2; // 24:23
			uint32_t DIFF_CNTRL : 1; // 25
			uint32_t FIFO_SELECT : 1; // 26
			uint32_t RANGE_CHECK : 1; // 27
			uint32_t rsvd28 : 4; // 31:28
		} STEPCONFIG13_bit;
	}; // 0xc4

	/* SYS_TSC_ADC_SS_STEPDELAY13 register bit field */
	union {
		volatile uint32_t STEPDELAY13;

		volatile struct {
			uint32_t OPENDELAY : 18; // 17:0
			uint32_t rsvd18 : 6; // 23:18
			uint32_t SAMPLEDELAY : 8; // 31:24
		} STEPDELAY13_bit;
	}; // 0xc8

	/* SYS_TSC_ADC_SS_STEPCONFIG14 register bit field */
	union {
		volatile uint32_t STEPCONFIG14;

		volatile struct {
			uint32_t MODE : 2; // 1:0
			uint32_t AVERAGING : 3; // 4:2
			uint32_t XPPSW_SWC : 1; // 5
			uint32_t XNNSW_SWC : 1; // 6
			uint32_t YPPSW_SWC : 1; // 7
			uint32_t YNNSW_SWC : 1; // 8
			uint32_t XNPSW_SWC : 1; // 9
			uint32_t YPNSW_SWC : 1; // 10
			uint32_t WPNSW_SWC : 1; // 11
			uint32_t SEL_RFP_SWC_2_0 : 3; // 14:12
			uint32_t SEL_INM_SWC_3_0 : 4; // 18:15
			uint32_t SEL_INP_SWC_3_0 : 4; // 22:19
			uint32_t SEL_RFM_SWC_1_0 : 2; // 24:23
			uint32_t DIFF_CNTRL : 1; // 25
			uint32_t FIFO_SELECT : 1; // 26
			uint32_t RANGE_CHECK : 1; // 27
			uint32_t rsvd28 : 4; // 31:28
		} STEPCONFIG14_bit;
	}; // 0xcc

	/* SYS_TSC_ADC_SS_STEPDELAY14 register bit field */
	union {
		volatile uint32_t STEPDELAY14;

		volatile struct {
			uint32_t OPENDELAY : 18; // 17:0
			uint32_t rsvd18 : 6; // 23:18
			uint32_t SAMPLEDELAY : 8; // 31:24
		} STEPDELAY14_bit;
	}; // 0xd0

	/* SYS_TSC_ADC_SS_STEPCONFIG15 register bit field */
	union {
		volatile uint32_t STEPCONFIG15;

		volatile struct {
			uint32_t MODE : 2; // 1:0
			uint32_t AVERAGING : 3; // 4:2
			uint32_t XPPSW_SWC : 1; // 5
			uint32_t XNNSW_SWC : 1; // 6
			uint32_t YPPSW_SWC : 1; // 7
			uint32_t YNNSW_SWC : 1; // 8
			uint32_t XNPSW_SWC : 1; // 9
			uint32_t YPNSW_SWC : 1; // 10
			uint32_t WPNSW_SWC : 1; // 11
			uint32_t SEL_RFP_SWC_2_0 : 3; // 14:12
			uint32_t SEL_INM_SWC_3_0 : 4; // 18:15
			uint32_t SEL_INP_SWC_3_0 : 4; // 22:19
			uint32_t SEL_RFM_SWC_1_0 : 2; // 24:23
			uint32_t DIFF_CNTRL : 1; // 25
			uint32_t FIFO_SELECT : 1; // 26
			uint32_t RANGE_CHECK : 1; // 27
			uint32_t rsvd28 : 4; // 31:28
		} STEPCONFIG15_bit;
	}; // 0xd4

	/* SYS_TSC_ADC_SS_STEPDELAY15 register bit field */
	union {
		volatile uint32_t STEPDELAY15;

		volatile struct {
			uint32_t OPENDELAY : 18; // 17:0
			uint32_t rsvd18 : 6; // 23:18
			uint32_t SAMPLEDELAY : 8; // 31:24
		} STEPDELAY15_bit;
	}; // 0xd8

	/* SYS_TSC_ADC_SS_STEPCONFIG16 register bit field */
	union {
		volatile uint32_t STEPCONFIG16;

		volatile struct {
			uint32_t MODE : 2; // 1:0
			uint32_t AVERAGING : 3; // 4:2
			uint32_t XPPSW_SWC : 1; // 5
			uint32_t XNNSW_SWC : 1; // 6
			uint32_t YPPSW_SWC : 1; // 7
			uint32_t YNNSW_SWC : 1; // 8
			uint32_t XNPSW_SWC : 1; // 9
			uint32_t YPNSW_SWC : 1; // 10
			uint32_t WPNSW_SWC : 1; // 11
			uint32_t SEL_RFP_SWC_2_0 : 3; // 14:12
			uint32_t SEL_INM_SWC_3_0 : 4; // 18:15
			uint32_t SEL_INP_SWC_3_0 : 4; // 22:19
			uint32_t SEL_RFM_SWC_1_0 : 2; // 24:23
			uint32_t DIFF_CNTRL : 1; // 25
			uint32_t FIFO_SELECT : 1; // 26
			uint32_t RANGE_CHECK : 1; // 27
			uint32_t rsvd28 : 4; // 31:28
		} STEPCONFIG16_bit;
	}; // 0xdc

	/* SYS_TSC_ADC_SS_STEPDELAY16 register bit field */
	union {
		volatile uint32_t STEPDELAY16;

		volatile struct {
			uint32_t OPENDELAY : 18; // 17:0
			uint32_t rsvd18 : 6; // 23:18
			uint32_t SAMPLEDELAY : 8; // 31:24
		} STEPDELAY16_bit;
	}; // 0xe0

	/* SYS_TSC_ADC_SS_FIFO0COUNT register bit field */
	union {
		volatile uint32_t FIFO0COUNT;

		volatile struct {
			uint32_t WORDS_IN_FIFO0 : 7; // 6:0
			uint32_t rsvd7 : 25; // 31:7
		} FIFO0COUNT_bit;
	}; // 0xe4

	/* SYS_TSC_ADC_SS_FIFO0THRESHOLD register bit field */
	union {
		volatile uint32_t FIFO0THRESHOLD;

		volatile struct {
			uint32_t FIFO0_THRESHOLD_LEVEL : 6; // 5:0
			uint32_t rsvd6 : 26; // 31:6
		} FIFO0THRESHOLD_bit;
	}; // 0xe8

	/* SYS_TSC_ADC_SS_DMA0REQ register bit field */
	union {
		volatile uint32_t DMA0REQ;

		volatile struct {
			uint32_t DMA_REQUEST_LEVEL : 6; // 5:0
			uint32_t rsvd6 : 26; // 31:6
		} DMA0REQ_bit;
	}; // 0xec

	/* SYS_TSC_ADC_SS_FIFO1COUNT register bit field */
	union {
		volatile uint32_t FIFO1COUNT;

		volatile struct {
			uint32_t WORDS_IN_FIFO0 : 7; // 6:0
			uint32_t rsvd7 : 25; // 31:7
		} FIFO1COUNT_bit;
	}; // 0xf0

	/* SYS_TSC_ADC_SS_FIFO1THRESHOLD register bit field */
	union {
		volatile uint32_t FIFO1THRESHOLD;

		volatile struct {
			uint32_t FIFO0_THRESHOLD_LEVEL : 6; // 5:0
			uint32_t rsvd6 : 26; // 31:6
		} FIFO1THRESHOLD_bit;
	}; // 0xf4

	/* SYS_TSC_ADC_SS_DMA1REQ register bit field */
	union {
		volatile uint32_t DMA1REQ;

		volatile struct {
			uint32_t DMA_REQUEST_LEVEL : 6; // 5:0
			uint32_t rsvd6 : 26; // 31:6
		} DMA1REQ_bit;
	}; // 0xf8

	uint8_t rsvdfc[4]; // 0xfc - 0xff

	/* SYS_TSC_ADC_SS_FIFO0DATA register bit field */
	union {
		volatile uint32_t FIFO0DATA;

		volatile struct {
			uint32_t ADCDATA : 12; // 11:0
			uint32_t rsvd12 : 4; // 15:12
			uint32_t ADCCHNLID : 4; // 19:16
			uint32_t rsvd20 : 12; // 31:20
		} FIFO0DATA_bit;
	}; // 0x100

	uint8_t rsvd104[252]; // 0x104 - 0x1ff

	/* SYS_TSC_ADC_SS_FIFO1DATA register bit field */
	union {
		volatile uint32_t FIFO1DATA;

		volatile struct {
			uint32_t ADCDATA : 12; // 11:0
			uint32_t rsvd12 : 4; // 15:12
			uint32_t ADCCHNLID : 4; // 19:16
			uint32_t rsvd20 : 12; // 31:20
		} FIFO1DATA_bit;
	}; // 0x200

} sysTscAdcSs;

/* Definition of Touchscreen/ADC register structures. */
#define ADC_TSC (*((volatile sysTscAdcSs*)0x44E0D000))

#endif /* _SYS_TSC_ADC_SS_H_ */
