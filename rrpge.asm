;
; RRPGE system assistance definitions
;
; Author    Sandor Zsuga (Jubatian)
; Copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
;           License) extended as RRPGEv2 (version 2 of the RRPGE License): see
;           LICENSE.GPLv3 and LICENSE.RRPGEv2 in the project root.
;

;
; Memory pointer modes
;

PTR8			equ	0b0000
PTR4			equ	0b0001
PTR2			equ	0b0010
PTR1			equ	0b0011
PTR16			equ	0b0100
PTR16I			equ	0b0110
PTR16D			equ	0b0111
PTR8I			equ	0b1000
PTR4I			equ	0b1001
PTR2I			equ	0b1010
PTR1I			equ	0b1011
PTR8D			equ	0b1100
PTR4D			equ	0b1101
PTR2D			equ	0b1110
PTR1D			equ	0b1111

;
; Important memory pages
;

PAGE_ROPD		equ	0x40E0
PAGE_APER		equ	0x7FFF
PAGE_VPER		equ	0xBFFF

;
; Important areas in the Read Only Process Descriptor
; (assumed to be mapped in Read page 0 of the CPU)
;

ROPD_RBK_0		equ	0x0D00
ROPD_WBK_0		equ	0x0D10
ROPD_RBK_1		equ	0x0D01
ROPD_WBK_1		equ	0x0D11
ROPD_RBK_2		equ	0x0D02
ROPD_WBK_2		equ	0x0D12
ROPD_RBK_3		equ	0x0D03
ROPD_WBK_3		equ	0x0D13
ROPD_RBK_4		equ	0x0D04
ROPD_WBK_4		equ	0x0D14
ROPD_RBK_5		equ	0x0D05
ROPD_WBK_5		equ	0x0D15
ROPD_RBK_6		equ	0x0D06
ROPD_WBK_6		equ	0x0D16
ROPD_RBK_7		equ	0x0D07
ROPD_WBK_7		equ	0x0D17
ROPD_RBK_8		equ	0x0D08
ROPD_WBK_8		equ	0x0D18
ROPD_RBK_9		equ	0x0D09
ROPD_WBK_9		equ	0x0D19
ROPD_RBK_10		equ	0x0D0A
ROPD_WBK_10		equ	0x0D1A
ROPD_RBK_11		equ	0x0D0B
ROPD_WBK_11		equ	0x0D1B
ROPD_RBK_12		equ	0x0D0C
ROPD_WBK_12		equ	0x0D1C
ROPD_RBK_13		equ	0x0D0D
ROPD_WBK_13		equ	0x0D1D
ROPD_RBK_14		equ	0x0D0E
ROPD_WBK_14		equ	0x0D1E
ROPD_RBK_15		equ	0x0D0F
ROPD_WBK_15		equ	0x0D1F

;
; Supervisor calls
;

kc_mem_bankrd		equ	0x0000
kc_mem_bankwr		equ	0x0001
kc_mem_bank		equ	0x0002
kc_mem_banksame		equ	0x0003
kc_sfi_loadbin		equ	0x0100
kc_sfi_loadnv		equ	0x0110
kc_sfi_savenv		equ	0x0111
kc_sfi_listnv		equ	0x0112
kc_sfi_loadfile		equ	0x0120
kc_sfi_savefile		equ	0x0121
kc_aud_sethnd		equ	0x0210
kc_vid_setpal		equ	0x0300
kc_vid_sethnd		equ	0x0310
kc_vid_getline		equ	0x0320
kc_inp_getdev		equ	0x0400
kc_inp_getbest		equ	0x0401
kc_inp_getprops		equ	0x0410
kc_inp_getdidesc	equ	0x0411
kc_inp_getdi		equ	0x0420
kc_inp_getap		equ	0x0421
kc_inp_getapext		equ	0x0422
kc_inp_popchar		equ	0x0423
kc_inp_settouch		equ	0x0430
kc_dly_delay		equ	0x0500
kc_usr_getlocal		equ	0x0600
kc_usr_getutf		equ	0x0601
kc_usr_getlang		equ	0x0610
kc_usr_getcolors	equ	0x0611
kc_net_send		equ	0x0700
kc_net_recv		equ	0x0701
kc_net_listusers	equ	0x0710
kc_net_setavail		equ	0x0720
kc_tsk_query		equ	0x0800
kc_tsk_discard		equ	0x0801
