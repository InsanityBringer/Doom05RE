	.386
	.MODEL  small
	INCLUDE defs.inc

extrn _update
extrn _screenbuffer
extrn _buffercrtc
extrn _ublocksource

.CODE

;================
;
; IO_BlitBlocks_
;
;================

IO_BlitBlocks_ PROC
PUBLIC   IO_BlitBlocks_

	PUSHR
	
	mov		dx, 03c4h
	mov		al, 02h
	out		dx, al
	inc		dx
	mov		ebx, 0c7h
	
	nop
	
.blockloop:
	test	byte ptr [_update+ebx],0ffh
	jnz		.copyupdate
	
	nop
	nop
	nop
	nop
	
	dec		ebx
	jns		.blockloop
	jmp		.leave
	
	
.copyupdate:
	dec		byte ptr [_update + ebx]
	mov		esi, dword ptr [_ublocksource + ebx * 04h]
	mov		edi, esi
	add		esi, OFFSET _screenbuffer
	add		edi, dword ptr [_buffercrtc]
	
	PLANENUM = 0
	PLANE = 0
	REPT 4
		mov		al, 1 shl PLANENUM
		out		dx,	al
		PROW = 0
		REPT 20
			mov		eax, dword ptr [esi + PROW + PLANE]
			mov		dword ptr [edi + PROW], eax
			PROW = PROW + 80
		ENDM
		
	PLANE = PLANE + PLANESIZE
	PLANENUM = PLANENUM + 1
	ENDM
	
	dec		ebx
	jns		.blockloop
	
.leave:
	POPR
	ret

ENDP

END
