; No content after label
MAIN:

; Two or more delimiters after label
MAIN2::
MAIN3:::

; No label name
: rts

; No label name and no content
:

; Bad name
3Main: rts
4Main:
4Main::

; this is 31 chars long, it's too long
a234567890123456789012345678901: rts

; this is 30 chars long, it's valid
a23456789012345678901234567890: rts

; Label exists
MAIN: rts
MAIN: rts

; Valid label, case sensitive
main: rts

; Label can't be keywords or registers
stop: rts
r0: rts
r7: rts
PSW:rts
PC:rts
SP:stop

; Not supporting two labels at the same line
JABA: THE: HAT: rts

stop
r9:rts
 