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
REPT    SCREENWIDTH+1
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


R_MapRowHigh_ PROC 
PUBLIC	R_MapRowHigh_
	PUSHR

	mov	eax,[_mr_x1]
	add eax,[_windowx]
	mov	ebx,[_mr_x2]
	add ebx,[_windowx]
	mov	eax,[mapcalls+eax*4]
	mov	[callpoint],eax       ; spot to jump into unwound
	mov	eax,[mapcalls+ebx*4]
	mov	[returnpoint],eax     ; spot to patch a ret at
	mov	BYTE PTR [eax], OP_RET

;
; build composite position
;
	mov	ecx,[_mr_xfrac]
	and	ecx,0ffff0000h
	mov	eax,[_mr_yfrac]
	shr	eax,010h
	or	ecx,eax

;
; build composite step
;
	mov	edx,[_mr_xstep]
	and	edx,0ffff0000h
	mov	eax,[_mr_ystep]
	shr	eax,010h
	or	edx,eax

	mov	esi,[_mr_picture]

	mov	edi,[_mr_y]
	add edi,[_windowy]
	mov	edi,[_planewidthlookup+edi*4]
	add edi,OFFSET _screenbuffer

	mov	eax,[_mr_colormap]
	shl eax,08h
	add eax,[_colormaps]

;
; feed the pipeline and jump in
;
	mov		ebp,0fffh		; used to mask off slop high bits from position
	shld	ebx,ecx,22				; shift y units in
	shld	ebx,ecx,6				; shift x units in
	and		ebx,ebp					; mask off slop bits
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
REPT	4
	MAPLABEL      %LINE
LINE    =	LINE + 1

	mov     al,[esi+ebx]            ; get source pixel
	shld	ebx,ecx,22				; shift y units in
	shld	ebx,ecx,6				; shift x units in
	mov     al,[eax]                ; translate color
	and		ebx,ebp					; mask off slop bits
	add		ecx,edx					; position += step
	mov     [edi+PLANE+PCOL],al       ; write pixel
PLANE	=	PLANE + PLANESIZE
ENDM
PCOL	=	PCOL + 1
ENDM
hmap320:
	ret

ENDP

END
