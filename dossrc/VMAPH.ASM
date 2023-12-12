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
; R_RawScaleMed_
;
;================

R_RawScaleMed_ PROC
PUBLIC   R_RawScaleMed_
	PUSHR

	mov		ebp,[_sp_y2]
	mov		ebx,ebp
	add		ebx,[_windowy]
	mov     edi,[_planewidthlookup+ebx*4]
	mov		ebx,[_sp_x]
	shl		ebx,1
	add		ebx,[_windowx]
	add     edi,[_highcollumntable + ebx*4]

	sub		ebp,[_sp_y1]

	mov     edx,[_sp_frac]
	shl		edx,9							; 7 significant bits, 25 frac
	
	mov		ecx,[_sp_fracstep]
	shl		ecx,9							; 7 significant bits, 25 frac

	mov     esi,[_sp_source]

	mov		eax,[_sp_colormap]
	shl		eax,8
	add     eax,[_colormaps]

	xor     ebx,ebx
	shld    ebx,edx,7						; get address of first location
	call    [scalecalls+4+ebp*4]
	
	POPR
	ret

;================
;
; R_RawScaleHigh_
;
;================

R_RawScaleHigh_ PROC
PUBLIC   R_RawScaleHigh_
	PUSHR

	mov		ebp,[_sp_y2]
	mov		ebx,ebp
	add		ebx,[_windowy]
	mov     edi,[_planewidthlookup+ebx*4]
	mov		ebx,[_sp_x]
	add		ebx,[_windowx]
	add     edi,[_highcollumntable + ebx*4]

	sub		ebp,[_sp_y1]

	mov     edx,[_sp_frac]
	shl		edx,9							; 7 significant bits, 25 frac
	
	mov		ecx,[_sp_fracstep]
	shl		ecx,9							; 7 significant bits, 25 frac

	mov     esi,[_sp_source]

	mov		eax,[_sp_colormap]
	shl		eax,8
	add     eax,[_colormaps]

	xor     ebx,ebx
	shld    ebx,edx,7						; get address of first location
	call    [scalecalls+4+ebp*4]
	
	POPR
	ret

;============ HIGH DETAIL ============

SCALELABEL      MACRO   number
vscale&number:
ENDM

LINE    =       SCREENHEIGHT
REPT SCREENHEIGHT-1
	SCALELABEL      %LINE
	mov     al,[esi+ebx]                    ; get source pixel
	add     edx,ecx                         ; calculate next location
	mov     al,[eax]                        ; translate the color
	xor     ebx,ebx
	shld    ebx,edx,7                      ; get address of next location
	mov     [edi-(LINE-1)*PLANEWIDTH],al   ; draw a pixel to the buffer
LINE    =       LINE-1
ENDM
vscale1:
	mov     al,[esi+ebx]
	mov     al,[eax]
	mov     [edi],al
vscale0:
	ret

ENDP

END
