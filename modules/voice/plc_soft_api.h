#ifndef PLC_SOFT_API_H
#define PLC_SOFT_API_H


#define PLC_OLA_WIN		        2//0 	//smart中该参数设置为2（内存限制），最佳值应该为0，为了与samrt对应，这里先修改为2
#define FRAMESZ                 120 		        //帧长
#define FRAMENUM                5//3//1 		    %以3帧为分界

/* LPC windowing */
#define LPCO					8                /* LPC order */
#define HAMWINDLEN				FRAMESZ          /* length of the assymetrical hamming window */

#if 1
//48k
#if !ADAPTER_USB_MIC_RX_EN
#define PITCH_MIN 160//160							/* minimum allowed pitch, 200 Hz */	//480 ->300 Hz
#define PITCH_MAX 650//480							/* maximum allowed pitch, 66 Hz */	//480 ->100 Hz
#else
#define PITCH_MIN 180//160							/* minimum allowed pitch, 200 Hz */	//480 ->300 Hz
#define PITCH_MAX 460//480							/* maximum allowed pitch, 66 Hz */	//480 ->100 Hz
#endif

#define PITCHDIFF (PITCH_MAX - PITCH_MIN)		/* register 8bit, max value:255 */
#define POVERLAPMAX (PITCH_MAX >> PLC_OLA_WIN)	/* maximum pitch OLA window */		//register 8bit, max value:255
#define HISTORYLEN (PITCH_MAX * 3 + FRAMESZ)	/* history buffer length FRAMESZ*/
#define NDEC 4									/* 2:1 decimation */
#define CORRLEN 280//280     						/* multiple of 8, correlation length */		//16的倍数,非16的倍数不兼容汇编
#define CORRBUFLEN (CORRLEN + PITCH_MAX)		/* correlation buffer length */
#define CORRMINPOWER 250						/* minimum power */
#define EOVERLAPINCR (64)						/* end OLA increment per frame, 4 ms */ //1.3ms

#define PITCH_MIN_8K 50//160							/* minimum allowed pitch, 200 Hz */	//480 ->300 Hz
#define PITCH_MAX_8K 180//480							/* maximum allowed pitch, 66 Hz */	//480 ->100 Hz
#define PITCHDIFF_8K (PITCH_MAX_8K - PITCH_MIN_8K)		/* register 8bit, max value:255 */
#define POVERLAPMAX_8K (PITCH_MAX_8K >> PLC_OLA_WIN)	/* maximum pitch OLA window */		//register 8bit, max value:255
#define HISTORYLEN_8K (PITCH_MAX_8K * 3 + FRAMESZ)	/* history buffer length FRAMESZ*/
#define NDEC_8K 4									/* 2:1 decimation */
#define CORRLEN_8K 64//280     						/* multiple of 8, correlation length */		//16的倍数,非16的倍数不兼容汇编
#define CORRBUFLEN_8K (CORRLEN_8K + PITCH_MAX_8K)		/* correlation buffer length */
#define CORRMINPOWER_8K 64						/* minimum power */
#define EOVERLAPINCR_8K (32)						/* end OLA increment per frame, 4 ms */ //1.3ms

////32k
//#define PITCH_MIN 180							/* minimum allowed pitch, 200 Hz */	//480 ->300 Hz
//#define PITCH_MAX 480							/* maximum allowed pitch, 66 Hz */	//480 ->100 Hz
//#define PITCHDIFF (PITCH_MAX - PITCH_MIN)		/* register 8bit, max value:255 */
//#define POVERLAPMAX (PITCH_MAX >> PLC_OLA_WIN)	/* maximum pitch OLA window */		//register 8bit, max value:255
//#define HISTORYLEN (PITCH_MAX * 3 + FRAMESZ)	/* history buffer length FRAMESZ*/
//#define NDEC 4									/* 2:1 decimation */
//#define CORRLEN 256								/* multiple of 8, correlation length */		//register 8bit, max value:255
//#define CORRBUFLEN (CORRLEN + PITCH_MAX)		/* correlation buffer length */
//#define CORRMINPOWER 250						/* minimum power */
//#define EOVERLAPINCR (64)						/* end OLA increment per frame, 4 ms */ //1.3ms

