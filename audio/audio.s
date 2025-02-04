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
	.include "sounds.s"
