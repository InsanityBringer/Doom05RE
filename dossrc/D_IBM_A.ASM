	.386
	.MODEL  small
	INCLUDE defs.inc

EXTRN _update:BYTE
EXTRN _ublocksource:DWORD
EXTRN _screenbuffer:DWORD
EXTRN _buffercrtc:DWORD

.CODE

IO_BlitBlocks_ PROC
PUBLIC IO_BlitBlocks_
	PUSHR
	
	mov dx,SC_INDEX ;Set the register index to the mapmask register
	mov al,02h
	out dx,al
	inc dx
	mov ebx,0C7h
	nop
	
topofloop: ; loops for each screen tile
	test byte ptr[ebx + offset _update],0FFh ; check if the cell has needed updates
	jnz blitloop
	nop
	nop
	nop
	nop
	dec ebx
	jns topofloop
	jmp done
	
	xchg ebx, ebx ;debugging marker?
	nop
	
blitloop:
	dec byte ptr[ebx + offset _update]
	mov esi,dword ptr[ebx * 4 + offset _ublocksource]
	mov edi,esi
	add esi,offset _screenbuffer
	add edi,dword ptr [_buffercrtc]
	
	mov al,01h
	out dx,al
PLANE = 0
PCOL = 0
	REPT 20
	mov eax,dword ptr [esi + PLANE + PCOL]
	mov dword ptr [edi + PCOL],eax
	PCOL = PCOL + PLANEWIDTH
	ENDM
	
	mov al,02h
	out dx,al
PLANE = PLANE + PLANESIZE
PCOL = 0
	REPT 20
	mov eax,dword ptr [esi + PLANE + PCOL]
	mov dword ptr [edi + PCOL],eax
	PCOL = PCOL + PLANEWIDTH
	ENDM
	
	mov al,04h
	out dx,al
PLANE = PLANE + PLANESIZE
PCOL = 0
	REPT 20
	mov eax,dword ptr [esi + PLANE + PCOL]
	mov dword ptr [edi + PCOL],eax
	PCOL = PCOL + PLANEWIDTH
	ENDM
	
	mov al,08h
	out dx,al
PLANE = PLANE + PLANESIZE
PCOL = 0
	REPT 20
	mov eax,dword ptr [esi + PLANE + PCOL]
	mov dword ptr [edi + PCOL],eax
	PCOL = PCOL + PLANEWIDTH
	ENDM
	
	dec ebx
	jns blitloop
	
done:
	POPR
	ret
	
ENDP

END
