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


MAPDEFINE     MACRO   number
	dd      hmap&number
ENDM

	ALIGN   4
mapcalls      LABEL
LINE    =       0
REPT    SCREENWIDTH / 2 + 1
	MAPDEFINE     %LINE
LINE    =       LINE+1
ENDM


callpoint	dd  0
returnpoint	dd	0


.CODE

;================
;
; R_MapRowHighColor
;
; Horizontal texture mapping
; [ISB] this function wasn't adapted for the 5/22 alpha's calling convention and is broken as a result. 
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
	
	mov	edi,[_mr_y]
	mov	edi,[_planewidthlookup+edi*4]
	
	mov	eax,[_mr_colormap]
	shl eax,09h
	add eax,[_wordcolormaps]
	shr eax,01h

;
; build composite position
;
	mov	ebx,[_mr_xfrac]
	mov	ecx,[_mr_yfrac]
	mov	esi,[_mr_picture]
	
	xor ebp,ebp
	shld ebp,ecx,06h
	shld ebp,ebx,06h

;
; feed the pipeline and jump in
;
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

	mov     al,[esi+ebp]            ; get source pixel
	add		ebx,dword ptr [_mr_xstep] ;regenerate composite position
	add		ecx,dword ptr [_mr_ystep]
	xor		ebp,ebp
	shld	ebp,ecx,06h
	shld	ebp,ebx,06h
	mov     edx,[eax*2]             ; translate color
	mov     [edi+PLANE+PCOL],dl     ; write pixel
	mov     [edi+PLANE+PLANESIZE+PCOL],dh ; write pixel
PLANE	=	PLANE + PLANESIZE * 2
ENDM
PCOL	=	PCOL + 1
ENDM
hmap160:
	ret

ENDP

END
