; this should appear in the extern file
.extern ABC

; The ARE flags are affected but instruction location is still first command
red ABC
red another
bne another

.extern another

; This should not affect the output at all
.extern ABC2
