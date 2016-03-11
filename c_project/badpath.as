; This file is intended to check the bad-path of the assembler.
; Each line (except a few) contains an error in the assembly language code.
; A comment preceding each line explains the error.
;
; Run the assembler on this file, and verify that it catches all the errors.
;
; Your assembler's messages need not be identical to the comments in this file.
; 
; Disclaimer: this list of errors is not exhaustive; 
;             you are encouraged to identify additional errors.

; 1. this line is ok 
Start:  dec r1
  
; 2. missing operand   
        sub #5

; 3. missing operand
        red
		
; 4. invalid target operand (immediate)		
        add  #5,#6		
	   
; 5. this line is ok (immediate target operand allowed)	   
Next:   cmp  #5,#6

; 6. invalid target operand (random)
        cmp  r1,***	   
	   
; 7. invalid operand (random) 
        jmp  *
	   
; 8. invalid operand (immediate)
        inc  #50	   

; 9. undefined instruction  	   
        and  r1,r2  

; 10. undefined instruction (case sensitivity)   
        jSr  Start
	   
; 11. this line is ok (r9 is a label defined below)
        add  r1,r9  
	 
; 12. invalid characters (,r2)
        cmp  r1,,r2
		
; 13. invalid characters (,r3)
	    add  #5,r1,r3	   
		
; 14. invalid characters (blabla)
        prn r1 blabla
        
; 15. invalid characters (*,r1)
        add ****,r1

; 16. invalid characters (123)
        inc  123
        
; 17. undefined label as first operand
        mov  data7,r1		

; 18. undefined label as second operand
        mov  r1,Str1
	   
; 19. label previously defined
Next:   clr  r2	   
	   
; 20. invalid source operand (random register)	   
	    lea  *,r1
	   
; 21. invalid characters (Start)  	   
 	    stop Start 
	   
; 22. invalid characters (4000) 
        .data   200 4000
		
; 23. this line is ok
DATA1:  .data   3000,	4000,	5000, 6000		

; 24. invalid characters (,3)
        .data   1, ,3
        
; 25. invalid character (,)
        .data   4,		
		
; 26. invalid characters (#123)
        .data   #123

; 27. invalid characters (.4)
        .data   12.4
		
; 28. invalid characters (-5) 
        .data   --5		
		
; 29. this line is ok (case sensitive labels)		
Data1:  .data   100, +200, -300

; 30. invalid label (cannot be an instruction)
mov:    .data   5

; 31. invalid label (cannot be a register)
r1:     .data   200,300

; 32. label previously defined
DATA1:  .data   300

; 33. invalid label(non-alphabetic first character)
1DATA:  .data   300

; 34. this line is ok (r9 is not the name of a register)
r9:		.data   200

; 35. data overflow (value does not fit in a word)
        .data   200000
		
; 36. data overflow (value does not fit in a word)		
	    .data   -400000
		
; 37. this line is ok (label declaration X is ignored)		
X:	    .entry  DATA1

; 38. local label cannot be declared as external 
        .extern Start
		
; 39. this line is ok (label declaration Y is ignored)		
Y:	    .extern DATA8

; 40. undefined label		
        .entry  DATA9
		
; 41. undefined label 
        .entry  X

; 42. undefined instruction (note: DATA2 is not a label declaration) 
DATA2   .data   4

; 43. undefined directive (case sensitivity)
        .DATA   5
		
; 44. invalid characters (blabla is not a string)  		 
        .string blabla

; 45. invalid characters (blabla)
        .string "abcdefg" blabla

; 46. this line is ok (comma within string is not a separator)
STR1:   .string "abc, ,defg"
		
; 47. invalid label (too long)
SuperCalifragilisticExpiAlidocious: .data 4		
		