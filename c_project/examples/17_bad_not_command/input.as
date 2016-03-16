; r0 declared but never resolved
LABEL: not r9

; no arguments
not

; argument can't be a command name
not rts
not .entry

; other addressing methods are not allowed
not #3
not *
