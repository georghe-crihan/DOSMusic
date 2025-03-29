.386p

_TEXT   segment use32 dword public 'CODE'
        assume  cs:_TEXT,ds:_DATA

start:
        jmp short _main

        db 'WATCOM'                     ; The "WATCOM" string is needed in
                                        ; order to run under DOS/4G and WD.

_main:
                sti
                cld

                mov     ax, 351Ch
                int     21h             ; DOS - 2+ - GET INTERRUPT VECTOR
                                        ; AL = interrupt number
                                        ; Return: ES:BX = value of interrupt vector
                mov     dword_10138, ebx
                mov     word_1013A, es
                push    ds
                push    cs
                pop     ds
                mov     edx, offset Timer ; Called 18.2 times per second.
                mov     ax, 251Ch
                int     21h             ; DOS - SET INTERRUPT VECTOR
                                        ; AL = interrupt number
                                        ; DS:DX = new vector to be used for specified interrupt
                pop     ds

@@busy_wait:                            ; CODE XREF: start+1Dâ†“j
                mov     ah, 0
                int     16h             ; KEYBOARD - READ CHAR FROM BUFFER, WAIT IF EMPTY
                                        ; Return: AH = scan code, AL = character
                cmp     al, 1Bh
                jnz     short @@busy_wait
                mov     edx, dword_10138
                mov     ds, word_1013A
                mov     ax, 251Ch
                int     21h             ; DOS - SET INTERRUPT VECTOR
                                        ; AL = interrupt number
                                        ; DS:DX = new vector to be used for specified interrupt
; === Speaker off ===
                in      al, 61h
                and     al, 0FCh
                out     61h, al
                mov     ah, 4Ch
                int     21h             ; DOS - 2+ - QUIT WITH EXIT CODE (EXIT)
                                        ; AL = exit code

; =============== S U B R O U T I N E =======================================

; Called 18.2 times per second.

Timer           proc far                ; DATA XREF: start+Fâ†‘o
                cli
                push    ds
                push    eax
                push    ebx
                mov     ax, _DATA
                mov     ds, ax
                cmp     byte ptr [byte_10194], 0FFh
                jz      short loc_102AF
                cmp     byte ptr [byte_1019B], 0FFh
                jz      short loc_10296
; === Speaker off ===
                in      al, 61h
                and     al, 0FCh
                out     61h, al

loc_10296:                              ; CODE XREF: Timer+14â†‘j
                not     byte ptr [byte_10194]
                not     byte ptr [byte_1019B]
                jmp     short loc_102AF
; ---------------------------------------------------------------------------
                cmp     byte ptr [byte_10194], 0FFh
                jnz     short loc_102AF
                not     byte ptr [byte_10194]

loc_102AF:                              ; CODE XREF: Timer+Câ†‘j Timer+26â†‘j ...
                cmp     byte ptr [byte_1019B], 0FFh
                jz      short @@Return
                mov     ah, byte ptr [cnt1]
                cmp     ah, byte ptr [Len]
                jl      short @@NextNote
                push    ecx
                push    edx
                mov     bx, ptr1
                mov     dx, Tbl1[bx]
                mov     cl, 5
                mov     bx, dx
                shr     bx, cl
                and     dl, 111111b
                dec     dl
                mov     byte ptr [Len], dl
                mov     ax, 13533
                mov     dx, 12h
                cmp     dx, bx
                jnb     short loc_102F9
                div     bx
                out     42h, al         ; Timer 8253-5 (AT: 8254.2).
                mov     al, ah
                out     42h, al         ; Timer 8253-5 (AT: 8254.2).
; === Speaker on ===
                in      al, 61h
                or      al, 3
                out     61h, al
                jmp     short @@reset_cnt1
; ---------------------------------------------------------------------------
; === Speaker off ===

loc_102F9:                              ; CODE XREF: Timer+6Dâ†‘j
                in      al, 61h
                and     al, 0FCh
                out     61h, al

@@reset_cnt1:                           ; CODE XREF: Timer+7Dâ†‘j
                nop
                mov     byte ptr [cnt1], 0
                mov     bx, ptr1
                add     bx, 2
                cmp     bx, end_ptr1
                jle     short @@PopReturn
                xor     bx, bx          ; Reset pointer

@@PopReturn:                            ; CODE XREF: Timer+99â†‘j
                mov     ptr1, bx
                pop     edx
                pop     ecx
                jmp     short @@Return
; ---------------------------------------------------------------------------

@@NextNote:                             ; CODE XREF: Timer+47â†‘j
                inc     ah
                mov     byte ptr [cnt1], ah

@@Return:                               ; CODE XREF: Timer+3Bâ†‘j Timer+A4â†‘j
                pop     ebx
                pop     eax
                pop     ds
                iret
Timer           endp

_TEXT   ends

_DATA   segment use32 dword public 'DATA'

dword_10138     dd 0                    ; DATA XREF: start+7â†‘w start+1Fâ†‘r
word_1013A      dw 0                    ; DATA XREF: start+Bâ†‘w start+24â†‘r
                db 56h dup(0), 0DAh, 0
byte_10194      db 0                    ; DATA XREF: Timer+6â†“r Timer:loc_10296â†“w ...
end_ptr1        dw 0DAh                 ; DATA XREF: Timer+94â†“r
ptr1            dw 0                    ; DATA XREF: Timer+4Bâ†“r Timer+8Câ†“r ...
cnt1            db 0                    ; DATA XREF: Timer+3Dâ†“r Timer+86â†“w ...
Len             db 0                    ; DATA XREF: Timer+42â†“r Timer+60â†“w
byte_1019B      db 0FFh                 ; DATA XREF: Timer+Eâ†“r Timer+21â†“w ...
Tbl1            dw 2BCAh, 5, 2, 2BC2h, 3705h, 5, 4185h, 5, 5782h, 5282h
                                        ; DATA XREF: Timer+50â†“r
                dw 4902h, 5282h, 578Ah, 5, 5285h, 5, 4902h, 2, 4182h, 3A42h
                dw 3702h, 3A42h, 418Ah, 5, 3A45h, 5, 3702h, 2, 370Ah, 20C5h
                dw 2, 20C2h, 2BC2h, 2, 20C2h, 2, 2BC2h, 2, 20C2h, 2, 2BCAh
                dw 5, 2, 2BC2h, 3705h, 5, 4185h, 5, 6E02h, 6202h, 5782h
                dw 6202h, 6E0Ah, 5, 6205h, 5, 5782h, 2, 5282h, 5782h, 6202h
                dw 5782h, 5282h, 2, 6202h, 2, 5282h, 2, 6202h, 2, 4902h
                dw 2, 5782h, 2, 5282h, 5782h, 6202h, 5782h, 5282h, 2, 6202h
                dw 2, 5282h, 2, 6202h, 2, 4902h, 2, 5782h, 2, 5282h, 2
                dw 5282h, 4902h, 4182h, 2, 4182h, 3A42h, 3702h, 2, 4182h
                dw 2, 578Ah, 5, 6202h, 2, 5287h, 5781h, 1, 5794h, 1111h
_DATA   ends

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; STACK
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
stack   segment use32 para stack 'STACK'
        db 1000h dup(?)
stack   ends

                end start
