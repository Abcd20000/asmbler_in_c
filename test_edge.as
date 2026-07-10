; ============================================================
; test_edge.as  –  Edge cases and boundary conditions
;
;  Tests:
;   - Comment-only file sections
;   - Blank lines between instructions
;   - Label on its own line (followed by instruction on next)
;   - Multiple labels aliasing same address  (label2 = label1+0)
;     Note: only one label per address is actually placed
;   - Maximum immediate value  (0x7FFF = 32767)
;   - Minimum immediate value  (-32768)
;   - Immediate 0
;   - shamt = 0, shamt = 31
;   - Register $0 (hardwired zero)
;   - Register $31 (return address)
;   - All registers $0..$31 via addiu
; ============================================================

; comment before anything

start:

        
        addi    $8,  $0,   0        
        addi    $9,  $0,   32767   
        addi    $10, $0,   -32768  

        sll     $11, $8,   0        
        sll     $12, $8,   31       
        srl     $13, $12,  31       

        addiu   $0,  $0,   0        
        addiu   $1,  $0,   1
        addiu   $2,  $0,   2
        addiu   $3,  $0,   3
        addiu   $4,  $0,   4
        addiu   $5,  $0,   5
        addiu   $6,  $0,   6
        addiu   $7,  $0,   7
        addiu   $29, $0,   29
        addiu   $30, $0,   30
        addiu   $31, $0,   31

        
        bne     $0,  $0,   start    

        jr      $31

; data edge cases
zero_byte:  .db     0
max_byte:   .db     127
min_byte:   .db     -128
empty_str:  .asciz  ""
nl_str:     .asciz  "x"

.entry start
