; without label, to register
red r2

; with label, to register
LABEL: red r1

; without label, to label that is already defined
red LABEL

; with label, to label that is defined later
IMLABEL: red OTHER

OTHER: stop
