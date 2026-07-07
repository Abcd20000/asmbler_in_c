; test.as  –  small smoke-test for the assembler

mcro SAVE
    sw   $8, 0($29)
    sw   $9, 4($29)
mcroend

.extern puts

start:  addi  $8, $0, 10
        addi  $9, $0, 20
        SAVE
        add   $10, $8, $9
        beq   $10, $0, done
        jal   puts
done:   jr    $31

msg:    .asciz "hello"
nums:   .dw    1, 2, 3
flags:  .db    0xFF, 0x00

.entry start
