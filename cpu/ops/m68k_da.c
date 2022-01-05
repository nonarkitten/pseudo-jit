#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

const char* opcodes[] = {
    "000000000x000xxx ORI.BW #,Dx",
    "000000000x01xxxx ORI.BW #,<ea>",
    "000000000x10xxxx ORI.BW #,<ea>",
    "000000000x110xxx ORI.BW #,<ea>",
    "000000000x11100x ORI.BW #,ABS",
    "0000000000111100 ORI to CCR",
    "0000000001111100 ORI to SR",
    "0000000010000xxx ORI.L #,Dx",
    "000000001001xxxx ORI.L #,<ea>",
    "000000001010xxxx ORI.L #,<ea>",
    "0000000010110xxx ORI.L #,<ea>",
    "000000001011100x ORI.L #,ABS",    
    
    "000000100x000xxx ANDI.BW #,Dx",
    "000000100x01xxxx ANDI.BW #,<ea>",
    "000000100x10xxxx ANDI.BW #,<ea>",
    "000000100x110xxx ANDI.BW #,<ea>",
    "000000100x11100x ANDI.BW #,ABS",    
    "0000001000111100 ANDI to CCR",
    "0000001001111100 ANDI to SR",
    "0000001010000xxx ANDI.L #,Dx",
    "000000101001xxxx ANDI.L #,<ea>",
    "000000101010xxxx ANDI.L #,<ea>",
    "0000001010110xxx ANDI.L #,<ea>",
    "000000101011100x ANDI.L #,ABS",    

    "000001000x000xxx SUBI.BW #,Dx",
    "000001000x01xxxx SUBI.BW #,<ea>",
    "000001000x10xxxx SUBI.BW #,<ea>",
    "000001000x110xxx SUBI.BW #,<ea>",
    "000001000x11100x SUBI.BW #,ABS",
    "0000010010000xxx SUBI.L #,Dx",
    "000001001001xxxx SUBI.L #,<ea>",
    "000001001010xxxx SUBI.L #,<ea>",
    "0000010010110xxx SUBI.L #,<ea>",
    "000001001011100x SUBI.L #,ABS",

    "000001100x000xxx ADDI.BW #,Dx",
    "000001100x01xxxx ADDI.BW #,<ea>",
    "000001100x10xxxx ADDI.BW #,<ea>",
    "000001100x110xxx ADDI.BW #,<ea>",
    "000001100x11100x ADDI.BW #,ABS",
    "0000011010000xxx ADDI.L #,Dx",
    "000001101001xxxx ADDI.L #,<ea>",
    "000001101010xxxx ADDI.L #,<ea>",
    "0000011010110xxx ADDI.L #,<ea>",
    "000001101011100x ADDI.L #,ABS",

    "000010100x000xxx EORI.BW #,Dx",
    "000010100x01xxxx EORI.BW #,<ea>",
    "000010100x10xxxx EORI.BW #,<ea>",
    "000010100x110xxx EORI.BW #,<ea>",
    "000010100x11100x EORI.BW #,ABS",    
    "0000101000111100 EORI to CCR",
    "0000101001111100 EORI to SR",
    "0000101010000xxx EORI.L #,Dx",
    "000010101001xxxx EORI.L #,<ea>",
    "000010101010xxxx EORI.L #,<ea>",
    "0000101010110xxx EORI.L #,<ea>",
    "000010101011100x EORI.L #,ABS",    
    
    "000011000x000xxx CMPI.BW #,Dx",
    "000011000x01xxxx CMPI.BW #,<ea>",
    "000011000x10xxxx CMPI.BW #,<ea>",
    "000011000x110xxx CMPI.BW #,<ea>",
    "000011000x11100x CMPI.BW #,ABS",
    "0000110010000xxx CMPI.L #,Dx",
    "000011001001xxxx CMPI.L #,<ea>",
    "000011001010xxxx CMPI.L #,<ea>",
    "0000110010110xxx CMPI.L #,<ea>",
    "000011001011100x CMPI.L #,ABS",

    "0000100000000xxx BCLR #,Dx",
    "000010000001xxxx BCLR #,<ea>",
    "000010000010xxxx BCLR #,<ea>",
    "0000100000110xxx BCLR #,<ea>",
    "000010000011100x BCLR #,ABS",
    "0000xxx100000xxx BCLR Dy,Dx",
    "0000xxx10001xxxx BCLR Dy,<ea>",
    "0000xxx10010xxxx BCLR Dy,<ea>",
    "0000xxx100110xxx BCLR Dy,<ea>",
    "0000xxx10011100x BCLR Dy,ABS",

    "0000100001000xxx BCHG #,Dx",
    "000010000101xxxx BCHG #,<ea>",
    "000010000110xxxx BCHG #,<ea>",
    "0000100001110xxx BCHG #,<ea>",
    "000010000111100x BCHG #,ABS",
    "0000xxx101000xxx BCHG Dy,Dx",
    "0000xxx10101xxxx BCHG Dy,<ea>",
    "0000xxx10110xxxx BCHG Dy,<ea>",
    "0000xxx101110xxx BCHG Dy,<ea>",
    "0000xxx10111100x BCHG Dy,ABS",
    
    "0000100010000xxx BCLR #,Dx",
    "000010001001xxxx BCLR #,<ea>",
    "000010001010xxxx BCLR #,<ea>",
    "0000100010110xxx BCLR #,<ea>",
    "000010001011100x BCLR #,ABS",
    "0000xxx110000xxx BCLR Dy,Dx",
    "0000xxx11001xxxx BCLR Dy,<ea>",
    "0000xxx11010xxxx BCLR Dy,<ea>",
    "0000xxx110110xxx BCLR Dy,<ea>",
    "0000xxx11011100x BCLR Dy,ABS",

    "0000100011000xxx BSET #,Dx",
    "000010001101xxxx BSET #,<ea>",
    "000010001110xxxx BSET #,<ea>",
    "0000100011110xxx BSET #,<ea>",
    "000010001111100x BSET #,ABS",
    "0000xxx111000xxx BSET Dy,Dx",
    "0000xxx11101xxxx BSET Dy,<ea>",
    "0000xxx11110xxxx BSET Dy,<ea>",
    "0000xxx111110xxx BSET Dy,<ea>",
    "0000xxx11111100x BSET Dy,ABS",
    
    "0000xxx1xx001xxx MOVEP",
    
    "0001xxx000000xxx MOVE.B Dx,Dy",
    "0001xxx00001xxxx MOVE.B <ea>,Dy",
    "0001xxx00010xxxx MOVE.B <ea>,Dy",
    "0001xxx000110xxx MOVE.B <ea>,Dy",
    "0001xxx0001110xx MOVE.B <ea>,Dy",
    "0001xxx000111100 MOVE.B #Imm,Dy",
    
    "0001xxx01x000xxx MOVE.B Dx,<ea>", 
    "0001xxx01x01xxxx MOVE.B <ea>,<ea>", 
    "0001xxx01x10xxxx MOVE.B <ea>,<ea>",
    "0001xxx01x110xxx MOVE.B <ea>,<ea>",
    "0001xxx01x1110xx MOVE.B <ea>,<ea>",
    "0001xxx01x111100 MOVE.B #Imm,<ea>",
    
    "0001xxx10x000xxx MOVE.B Dx,<ea>",
    "0001xxx10x01xxxx MOVE.B <ea>,<ea>",
    "0001xxx10x10xxxx MOVE.B <ea>,<ea>",
    "0001xxx10x110xxx MOVE.B <ea>,<ea>",
    "0001xxx10x1110xx MOVE.B <ea>,<ea>",
    "0001xxx10x111100 MOVE.B #Imm,<ea>",
    
    "0001xxx110000xxx MOVE.B Dx,<ea>",
    "0001xxx11001xxxx MOVE.B <ea>,<ea>",
    "0001xxx11010xxxx MOVE.B <ea>,<ea>",
    "0001xxx110110xxx MOVE.B <ea>,<ea>",
    "0001xxx1101110xx MOVE.B <ea>,<ea>",
    "0001xxx110111100 MOVE.B #Imm,<ea>",
    
    "000100x111000xxx MOVE.B Dx,ABS",    
    "000100x11101xxxx MOVE.B <ea>,ABS",    
    "000100x11110xxxx MOVE.B <ea>,ABS",
    "000100x111110xxx MOVE.B <ea>,ABS",    
    "000100x1111110xx MOVE.B <ea>,ABS",
    "000100x111111100 MOVE.B #Imm,ABS",

    "0010xxx0000xxxxx MOVE.L <ea>,Dy",
    "0010xxx00010xxxx MOVE.L <ea>,Dy",
    "0010xxx000110xxx MOVE.L <ea>,Dy",
    "0010xxx0001110xx MOVE.L <ea>,Dy",
    "0010xxx000111100 MOVE.L <ea>,Dy",
    "0010xxx0010xxxxx MOVEA.L <ea>,Ay",
    "0010xxx00110xxxx MOVEA.L <ea>,Ay",
    "0010xxx001110xxx MOVEA.L <ea>,Ay",
    "0010xxx0011110xx MOVEA.L <ea>,Ay",
    "0010xxx001111100 MOVEA.L <ea>,Ay",
    "0010xxx01x0xxxxx MOVE.L <ea>,<ea>",
    "0010xxx01x10xxxx MOVE.L <ea>,<ea>",
    "0010xxx01x110xxx MOVE.L <ea>,<ea>",
    "0010xxx01x1110xx MOVE.L <ea>,<ea>",
    "0010xxx01x111100 MOVE.L <ea>,<ea>",
    "0010xxx10x0xxxxx MOVE.L <ea>,<ea>",
    "0010xxx10x10xxxx MOVE.L <ea>,<ea>",
    "0010xxx10x110xxx MOVE.L <ea>,<ea>",
    "0010xxx10x1110xx MOVE.L <ea>,<ea>",
    "0010xxx10x111100 MOVE.L <ea>,<ea>",
    "0010xxx1100xxxxx MOVE.L <ea>,<ea>",
    "0010xxx11010xxxx MOVE.L <ea>,<ea>",
    "0010xxx110110xxx MOVE.L <ea>,<ea>",
    "0010xxx1101110xx MOVE.L <ea>,<ea>",
    "0010xxx110111100 MOVE.L <ea>,<ea>",
    "001000x1110xxxxx MOVE.L <ea>,ABS",    
    "001000x11110xxxx MOVE.L <ea>,ABS",
    "001000x111110xxx MOVE.L <ea>,ABS",    
    "001000x1111110xx MOVE.L <ea>,ABS",
    "001000x111111100 MOVE.L <ea>,ABS",

    "0011xxx0000xxxxx MOVE.W <ea>,Dy",
    "0011xxx00010xxxx MOVE.W <ea>,Dy",
    "0011xxx000110xxx MOVE.W <ea>,Dy",
    "0011xxx0001110xx MOVE.W <ea>,Dy",
    "0011xxx000111100 MOVE.W <ea>,Dy",
    "0011xxx0010xxxxx MOVEA.W <ea>,Ay",
    "0011xxx00110xxxx MOVEA.W <ea>,Ay",
    "0011xxx001110xxx MOVEA.W <ea>,Ay",
    "0011xxx0011110xx MOVEA.W <ea>,Ay",
    "0011xxx001111100 MOVEA.W <ea>,Ay",
    "0011xxx01x0xxxxx MOVE.W <ea>,<ea>",
    "0011xxx01x10xxxx MOVE.W <ea>,<ea>",
    "0011xxx01x110xxx MOVE.W <ea>,<ea>",
    "0011xxx01x1110xx MOVE.W <ea>,<ea>",
    "0011xxx01x111100 MOVE.W <ea>,<ea>",
    "0011xxx10x0xxxxx MOVE.W <ea>,<ea>",
    "0011xxx10x10xxxx MOVE.W <ea>,<ea>",
    "0011xxx10x110xxx MOVE.W <ea>,<ea>",
    "0011xxx10x1110xx MOVE.W <ea>,<ea>",
    "0011xxx10x111100 MOVE.W <ea>,<ea>",
    "0011xxx1100xxxxx MOVE.W <ea>,<ea>",
    "0011xxx11010xxxx MOVE.W <ea>,<ea>",
    "0011xxx110110xxx MOVE.W <ea>,<ea>",
    "0011xxx1101110xx MOVE.W <ea>,<ea>",
    "0011xxx110111100 MOVE.W <ea>,<ea>",
    "001100x1110xxxxx MOVE.W <ea>,ABS",    
    "001100x11110xxxx MOVE.W <ea>,ABS",
    "001100x111110xxx MOVE.W <ea>,ABS",
    "001100x1111110xx MOVE.W <ea>,ABS",
    "001100x111111100 MOVE.W <ea>,ABS",
    
    "0100000011000xxx MOVE SR,Dx",
    "010000001101xxxx MOVE SR,<ea>",
    "010000001110xxxx MOVE SR,<ea>",
    "0100000011110xxx MOVE SR,<ea>",
    "010000001111100x MOVE SR,ABS",

    "0100010011000xxx MOVE Dx,CCR",
    "010001001101xxxx MOVE <ea>,CCR",
    "010001001110xxxx MOVE <ea>,CCR",
    "0100010011110xxx MOVE <ea>,CCR",
	"01000100111110xx MOVE <ea>,CCR",
	"0100010011111100 MOVE #Imm,CCR",
    
    "0100011011000xxx MOVE Dx,SR",
    "010001101101xxxx MOVE <ea>,SR",
    "010001101110xxxx MOVE <ea>,SR",
    "0100011011110xxx MOVE <ea>,SR",
	"01000110111110xx MOVE <ea>,SR",
	"0100011011111100 MOVE #Imm,SR",
	
    "010000000x000xxx NEGX.BW #,Dx",
    "010000000x01xxxx NEGX.BW #,<ea>",
    "010000000x10xxxx NEGX.BW #,<ea>",
    "010000000x110xxx NEGX.BW #,<ea>",
    "010000000x11100x NEGX.BW #,ABS",
    "0100000010000xxx NEGX.L #,Dx",
    "010000001001xxxx NEGX.L #,<ea>",
    "010000001010xxxx NEGX.L #,<ea>",
    "0100000010110xxx NEGX.L #,<ea>",
    "010000001011100x NEGX.L #,ABS",    

    "010000100x000xxx CLR.BW #,Dx",
    "010000100x01xxxx CLR.BW #,<ea>",
    "010000100x10xxxx CLR.BW #,<ea>",
    "010000100x110xxx CLR.BW #,<ea>",
    "010000100x11100x CLR.BW #,ABS",
    "0100001010000xxx CLR.L #,Dx",
    "010000101001xxxx CLR.L #,<ea>",
    "010000101010xxxx CLR.L #,<ea>",
    "0100001010110xxx CLR.L #,<ea>",
    "010000101011100x CLR.L #,ABS",    

    "010001000x000xxx NEG.BW #,Dx",
    "010001000x01xxxx NEG.BW #,<ea>",
    "010001000x10xxxx NEG.BW #,<ea>",
    "010001000x110xxx NEG.BW #,<ea>",
    "010001000x11100x NEG.BW #,ABS",
    "0100010010000xxx NEG.L #,Dx",
    "010001001001xxxx NEG.L #,<ea>",
    "010001001010xxxx NEG.L #,<ea>",
    "0100010010110xxx NEG.L #,<ea>",
    "010001001011100x NEG.L #,ABS",    

    "010001100x000xxx NOT.BW #,Dx",
    "010001100x01xxxx NOT.BW #,<ea>",
    "010001100x10xxxx NOT.BW #,<ea>",
    "010001100x110xxx NOT.BW #,<ea>",
    "010001100x11100x NOT.BW #,ABS",
    "0100011010000xxx NOT.L #,Dx",
    "010001101001xxxx NOT.L #,<ea>",
    "010001101010xxxx NOT.L #,<ea>",
    "0100011010110xxx NOT.L #,<ea>",
    "010001101011100x NOT.L #,ABS", 

    "0100100010000xxx EXT",
    "0100100011000xxx EXT",

    "0100100000000xxx NBCD #,Dx",
    "010010000001xxxx NBCD #,<ea>",
    "010010000010xxxx NBCD #,<ea>",
    "0100100000110xxx NBCD #,<ea>",
    "010010000011100x NBCD #,ABS",
  
    "0100100001000xxx SWAP",

    "0100100001010xxx PEA #,<ea>",
    "0100100001101xxx PEA #,<ea>",
    "0100100001110xxx PEA #,<ea>",
    "01001000011110xx PEA #,<ea>",
  
    "0100101011111100 ILLEGAL",

    "0100101011000xxx TAS #,Dx",
    "010010101101xxxx TAS #,<ea>",
    "010010101110xxxx TAS #,<ea>",
    "0100101011110xxx TAS #,<ea>",
    "010010101111100x TAS #,ABS",
  
   //01001010ssmmmxxx
   //        --       00,01,10 B W L
    "010010100x000xxx TST.BW #,Dx",
    "010010100x01xxxx TST.BW #,<ea>",
    "010010100x10xxxx TST.BW #,<ea>",
    "010010100x110xxx TST.BW #,<ea>",
    "010010100x11100x TST.BW #,ABS",
    "0100101010000xxx TST.L #,Dx",
    "010010101001xxxx TST.L #,<ea>",
    "010010101010xxxx TST.L #,<ea>",
    "0100101010110xxx TST.L #,<ea>",
    "010010101011100x TST.L #,ABS",
  
    "010011100100xxxx TRAP #",
    "0100111001010xxx LINK Ax",
    "0100111001011xxx UNLK Ax",
    "0100111001100xxx MOVE Ax,USP",
    "0100111001101xxx MOVE USP,Ax",

    "0100111001110000 RESET",
    "0100111001110001 NOP",
    "0100111001110010 STOP",
    "0100111001110011 RTE",
    "0100111001110101 RTS",
    "0100111001110110 TRAPV",
    "0100111001110111 RTR",

    "0100111010010xxx JSR <ea>",
    "0100111010101xxx JSR <ea>",
    "0100111010110xxx JSR <ea>",
    "01001110101110xx JSR <ea>",
        
    "0100111011010xxx JMP <ea>",
    "0100111011101xxx JMP <ea>",
    "0100111011110xxx JMP <ea>",
    "01001110111110xx JMP <ea>",

   //01001d001smmmrrr d=0/1, s=0/1
   // d=0 regs->ea		s=0 word
   // d=1 ea->regs		s=1 long
   
   // d=0 regs->ea
    "010010001x010xxx MOVEM <list>,<ea>",
    "010010001x10xxxx MOVEM <list>,<ea>",
    "010010001x110xxx MOVEM <list>,<ea>",
    "010010001x11100x MOVEM <list>,<ea>",
	
   // d=1 ea->regs	
    //"010011001smmmrrr"
    "010011001x01xxxx MOVEM <ea>,<list>",
    "010011001x101xxx MOVEM <ea>,<list>",
    "010011001x110xxx MOVEM <ea>,<list>",
    "010011001x1110xx MOVEM <ea>,<list>",

   //0100rrr111mmmrrr ea (An), (d16,An), (d8,An,Xn)
   //    ---          all valid
   //          
    "0100xxx111010xxx LEA <ea>,Ay", // (An)
    "0100xxx111101xxx LEA <ea>,Ay", // (d16,An)
    "0100xxx111110xxx LEA <ea>,Ay", // (d8,An,Xn)
    "0100xxx1111110xx LEA <ea>,Ay",

    "0100xxx110000xxx CHK DX,Dy",
    "0100xxx11001xxxx CHK <ea>,Dy",
    "0100xxx11010xxxx CHK <ea>,Dy",
    "0100xxx110110xxx CHK <ea>,Dy",
    "0100xxx11011100x CHK <ea>,Dy",

    "0101xxx00x000xxx ADDQ.BW #,<ea>",
    "0101xxx00x01xxxx ADDQ.BW #,<ea>",
    "0101xxx00x10xxxx ADDQ.BW #,<ea>",
    "0101xxx00x110xxx ADDQ.BW #,<ea>",
    "0101xxx00x11100x ADDQ.BW #,ABS",
    "0101xxx0100xxxxx ADDQ.L #,<ee>",
    "0101xxx01010xxxx ADDQ.L #,<ea>",
    "0101xxx010110xxx ADDQ.L #,<ea>",
    "0101xxx01011100x ADDQ.L #,ABS",    

    "0101xxx10x000xxx SUBQ.BW #,<ea>",
    "0101xxx10x01xxxx SUBQ.BW #,<ea>",
    "0101xxx10x10xxxx SUBQ.BW #,<ea>",
    "0101xxx10x110xxx SUBQ.BW #,<ea>",
    "0101xxx10x11100x SUBQ.BW #,ABS",
    "0101xxx1100xxxxx SUBQ.L #,<ee>",
    "0101xxx11010xxxx SUBQ.L #,<ea>",
    "0101xxx110110xxx SUBQ.L #,<ea>",
    "0101xxx11011100x SUBQ.L #,ABS",    

    "0101xxxx11000xxx SCC Dx",
    "0101xxxx1101xxxx SCC <ea>",
    "0101xxxx1110xxxx SCC <ea>",
    "0101xxxx11110xxx SCC <ea>",
    "0101xxxx1111100x SCC ABS",

    "0101xxxx11001xxx DBcc Dx,label",

	"01100000xxxxxxxx BRA label",
	"01100001xxxxxxxx BSR label",
	"01101xxxxxxxxxxx Bcc label",
	"011001xxxxxxxxxx Bcc label",
	"0110001xxxxxxxxx Bcc label",
	
	"0111xxx0xxxxxxxx MOVEQ Dx",
		
    "1000xxx011000xxx DIVU Dx,Dy",
    "1000xxx01101xxxx DIVU Dx,<ea>",
    "1000xxx01110xxxx DIVU Dx,<ea>",
    "1000xxx011110xxx DIVU Dx,<ea>",
    "1000xxx0111110xx DIVU Dx,ABS",
    "1000xxx011111100 DIVU Dx,#Imm",

    "1000xxx111000xxx DIVS Dx,Dy",
    "1000xxx11101xxxx DIVS Dx,<ea>",
    "1000xxx11110xxxx DIVS Dx,<ea>",
    "1000xxx111110xxx DIVS Dx,<ea>",
    "1000xxx1111110xx DIVS Dx,ABS",
    "1000xxx111111100 DIVS Dx,#Imm",

    "1000xxx100000xxx SBCD Dx,Dy",
    "1000xxx100001xxx SBCD -(Ax),-(Ay)",

    "1000xxx00x000xxx OR.BW Dx,Dy",		// 0 = reg dest, ea source, all ea modes
    "1000xxx00x01xxxx OR.BW <ea>,Dx",
    "1000xxx00x10xxxx OR.BW <ea>,Dx",
    "1000xxx00x110xxx OR.BW <ea>,Dx",
    "1000xxx00x1110xx OR.BW ABS,Dx",
    "1000xxx00x111100 OR.BW #Imm,Dx",
    "1000xxx001001xxx OR.W Ay,Dx",
    "1000xxx10x01xxxx OR.BW Dx,<ea>",		// 1 = ea dest, no PC, Imm or Reg direct
    "1000xxx10x10xxxx OR.BW Dx,<ea>",
    "1000xxx10x110xxx OR.BW Dx,<ea>",
    "1000xxx10x11100x OR.BW Dx,ABS",
    
    "1000xxx010000xxx OR.L Dx,Dy",
    "1000xxx010001xxx OR.L Ay,Dx",
    "1000xxx01001xxxx OR.L <ea>,Dx",
    "1000xxx01010xxxx OR.L <ea>,Dx",
    "1000xxx010110xxx OR.L <ea>,Dx",
    "1000xxx0101110xx OR.L ABS,Dx",
    "1000xxx010111100 OR.L #Imm,Dx",
    "1000xxx11001xxxx OR.L Dx,<ea>",
    "1000xxx11010xxxx OR.L Dx,<ea>",
    "1000xxx110110xxx OR.L Dx,<ea>",
    "1000xxx11011100x OR.L Dx,ABS",  

    "1001xxx00x000xxx SUB.BW Dx,Dy",		// 0 = reg dest, ea source, all ea modes
    "1001xxx00x01xxxx SUB.BW <ea>,Dx",
    "1001xxx00x10xxxx SUB.BW <ea>,Dx",
    "1001xxx00x110xxx SUB.BW <ea>,Dx",
    "1001xxx00x1110xx SUB.BW ABS,Dx",
    "1001xxx00x111100 SUB.BW #Imm,Dx",
    "1001xxx001001xxx SUB.W Ay,Dx",
    "1001xxx10x01xxxx SUB.BW Dx,<ea>",		// 1 = ea dest, no PC, Imm or Reg direct
    "1001xxx10x10xxxx SUB.BW Dx,<ea>",
    "1001xxx10x110xxx SUB.BW Dx,<ea>",
    "1001xxx10x11100x SUB.BW Dx,ABS",
    
    "1001xxx010000xxx SUB.L Dx,Dy",
    "1001xxx010001xxx SUB.L Ay,Dx",
    "1001xxx01001xxxx SUB.L <ea>,Dx",
    "1001xxx01010xxxx SUB.L <ea>,Dx",
    "1001xxx010110xxx SUB.L <ea>,Dx",
    "1001xxx0101110xx SUB.L ABS,Dx",
    "1001xxx010111100 SUB.L #Imm,Dx",
    "1001xxx11001xxxx SUB.L Dx,<ea>",
    "1001xxx11010xxxx SUB.L Dx,<ea>",
    "1001xxx110110xxx SUB.L Dx,<ea>",
    "1001xxx11011100x SUB.L Dx,ABS",

    "1001xxx10x000xxx SUBX.BW Dx,Dy",
    "1001xxx10x001xxx SUBX.BW -(Ax),-(Ay)",
    "1001xxx110000xxx SUBX.L Dx,Dy",
    "1001xxx110001xxx SUBX.L -(Ax),-(Ay)",
    
    "1001xxx0110xxxxx SUBA.W <ea>,An",
    "1001xxx01110xxxx SUBA.W <ea>,An",
    "1001xxx011110xxx SUBA.W <ea>,An",
    "1001xxx0111110xx SUBA.W <ea>,An",
    "1001xxx011111100 SUBA.W #Imm,An",
    
    "1001xxx1110xxxxx SUBA.L <ea>,An",
    "1001xxx11110xxxx SUBA.L <ea>,An",
    "1001xxx111110xxx SUBA.L <ea>,An",
    "1001xxx1111110xx SUBA.L <ea>,An",
    "1001xxx111111100 SUBA.L #Imm,An",    
    
    "1010xxxxxxxxxxxx ALINE Exception",
    
    "1011xxx000000xxx CMP.B <ea>,Dx",
    "1011xxx00001xxxx CMP.B <ea>,Dx",

    "1011xxx0010xxxxx CMP.W <ea>,Dx",
    
    "1011xxx00x10xxxx CMP.BW <ea>,Dx",
    "1011xxx00x110xxx CMP.BW <ea>,Dx",
    "1011xxx00x1110xx CMP.BW ABS,Dx",
    "1011xxx00x111100 CMP.BW #Imm,Dx",
    "1011xxx0100xxxxx CMP.L <ea>,Dx",
    "1011xxx01010xxxx CMP.L <ea>,Dx",
    "1011xxx010110xxx CMP.L <ea>,Dx",
    "1011xxx0101110xx CMP.L ABS,Dx",
    "1011xxx010111100 CMP.L #Imm,Dx",
    
    "1011xxx0110xxxxx CMPA.W <ea>,An",
    "1011xxx01110xxxx CMPA.W <ea>,An",
    "1011xxx011110xxx CMPA.W <ea>,An",
    "1011xxx0111110xx CMPA.W <ea>,An",
    "1011xxx011111100 CMPA.W #Imm,An",
    
    "1011xxx1110xxxxx CMPA.L <ea>,An",
    "1011xxx11110xxxx CMPA.L <ea>,An",
    "1011xxx111110xxx CMPA.L <ea>,An",
    "1011xxx1111110xx CMPA.L <ea>,An",
    "1011xxx111111100 CMPA.L #Imm,An",
    
    "1011xxx100001xxx CMPM.B (Ax)+,(Ay)+",
    "1011xxx101001xxx CMPM.W (Ax)+,(Ay)+",
    "1011xxx110001xxx CMPM.L (Ax)+,(Ay)+",
    
    "1011xxx10x000xxx EOR.BW Dx,Dy",
    "1011xxx10x01xxxx EOR.BW Dx,<ea>",
    "1011xxx10x10xxxx EOR.BW Dx,<ea>",
    "1011xxx10x110xxx EOR.BW Dx,<ea>",
    "1011xxx10x11100x EOR.BW Dx,ABS",
    "1011xxx110000xxx EOR.L Dx,Dy",
    "1011xxx11001xxxx EOR.L Dx,<ea>",
    "1011xxx11010xxxx EOR.L Dx,<ea>",
    "1011xxx110110xxx EOR.L Dx,<ea>",
    "1011xxx11011100x EOR.L Dx,ABS",
    
    "1100xxx00x000xxx AND.BW Dx,Dy",		// 0 = reg dest, ea source, all ea modes
    "1100xxx00x01xxxx AND.BW <ea>,Dx",
    "1100xxx00x10xxxx AND.BW <ea>,Dx",
    "1100xxx00x110xxx AND.BW <ea>,Dx",
    "1100xxx00x1110xx AND.BW ABS,Dx",
    "1100xxx00x111100 AND.BW #Imm,Dx",
    "1100xxx001001xxx AND.W Ay,Dx",
    "1100xxx10x01xxxx AND.BW Dx,<ea>",		// 1 = ea dest, no PC, Imm or Reg direct
    "1100xxx10x10xxxx AND.BW Dx,<ea>",
    "1100xxx10x110xxx AND.BW Dx,<ea>",
    "1100xxx10x11100x AND.BW Dx,ABS",
    
    "1100xxx010000xxx AND.L Dx,Dy",
    "1100xxx010001xxx AND.L Ay,Dx",
    "1100xxx01001xxxx AND.L <ea>,Dx",
    "1100xxx01010xxxx AND.L <ea>,Dx",
    "1100xxx010110xxx AND.L <ea>,Dx",
    "1100xxx0101110xx AND.L ABS,Dx",
    "1100xxx010111100 AND.L #Imm,Dx",
    "1100xxx11001xxxx AND.L Dx,<ea>",
    "1100xxx11010xxxx AND.L Dx,<ea>",
    "1100xxx110110xxx AND.L Dx,<ea>",
    "1100xxx11011100x AND.L Dx,ABS",

    "1100xxx011000xxx MULU Dx,Dy",
    "1100xxx01101xxxx MULU Dx,<ea>",
    "1100xxx01110xxxx MULU Dx,<ea>",
    "1100xxx011110xxx MULU Dx,<ea>",
    "1100xxx0111110xx MULU Dx,ABS",
    "1100xxx011111100 MULU Dx,#Imm",

    "1100xxx111000xxx MULS Dx,Dy",
    "1100xxx11101xxxx MULS Dx,<ea>",
    "1100xxx11110xxxx MULS Dx,<ea>",
    "1100xxx111110xxx MULS Dx,<ea>",
    "1100xxx1111110xx MULS Dx,ABS",
    "1100xxx111111100 MULS Dx,#Imm",

    "1100xxx100000xxx ABCD Dx, Dy",
    "1100xxx100001xxx ABCD -(Ax), -(Ay)",
    
    "1100xxx101000xxx EXG Dx, Dy",
    "1100xxx101001xxx EXG Ax, Ay",
    "1100xxx110001xxx EXG Dx, Ay",
    
    "1101xxx00x000xxx ADD.BW Dx,Dy",		// 0 = reg dest, ea source, all ea modes
    "1101xxx00x01xxxx ADD.BW <ea>,Dx",
    "1101xxx00x10xxxx ADD.BW <ea>,Dx",
    "1101xxx00x110xxx ADD.BW <ea>,Dx",
    "1101xxx00x1110xx ADD.BW ABS,Dx",
    "1101xxx00x111100 ADD.BW #Imm,Dx",
    "1101xxx001001xxx ADD.W Ay,Dx",
    "1101xxx10x01xxxx ADD.BW Dx,<ea>",		// 1 = ea dest, no PC, Imm or Reg direct
    "1101xxx10x10xxxx ADD.BW Dx,<ea>",
    "1101xxx10x110xxx ADD.BW Dx,<ea>",
    "1101xxx10x11100x ADD.BW Dx,ABS",
    
    "1101xxx010000xxx ADD.L Dx,Dy",
    "1101xxx010001xxx ADD.L Ay,Dx",
    "1101xxx01001xxxx ADD.L <ea>,Dx",
    "1101xxx01010xxxx ADD.L <ea>,Dx",
    "1101xxx010110xxx ADD.L <ea>,Dx",
    "1101xxx0101110xx ADD.L ABS,Dx",
    "1101xxx010111100 ADD.L #Imm,Dx",
    "1101xxx11001xxxx ADD.L Dx,<ea>",
    "1101xxx11010xxxx ADD.L Dx,<ea>",
    "1101xxx110110xxx ADD.L Dx,<ea>",
    "1101xxx11011100x ADD.L Dx,ABS",

    "1101xxx10x000xxx ADDX.BW Dx,Dy",
    "1101xxx10x001xxx ADDX.BW -(Ax),-(Ay)",
    "1101xxx110000xxx ADDX.L Dx,Dy",
    "1101xxx110001xxx ADDX.L -(Ax),-(Ay)",
    
    "1101xxx0110xxxxx ADDA.W <ea>,An",
    "1101xxx01110xxxx ADDA.W <ea>,An",
    "1101xxx011110xxx ADDA.W <ea>,An",
    "1101xxx0111110xx ADDA.W <ea>,An",
    "1101xxx011111100 ADDA.W #Imm,An",
    
    "1101xxx1110xxxxx ADDA.L <ea>,An",
    "1101xxx11110xxxx ADDA.L <ea>,An",
    "1101xxx111110xxx ADDA.L <ea>,An",
    "1101xxx1111110xx ADDA.L <ea>,An",
    "1101xxx111111100 ADDA.L #Imm,An",
    
    // Memory shifs
    "1110000x1101xxxx ASd <ea>",
    "1110000x1110xxxx ASd <ea>",
    "1110000x11110xxx ASd <ea>",
    "1110000x1111100x ASd ABS",

    "1110001x1101xxxx LSd <ea>",
    "1110001x1110xxxx LSd <ea>",
    "1110001x11110xxx LSd <ea>",
    "1110001x1111100x LSd ABS",
    
    "1110010x1101xxxx ROXd <ea>",
    "1110010x1110xxxx ROXd <ea>",
    "1110010x11110xxx ROXd <ea>",
    "1110010x1111100x ROXd ABS",

    "1110011x1101xxxx ROd <ea>",
    "1110011x1110xxxx ROd <ea>",
    "1110011x11110xxx ROd <ea>",
    "1110011x1111100x ROd ABS",

    // Register shifts
    "1110xxxx0xx00xxx ASd.BW Dx/#Imm,Dy",
    "1110xxxx10x00xxx ASd.L Dx/#Imm,Dy",
    
    "1110xxxx0xx01xxx LSd.BW Dx/#Imm,Dy",
    "1110xxxx10x01xxx LSd.L Dx/#Imm,Dy",
    
    "1110xxxx0xx10xxx ROXd.BW Dx/#Imm,Dy",
    "1110xxxx10x10xxx ROXd.L Dx/#Imm,Dy",
    
    "1110xxxx0xx11xxx ROd.BW Dx/#Imm,Dy",
    "1110xxxx10x11xxx ROd.L Dx/#Imm,Dy",

    "1111xxxxxxxxxxxx FLINE Exception",

};

