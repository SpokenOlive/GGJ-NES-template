.import FamiToneInit
.import FamiToneSfxInit
.import FamiToneUpdate
.import FamiToneMusicPlay
.import FamiToneMusicPause
.import FamiToneMusicStop
.import FamiToneSfxPlay
.import px_uxrom_bank, px_uxrom_select

.code

.export _music_init
.proc _music_init ; u16 addr
	; ax -> xy
	pha
	txa
	tay
	pla
	tax
	lda #1 ; TODO Hardcoded NTSC
	jsr FamiToneInit
	
	rts
.endproc

.export _sound_init
.proc _sound_init ; u16 addr
	; ax -> xy
	pha
	txa
	tay
	pla
	tax
	jsr FamiToneSfxInit
	
	rts
.endproc

nmi_callback:
	; save the current bank to the stack
	lda px_uxrom_bank
	pha
	
	; audio is in PRG0
	ldy #0
	jsr px_uxrom_select
	
	jsr FamiToneUpdate
	
	; restore previous bank
	pla
	tay
	jsr px_uxrom_select
	rts

.export _music_play = FamiToneMusicPlay
.export _music_pause = FamiToneMusicPause
.export _music_stop = FamiToneMusicStop
.export _sound_play = FamiToneSfxPlay

.export _px_nmi_callback = nmi_callback

.segment "RODATA"

FT_DPCM_PTR = (FT_DPCM_OFF & $3fff) >> 6
FT_DPCM_OFF:
	; .incbin "after_the_rain.dmc"

.export _MUSIC
_MUSIC:
	.include "GameJamMusic.s"

.export _SOUNDS
_SOUNDS:
	; .include "sounds.s"
sounds:
	.word @ntsc
	.word @ntsc
@ntsc:
	.word @sfx_ntsc_0
	.word @sfx_ntsc_1
	.word @sfx_ntsc_2
	.word @sfx_ntsc_3

@sfx_ntsc_0:
	.byte $83,$36,$84,$c8,$85,$0b,$01,$84,$98,$01,$84,$68,$01,$84,$38,$01
	.byte $84,$08,$01,$84,$d8,$85,$0a,$01,$84,$a8,$01,$83,$35,$84,$78,$01
	.byte $83,$34,$84,$48,$01,$84,$18,$01,$83,$33,$84,$e8,$85,$09,$01,$84
	.byte $b8,$01,$00
@sfx_ntsc_1:
	.byte $83,$36,$84,$88,$85,$0b,$01,$84,$18,$01,$84,$a8,$85,$0a,$01,$84
	.byte $38,$01,$84,$c8,$85,$09,$01,$84,$58,$01,$84,$e8,$85,$08,$01,$83
	.byte $35,$84,$78,$01,$83,$34,$84,$08,$01,$84,$00,$01,$83,$33,$02,$00
@sfx_ntsc_2:
	.byte $83,$39,$84,$82,$85,$09,$01,$84,$71,$01,$83,$38,$84,$60,$01,$84
	.byte $4f,$01,$84,$3e,$01,$83,$37,$84,$2d,$01,$84,$1c,$01,$84,$0b,$01
	.byte $83,$36,$84,$fa,$85,$08,$01,$84,$e9,$01,$83,$35,$84,$d8,$01,$84
	.byte $c7,$01,$83,$34,$84,$b6,$01,$84,$a5,$01,$84,$94,$01,$83,$33,$84
	.byte $83,$01,$84,$72,$01,$84,$61,$01,$83,$32,$84,$50,$01,$84,$3f,$01
	.byte $83,$31,$84,$2e,$01,$84,$1d,$01,$84,$0c,$01,$00
@sfx_ntsc_3:
	.byte $83,$34,$84,$8e,$85,$09,$89,$34,$8a,$00,$01,$83,$35,$84,$89,$89
	.byte $35,$01,$83,$36,$84,$85,$89,$36,$01,$84,$83,$01,$84,$7f,$01,$84
	.byte $7d,$01,$83,$37,$84,$79,$89,$37,$01,$84,$74,$01,$83,$38,$84,$6f
	.byte $89,$38,$01,$84,$6a,$01,$83,$39,$84,$65,$89,$39,$01,$83,$38,$84
	.byte $61,$89,$38,$01,$83,$37,$84,$5f,$89,$37,$01,$84,$5b,$01,$84,$58
	.byte $01,$84,$55,$01,$84,$50,$01,$83,$36,$84,$4b,$89,$36,$01,$84,$46
	.byte $01,$84,$41,$01,$84,$3d,$01,$84,$3a,$01,$84,$37,$01,$84,$34,$01
	.byte $84,$31,$01,$83,$35,$84,$2c,$89,$35,$01,$84,$27,$01,$84,$23,$01
	.byte $83,$36,$84,$1d,$89,$36,$01,$84,$19,$01,$83,$37,$84,$16,$89,$37
	.byte $01,$84,$13,$01,$84,$10,$01,$84,$0d,$01,$84,$09,$01,$84,$03,$01
	.byte $84,$ff,$85,$08,$01,$83,$38,$84,$f9,$89,$38,$01,$84,$f5,$01,$83
	.byte $39,$84,$f2,$89,$39,$01,$84,$ef,$01,$84,$ec,$01,$83,$38,$84,$e9
	.byte $89,$38,$01,$84,$e5,$01,$84,$df,$01,$83,$37,$84,$db,$89,$37,$01
	.byte $84,$d6,$01,$84,$d1,$01,$84,$ce,$01,$84,$cb,$01,$84,$c8,$01,$84
	.byte $c5,$01,$84,$c1,$01,$84,$bc,$01,$84,$b7,$01,$83,$36,$84,$b2,$89
	.byte $36,$01,$84,$ad,$01,$84,$aa,$01,$84,$a7,$01,$84,$a3,$01,$83,$35
	.byte $84,$a1,$89,$35,$01,$84,$9d,$01,$84,$98,$01,$84,$93,$01,$83,$34
	.byte $84,$8e,$89,$34,$01,$83,$33,$84,$89,$89,$33,$01,$84,$85,$01,$84
	.byte $83,$01,$83,$32,$84,$7f,$89,$32,$01,$84,$7d,$01,$83,$31,$84,$79
	.byte $89,$31,$01,$84,$74,$01,$84,$6f,$01,$84,$6a,$01,$84,$65,$01,$83
	.byte $30,$00
