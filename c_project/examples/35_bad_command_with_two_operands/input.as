mov 123, 123
mov r3, 123
mov 123, r1

mov r1, #123
mov #123, #123
mov LABEL, #123

mov 123,
mov #123,
mov #123
LABEL: mov r2
mov LABEL

; These two are OK
mov LABEL, r3
mov LABEL , r3

mov LABEL, #123, r2
mov LABEL, r3, r2
