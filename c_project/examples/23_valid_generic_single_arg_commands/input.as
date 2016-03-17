; 1AC
LABEL1: clr r0

; 1EC
inc r1

; 1GC
LABEL3: dec r2

; 1IC
LABEL4: jmp r3

; bne = 10 - 1KC
LABEL5: bne r4

; red = 11 - 1MC
LABEL6: red r5

; jsr = 13 - 1QC
LABEL7: jsr r6

clr LABEL1
inc LABEL1
dec LABEL3
jmp LABEL4
bne LABEL5
red LABEL6
jsr LABEL7
