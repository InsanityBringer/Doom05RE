	.386
	.MODEL  small
	INCLUDE defs.inc


;============================================================================
;
; unwound vertical scaling code
;
; eax   light table pointer, 0 lowbyte overwritten
; ebx   all 0, low byte overwritten
; ecx   fractional step value
; edx   fractional scale value
; esi   start of source pixels
; edi   bottom pixel in screenbuffer to blit into
;
; ebx should be set to 0 0 0 dh to feed the pipeline
;
; The graphics wrap vertically at 128 pixels
;============================================================================

.DATA

SCALEDEFINE     MACRO   number
	dd      vscale&number
ENDM

	ALIGN   4
scalecalls LABEL
LINE    =       0
REPT    SCREENHEIGHT+1
	SCALEDEFINE     %LINE
LINE    =       LINE+1
ENDM


;=================================


.CODE

;================
;
; R_RawScaleHighColor
;
;================

R_RawScaleHighColor_ PROC
PUBLIC   R_RawScaleHighColor_
	PUSHR

	mov		ebp,[_sp_y2]
	mov     edi,[_planewidthlookup+ebp*4]
	add		edi,[_sp_x]
	sub		ebp,[_sp_y1]

	mov     edx,[_sp_frac]
	mov		ecx,[_sp_fracstep]

	mov     esi,[_sp_source]

	mov		eax,[_sp_colormap]
	shl		eax,9
	add     eax,[_wordcolormaps]

	shr		eax,1
	xor     ebx,ebx
	shld    ebx,edx,010h						; get address of first location
	call    [scalecalls+4+ebp*4]
	
	POPR
	ret

;============ HIGH COLOR ============

SCALELABEL      MACRO   number
vscale&number:
ENDM

LINE    =       SCREENHEIGHT
INVLINE = 0
REPT SCREENHEIGHT-1
	SCALELABEL      %LINE
	mov     al,[esi+ebx]      				; get source pixel
	mov		ebx,[eax*2]						; translate the color
	mov     [edi-(LINE-1)*PLANEWIDTH],bl   	; draw a pixel to the buffer
	mov     [edi+(INVLINE+1)*PLANEWIDTH],bh ; draw a pixel to the buffer
	add     edx,ecx                         ; calculate next location                    
	xor     ebx,ebx
	shld    ebx,edx,010h                      ; get address of next location
LINE    =       LINE-1
INVLINE =		INVLINE+1
ENDM
vscale1:
	mov     al,[esi+ebx]
	mov     ebx,[eax*2]
	mov     [edi],al   	; draw a pixel to the buffer
	mov     [edi+PLANESIZE],al ; draw a pixel to the buffer
vscale0:
	ret

ENDP

END