#else
//24k
//#define PITCH_MIN 80//180//160					/* minimum allowed pitch, 200 Hz */	//480 ->300 Hz
//#define PITCH_MAX 240//435//350					/* maximum allowed pitch, 66 Hz */	//480 ->100 Hz
//#define PITCHDIFF (PITCH_MAX - PITCH_MIN)		/* register 8bit, max value:255 */
//#define POVERLAPMAX (PITCH_MAX >> PLC_OLA_WIN)	/* maximum pitch OLA window */		//register 8bit, max value:255
//#define HISTORYLEN (PITCH_MAX * 3 + FRAMESZ)	/* history buffer length FRAMESZ*/
//#define NDEC 4									/* 2:1 decimation */
//#define CORRLEN 240//255//160						/* 20 ms correlation length */		//register 8bit, max value:255
//#define CORRBUFLEN (CORRLEN + PITCH_MAX)		/* correlation buffer length */
//#define CORRMINPOWER 250						/* minimum power */
//#define EOVERLAPINCR (64)						/* end OLA increment per frame, 4 ms */ //1.3ms

//16k
#define PITCH_MIN 50		//40					/* maximum allowed pitch, 66 Hz  */
#define PITCH_MAX 180		//240					/* minimum allowed pitch, 200 Hz */
#define PITCHDIFF (PITCH_MAX - PITCH_MIN)		//60
#define POVERLAPMAX (PITCH_MAX >> PLC_OLA_WIN)			/* maximum pitch OLA window */	//30
#define HISTORYLEN (PITCH_MAX * 3 + POVERLAPMAX)	/* history buffer length*/	//120*3+30=390
#define NDEC   4  //2 									/* 2:1 decimation */
#define CORRLEN 160								/* 20 ms correlation length */	//40ms
#define CORRBUFLEN (CORRLEN + PITCH_MAX)			/* correlation buffer length */	//320+120=440
#define CORRMINPOWER 16	  //256							/* minimum power */
#define EOVERLAPINCR 32*2							/* end OLA increment per frame, 4 ms */ //8ms

#endif

#define OVERLALEN			52
#define ATTENFAC_DIVIDER	5//15
#define ATTENINCR			1

#define SAMPLE_RATE_48K     0
#define SAMPLE_RATE_32K     1
#define SAMPLE_RATE_24K     2
#define SAMPLE_RATE_16K     3
#define SAMPLE_RATE_12K     4
#define SAMPLE_RATE_8K      5

typedef struct {
	//u8 ff_flg;                  /* first flame flag */
	u16 erasecnt; /* consecutive erased frames */
	s16 poverlap; /* overlap based on pitch */
	s16 poffset; /* offset into pitch period */
	s16 pitch; /* pitch estimate */
	s16 pitchblen; /* current pitch buffer length */
	s32 PitchCnt;
	s32 overlapcnt;
	s16 *histrybufend; /* end of pitch buffer */
	s16 *histrybufstart; /* start of pitch buffer */
	s16 *pitchbufend; /* end of pitch buffer */
	s16 *pitchbufstart; /* start of pitch buffer */
	s16 *outbuf; /* buffer for cycles of speech */
	s16 *history_ptr;
	s16 *pitchbuf_ptr;
	s16 *lastq_ptr;

	s16 al[LPCO+1]; /* saved last quarter wavelength */
	s16 stsyml[LPCO];
	s32	rl[1+LPCO];
	s16	ltring[OVERLALEN];			//临时缓存变量，240帧长时：40 /* Q1 */
	s16 ring[OVERLALEN+LPCO];	    //临时缓存变量，240帧长时：30+8=38
	s16 *pring;
	bool enable;
	bool lats_save_bfi;

	u16 pitch_min;
	u16 pitch_max;
	u16 pitchdiff;
	u16 poverlapmax;
	u16 historylen;
	u8  ndec;
	u16 corrlen;
	u16 corrbuflen;
	u16 corrminpower;
	u16 eoverlapincr;
	s16 history[HISTORYLEN+FRAMESZ+OVERLALEN]; /* history buffer */
	s16 pitchbuf[HISTORYLEN]; /* buffer for cycles of speech */
    s16 lastq[POVERLAPMAX]; /* saved last quarter wavelength */
}plc_proc_t;


bool plc_soft_init_do(plc_proc_t *st, uint pkt_len);
void plc_soft_process_do(s16 *int_data, u32 samples, u8 bfi, plc_proc_t *st);
void plc_soft_exit_do(plc_proc_t *st);

#endif
