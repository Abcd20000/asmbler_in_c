; ============================================================
; test_branch_jumps.as  –  Branch offset and jump address tests
;
;  Tests:
;   - Forward branch (positive offset)
;   - Backward branch (negative offset)
;   - j to forward label
;   - j to backward label
;   - jal to local label
;   - Nested conditional loop
; ============================================================

start:
        addi    $8,  $0,  5         

loop:   beq     $8,  $0,  done      
        addi    $8,  $8,  -1        
        j       loop                

done:   addi    $9,  $0,  10
        jal     helper
        j       end_prog

helper: addi    $9,  $9,  1
        jr      $31

end_prog:
        bne     $9,  $0,  skip_end  
        j       end_prog            
skip_end:
        jr      $31

; -- data -----------------------------------------------------
counter: .dw    0

.entry start
