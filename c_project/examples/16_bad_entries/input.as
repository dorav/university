.extern LABEL

; can't define entry when defined as external.. 
.extern MAIN
.entry MAIN

.entry NOPE

rts

; already defined external..
LABEL:stop
