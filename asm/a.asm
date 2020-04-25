// define our constants
value EQU 0x10

// global variables in the DATA segment. label is the addr in the DATA segment
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
    ADD [lives]
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
    AND 0xFF
    AND [value]
    OR 0x01
    XOR 0xFF

    // loads/stores
    LDA [0xFFFF]  // A <- [addr16]
    LDA [value]   // A <- [addr16]
    LDA 0x10    // A <- imm8
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
    LDA 10
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
    PUSH X
    PUSH SP     // get the stack frame pointer in X
    POP X

    LEAX 12     // get the address of the value
    LEAX 'c'
    LAX         // load the value at [X] into A

    POP X
    RET

//
// Input: X points to the string
// Return: A contains the length
//
_strlen:
    LDA 0
    PUSH A

    LAX
    
    RET