//const int OPCODE_COUNT = sizeof(opcodes) / sizeof(char*);
#define OPCODE_COUNT (sizeof(opcodes) / sizeof(char*))

typedef struct {
	uint16_t match;
	uint16_t equal;
	const char *op;
} opcode_t;

char *ea(uint16_t ea) {
	static char buffer[32];
	switch(ea & 0x38) {
	case 0x00: sprintf( buffer, "D%d", ea & 7 ); break;
	case 0x08: sprintf( buffer, "A%d", ea & 7 ); break;
	case 0x10: sprintf( buffer, "(A%d)", ea & 7 ); break;
	case 0x18: sprintf( buffer, "(A%d)+", ea & 7 ); break;
	case 0x20: sprintf( buffer, "-(A%d)", ea & 7 ); break;
	case 0x28: sprintf( buffer, "(d16,A%d)", ea & 7 ); break;
	case 0x30: sprintf( buffer, "(d8,A%d,Xn)", ea & 7 ); break;
	case 0x38:
		switch(ea & 7) {
		case 0: sprintf(buffer, "ABS.W"); break;		
		case 1: sprintf(buffer, "ABS.L"); break;		
		case 2: sprintf(buffer, "(d16,PC)"); break;		
		case 3: sprintf(buffer, "(d8,PC,Xn)"); break;
		case 4: sprintf(buffer, "#Immd"); break;	
		}
		break;
	} // end switch
	return buffer;
}

