; ============================================================
; test_errors.as  –  Error detection cases
;
;  The assembler should report errors and NOT produce .ob/.ent/.ext
;  for a file with errors.
;
;  Errors planted:
;   1. Undefined label in branch target (line ~18)
;   2. Unknown instruction mnemonic     (line ~20)
;   3. Wrong number of operands for add (line ~22)
;   4. Invalid register ($99)           (line ~24)
;   5. .asciz string not closed         (line ~26)
; ============================================================

start:  addi    $8,  $0,  1

        
        beq     $8,  $0,  nowhere

        
        mov     $9,  $8

        
        add     $10, $8

        
        addi    $11, $99, 0

        
        bad:    .asciz  "unterminated

        jr      $31
