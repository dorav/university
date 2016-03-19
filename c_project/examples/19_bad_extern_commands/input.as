; can't have label on .extern or .entry instructions
LABEL: .entry OTHER
ANOTHER: .extern OTHER

; label name must be valid
.extern 3ABC
.extern #abc
.extern ThisNameIsGoingToBeWayTooLongForAnyOneToBotherToRead 

LABEL2: rts

; Can't define extern on existing label
.extern LABEL2

; Too many args
.extern SOMETHING SOMETHING2
.extern SOMETHING,SOMETHING
.extern SOMETHING,
