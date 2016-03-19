LABEL: stop

mov #3, r3
mov #3, LABEL

mov LABEL, r3
mov LABEL, LABEL

mov r2, r3
mov r2, LABEL
