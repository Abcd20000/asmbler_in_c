; ============================================================
; test_data.as  –  Data directives only
;
;  Tests:
;   - .db  (byte,  1 byte each, signed)
;   - .dh  (half,  2 bytes each, little-endian)
;   - .dw  (word,  4 bytes each, little-endian)
;   - .asciz (NUL-terminated string)
;   - Multiple values per directive
;   - Negative values
;   - Hex values
;   - Labels on data directives
; ============================================================

start:  jr   $31

b1:     .db     0
b2:     .db     127, -128, 255
h1:     .dh     0x1234
h2:     .dh     -1, 0x7FFF
w1:     .dw     0
w2:     .dw     0x12345678, -1, 0x7FFFFFFF
s1:     .asciz  "abc"
s2:     .asciz  ""
s3:     .asciz  "line1"

.entry start
