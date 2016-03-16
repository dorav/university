; without label, to register
not r2

; with label, to register
LABEL: not r1

; without label, to label that is already defined
not LABEL

; with label, to label that is defined later
IMLABEL: not OTHER

OTHER: stop
