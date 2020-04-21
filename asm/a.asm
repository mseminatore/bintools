value EQU 0x10

lives DB 0
switch DB '/'
timer DB 0xFF

score DW 0
//name ds "daykrew"

START:
    NOP         // do nothing!
    ADD 23      // A <- A + 23
    ADD 'c'     // A <- A + 'c'
    ADD 0xFF
    LDI 0x10    // A <- imm8
    LDA 0xFFFF  // A <- addr16
    LDA value   // A <- addr16
    STA 0xFFFE  // addr16 <- A
    LDX 0x100   // X <- imm16
    JMP start