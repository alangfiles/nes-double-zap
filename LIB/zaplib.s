;written by Doug Fraker 2019-2020
;NES zapper gun code


.export _zap_shoot, _zap_read, _zap_read2




;from NESDEV WIKI
;7  bit  0
;---- ----
;xxxT WxxS
;   | |  |
;   | |  +- Serial data (Vs.)
;   | +---- Light sense (0: detected; 1: not detected) (NES/FC)
;   +------ Trigger (0: released; 1: pulled) (NES/FC)


;assumed that dpmc errors won't happen
;usually zappers are in controller port 2
;but this can do either port
;in value = 0 or 1, which port
;out value = 0 or 1, 1 if zapper trigger pulled

_zap_shoot:
	tay
	ldx #1
	stx CTRL_PORT1
	dex ;#0
	stx CTRL_PORT1
	
	lda CTRL_PORT1,y
	and #$10
	beq @exit
	ldx #1 ;x is 0 otherwise
	
@exit:
	txa
	ldx #0
	rts
	
	
	
;in value = 0 or 1, which port
;out value = 0 or 1, whether it detected light
;please, only display 1 white box per frame
	
_zap_read:	
	tay
	ldx <FRAME_CNT1 ;ticks up during nmi code
	
@loop:
	lda #1
	sta CTRL_PORT1
	lda #0
	sta CTRL_PORT1
	lda CTRL_PORT1,y
	and #$08
	beq @hit_yes ;0 = light, 8 = no light
	cpx <FRAME_CNT1
	beq @loop 
	
@hit_no: ;ran past the frame and into the next nmi
	lda #0
	tax
	rts
	
@hit_yes:
	lda #1
	ldx #0
	rts
	

	
; changed version, now the a = scanlines to check
; a should be 1-224, NOT ZERO
	
;value vs real (scanlines)
;16 = 17
;32 = 32
;64 = 63
;100 = 99
;128 = 127
;200 = 198
;224 = 221
	
	
_zap_read2:
	ldx #0
	stx <TEMP
	asl a
	rol <TEMP
	asl a
	rol <TEMP

	tax
	beq @zero
;x is not zero, will exit early, add 1
	inc <TEMP
@zero:	
	ldy #1
@loop:
	lda #1
	sta CTRL_PORT1
	lda #0
	sta CTRL_PORT1
	lda CTRL_PORT1,y
	and #$08
	beq @hit_yes ;0 = light, 8 = no light

	ora $00 ; waste 3 cycles

	dex
	bne @loop

	dec <TEMP
	bmi @hit_no ;if started zero, rolled to ff, exit
	bne @loop ;positive, but above zero
	
@hit_no:
	lda #0
	tax
	rts
	
@hit_yes:
	lda #1
	ldx #0
	rts	
	
	