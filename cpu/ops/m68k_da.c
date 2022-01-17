#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char* opcodes[] = {
    "000000000x000xxx ORI.BW #,%E",
    "000000000x01xxxx ORI.BW #,%E",
    "000000000x10xxxx ORI.BW #,%E",
    "000000000x110xxx ORI.BW #,%E",
    "000000000x11100x ORI.BW #,%E",
    "0000000000111100 ORI to CCR",
    "0000000001111100 ORI to SR",
    "0000000010000xxx ORI.L #,%E",
    "000000001001xxxx ORI.L #,%E",
    "000000001010xxxx ORI.L #,%E",
    "0000000010110xxx ORI.L #,%E",
    "000000001011100x ORI.L #,%E",    
    
    "000000100x000xxx ANDI.BW #,%E",
    "000000100x01xxxx ANDI.BW #,%E",
    "000000100x10xxxx ANDI.BW #,%E",
    "000000100x110xxx ANDI.BW #,%E",
    "000000100x11100x ANDI.BW #,%E",    
    "0000001000111100 ANDI to CCR",
    "0000001001111100 ANDI to SR",
    "0000001010000xxx ANDI.L #,%E",
    "000000101001xxxx ANDI.L #,%E",
    "000000101010xxxx ANDI.L #,%E",
    "0000001010110xxx ANDI.L #,%E",
    "000000101011100x ANDI.L #,%E",    

    "000001000x000xxx SUBI.BW #,%E",
    "000001000x01xxxx SUBI.BW #,%E",
    "000001000x10xxxx SUBI.BW #,%E",
    "000001000x110xxx SUBI.BW #,%E",
    "000001000x11100x SUBI.BW #,%E",
    "0000010010000xxx SUBI.L #,%E",
    "000001001001xxxx SUBI.L #,%E",
    "000001001010xxxx SUBI.L #,%E",
    "0000010010110xxx SUBI.L #,%E",
    "000001001011100x SUBI.L #,%E",

    "000001100x000xxx ADDI.BW #,%E",
    "000001100x01xxxx ADDI.BW #,%E",
    "000001100x10xxxx ADDI.BW #,%E",
    "000001100x110xxx ADDI.BW #,%E",
    "000001100x11100x ADDI.BW #,%E",
    "0000011010000xxx ADDI.L #,%E",
    "000001101001xxxx ADDI.L #,%E",
    "000001101010xxxx ADDI.L #,%E",
    "0000011010110xxx ADDI.L #,%E",
    "000001101011100x ADDI.L #,%E",

    "000010100x000xxx EORI.BW #,%E",
    "000010100x01xxxx EORI.BW #,%E",
    "000010100x10xxxx EORI.BW #,%E",
    "000010100x110xxx EORI.BW #,%E",
    "000010100x11100x EORI.BW #,%E",    
    "0000101000111100 EORI to CCR",
    "0000101001111100 EORI to SR",
    "0000101010000xxx EORI.L #,%E",
    "000010101001xxxx EORI.L #,%E",
    "000010101010xxxx EORI.L #,%E",
    "0000101010110xxx EORI.L #,%E",
    "000010101011100x EORI.L #,%E",    
    
    "000011000x000xxx CMPI.BW #,%E",
    "000011000x01xxxx CMPI.BW #,%E",
    "000011000x10xxxx CMPI.BW #,%E",
    "000011000x110xxx CMPI.BW #,%E",
    "000011000x11100x CMPI.BW #,%E",
    "0000110010000xxx CMPI.L #,%E",
    "000011001001xxxx CMPI.L #,%E",
    "000011001010xxxx CMPI.L #,%E",
    "0000110010110xxx CMPI.L #,%E",
    "000011001011100x CMPI.L #,%E",

    "0000100000000xxx BCLR #,%E",
    "000010000001xxxx BCLR #,%E",
    "000010000010xxxx BCLR #,%E",
    "0000100000110xxx BCLR #,%E",
    "000010000011100x BCLR #,%E",
    "0000xxx100000xxx BCLR D%N,%E",
    "0000xxx10001xxxx BCLR D%N,%E",
    "0000xxx10010xxxx BCLR D%N,%E",
    "0000xxx100110xxx BCLR D%N,%E",
    "0000xxx10011100x BCLR D%N,%E",

    "0000100001000xxx BCHG #,%E",
    "000010000101xxxx BCHG #,%E",
    "000010000110xxxx BCHG #,%E",
    "0000100001110xxx BCHG #,%E",
    "000010000111100x BCHG #,%E",
    "0000xxx101000xxx BCHG D%N,%E",
    "0000xxx10101xxxx BCHG D%N,%E",
    "0000xxx10110xxxx BCHG D%N,%E",
    "0000xxx101110xxx BCHG D%N,%E",
    "0000xxx10111100x BCHG D%N,%E",
    
    "0000100010000xxx BCLR #,%E",
    "000010001001xxxx BCLR #,%E",
    "000010001010xxxx BCLR #,%E",
    "0000100010110xxx BCLR #,%E",
    "000010001011100x BCLR #,%E",
    "0000xxx110000xxx BCLR D%N,%E",
    "0000xxx11001xxxx BCLR D%N,%E",
    "0000xxx11010xxxx BCLR D%N,%E",
    "0000xxx110110xxx BCLR D%N,%E",
    "0000xxx11011100x BCLR D%N,%E",

    "0000100011000xxx BSET #,%E",
    "000010001101xxxx BSET #,%E",
    "000010001110xxxx BSET #,%E",
    "0000100011110xxx BSET #,%E",
    "000010001111100x BSET #,%E",
    "0000xxx111000xxx BSET D%N,%E",
    "0000xxx11101xxxx BSET D%N,%E",
    "0000xxx11110xxxx BSET D%N,%E",
    "0000xxx111110xxx BSET D%N,%E",
    "0000xxx11111100x BSET D%N,%E",
    
    "0000xxx10x001xxx MOVEP %E,D%N", //M->R
    "0000xxx11x001xxx MOVEP D%N,%E", //R->M
    
    "0001xxx000000xxx MOVE.B %E,%E",
    "0001xxx00001xxxx MOVE.B %E,%E",
    "0001xxx00010xxxx MOVE.B %E,%E",
    "0001xxx000110xxx MOVE.B %E,%E",
    "0001xxx0001110xx MOVE.B %E,%E",
    "0001xxx000111100 MOVE.B %E,%E",
  
    "0001xxx01x000xxx MOVE.B %E,%E", 
    "0001xxx01x01xxxx MOVE.B %E,%E", 
    "0001xxx01x10xxxx MOVE.B %E,%E",
    "0001xxx01x110xxx MOVE.B %E,%E",
    "0001xxx01x1110xx MOVE.B %E,%E",
    "0001xxx01x111100 MOVE.B %E,%E",
    
    "0001xxx10x000xxx MOVE.B %E,%E",
    "0001xxx10x01xxxx MOVE.B %E,%E",
    "0001xxx10x10xxxx MOVE.B %E,%E",
    "0001xxx10x110xxx MOVE.B %E,%E",
    "0001xxx10x1110xx MOVE.B %E,%E",
    "0001xxx10x111100 MOVE.B %E,%E",
    
    "0001xxx110000xxx MOVE.B %E,%E",
    "0001xxx11001xxxx MOVE.B %E,%E",
    "0001xxx11010xxxx MOVE.B %E,%E",
    "0001xxx110110xxx MOVE.B %E,%E",
    "0001xxx1101110xx MOVE.B %E,%E",
    "0001xxx110111100 MOVE.B %E,%E",
    
    "000100x111000xxx MOVE.B %E,%E",    
    "000100x11101xxxx MOVE.B %E,%E",    
    "000100x11110xxxx MOVE.B %E,%E",
    "000100x111110xxx MOVE.B %E,%E",    
    "000100x1111110xx MOVE.B %E,%E",
    "000100x111111100 MOVE.B %E,%E",

    "0010xxx0000xxxxx MOVE.L %E,%E",
    "0010xxx00010xxxx MOVE.L %E,%E",
    "0010xxx000110xxx MOVE.L %E,%E",
    "0010xxx0001110xx MOVE.L %E,%E",
    "0010xxx000111100 MOVE.L %E,%E",
    "0010xxx0010xxxxx MOVEA.L %E,%E",
    "0010xxx00110xxxx MOVEA.L %E,%E",
    "0010xxx001110xxx MOVEA.L %E,%E",
    "0010xxx0011110xx MOVEA.L %E,%E",
    "0010xxx001111100 MOVEA.L %E,%E",
    "0010xxx01x0xxxxx MOVE.L %E,%E",
    "0010xxx01x10xxxx MOVE.L %E,%E",
    "0010xxx01x110xxx MOVE.L %E,%E",
    "0010xxx01x1110xx MOVE.L %E,%E",
    "0010xxx01x111100 MOVE.L %E,%E",
    "0010xxx10x0xxxxx MOVE.L %E,%E",
    "0010xxx10x10xxxx MOVE.L %E,%E",
    "0010xxx10x110xxx MOVE.L %E,%E",
    "0010xxx10x1110xx MOVE.L %E,%E",
    "0010xxx10x111100 MOVE.L %E,%E",
    "0010xxx1100xxxxx MOVE.L %E,%E",
    "0010xxx11010xxxx MOVE.L %E,%E",
    "0010xxx110110xxx MOVE.L %E,%E",
    "0010xxx1101110xx MOVE.L %E,%E",
    "0010xxx110111100 MOVE.L %E,%E",
    "001000x1110xxxxx MOVE.L %E,%E",    
    "001000x11110xxxx MOVE.L %E,%E",
    "001000x111110xxx MOVE.L %E,%E",    
    "001000x1111110xx MOVE.L %E,%E",
    "001000x111111100 MOVE.L %E,%E",

    "0011xxx0000xxxxx MOVE.W %E,%E",
    "0011xxx00010xxxx MOVE.W %E,%E",
    "0011xxx000110xxx MOVE.W %E,%E",
    "0011xxx0001110xx MOVE.W %E,%E",
    "0011xxx000111100 MOVE.W %E,%E",
    "0011xxx0010xxxxx MOVEA.W %E,%E",
    "0011xxx00110xxxx MOVEA.W %E,%E",
    "0011xxx001110xxx MOVEA.W %E,%E",
    "0011xxx0011110xx MOVEA.W %E,%E",
    "0011xxx001111100 MOVEA.W %E,%E",
    "0011xxx01x0xxxxx MOVE.W %E,%E",
    "0011xxx01x10xxxx MOVE.W %E,%E",
    "0011xxx01x110xxx MOVE.W %E,%E",
    "0011xxx01x1110xx MOVE.W %E,%E",
    "0011xxx01x111100 MOVE.W %E,%E",
    "0011xxx10x0xxxxx MOVE.W %E,%E",
    "0011xxx10x10xxxx MOVE.W %E,%E",
    "0011xxx10x110xxx MOVE.W %E,%E",
    "0011xxx10x1110xx MOVE.W %E,%E",
    "0011xxx10x111100 MOVE.W %E,%E",
    "0011xxx1100xxxxx MOVE.W %E,%E",
    "0011xxx11010xxxx MOVE.W %E,%E",
    "0011xxx110110xxx MOVE.W %E,%E",
    "0011xxx1101110xx MOVE.W %E,%E",
    "0011xxx110111100 MOVE.W %E,%E",
    "001100x1110xxxxx MOVE.W %E,%E",    
    "001100x11110xxxx MOVE.W %E,%E",
    "001100x111110xxx MOVE.W %E,%E",
    "001100x1111110xx MOVE.W %E,%E",
    "001100x111111100 MOVE.W %E,%E",
    
    "0100000011000xxx MOVE SR,%E",
    "010000001101xxxx MOVE SR,%E",
    "010000001110xxxx MOVE SR,%E",
    "0100000011110xxx MOVE SR,%E",
    "010000001111100x MOVE SR,%E",

    "0100010011000xxx MOVE %E,CCR",
    "010001001101xxxx MOVE %E,CCR",
    "010001001110xxxx MOVE %E,CCR",
    "0100010011110xxx MOVE %E,CCR",
	"01000100111110xx MOVE %E,CCR",
	"0100010011111100 MOVE %E,CCR",
    
    "0100011011000xxx MOVE %E,SR",
    "010001101101xxxx MOVE %E,SR",
    "010001101110xxxx MOVE %E,SR",
    "0100011011110xxx MOVE %E,SR",
	"01000110111110xx MOVE %E,SR",
	"0100011011111100 MOVE %E,SR",
	
    "010000000x000xxx NEGX.BW %E",
    "010000000x01xxxx NEGX.BW %E",
    "010000000x10xxxx NEGX.BW %E",
    "010000000x110xxx NEGX.BW %E",
    "010000000x11100x NEGX.BW %E",
    "0100000010000xxx NEGX.L %E",
    "010000001001xxxx NEGX.L %E",
    "010000001010xxxx NEGX.L %E",
    "0100000010110xxx NEGX.L %E",
    "010000001011100x NEGX.L %E",    

    "010000100x000xxx CLR.BW %E",
    "010000100x01xxxx CLR.BW %E",
    "010000100x10xxxx CLR.BW %E",
    "010000100x110xxx CLR.BW %E",
    "010000100x11100x CLR.BW %E",
    "0100001010000xxx CLR.L %E",
    "010000101001xxxx CLR.L %E",
    "010000101010xxxx CLR.L %E",
    "0100001010110xxx CLR.L %E",
    "010000101011100x CLR.L %E",    

    "010001000x000xxx NEG.BW %E",
    "010001000x01xxxx NEG.BW %E",
    "010001000x10xxxx NEG.BW %E",
    "010001000x110xxx NEG.BW %E",
    "010001000x11100x NEG.BW %E",
    "0100010010000xxx NEG.L %E",
    "010001001001xxxx NEG.L %E",
    "010001001010xxxx NEG.L %E",
    "0100010010110xxx NEG.L %E",
    "010001001011100x NEG.L %E",    

    "010001100x000xxx NOT.BW %E",
    "010001100x01xxxx NOT.BW %E",
    "010001100x10xxxx NOT.BW %E",
    "010001100x110xxx NOT.BW %E",
    "010001100x11100x NOT.BW %E",
    "0100011010000xxx NOT.L %E",
    "010001101001xxxx NOT.L %E",
    "010001101010xxxx NOT.L %E",
    "0100011010110xxx NOT.L %E",
    "010001101011100x NOT.L %E", 

    "0100100010000xxx EXT.W %E",
    "0100100011000xxx EXT.L %E",

    "0100100000000xxx NBCD #,%E",
    "010010000001xxxx NBCD #,%E",
    "010010000010xxxx NBCD #,%E",
    "0100100000110xxx NBCD #,%E",
    "010010000011100x NBCD #,%E",
  
    "0100100001000xxx SWAP %E",

    "0100100001010xxx PEA %E",
    "0100100001101xxx PEA %E",
    "0100100001110xxx PEA %E",
    "01001000011110xx PEA %E",
  
    "0100101011111100 ILLEGAL",

    "0100101011000xxx TAS %E",
    "010010101101xxxx TAS %E",
    "010010101110xxxx TAS %E",
    "0100101011110xxx TAS %E",
    "010010101111100x TAS %E",
  
   //01001010ssmmmxxx
   //        --       00,01,10 B W L
    "010010100x000xxx TST.BW %E",
    "010010100x01xxxx TST.BW %E",
    "010010100x10xxxx TST.BW %E",
    "010010100x110xxx TST.BW %E",
    "010010100x11100x TST.BW %E",
    "0100101010000xxx TST.L %E",
    "010010101001xxxx TST.L %E",
    "010010101010xxxx TST.L %E",
    "0100101010110xxx TST.L %E",
    "010010101011100x TST.L %E",
  
    "0100111001000xxx TRAP #%R",
    "0100111001001000 TRAP #8",
    "0100111001001001 TRAP #9",
    "0100111001001010 TRAP #10",
    "0100111001001011 TRAP #11",
    "0100111001001100 TRAP #12",
    "0100111001001101 TRAP #13",
    "0100111001001110 TRAP #14",
    "0100111001001111 TRAP #15",
	
    "0100111001010xxx LINK A%R",
    "0100111001011xxx UNLK A%R",
    "0100111001100xxx MOVE A%R,USP",
    "0100111001101xxx MOVE USP,A%R",

    "0100111001110000 RESET",
    "0100111001110001 NOP",
    "0100111001110010 STOP",
    "0100111001110011 RTE",
    "0100111001110101 RTS",
    "0100111001110110 TRAPV",
    "0100111001110111 RTR",

    "0100111010010xxx JSR %E",
    "0100111010101xxx JSR %E",
    "0100111010110xxx JSR %E",
    "01001110101110xx JSR %E",
        
    "0100111011010xxx JMP %E",
    "0100111011101xxx JMP %E",
    "0100111011110xxx JMP %E",
    "01001110111110xx JMP %E",

   //01001d001smmmrrr d=0/1, s=0/1
   // d=0 regs->ea		s=0 word
   // d=1 ea->regs		s=1 long
   
   // d=0 regs->ea
    "010010001x010xxx MOVEM <list>,%E",
    "010010001x10xxxx MOVEM <list>,%E",
    "010010001x110xxx MOVEM <list>,%E",
    "010010001x11100x MOVEM <list>,%E",
	
   // d=1 ea->regs	
    //"010011001smmmrrr"
    "010011001x01xxxx MOVEM %E,<list>",
    "010011001x101xxx MOVEM %E,<list>",
    "010011001x110xxx MOVEM %E,<list>",
    "010011001x1110xx MOVEM %E,<list>",

   //0100rrr111mmmrrr ea (Ax), (d16,Ax), (d8,Ax,Xn)
   //    ---          all valid
   //          
    "0100xxx111010xxx LEA %E,A%R", // (Ax)
    "0100xxx111101xxx LEA %E,A%R", // (d16,Ax)
    "0100xxx111110xxx LEA %E,A%R", // (d8,Ax,Xn)
    "0100xxx1111110xx LEA %E,A%R",

    "0100xxx110000xxx CHK %E,D%N",
    "0100xxx11001xxxx CHK %E,D%N",
    "0100xxx11010xxxx CHK %E,D%N",
    "0100xxx110110xxx CHK %E,D%N",
    "0100xxx11011100x CHK %E,D%N",

    "0101xxx00x000xxx ADDQ.BW #%N,%E",
    "0101xxx00x01xxxx ADDQ.BW #%N,%E",
    "0101xxx00x10xxxx ADDQ.BW #%N,%E",
    "0101xxx00x110xxx ADDQ.BW #%N,%E",
    "0101xxx00x11100x ADDQ.BW #%N,%E",
    "0101xxx0100xxxxx ADDQ.L #%N,%E",
    "0101xxx01010xxxx ADDQ.L #%N,%E",
    "0101xxx010110xxx ADDQ.L #%N,%E",
    "0101xxx01011100x ADDQ.L #%N,%E",    

    "0101xxx10x000xxx SUBQ.BW #%N,%E",
    "0101xxx10x01xxxx SUBQ.BW #%N,%E",
    "0101xxx10x10xxxx SUBQ.BW #%N,%E",
    "0101xxx10x110xxx SUBQ.BW #%N,%E",
    "0101xxx10x11100x SUBQ.BW #%N,%E",
    "0101xxx1100xxxxx SUBQ.L #%N,%E",
    "0101xxx11010xxxx SUBQ.L #%N,%E",
    "0101xxx110110xxx SUBQ.L #%N,%E",
    "0101xxx11011100x SUBQ.L #%N,%E",    

    "0101xxxx11000xxx SCC %E",
    "0101xxxx1101xxxx SCC %E",
    "0101xxxx1110xxxx SCC %E",
    "0101xxxx11110xxx SCC %E",
    "0101xxxx1111100x SCC %E",

    "0101xxxx11001xxx DBcc D%R,label",

	"01100000xxxxxxxx BRA label",
	"01100001xxxxxxxx BSR label",
	"01101xxxxxxxxxxx Bcc label",
	"011001xxxxxxxxxx Bcc label",
	"0110001xxxxxxxxx Bcc label",
	
	"0111xxx0xxxxxxxx MOVEQ #Immd,D%N",
		
    "1000xxx011000xxx DIVU %E,D%N",
    "1000xxx01101xxxx DIVU %E,D%N",
    "1000xxx01110xxxx DIVU %E,D%N",
    "1000xxx011110xxx DIVU %E,D%N",
    "1000xxx0111110xx DIVU %E,D%N",
    "1000xxx011111100 DIVU %E,D%N",

    "1000xxx111000xxx DIVS %E,D%N",
    "1000xxx11101xxxx DIVS %E,D%N",
    "1000xxx11110xxxx DIVS %E,D%N",
    "1000xxx111110xxx DIVS %E,D%N",
    "1000xxx1111110xx DIVS %E,D%N",
    "1000xxx111111100 DIVS %E,D%N",

    "1000xxx100000xxx SBCD D%R,D%N",
    "1000xxx100001xxx SBCD -(A%R),-(A%N)",

    "1000xxx00x000xxx OR.BW %E,D%N",		// 0 = reg dest, ea source, all ea modes
    "1000xxx00x01xxxx OR.BW %E,D%N",
    "1000xxx00x10xxxx OR.BW %E,D%N",
    "1000xxx00x110xxx OR.BW %E,D%N",
    "1000xxx00x1110xx OR.BW %E,D%N",
    "1000xxx00x111100 OR.BW %E,D%N",
    "1000xxx001001xxx OR.W A%N,%E",
    "1000xxx10x01xxxx OR.BW D%N,%E",		// 1 = ea dest, no PC, Imm or Reg direct
    "1000xxx10x10xxxx OR.BW D%N,%E",
    "1000xxx10x110xxx OR.BW D%N,%E",
    "1000xxx10x11100x OR.BW D%N,%E",
    
    "1000xxx010000xxx OR.L %E,D%N",
    "1000xxx010001xxx OR.L %E,D%N",
    "1000xxx01001xxxx OR.L %E,D%N",
    "1000xxx01010xxxx OR.L %E,D%N",
    "1000xxx010110xxx OR.L %E,D%N",
    "1000xxx0101110xx OR.L %E,D%N",
    "1000xxx010111100 OR.L %E,D%N",
    "1000xxx11001xxxx OR.L D%N,%E",
    "1000xxx11010xxxx OR.L D%N,%E",
    "1000xxx110110xxx OR.L D%N,%E",
    "1000xxx11011100x OR.L D%N,%E",  

    "1001xxx00x000xxx SUB.BW %E,D%N",		// 0 = reg dest, ea source, all ea modes
    "1001xxx00x01xxxx SUB.BW %E,D%N",
    "1001xxx00x10xxxx SUB.BW %E,D%N",
    "1001xxx00x110xxx SUB.BW %E,D%N",
    "1001xxx00x1110xx SUB.BW %E,D%N",
    "1001xxx00x111100 SUB.BW %E,D%N",
    "1001xxx001001xxx SUB.W A%N,%E",
    "1001xxx10x01xxxx SUB.BW D%N,%E",		// 1 = ea dest, no PC, Imm or Reg direct
    "1001xxx10x10xxxx SUB.BW D%N,%E",
    "1001xxx10x110xxx SUB.BW D%N,%E",
    "1001xxx10x11100x SUB.BW D%N,%E",
   
    "1001xxx010000xxx SUB.L %E,D%N",
    "1001xxx010001xxx SUB.L %E,D%N",
    "1001xxx01001xxxx SUB.L %E,D%N",
    "1001xxx01010xxxx SUB.L %E,D%N",
    "1001xxx010110xxx SUB.L %E,D%N",
    "1001xxx0101110xx SUB.L %E,D%N",
    "1001xxx010111100 SUB.L %E,D%N",
    "1001xxx11001xxxx SUB.L D%N,%E",
    "1001xxx11010xxxx SUB.L D%N,%E",
    "1001xxx110110xxx SUB.L D%N,%E",
    "1001xxx11011100x SUB.L D%N,%E",

    "1001xxx10x000xxx SUBX.BW D%R,D%N",
    "1001xxx10x001xxx SUBX.BW -(A%R),-(A%N)",
    "1001xxx110000xxx SUBX.L D%R,D%N",
    "1001xxx110001xxx SUBX.L -(A%R),-(A%N)",
    
    "1001xxx0110xxxxx SUBA.W %E,A%N",
    "1001xxx01110xxxx SUBA.W %E,A%N",
    "1001xxx011110xxx SUBA.W %E,A%N",
    "1001xxx0111110xx SUBA.W %E,A%N",
    "1001xxx011111100 SUBA.W %E,A%N",
    
    "1001xxx1110xxxxx SUBA.L %E,A%N",
    "1001xxx11110xxxx SUBA.L %E,A%N",
    "1001xxx111110xxx SUBA.L %E,A%N",
    "1001xxx1111110xx SUBA.L %E,A%N",
    "1001xxx111111100 SUBA.L %E,A%N",    
    
    "1010xxxxxxxxxxxx ALINE Exception",
    
    "1011xxx000000xxx CMP.B %E,D%N",
    "1011xxx00001xxxx CMP.B %E,D%N",
    "1011xxx0010xxxxx CMP.W %E,D%N",
    "1011xxx00x10xxxx CMP.BW %E,D%N",
    "1011xxx00x110xxx CMP.BW %E,D%N",
    "1011xxx00x1110xx CMP.BW %E,D%N",
    "1011xxx00x111100 CMP.BW %E,D%N",
    "1011xxx0100xxxxx CMP.L %E,D%N",
    "1011xxx01010xxxx CMP.L %E,D%N",
    "1011xxx010110xxx CMP.L %E,D%N",
    "1011xxx0101110xx CMP.L %E,D%N",
    "1011xxx010111100 CMP.L %E,D%N",
    
    "1011xxx0110xxxxx CMPA.W %E,A%N",
    "1011xxx01110xxxx CMPA.W %E,A%N",
    "1011xxx011110xxx CMPA.W %E,A%N",
    "1011xxx0111110xx CMPA.W %E,A%N",
    "1011xxx011111100 CMPA.W %E,A%N",
    
    "1011xxx1110xxxxx CMPA.L %E,A%N",
    "1011xxx11110xxxx CMPA.L %E,A%N",
    "1011xxx111110xxx CMPA.L %E,A%N",
    "1011xxx1111110xx CMPA.L %E,A%N",
    "1011xxx111111100 CMPA.L %E,A%N",
    
    "1011xxx100001xxx CMPM.B (A%R)+,(A%N)+",
    "1011xxx101001xxx CMPM.W (A%R)+,(A%N)+",
    "1011xxx110001xxx CMPM.L (A%R)+,(A%N)+",
    
    "1011xxx10x000xxx EOR.BW %E,D%N",
    "1011xxx10x01xxxx EOR.BW %E,D%N",
    "1011xxx10x10xxxx EOR.BW %E,D%N",
    "1011xxx10x110xxx EOR.BW %E,D%N",
    "1011xxx10x11100x EOR.BW %E,D%N",
    "1011xxx110000xxx EOR.L D%N,%E",
    "1011xxx11001xxxx EOR.L D%N,%E",
    "1011xxx11010xxxx EOR.L D%N,%E",
    "1011xxx110110xxx EOR.L D%N,%E",
    "1011xxx11011100x EOR.L D%N,%E",
    
    "1100xxx00x000xxx AND.BW %E,D%N",		// 0 = reg dest, ea source, all ea modes
    "1100xxx00x01xxxx AND.BW %E,D%N",
    "1100xxx00x10xxxx AND.BW %E,D%N",
    "1100xxx00x110xxx AND.BW %E,D%N",
    "1100xxx00x1110xx AND.BW %E,D%N",
    "1100xxx00x111100 AND.BW %E,D%N",
    "1100xxx001001xxx AND.W A%N,%E",
    "1100xxx10x01xxxx AND.BW D%N,%E",		// 1 = ea dest, no PC, Imm or Reg direct
    "1100xxx10x10xxxx AND.BW D%N,%E",
    "1100xxx10x110xxx AND.BW D%N,%E",
    "1100xxx10x11100x AND.BW D%N,%E",
    
    "1100xxx010000xxx AND.L %E,D%N",
    "1100xxx010001xxx AND.L %E,D%N",
    "1100xxx01001xxxx AND.L %E,D%N",
    "1100xxx01010xxxx AND.L %E,D%N",
    "1100xxx010110xxx AND.L %E,D%N",
    "1100xxx0101110xx AND.L %E,D%N",
    "1100xxx010111100 AND.L %E,D%N",
    "1100xxx11001xxxx AND.L D%N,%E",
    "1100xxx11010xxxx AND.L D%N,%E",
    "1100xxx110110xxx AND.L D%N,%E",
    "1100xxx11011100x AND.L D%N,%E",

    "1100xxx011000xxx MULU %E,D%N",
    "1100xxx01101xxxx MULU %E,D%N",
    "1100xxx01110xxxx MULU %E,D%N",
    "1100xxx011110xxx MULU %E,D%N",
    "1100xxx0111110xx MULU %E,D%N",
    "1100xxx011111100 MULU %E,D%N",

    "1100xxx111000xxx MULS %E,D%N",
    "1100xxx11101xxxx MULS %E,D%N",
    "1100xxx11110xxxx MULS %E,D%N",
    "1100xxx111110xxx MULS %E,D%N",
    "1100xxx1111110xx MULS %E,D%N",
    "1100xxx111111100 MULS %E,D%N",

    "1100xxx100000xxx ABCD D%R, D%N",
    "1100xxx100001xxx ABCD -(A%R),-(A%N)",
    
    "1100xxx101000xxx EXG D%R,D%N",
    "1100xxx101001xxx EXG A%R,A%N",
    "1100xxx110001xxx EXG D%R,A%N",
    
    "1101xxx00x000xxx ADD.BW %E,D%N",		// 0 = reg dest, ea source, all ea modes
    "1101xxx00x01xxxx ADD.BW %E,D%N",
    "1101xxx00x10xxxx ADD.BW %E,D%N",
    "1101xxx00x110xxx ADD.BW %E,D%N",
    "1101xxx00x1110xx ADD.BW %E,D%N",
    "1101xxx00x111100 ADD.BW %E,D%N",
    "1101xxx001001xxx ADD.W A%N,%E",
    "1101xxx10x01xxxx ADD.BW D%N,%E",		// 1 = ea dest, no PC, Imm or Reg direct
    "1101xxx10x10xxxx ADD.BW D%N,%E",
    "1101xxx10x110xxx ADD.BW D%N,%E",
    "1101xxx10x11100x ADD.BW D%N,%E",
    
    "1101xxx010000xxx ADD.L %E,D%N",
    "1101xxx010001xxx ADD.L %E,D%N",
    "1101xxx01001xxxx ADD.L %E,D%N",
    "1101xxx01010xxxx ADD.L %E,D%N",
    "1101xxx010110xxx ADD.L %E,D%N",
    "1101xxx0101110xx ADD.L %E,D%N",
    "1101xxx010111100 ADD.L %E,D%N",
    "1101xxx11001xxxx ADD.L D%N,%E",
    "1101xxx11010xxxx ADD.L D%N,%E",
    "1101xxx110110xxx ADD.L D%N,%E",
    "1101xxx11011100x ADD.L D%N,%E",

    "1101xxx10x000xxx ADDX.BW D%R,D%N",
    "1101xxx10x001xxx ADDX.BW -(A%R),-(A%N)",
    "1101xxx110000xxx ADDX.L D%R,D%N",
    "1101xxx110001xxx ADDX.L -(A%R),-(A%N)",
    
    "1101xxx0110xxxxx ADDA.W %E,A%N",
    "1101xxx01110xxxx ADDA.W %E,A%N",
    "1101xxx011110xxx ADDA.W %E,A%N",
    "1101xxx0111110xx ADDA.W %E,A%N",
    "1101xxx011111100 ADDA.W %E,A%N",
    
    "1101xxx1110xxxxx ADDA.L %E,A%N",
    "1101xxx11110xxxx ADDA.L %E,A%N",
    "1101xxx111110xxx ADDA.L %E,A%N",
    "1101xxx1111110xx ADDA.L %E,A%N",
    "1101xxx111111100 ADDA.L %E,A%N",
    
    // Memory shifs
    "1110000x1101xxxx ASd %E",
    "1110000x1110xxxx ASd %E",
    "1110000x11110xxx ASd %E",
    "1110000x1111100x ASd %E",

    "1110001x1101xxxx LSd %E",
    "1110001x1110xxxx LSd %E",
    "1110001x11110xxx LSd %E",
    "1110001x1111100x LSd %E",
    
    "1110010x1101xxxx ROXd %E",
    "1110010x1110xxxx ROXd %E",
    "1110010x11110xxx ROXd %E",
    "1110010x1111100x ROXd %E",

    "1110011x1101xxxx ROd %E",
    "1110011x1110xxxx ROd %E",
    "1110011x11110xxx ROd %E",
    "1110011x1111100x ROd %E",

    // Register shifts
    "1110xxxx0x000xxx ASd.BW %N,D%R",
    "1110xxxx10000xxx ASd.L %N,D%R",
    
    "1110xxxx0x001xxx LSd.BW %N,D%R",
    "1110xxxx10001xxx LSd.L %N,D%R",
    
    "1110xxxx0x010xxx ROXd.BW %N,D%R",
    "1110xxxx10010xxx ROXd.L %N,D%R",
    
    "1110xxxx0x011xxx ROd.BW %N,D%R",
    "1110xxxx10011xxx ROd.L %N,D%R",


    "1110xxxx0x100xxx ASd.BW D%N,D%R",
    "1110xxxx10100xxx ASd.L D%N,D%R",
    
    "1110xxxx0x101xxx LSd.BW D%N,D%R",
    "1110xxxx10101xxx LSd.L D%N,D%R",
    
    "1110xxxx0x110xxx ROXd.BW D%N,D%R",
    "1110xxxx10110xxx ROXd.L D%N,D%R",
    
    "1110xxxx0x111xxx ROd.BW D%N,D%R",
    "1110xxxx10111xxx ROd.L D%N,D%R",


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
	return strdup(buffer);
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
 			// fprintf( stderr, "Parsing %s ", opcodes[i]);
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
			sprintf( buffer, "%s", &opcodes[i][17] );
			ops[i].match = match;
			ops[i].equal = equal;
			ops[i].op = strdup(buffer);
			total_ops += 1 << count;
 			// for(int s=strlen(ops[i].op); s<20; s++) 
 			// 	fprintf( stderr, " ");
 			// fprintf( stderr, "%04x %04x\n", match, equal);
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

	static char buffer2[64];
	extern int debug;
	
	for(int i=0; i<OPCODE_COUNT; i++) {
		if((op & ops[i].match) == ops[i].equal) {
			//int x = strlen(ops[i].op) - 5; // back up over \n
			//if(debug) printf("@ Disassembling opcode %04X\n", op);
			char* ea1 = strstr(ops[i].op, "%E");
			char* ea2 = ea1 ? strstr(ea1 + 1, "%E") : 0;
			char* imm = strstr(ops[i].op, "%N");
			char* reg = strstr(ops[i].op, "%R");
			
			int ea1_off = ea1 - ops[i].op;
			int ea2_off = ea2 - ops[i].op;
			int imm_off = imm - ops[i].op;
			int reg_off = reg - ops[i].op;
			
			strcpy(buffer, ops[i].op);
			
			unsigned _i = (op >> 9) & 7;
			unsigned _r = (op & 7);
			
			if(ea1) buffer[ea1_off + 1] = 's';
			//if(ea1 && debug) printf("@ ea1_off=0x%02x\n", ea1_off);
			if(ea2) buffer[ea2_off + 1] = 's';
			//if(ea2 && debug) printf("@ ea2_off=0x%02x\n", ea2_off);
			if(imm) buffer[imm_off + 1] = 'd';
			//if(imm && debug) printf("@ imm_off=0x%02x\n", imm_off);
			if(reg) buffer[reg_off + 1] = 'd';
			//if(reg && debug) printf("@ reg_off=0x%02x\n", reg_off);
			
			//if(debug) printf("@ format=%s", buffer);
			
			if(ea1 && ea2) {
				char* _ea1 = ea(op & 0x3F);
				char* _ea2 = ea(((op >> 9) & 0x07) | ((op >> 3) & 0x38));
				sprintf(buffer2, buffer, _ea1, _ea2);
				free(_ea1); 
				free(_ea2);
				
			} else if(ea1) {
				char* _ea = ea(op & 0x3F);
				if(imm && reg) {
					if (ea1 < imm) {
						if (imm < reg)      sprintf(buffer2, buffer, _ea,_i,_r);  //return [ea1,imm,reg];
						else if (ea1 < reg) sprintf(buffer2, buffer, _ea,_r,_i);  //return [ea1,reg,imm];
						else                sprintf(buffer2, buffer, _r,_ea,_i);  //return [reg,ea1,imm];
					} else {
						if (ea1 < reg)      sprintf(buffer2, buffer, _i,_ea,_r);  //return [imm,ea1,reg];
						else if (imm < reg) sprintf(buffer2, buffer, _i,_r,_ea);  //return [imm,reg,ea1];
						else                sprintf(buffer2, buffer, _r,_i,_ea);  //return [reg,imm,ea1];
					}
				} else if(imm) {
					if (ea1 < imm)          sprintf(buffer2, buffer, _ea,_i);  //return [reg,ea1,imm];
					else                    sprintf(buffer2, buffer, _i,_ea);  //return [reg,imm,ea1];
					
				} else if(reg) {
					if (ea1 < reg)          sprintf(buffer2, buffer, _ea,_r);  //return [reg,ea1,imm];
					else                    sprintf(buffer2, buffer, _r,_ea);  //return [reg,imm,ea1];
					
				} else                      sprintf(buffer2, buffer, _ea);  //return [reg,imm,ea1];
				free(_ea);
				
			} else {
				if(imm && reg) {
					if (imm < reg)          sprintf(buffer2, buffer, _i,_r);  //return [ea1,imm,reg];
					else                    sprintf(buffer2, buffer, _r,_i);  //return [reg,ea1,imm];
					
				} 
				else if(imm)                sprintf(buffer2, buffer, _i);  //return [reg,ea1,imm];
				else if(reg)                sprintf(buffer2, buffer, _r);  //return [reg,ea1,imm];
				else                        sprintf(buffer2, buffer);  //return [reg,imm,ea1];
				
			}
			return buffer2;
		}
	}
	
	return "ILLEGAL\n";
}







