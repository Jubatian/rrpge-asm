;
; RRPGE system assistance definitions
;
; Author    Sandor Zsuga (Jubatian)
; Copyright 2013 - 2015, GNU GPLv3 (version 3 of the GNU General Public
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

kc_sfi_loadbin		equ	0x00
kc_sfi_load		equ	0x01
kc_sfi_save		equ	0x02
kc_sfi_next		equ	0x03
kc_sfi_move		equ	0x04
kc_vid_setpal		equ	0x08
kc_vid_mode		equ	0x09
kc_vid_setst3d		equ	0x0A
kc_inp_getprops		equ	0x10
kc_inp_dropdev		equ	0x11
kc_inp_getdidesc	equ	0x12
kc_inp_getaidesc	equ	0x13
kc_inp_getname		equ	0x14
kc_inp_getdi		equ	0x16
kc_inp_getai		equ	0x17
kc_inp_popchar		equ	0x18
kc_inp_checkarea	equ	0x19
kc_dly_delay		equ	0x1F
kc_usr_getlocal		equ	0x20
kc_usr_getutf		equ	0x21
kc_usr_getlang		equ	0x22
kc_usr_getcolors	equ	0x23
kc_usr_getst3d		equ	0x24
kc_net_send		equ	0x28
kc_net_recv		equ	0x29
kc_net_listusers	equ	0x2A
kc_net_setavail		equ	0x2B
kc_net_getavail		equ	0x2C
kc_tsk_query		equ	0x2E
kc_tsk_discard		equ	0x2F

;
; User Library functions
;

us_ptr_set1i		equ	0xE000
us_ptr_set1w		equ	0xE002
us_ptr_set2i		equ	0xE004
us_ptr_set2w		equ	0xE006
us_ptr_set4i		equ	0xE008
us_ptr_set4w		equ	0xE00A
us_ptr_set8i		equ	0xE00C
us_ptr_set8w		equ	0xE00E
us_ptr_set16i		equ	0xE010
us_ptr_set16w		equ	0xE012
us_ptr_setwi		equ	0xE014
us_ptr_setww		equ	0xE016
us_ptr_setgenwi		equ	0xE018
us_ptr_setgenww		equ	0xE01A
us_ptr_setgen		equ	0xE01C
us_copy_pfc		equ	0xE020
us_copy_cfp		equ	0xE022
us_copy_pfp		equ	0xE024
us_copy_cfc		equ	0xE026
us_set_p		equ	0xE028
us_set_c		equ	0xE02A
us_copy_pfp_l		equ	0xE02C
us_set_p_l		equ	0xE02E
us_dloff_from		equ	0xE030
us_dloff_to		equ	0xE032
us_dlist_setptr		equ	0xE034
us_dlist_add		equ	0xE036
us_dlist_addxy		equ	0xE038
us_dlist_addbg		equ	0xE03A
us_dlist_addlist	equ	0xE03C
us_dlist_clear		equ	0xE03E
us_dloff_clip		equ	0xE040
us_dbuf_init		equ	0xE042
us_dlist_sb_setptr	equ	0xE044
us_dlist_sb_add		equ	0xE046
us_dlist_sb_addxy	equ	0xE048
us_dlist_sb_addbg	equ	0xE04A
us_dlist_sb_addlist	equ	0xE04C
us_dlist_sb_clear	equ	0xE04E
us_dbuf_flip		equ	0xE050
us_dbuf_getlist		equ	0xE052
us_dlist_db_setptr	equ	0xE054
us_dlist_db_add		equ	0xE056
us_dlist_db_addxy	equ	0xE058
us_dlist_db_addbg	equ	0xE05A
us_dlist_db_addlist	equ	0xE05C
us_dlist_db_clear	equ	0xE05E
us_dbuf_addfliphook	equ	0xE060
us_dbuf_remfliphook	equ	0xE062
us_dbuf_addframehook	equ	0xE064
us_dbuf_remframehook	equ	0xE066
us_dbuf_addinithook	equ	0xE068
us_dbuf_reminithook	equ	0xE06A
us_sprite_reset		equ	0xE06C
us_smux_reset		equ	0xE06E
us_sprite_setbounds	equ	0xE070
us_smux_setbounds	equ	0xE072
us_sprite_add		equ	0xE074
us_smux_add		equ	0xE076
us_sprite_addxy		equ	0xE078
us_smux_addxy		equ	0xE07A
us_sprite_addlist	equ	0xE07C
us_smux_addlist		equ	0xE07E
us_sin			equ	0xE080
us_cos			equ	0xE082
us_sincos		equ	0xE084
us_tfreq		equ	0xE086
us_mul32		equ	0xE088
us_div32		equ	0xE08A
us_rec16		equ	0xE08C
us_rec32		equ	0xE08E
us_sqrt16		equ	0xE090
us_sqrt32		equ	0xE092
us_dsurf_new		equ	0xE094
us_dsurf_newdbuf	equ	0xE096
us_dsurf_newm		equ	0xE098
us_dsurf_newmdbuf	equ	0xE09A
us_dsurf_get		equ	0xE09C
us_dsurf_getacc		equ	0xE09E
us_dsurf_getpw		equ	0xE0A0
us_dsurf_init		equ	0xE0A2
us_dsurf_flip		equ	0xE0A4
us_tile_new		equ	0xE0A6
us_tile_acc		equ	0xE0A8
us_tile_blit		equ	0xE0AA
us_tile_gethw		equ	0xE0AC
us_btile_new		equ	0xE0AE
us_btile_acc		equ	0xE0B0
us_btile_blit		equ	0xE0B2
us_btile_gethw		equ	0xE0B4
us_tmap_new		equ	0xE0B6
us_tmap_acc		equ	0xE0B8
us_tmap_accxy		equ	0xE0BA
us_tmap_accxfy		equ	0xE0BC
us_tmap_blit		equ	0xE0BE
us_tmap_gethw		equ	0xE0C0
us_tmap_gettilehw	equ	0xE0C2
us_tmap_gettile		equ	0xE0C4
us_tmap_settile		equ	0xE0C6
us_tmap_setptr		equ	0xE0C8
us_fastmap_new		equ	0xE0CA
us_fastmap_mark		equ	0xE0CC
us_fastmap_gethw	equ	0xE0CE
us_fastmap_getyx	equ	0xE0D0
us_fastmap_setdly	equ	0xE0D2
us_fastmap_draw		equ	0xE0D4
us_cr_new		equ	0xE0D6
us_cr_setsi		equ	0xE0D8
us_cr_getnc		equ	0xE0DA
us_cw_new		equ	0xE0DC
us_cw_setnc		equ	0xE0DE
us_cw_setst		equ	0xE0E0
us_cw_init		equ	0xE0E2
us_cwr_new		equ	0xE0E4
us_cwr_nextsi		equ	0xE0E6
us_utf32f8		equ	0xE0E8
us_utf8f32		equ	0xE0EA
us_utf8len		equ	0xE0EC
us_idfutf32		equ	0xE0EE
us_cr_cbyte_new		equ	0xE0F0
us_cr_cbyte_setsi	equ	0xE0F2
us_cr_cbyte_getnc	equ	0xE0F4
us_cr_pbyte_new		equ	0xE0F6
us_cr_pbyte_setsb	equ	0xE0F8
us_cr_pbyte_setsi	equ	0xE0FA
us_cr_pbyte_getnc	equ	0xE0FC
us_cr_cutf8_new		equ	0xE0FE
us_cr_cutf8_setsi	equ	0xE100
us_cr_cutf8_getnc	equ	0xE102
us_cr_putf8_new		equ	0xE104
us_cr_putf8_setsb	equ	0xE106
us_cr_putf8_setsi	equ	0xE108
us_cr_putf8_getnc	equ	0xE10A
us_cwr_cbyte_new	equ	0xE10C
us_cwr_cbyte_newz	equ	0xE10E
us_cwr_cbyte_setnc	equ	0xE110
us_cwr_cbyte_nextsi	equ	0xE112
us_cwr_pbyte_new	equ	0xE114
us_cwr_pbyte_newz	equ	0xE116
us_cwr_pbyte_setnc	equ	0xE118
us_cwr_pbyte_nextsi	equ	0xE11A
us_cwr_cutf8_new	equ	0xE11C
us_cwr_cutf8_newz	equ	0xE11E
us_cwr_cutf8_setnc	equ	0xE120
us_cwr_cutf8_nextsi	equ	0xE122
us_cwr_putf8_new	equ	0xE124
us_cwr_putf8_newz	equ	0xE126
us_cwr_putf8_setnc	equ	0xE128
us_cwr_putf8_nextsi	equ	0xE12A
us_cw_tile_new		equ	0xE12C
us_cw_tile_setnc	equ	0xE12E
us_cw_tile_setst	equ	0xE130
us_cw_tile_init		equ	0xE132
us_cw_tile_setxy	equ	0xE134
us_ftile_new		equ	0xE136
us_ftile_acc		equ	0xE138
us_ftile_blit		equ	0xE13A
us_ftile_gethw		equ	0xE13C
us_ftile_setch		equ	0xE13E
us_strcpynz		equ	0xE140
us_strcpy		equ	0xE142
us_printfnz		equ	0xE144
us_printf		equ	0xE146


;
; User Library locations (pointers)
;

up_cr_utf8		equ	0xFD88
up_cr_byte		equ	0xFD8C
up_font_4		equ	0xFD9C
up_font_4i		equ	0xFDA4
up_font_8		equ	0xFDAC
up_font_8i		equ	0xFDB4
up_dsurf		equ	0xFDC0
up_ffutf_h		equ	0x001F
up_ffutf_l		equ	0x8200
up_uf437_h		equ	0x001F
up_uf437_l		equ	0x8900
