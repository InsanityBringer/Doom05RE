	.386
	.MODEL  small
	INCLUDE defs.inc
	
;============================================================================
;
; unwound horizontal texture mapping code
;
; eax   pixel and offset
; ebx   position 9.23 bits x
; ecx   position 9.23 bits y
; edx   step 9.23 bits x
; esi   start of block
; edi   dest
; ebp   step 9.23 bits y

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
; D_PolygonRow
;
; Horizontal texture mapping
;
;================


D_PolygonRow_ PROC 
PUBLIC	D_PolygonRow_
	PUSHR

	mov	eax,[_dm_x2]
	mov	ebx,[_dm_x1]
	mov	eax,[mapcalls+eax*4]
	mov	[callpoint],eax       ; spot to jump into unwound
	mov	eax,[mapcalls+ebx*4]
	mov	[returnpoint],eax     ; spot to patch a ret at
	mov	BYTE PTR [eax], OP_RET
	mov edi,[_dm_dest]
	mov esi,[_dm_picture]

;
; build composite position
;
	mov	ebx,[_dm_xfrac]
	shl ebx,07h
	mov	ecx,[_dm_yfrac]
	shl	ecx,07h

;
; build composite step
;
	mov	edx,[_dm_xstep]
	shl edx,07h
	mov	ebp,[_dm_ystep]
	shl	ebp,07h
	
;
; feed the pipeline and jump in
;
	xor	eax,eax
	shld eax,ecx,09h ; shift y units in
	shld eax,ebx,09h ; shift x units in

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

	mov     al,[esi+eax]            ; get source pixel
	add		ebx,edx					; add x frac
	add		ecx,ebp					; add y frac
	mov     [edi+PLANE+PCOL],al     ; write pixel
	xor		eax,eax					
	shld	eax,ecx,09h 			; shift y units in
	shld	eax,ebx,09h 			; shift x units in
PLANE	=	PLANE + PLANESIZE
ENDM
PCOL	=	PCOL + 1
ENDM
hmap320:
	ret

ENDP

END
