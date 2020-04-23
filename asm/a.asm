// define our constants
value EQU 0x10

// global variables
lives DB 0
switch DB '/'
timer DB 0xFF

score DW 0x1234
//name ds "daykrew"

    // program origin
//   ORG 0x20

START:
    NOP         // do nothing!

    // arithmetic
    ADD 23      // A <- A + 23
    ADD 'c'     // A <- A + 'c'
    ADD 0xFF
    ADD value
    ADD [0x100]
    ADD [value]
    SUB 23
    SUB 'a'
    SUB 0x01
    SUB value
    SUB [0xFFFF]
    SUB [value]
    SUB [timer]

    // branching
//    call div

    // logic

    // loads/stores
    LDA 0xFFFF  // A <- [addr16]
    LDA value   // A <- [addr16]
    LDI 0x10    // A <- imm8
    LDX 0x100   // X <- imm16
    LAX         // A <- [X]
    STA 0xFFFE  // addr16 <- A
    STA timer   // addr16 <- A

    // pushes and pops
    PUSH A
    POP A
    PUSH A,X
    PUSH A CC
    POP A,X

    // for i = 1, 10 loop
    LDI 10
loop1:
    PUSH A

    // do something!

    POP A
    SUB 1
    JNE loop1

done:    
    JMP done   // loop forever

//
// divide
//
div:
    RET