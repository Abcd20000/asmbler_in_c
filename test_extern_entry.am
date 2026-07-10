; ============================================================
; test_extern_entry.as  –  .extern and .entry interactions
;
;  Tests:
;   - .extern declaration
;   - .entry declaration
;   - jal to extern → produces .ext line
;   - j   to extern → produces .ext line
;   - beq to extern → produces .ext line
;   - .entry on code label
;   - .entry on data label
; ============================================================

.extern lib_print
.extern lib_init

foo:    addi    $8, $0, 42
        beq     $8, $0, lib_print   
        jal     lib_init            
        j       bar

bar:    jr      $31

result: .dw     0

.entry foo
.entry bar
.entry result
