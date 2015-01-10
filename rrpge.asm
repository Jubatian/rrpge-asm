;
; RRPGE system assistance definitions
;
; Author    Sandor Zsuga (Jubatian)
; Copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
;           License) extended as RRPGEvt (temporary version of the RRPGE
;           License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
;           root.
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
PTR16W			equ	0b0111
PTR8I			equ	0b1000
PTR4I			equ	0b1001
PTR2I			equ	0b1010
PTR1I			equ	0b1011
PTR8W			equ	0b1100
PTR4W			equ	0b1101
PTR2W			equ	0b1110
PTR1W			equ	0b1111

;
; User Peripheral Area
;

NULL			equ	0x0000	; Always reads 0, for null pointers
P_CLOCK			equ	0x0001	; 187.5Hz clock
P_AUDIO_CTR		equ	0x0002	; Audio DMA sample counter / read offset
P_AUDIO_CLK		equ	0x0003	; Audio DMA base clock
P_AUDIO_LOFF		equ	0x0004	; Audio DMA left channel start offset
P_AUDIO_ROFF		equ	0x0005	; Audio DMA right channel start offset
P_AUDIO_SIZE		equ	0x0006	; Audio DMA buffer size mask bits
P_AUDIO_DIV		equ	0x0007	; Audio clock divider
P_MFIFO_LS		equ	0x0008	; Mixer FIFO location & size
P_MFIFO_STAT		equ	0x0009	; Mixer FIFO status
P_MFIFO_ADDR		equ	0x000A	; Mixer FIFO address
P_MFIFO_DATA		equ	0x000B	; Mixer FIFO data
P_GFIFO_LS		equ	0x000C	; Graphics FIFO location & size
P_GFIFO_STAT		equ	0x000D	; Graphics FIFO status
P_GFIFO_ADDR		equ	0x000E	; Graphics FIFO address
P_GFIFO_DATA		equ	0x000F	; Graphics FIFO data
P_GDG_MCK0		equ	0x0010	; GDG Mask / Colorkey definition 0
P_GDG_MCK1		equ	0x0011	; GDG Mask / Colorkey definition 1
P_GDG_MCK2		equ	0x0012	; GDG Mask / Colorkey definition 2
P_GDG_MCK3		equ	0x0013	; GDG Mask / Colorkey definition 3
P_GDG_SMRA		equ	0x0014	; GDG Shift mode region A
P_GDG_SMRB		equ	0x0015	; GDG Shift mode region B
P_GDG_DLCLR		equ	0x0016	; GDG Display List Clear controls
P_GDG_DLDEF		equ	0x0017	; GDG Display List Definition & Process flags
P_GDG_SA0		equ	0x0018	; GDG Source definition A0
P_GDG_SA1		equ	0x0019	; GDG Source definition A1
P_GDG_SA2		equ	0x001A	; GDG Source definition A2
P_GDG_SA3		equ	0x001B	; GDG Source definition A3
P_GDG_SB0		equ	0x001C	; GDG Source definition B0
P_GDG_SB1		equ	0x001D	; GDG Source definition B1
P_GDG_SB2		equ	0x001E	; GDG Source definition B2
P_GDG_SB3		equ	0x001F	; GDG Source definition B3
P0_AH			equ	0x0020	; Pointer 0 Address, high
P0_AL			equ	0x0021	; Pointer 0 Address, low
P0_IH			equ	0x0022	; Pointer 0 Increment, high
P0_IL			equ	0x0023	; Pointer 0 Increment, low
P0_DS			equ	0x0024	; Pointer 0 Data unit size
P0_RW_NI		equ	0x0026	; Pointer 0 Read / Write with no post-increment
P0_RW			equ	0x0027	; Pointer 0 Read / Write
P1_AH			equ	0x0028	; Pointer 1 Address, high
P1_AL			equ	0x0029	; Pointer 1 Address, low
P1_IH			equ	0x002A	; Pointer 1 Increment, high
P1_IL			equ	0x002B	; Pointer 1 Increment, low
P1_DS			equ	0x002C	; Pointer 1 Data unit size
P1_RW_NI		equ	0x002E	; Pointer 1 Read / Write with no post-increment
P1_RW			equ	0x002F	; Pointer 1 Read / Write
P2_AH			equ	0x0030	; Pointer 2 Address, high
P2_AL			equ	0x0031	; Pointer 2 Address, low
P2_IH			equ	0x0032	; Pointer 2 Increment, high
P2_IL			equ	0x0033	; Pointer 2 Increment, low
P2_DS			equ	0x0034	; Pointer 2 Data unit size
P2_RW_NI		equ	0x0036	; Pointer 2 Read / Write with no post-increment
P2_RW			equ	0x0037	; Pointer 2 Read / Write
P3_AH			equ	0x0038	; Pointer 3 Address, high
P3_AL			equ	0x0039	; Pointer 3 Address, low
P3_IH			equ	0x003A	; Pointer 3 Increment, high
P3_IL			equ	0x003B	; Pointer 3 Increment, low
P3_DS			equ	0x003C	; Pointer 3 Data unit size
P3_RW_NI		equ	0x003E	; Pointer 3 Read / Write with no post-increment
P3_RW			equ	0x003F	; Pointer 3 Read / Write

;
; Supervisor calls
;

kc_sfi_loadbin		equ	0x0100
kc_sfi_load		equ	0x0110
kc_sfi_save		equ	0x0111
kc_sfi_next		equ	0x0112
kc_sfi_move		equ	0x0113
kc_vid_setpal		equ	0x0300
kc_vid_mode		equ	0x0330
kc_vid_setst3d		equ	0x0340
kc_inp_getprops		equ	0x0410
kc_inp_dropdev		equ	0x0411
kc_inp_getdidesc	equ	0x0412
kc_inp_getaidesc	equ	0x0413
kc_inp_getname		equ	0x0414
kc_inp_getdi		equ	0x0422
kc_inp_getai		equ	0x0423
kc_inp_popchar		equ	0x0424
kc_inp_checkarea	equ	0x0425
kc_dly_delay		equ	0x0500
kc_usr_getlocal		equ	0x0600
kc_usr_getutf		equ	0x0601
kc_usr_getlang		equ	0x0610
kc_usr_getcolors	equ	0x0611
kc_usr_getst3d		equ	0x0612
kc_net_send		equ	0x0700
kc_net_recv		equ	0x0701
kc_net_listusers	equ	0x0710
kc_net_setavail		equ	0x0720
kc_net_getavail		equ	0x0721
kc_tsk_query		equ	0x0800
kc_tsk_discard		equ	0x0801

;
; User Library functions
;

us_ptr_set1i		equ	0xF000
us_ptr_set1w		equ	0xF002
us_ptr_set2i		equ	0xF004
us_ptr_set2w		equ	0xF006
us_ptr_set4i		equ	0xF008
us_ptr_set4w		equ	0xF00A
us_ptr_set8i		equ	0xF00C
us_ptr_set8w		equ	0xF00E
us_ptr_set16i		equ	0xF010
us_ptr_set16w		equ	0xF012
us_ptr_setgen16i	equ	0xF018
us_ptr_setgen16w	equ	0xF01A
us_ptr_setgen		equ	0xF01C
us_copy_pfc		equ	0xF020
us_copy_cfp		equ	0xF022
us_copy_pfp		equ	0xF024
us_copy_cfc		equ	0xF026
us_set_p		equ	0xF028
us_set_c		equ	0xF02A
us_copy_pfp_l		equ	0xF02C
us_set_p_l		equ	0xF02E
us_dloff_from		equ	0xF030
us_dloff_to		equ	0xF032
us_dlist_setptr		equ	0xF034
us_dlist_add		equ	0xF036
us_dlist_addxy		equ	0xF038
us_dlist_addbg		equ	0xF03A
us_dlist_addlist	equ	0xF03C
us_dlist_clear		equ	0xF03E
us_dloff_clip		equ	0xF040
us_dbuf_init		equ	0xF042
us_dlist_sb_setptr	equ	0xF044
us_dlist_sb_add		equ	0xF046
us_dlist_sb_addxy	equ	0xF048
us_dlist_sb_addbg	equ	0xF04A
us_dlist_sb_addlist	equ	0xF04C
us_dlist_sb_clear	equ	0xF04E
us_dbuf_flip		equ	0xF050
us_dbuf_getlist		equ	0xF052
us_dlist_db_setptr	equ	0xF054
us_dlist_db_add		equ	0xF056
us_dlist_db_addxy	equ	0xF058
us_dlist_db_addbg	equ	0xF05A
us_dlist_db_addlist	equ	0xF05C
us_dlist_db_clear	equ	0xF05E
us_dbuf_addfliphook	equ	0xF060
us_dbuf_remfliphook	equ	0xF062
us_dbuf_addframehook	equ	0xF064
us_dbuf_remframehook	equ	0xF066
us_dbuf_addinithook	equ	0xF068
us_dbuf_reminithook	equ	0xF06A
us_sprite_reset		equ	0xF06C
us_smux_reset		equ	0xF06E
us_sprite_setbounds	equ	0xF070
us_smux_setbounds	equ	0xF072
us_sprite_add		equ	0xF074
us_smux_add		equ	0xF076
us_sprite_addxy		equ	0xF078
us_smux_addxy		equ	0xF07A
us_sprite_addlist	equ	0xF07C
us_smux_addlist		equ	0xF07E
us_sin			equ	0xF080
us_cos			equ	0xF082
us_sincos		equ	0xF084
us_tfreq		equ	0xF086
us_mul32		equ	0xF088
us_div32		equ	0xF08A
us_rec16		equ	0xF08C
us_rec32		equ	0xF08E
us_sqrt16		equ	0xF090
us_sqrt32		equ	0xF092
us_dsurf_set		equ	0xF094
us_dsurf_setdbuf	equ	0xF096
us_dsurf_setm		equ	0xF098
us_dsurf_setmdbuf	equ	0xF09A
us_dsurf_get		equ	0xF09C
us_dsurf_getacc		equ	0xF09E
us_dsurf_getpw		equ	0xF0A0
us_dsurf_setaccpart	equ	0xF0A2
us_dsurf_init		equ	0xF0A4
us_dsurf_flip		equ	0xF0A6
us_tile_set		equ	0xF0A8
us_tile_getacc		equ	0xF0AA
us_tile_blit		equ	0xF0AC
us_tile_blitb		equ	0xF0AE
us_tile_gethw		equ	0xF0B0
us_tmap_set		equ	0xF0B2
us_tmap_setfn		equ	0xF0B4
us_tmap_getacc		equ	0xF0B6
us_tmap_getaccxy	equ	0xF0B8
us_tmap_getaccxfy	equ	0xF0BA
us_tmap_blit		equ	0xF0BC
us_tmap_gethw		equ	0xF0BE
us_tmap_gettilehw	equ	0xF0C0
us_tmap_gettile		equ	0xF0C2
us_tmap_settile		equ	0xF0C4
us_tmap_setptr		equ	0xF0C6

;
; User Library locations (pointers)
;

up_font_4		equ	0xFAAC
up_font_4i		equ	0xFAB0
up_font_8		equ	0xFAB4
up_font_8i		equ	0xFAB8
up_dsurf		equ	0xFAC0
