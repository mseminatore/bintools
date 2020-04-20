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
    LDA
    STA
    LDX
    JMP start