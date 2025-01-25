.macpack generic

.include "zeropage.inc"
.import incsp3, incsp4
.import __hextab, pusha

.include "pixler.inc"
.importzp px_sprite_cursor, PX_scroll_y

.zeropage

; TODO export as u8 and u16?
.exportzp _ix, _iy, _idx, _tmp
_ix: .word 0
_iy: .word 0
_idx: .word 0
_tmp: .word 0

.code

.export _meta_spr
.proc _meta_spr ; (u8 x, u8 y, u8 pal, void* meta) -> void
sprx = tmp1
spry = tmp2
pal = tmp3
meta = ptr1

	sta meta+0
	stx meta+1

	ldy #2
	lda (sp),y
	sta sprx
	
	dey
	lda (sp),y
	sta spry
	
	dey
	lda (sp),y
	sta pal
	
	ldx px_sprite_cursor

@loop:
	lda (meta), y
	cmp #$80
	beq @return
	add sprx
	sta OAM_X, x
	iny
	
	lda (meta), y
	add spry
	sta OAM_Y, x
	iny
	
	lda (meta), y
	sta OAM_CHR, x
	iny
	
	lda (meta), y
	ora pal
	sta OAM_ATTR, x
	iny
	
	inx
	inx
	inx
	inx
	jmp @loop

@return:
	stx px_sprite_cursor
	jmp incsp3
.endproc

.export _meta_spr2
.proc _meta_spr2 ; (u8 x, s16 y, u8 flip, void* meta) -> void
flip = tmp1
sprx = ptr1
spry = ptr2
meta = ptr3

	sta meta+0
	stx meta+1

	ldy #3
	lda (sp),y
	sta sprx
	
	dey
	lda (sp),y
	sta spry+1
	dey
	lda (sp),y
	sta spry+0
	
	; y += scroll.y
	lda spry+0
	sub PX_scroll_y+0
	sta spry+0
	lda spry+1
	sbc PX_scroll_y+1
	sta spry+1
	
	dey
	lda (sp),y
	beq :+
		lda #64
		jmp :++
	:
		lda #0
	:
	sta flip
	
	ldx px_sprite_cursor

@loop:
	lda (meta), y
	cmp #$80
	beq @return
	bit flip
	; flipx flag is $40 which bit conveniently loads into v flag
	bvc :+
		add #8
		eor #$FF
	:
	add sprx
	sta OAM_X, x
	iny
	
	lda (meta), y
	add spry+0
	sta OAM_Y, x
	lda #$00
	add spry+1
	beq :+
		iny
		iny
		iny
		jmp @loop
	:
	iny
	
	lda (meta), y
	sta OAM_CHR, x
	iny
	
	lda (meta), y
	eor flip
	sta OAM_ATTR, x
	iny
	
	inx
	inx
	inx
	inx
	jmp @loop
	
@return:
	stx px_sprite_cursor
	jmp incsp4
.endproc
