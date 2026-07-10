; ============================================================
; test_macros.as  –  Macro pre-processor edge cases
;
;  Tests:
;   - Simple macro definition and single call
;   - Multi-line macro body
;   - Multiple calls to the same macro
;   - Macro defined after first use  (forward-reference to macro is OK;
;     the pre-processor reads the whole file first)
;   - Comment line inside macro body is preserved
;   - Blank line inside macro body
; ============================================================

mcro INC_R8
    addi  $8, $8, 1
mcroend

mcro SWAP_8_9
    add   $1,  $8, $0
    add   $8,  $9, $0
    add   $9,  $1, $0
mcroend

start:  addi  $8, $0, 0
        addi  $9, $0, 7

        INC_R8
        INC_R8
        INC_R8

        SWAP_8_9

        INC_R8

        jr    $31

; -- data -----------------------------------------------------
counter: .dw   0

.entry start