const char* m68k_disasm(uint16_t op) {
	static bool inited = false;
	static opcode_t ops[OPCODE_COUNT] = { 0 };
	static char buffer[64];
	
	if(!inited) {
		int total_ops = 0;
		inited = true;
		for(int i=0; i<OPCODE_COUNT; i++) {
			uint16_t match = 0, equal = 0, count = 0;
// 			fprintf( stderr, "Parsing %s ", opcodes[i]);
			for(int b=0; b<16; b++) {
				uint16_t bit = 0x8000 >> b;
				char c = opcodes[i][b];
				if(c != 'x') {
					match |= bit;
					if(c == '1') equal |= bit;
				} else {
					count++;
				}
			}
			sprintf( buffer, "%s\n", &opcodes[i][17] );
			ops[i].match = match;
			ops[i].equal = equal;
			ops[i].op = strdup(buffer);
			total_ops += 1 << count;
// 			for(int s=strlen(ops[i].op); s<20; s++) 
// 				fprintf( stderr, " ");
// 			fprintf( stderr, "%04x %04x\n", match, equal);
		}
		for(int i=0; i<0x10000; i++) {
			int count = 0;
			for(int j=0; j<OPCODE_COUNT; j++) {
				if((i & ops[j].match) == ops[j].equal) {
					count++;
				}
			}
			if(count > 1) {
				printf("Verify error with opcode 0x%04X, %d matches:\n", i, count);
				for(int j=0; j<OPCODE_COUNT; j++) {
					if(((i & ops[j].match) == ops[j].equal) && (ops[j].op)) {
						printf(" %d %s", j, opcodes[j]);
					}
				}
				printf("\n");
			}
		}
		printf("%d total opcodes loaded.\n", total_ops);
	}

	//char buffer[64];
	for(int i=0; i<OPCODE_COUNT; i++) {
		if((op & ops[i].match) == ops[i].equal) {
			int x = strlen(ops[i].op) - 5; // back up over \n
			if(memcmp(&ops[i].op[x], "<ea>", 4)) {
				return ops[i].op;
			} else {
				memcpy(buffer, ops[i].op, x);
				strcpy(buffer + x, ea(op & 0x3F));
				strcat(buffer, "\n");
				return buffer;
			}
		}
	}
	
	return "ILLEGAL\n";
}







