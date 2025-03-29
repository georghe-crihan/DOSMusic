                .8086
                .model tiny

seg000          segment byte public 'CODE'
                assume cs:seg000
                org 100h
                assume es:nothing, ss:nothing, ds:seg000

                public start
start           proc near
                push    cs
                pop     ds
                mov     ax, 351Ch
                int     21h             ; DOS - 2+ - GET INTERRUPT VECTOR
                                        ; AL = interrupt number
                                        ; Return: ES:BX = value of interrupt vector
                mov     word_10138, bx
                mov     word_1013A, es
                mov     dx, offset Timer ; Called 18.2 times per second.
                mov     ax, 251Ch
                int     21h             ; DOS - SET INTERRUPT VECTOR
                                        ; AL = interrupt number
                                        ; DS:DX = new vector to be used for specified interrupt

@@busy_wait:                            ; CODE XREF: start+1D↓j
                mov     ah, 0
                int     16h             ; KEYBOARD - READ CHAR FROM BUFFER, WAIT IF EMPTY
                                        ; Return: AH = scan code, AL = character
                cmp     al, 1Bh
                jnz     short @@busy_wait
                mov     dx, cs:word_10138
                mov     ds, cs:word_1013A
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
start           endp                    ; AL = exit code

; ---------------------------------------------------------------------------
word_10138      dw 0                    ; DATA XREF: start+7↑w start+1F↑r
word_1013A      dw 0                    ; DATA XREF: start+B↑w start+24↑r
                db 56h dup(0), 0DAh, 0
byte_10194      db 0                    ; DATA XREF: Timer+6↓r Timer:loc_10296↓w ...
end_ptr1        dw 0DAh                 ; DATA XREF: Timer+94↓r
ptr1            dw 0                    ; DATA XREF: Timer+4B↓r Timer+8C↓r ...
cnt1            db 0                    ; DATA XREF: Timer+3D↓r Timer+86↓w ...
Length          db 0                    ; DATA XREF: Timer+42↓r Timer+60↓w
byte_1019B      db 0FFh                 ; DATA XREF: Timer+E↓r Timer+21↓w ...
Tbl1            dw 2BCAh, 5, 2, 2BC2h, 3705h, 5, 4185h, 5, 5782h, 5282h
                                        ; DATA XREF: Timer+50↓r
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

; =============== S U B R O U T I N E =======================================

; Called 18.2 times per second.

Timer           proc far                ; DATA XREF: start+F↑o
                cli
                push    ds
                push    ax
                push    bx
                push    cs
                pop     ds
                cmp     cs:byte_10194, 0FFh
                jz      short loc_102AF
                cmp     cs:byte_1019B, 0FFh
                jz      short loc_10296
; === Speaker off ===
                in      al, 61h
                and     al, 0FCh
                out     61h, al

loc_10296:                              ; CODE XREF: Timer+14↑j
                not     cs:byte_10194
                not     cs:byte_1019B
                jmp     short loc_102AF
; ---------------------------------------------------------------------------
                cmp     cs:byte_10194, 0FFh
                jnz     short loc_102AF
                not     cs:byte_10194

loc_102AF:                              ; CODE XREF: Timer+C↑j Timer+26↑j ...
                cmp     cs:byte_1019B, 0FFh
                jz      short @@Return
                mov     ah, cs:cnt1
                cmp     ah, cs:Length
                jl      short @@NextNote
                push    cx
                push    dx
                mov     bx, cs:ptr1
                mov     dx, cs:Tbl1[bx]
                mov     cl, 5
                mov     bx, dx
                shr     bx, cl
                and     dl, 111111b
                dec     dl
                mov     cs:Length, dl
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

loc_102F9:                              ; CODE XREF: Timer+6D↑j
                in      al, 61h
                and     al, 0FCh
                out     61h, al

@@reset_cnt1:                           ; CODE XREF: Timer+7D↑j
                nop
                mov     cs:cnt1, 0
                mov     bx, cs:ptr1
                add     bx, 2
                cmp     bx, cs:end_ptr1
                jle     short @@PopReturn
                xor     bx, bx          ; Reset pointer

@@PopReturn:                            ; CODE XREF: Timer+99↑j
                mov     cs:ptr1, bx
                pop     dx
                pop     cx
                jmp     short @@Return
; ---------------------------------------------------------------------------

@@NextNote:                             ; CODE XREF: Timer+47↑j
                inc     ah
                mov     cs:cnt1, ah

@@Return:                               ; CODE XREF: Timer+3B↑j Timer+A4↑j
                pop     bx
                pop     ax
                pop     ds
                iret
Timer           endp

seg000          ends


                end start
