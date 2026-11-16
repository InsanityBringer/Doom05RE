	.386
	.MODEL  small
	INCLUDE defs.inc
	
;============================================================================
;
; unwound horizontal texture mapping code
;
; eax   lighttable
; ebx   scratch register
; ecx   position 6.10 bits x, 6.10 bits y
; edx   step 6.10 bits x, 6.10 bits y
; esi   start of block
; edi   dest
; ebp   fff to mask bx
;
; ebp should by preset from ebx / ecx before calling
;============================================================================

OP_SHLD	=	0fh


.DATA

extrn _wordcolormaps


MAPDEFINE     MACRO   number
	dd      hmap&number
ENDM

	ALIGN   4
mapcalls      LABEL
LINE    =       0
REPT    SCREENWIDTH/2+1
	MAPDEFINE     %LINE
LINE    =       LINE+1
ENDM


callpoint	dd  0
returnpoint	dd	0


.CODE

;================
;
; R_MapRowHigh
;
; Horizontal texture mapping
;
;================


R_MapRowHighColor_ PROC 
PUBLIC	R_MapRowHighColor_
	PUSHR

	mov	eax,[_mr_x1]
	mov	ebx,[_mr_x2]
	mov	eax,[mapcalls+eax*4]
	mov	[callpoint],eax       ; spot to jump into unwound
	mov	eax,[mapcalls+ebx*4]
	mov	[returnpoint],eax     ; spot to patch a ret at
	mov	BYTE PTR [eax], OP_RET
	
	mov eax,[_mr_colormap]
	shl eax, 09h
	add eax, _wordcolormaps
	shr eax, 01h

	mov	ebx,[_mr_xfrac]
	mov	ecx,[_mr_yfrac]
	mov	esi,[_mr_picture]

;
; feed the pipeline and jump in
;
	xor ebp, ebp
	shld ebp, ecx, 06h
	shld ebp, ebx, 06h
	
	call    [callpoint]

	mov	ebx,[returnpoint]
	mov	BYTE PTR [ebx],OP_MOVAL		; remove the ret patched in

	POPR
	ret


;============= HIGH DETAIL ============

.CODE

MAPLABEL      MACRO   number
hmap&number:
ENDM

LINE    =      0
PCOL	=	0
REPT SCREENWIDTH/4
PLANE	=	0
REPT	2
	MAPLABEL      %LINE
LINE    =	LINE + 1

	mov     al,[esi+ebp]                   ; get source pixel
	add		ebx, dword ptr [_mr_xstep]     ; increment x position
	add		ecx, dword ptr [_mr_ystep]     ; increment y position
	xor		ebp, ebp                       ; reload composite position
	shld	ebp, ecx, 06h
	shld	ebp, ebx, 06h
	mov     edx, [eax * 02h]               ; translate color
	mov     [edi+PLANE+PCOL],dl            ; write pixel
	mov     [edi+PLANE+PLANESIZE+PCOL],dh  ; write pixel
PLANE	=	PLANE + PLANESIZE + PLANESIZE
ENDM
PCOL	=	PCOL + 1
ENDM
hmap160:
	ret

ENDP

END
