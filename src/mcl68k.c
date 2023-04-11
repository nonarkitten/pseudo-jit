//
//
//  File Name   :  MCL68.ino
//  Used on     :
//  Author      :  Ted Fried, MicroCore Labs
//  Creation    :  7/14/2020
//
//   Description:
//   ============
//
//  Motorola 68000 Emulator written in C
//
//  Contains Tiny BASIC held in array memory.
//
//------------------------------------------------------------------------
//
// Modification History:
// =====================
//
// Revision 1 7/14/2020
// Initial revision
//
// Revision 2 11/5/2022
// Swapped incorrect stacking order for exception handler and RTE opcode
//
//
//------------------------------------------------------------------------
//
// Copyright (c) 2020 Ted Fried
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

//------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>


// Defines
// ----------------------------------------------------------------------


// Universal defines
#define TRUE                    1
#define FALSE                   0
#define SIZE_BYTE               8
#define SIZE_WORD               16

#define ADDRESS_REG             1
#define DATA_REG                2
#define IMMEDIATE               3
#define MEMORY                  4


// Calculate data sizes for different opcode encoding types
#define DATA_SIZE_TYPE_A        (((0x00C0&first_opcode)>>6)==0)  ? 8  : (((0x00C0&first_opcode)>>6)==1)  ? 16 : 32
#define DATA_SIZE_TYPE_B        (((0x3000&first_opcode)>>12)==1) ? 8  : (((0x3000&first_opcode)>>12)==3) ? 16 : 32
#define DATA_SIZE_TYPE_C        (((0x0040&first_opcode)>>6)==0)  ? 16 : 32
#define DATA_SIZE_TYPE_D        (((0x0100&first_opcode)>>8)==0)  ? 16 : 32


// Flag bits
#define mc68k_flag_T            ((mc68k_flags & 0x8000) >> 15)  // 15
#define mc68k_flag_S            ((mc68k_flags & 0x2000) >> 13)  // 13
#define mc68k_flag_X            ((mc68k_flags & 0x0010) >> 4)   // 4
#define mc68k_flag_N            ((mc68k_flags & 0x0008) >> 3)   // 3
#define mc68k_flag_Z            ((mc68k_flags & 0x0004) >> 2)   // 2
#define mc68k_flag_V            ((mc68k_flags & 0x0002) >> 1)   // 1
#define mc68k_flag_C            ( mc68k_flags & 0x0001)         // 0


// FPGA Avalon bus BIU register addresses
#define BIU_BASE_ADDRESS        0x1234

#define BIU_REG_COMMAND         0x0000
#define BIU_REG_PFQ_STROBE      0x0004
#define BIU_REG_JUMP_ADDRESS    0x0008
#define BIU_REG_ADDRESS         0x000C
#define BIU_REG_DATAOUT         0x0010
#define BIU_REG_SIZE            0x0014
#define BIU_REG_RESPONSE        0x0018
#define BIU_REG_INTERRUPTS      0x001C
#define BIU_REG_PFQ_EMPTY       0x0020
#define BIU_REG_DONE            0x0024
#define BIU_REG_PFQ_TOP         0x0028
#define BIU_REG_DATAIN          0x002C
#define BIU_REG_BUS_ERROR       0x0030
#define BIU_REG_FAIL_TYPE       0x0034
#define BIU_REG_FLAGS           0x0038
#define BIU_REG_STATUS          0x003C
#define BIU_REG_ATOMIC          0x0040


// Variables
// ----------------------------------------------------------------------
uint8_t  biu_size;
uint8_t  reg_num;
uint8_t  data_size;
uint8_t  EA_register;
uint8_t  ea_type;
uint8_t  reset_status_d;
uint8_t  source_ea_type;
uint8_t  destination_ea_type;
uint8_t  last_mc68k_flag_T;

uint16_t  last_exception=0;
uint16_t  mc68k_flags=0x2700;
uint16_t  biu_read_data;
uint16_t  biu_dataout;
uint16_t  first_opcode;

uint32_t  immediate;
uint32_t  m68k_data_reg[8];
uint32_t  m68k_address_reg[8];
uint32_t  m68k_a7_S=0x4000;
uint32_t  mc68k_pc=0;
uint32_t  biu_address;
uint32_t  calculated_EA;
uint32_t  EA_Data;
uint32_t  result;
uint32_t  access_address;
uint32_t  source_ea;
uint32_t  destination_ea;
uint32_t  num=0;
uint32_t  original_mc68k_pc;

uint32_t  memory_base;

void Exception_Handler(uint16_t exception_type);

#include "mem_be.h"

// Bus Interface Unit - BIU
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
uint32_t IORD_32DIRECT(uint32_t base_address , uint32_t reg_address) {
    if      (reg_address==BIU_REG_PFQ_EMPTY    )   { return 0x0; }      // PFQ always has data (not empty)
    else if (reg_address==BIU_REG_STATUS       )   { return 0x0; }
    else if (reg_address==BIU_REG_INTERRUPTS   )   { return 0xFF; }
    else if (reg_address==BIU_REG_DONE         )   { return 0x1; }      // always return "Done"
    else if (reg_address==BIU_REG_FAIL_TYPE    )   { return 0x0; }
    else if (reg_address==BIU_REG_PFQ_TOP      )   { return read_BE_word(memory_base + mc68k_pc); }
    else if (reg_address==BIU_REG_DATAIN       )   { return  biu_read_data; }
    else if (reg_address==BIU_REG_BUS_ERROR    )   { return  0x0; }
    return 0;
}

void IOWR_32DIRECT(uint32_t base_address , uint32_t reg_address , uint32_t write_data) {
  if      (reg_address==BIU_REG_PFQ_STROBE && write_data==0x1)   {  }                             // command: advance prefetch queue
  else if (reg_address==BIU_REG_JUMP_ADDRESS)                    { mc68k_pc = write_data & 0xFFFFFF; }       // write jump address
  else if (reg_address==BIU_REG_ADDRESS)                         { biu_address = write_data & 0xFFFFFF; }    // write the BIU Bus address
  else if (reg_address==BIU_REG_DATAOUT)                         { biu_dataout = write_data; }    // write the BIU Bus write data
  else if (reg_address==BIU_REG_SIZE)                            { biu_size = write_data; }       // write the BIU Bus data size
  else if (reg_address==BIU_REG_FLAGS)                           {  }                             // write the BIU Bus flags
  else if (reg_address==BIU_REG_COMMAND) {
    if      (write_data==0x99)                                     {  }                           // command: BIU RESET assert for 140 clocks
    else if (write_data==0x8)                                      {  }                           // command: execute the jump
    else if (write_data==0x3)                                      { biu_read_data=0x3; }         // command: execute IACK
    // Read Operations
    else if (write_data==0x2) {
      if      (biu_size==8 && biu_address==0x22) { biu_read_data = UART0Ready() ? 0xFF : 0x00; }  // UART Receive character
      else if (biu_size==8 && biu_address==0x23) { biu_read_data = UART0Read(); }                 // UART Receive character
      else if (biu_size==8) { biu_read_data=read_BE_byte(memory_base + biu_address); }                                   // command: execute BIU Read Cycle - Byte
      else if (biu_size==16) { biu_read_data=read_BE_word(memory_base + biu_address); }        // command: execute BIU Read Cycle - Word
    }
    // Write Operations
    else if (write_data==0x1) {
      if      (biu_size==8 && biu_address==0x21) { UART0Write(biu_dataout); }                     // UART Transmit character
      else if (biu_size==8) { write_BE_byte(memory_base + biu_address, biu_dataout); }                                     // command: BIU Write Data - Byte
      else if (biu_size==16) { write_BE_word(memory_base + biu_address, biu_dataout); } // command: BIU Write Data - Word
    }
  }
}


// Retrieves the interrupt status from the BIU
// ----------------------------------------------------------------------
uint32_t BIU_Get_Failed_Access_Type() {
    return IORD_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_FAIL_TYPE);
}


// Update the BIU with the new System Flag values
// ----------------------------------------------------------------------
void BIU_Update_Flags(uint16_t new_flag_value) {
    IOWR_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_FLAGS, new_flag_value);
    return;
}


// ----------------------------------------------------------------------
void Update_System_Flags(uint16_t new_flag_value) {
  // If any of the System Flag values have changed, inform the BIU
  if ( (0x2700&mc68k_flags) != (0x2700&new_flag_value) )  { BIU_Update_Flags(new_flag_value); }

  mc68k_flags = new_flag_value;

  return;
}


// Send command for the BIU to perform an atomic Read-Modify-Write Cycle
// ----------------------------------------------------------------------
void BIU_RMW(uint8_t value) {
    IOWR_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_ATOMIC, value);
    return;
}


// Forces the assertion of the RESET signal for 124 clocks
// ----------------------------------------------------------------------
void BIU_Force_Reset() {
    uint16_t biu_response=0;

    IOWR_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_COMMAND, 0x99);                                         // Send command to the BIU to assert the RESET

    while (biu_response == 0x0)                                                                     // Poll BIU for cycle complete
      {
        biu_response = IORD_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_DONE);
      }

    IOWR_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_COMMAND, 0x00);                                         // Debounce the BIU command
    return;
}


// Fetches the next word from the prefetch queue
// ----------------------------------------------------------------------
uint16_t BIU_PFQ_Fetch() {
    uint32_t prefech_temp=1;

    while (prefech_temp==0x1){ prefech_temp = IORD_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_PFQ_EMPTY); } // Poll BIU for cycle complete

    prefech_temp = IORD_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_PFQ_TOP);                                // Fetch the data

    IOWR_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_PFQ_STROBE, 0x01);                                      // Strobe the prefetch queue
    IOWR_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_PFQ_STROBE, 0x00);                                      // Debounce

    mc68k_pc = (mc68k_pc + 0x2) & 0xFFFFFF;                                                         // Increment the local PC

    return prefech_temp;                                                                            // Return the 16-bit opcode
}


// Flushes the PFQ and begins fetching instructions from new address
// ----------------------------------------------------------------------
void BIU_Jump(uint32_t jump_address) {
    uint16_t biu_response=0;

    jump_address = 0xFFFFFFFF & jump_address;

    // Check Alignment for Words
    if ( (0x1&jump_address)==1) { access_address=jump_address; Exception_Handler(3); }
    else
    {
      IOWR_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_JUMP_ADDRESS, jump_address);                          // Send Jump Address to the BIU
      IOWR_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_COMMAND, 0x08);                                       // Send Jump command to the BIU

      while (biu_response==0x0){ biu_response = IORD_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_DONE); }    // Poll BIU for cycle complete

      IOWR_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_COMMAND, 0x00);                                       // Debounce the BIU command

      if ( IORD_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_BUS_ERROR)==1) { access_address=jump_address; Exception_Handler(2); }
    }
    return;

}


// Fetches the current Reset status from the BIU
// ----------------------------------------------------------------------
uint16_t BIU_RESET_STATUS() {
    return (0x0001 & IORD_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_STATUS) ); // bit[0]=Reset
}


// Fetches the current IRQ signals from the BIU
// ----------------------------------------------------------------------
uint16_t BIU_IRQ_LEVEL() {
    return (0x0700 & IORD_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_INTERRUPTS) );
}


// Requests BIU to perform an IACK cycle to retrieve the interrupt vector
// ----------------------------------------------------------------------
uint16_t BIU_IACK() {
    uint16_t biu_response=0;

    IOWR_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_COMMAND, 0x03);                                         // Send IACK command to the BIU

    while (biu_response==0x0){ biu_response = IORD_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_DONE); }      // Poll BIU for cycle complete

    biu_response = IORD_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_DATAIN);                                 // Retrieve the BIU read data
    IOWR_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_COMMAND, 0x00);                                         // Debounce the BIU command

    if ( IORD_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_BUS_ERROR)==1) { access_address=0xdeadbeef; Exception_Handler(2); }

    return biu_response;
}


// Requests BIU to perform a Data Write cycle
// ----------------------------------------------------------------------
void BIU_Write(uint32_t write_address , uint32_t write_data , uint8_t write_size )  // possible add atomic rmw flag?
{
    uint8_t biu_response=0;

    // Check Alignment for Words
    if ( (0x1&write_address)==1 && write_size>8) { access_address=write_address; Exception_Handler(3); }
    else
    {
      IOWR_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_ADDRESS, write_address);                              // Address of the Data to be written
      IOWR_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_DATAOUT, write_data);                                 // Data to be written
      IOWR_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_SIZE, write_size);                                    // Size of data B/W
      IOWR_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_COMMAND, 0x01);                                       // Send command to perform the write

      while (biu_response==0x0){ biu_response = IORD_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_DONE); }    // Poll BIU for cycle complete

      IOWR_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_COMMAND, 0x00);                                       // Debounce the BIU command

      if ( IORD_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_BUS_ERROR)==1) { access_address=write_address; Exception_Handler(2); }
    }
    return;
}


// Requests BIU to perform a Data Read cycle
// ----------------------------------------------------------------------
uint16_t BIU_Read(uint32_t read_address , uint8_t read_size )
{
    uint16_t biu_response=0;

    // Check Alignment for Words
    if ( (0x1&read_address)==1 && read_size>8) { Exception_Handler(3); }
    else
    {
      IOWR_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_ADDRESS, read_address);                               // Address of the Data to be read
      IOWR_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_SIZE, read_size);                                     // Size of data B/W
      IOWR_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_COMMAND, 0x02);                                       // Send command to perform the read

      while (biu_response==0x0){ biu_response = IORD_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_DONE); }    // Poll BIU for cycle complete

      biu_response = IORD_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_DATAIN);                               // Retrieve the BIU read data
      IOWR_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_COMMAND, 0x00);                                       // Debounce the BIU command

      if ( IORD_32DIRECT(BIU_BASE_ADDRESS, BIU_REG_BUS_ERROR)==1) { access_address=read_address; Exception_Handler(2); }
    }
    return biu_response;
}


// ----------------------------------------------------------------------
uint32_t BIU_Read_32(uint32_t local_address) {
  return (BIU_Read(local_address , 16) << 16) | BIU_Read(local_address+0x2 , 16);
}


// ----------------------------------------------------------------------
void BIU_Write_32(uint32_t local_address , uint32_t local_data ) {
  BIU_Write(local_address ,     (0xFFFF0000&local_data)>>16 , 16);                                  // Write upper word of the 32-bit data
  BIU_Write(local_address+0x02,  (0x0000FFFF&local_data)     , 16);                                  // Write lower word of the 32-bit data at next word address
  return;
}


// Write data back to the Data register pool with the correct data size
// ----------------------------------------------------------------------
void Store_Data_Register(uint8_t reg_num , uint32_t reg_data , uint8_t reg_size) {
    if      (reg_size==8)  { m68k_data_reg[reg_num] = ( (m68k_data_reg[reg_num]&0xFFFFFF00)  | (reg_data&0x000000FF) ); }
    else if (reg_size==16) { m68k_data_reg[reg_num] = ( (m68k_data_reg[reg_num]&0xFFFF0000)  | (reg_data&0x0000FFFF) ); }
    else if (reg_size==32) { m68k_data_reg[reg_num] = (reg_data&0xFFFFFFFF); }
    return;
}

// Read data from the Data register pool
// ----------------------------------------------------------------------
uint32_t Fetch_Data_Register(uint8_t reg_num , uint8_t size) {
    if       (size==8)   { return (m68k_data_reg[reg_num]&0x000000FF); }
    else if  (size==16)  { return (m68k_data_reg[reg_num]&0x0000FFFF); }
    else if  (size==32)  { return (m68k_data_reg[reg_num]&0xFFFFFFFF); }
    else return 0xEEEEEEEE;
}


// Return the sign-extended value of a B/W/L register
// ----------------------------------------------------------------------
uint32_t Sign_Extend(uint32_t reg_data , uint8_t reg_size) {
    if (reg_size==8)
      {
        if ((reg_data&0x0080)!=0x0)   { return (reg_data | 0xFFFFFF00); } else { return (reg_data & 0x000000FF);  }
      }
    else if (reg_size==16)
      {
        if ((reg_data&0x8000)!=0x0)   { return (reg_data | 0xFFFF0000); } else { return (reg_data & 0x0000FFFF);  }
      }
    else { return  (reg_data&0xFFFFFFFF);  }
}


// Write data back to the Address register pool
// Byte writes cause an exception
// Write back to the proper SP (A7) depending on the Supervisor mode
// ----------------------------------------------------------------------
void Store_Address_Register(uint8_t reg_num , uint32_t reg_data , uint8_t reg_size) {
    if (reg_size==8) { Exception_Handler(3); }

    else if (mc68k_flag_S==1 && reg_num==0x7) {  m68k_a7_S                 = (reg_data&0xFFFFFFFF);  }
    else                                      {  m68k_address_reg[reg_num] = (reg_data&0xFFFFFFFF);  }

    return;
}



// Read data from the Address register pool
// ----------------------------------------------------------------------
uint32_t Fetch_Address_Register(uint8_t reg_num , uint8_t size) {
    if (mc68k_flag_S==1 && reg_num==0x7)
      { if       (size==8)   { return (m68k_a7_S&0x000000FF); }
        else if  (size==16)  { return (m68k_a7_S&0x0000FFFF); }
        else if  (size==32)  { return (m68k_a7_S&0xFFFFFFFF); }
        else return 0xEEEEEEEE;
      }
    else
      { if       (size==8)   { return (m68k_address_reg[reg_num]&0x000000FF); }
        else if  (size==16)  { return (m68k_address_reg[reg_num]&0x0000FFFF); }
        else if  (size==32)  { return (m68k_address_reg[reg_num]&0xFFFFFFFF); }
        else return 0xEEEEEEEE;
      }
}



// Calculate the selected condition results
// ----------------------------------------------------------------------
uint8_t Test_Condition(uint16_t local_opcode) {
    uint16_t condition_code;

    condition_code = (local_opcode&0x0F00) >> 8; // Isolate opcode bits[11:8]

    switch(condition_code)
    {
        case 0x0: return TRUE;                                                                              // Always
        case 0x1: return FALSE;                                                                             // Never
        case 0x2: if (mc68k_flag_C==0 && mc68k_flag_Z==0)                return TRUE; else return FALSE;    // Higher Than         C=0 AND Z=0
        case 0x3: if (mc68k_flag_C==1 || mc68k_flag_Z==1)                return TRUE; else return FALSE;    // Lower or Same       C=1 OR Z=1
        case 0x4: if (mc68k_flag_C==0)                                   return TRUE; else return FALSE;    // Carry Clear         C=0
        case 0x5: if (mc68k_flag_C==1)                                   return TRUE; else return FALSE;    // Carry Set           C=1
        case 0x6: if (mc68k_flag_Z==0)                                   return TRUE; else return FALSE;    // Not Equal           Z=0
        case 0x7: if (mc68k_flag_Z==1)                                   return TRUE; else return FALSE;    // Equal               Z=1
        case 0x8: if (mc68k_flag_V==0)                                   return TRUE; else return FALSE;    // V Clear             V=0
        case 0x9: if (mc68k_flag_V==1)                                   return TRUE; else return FALSE;    // V Set               V=1
        case 0xA: if (mc68k_flag_N==0)                                   return TRUE; else return FALSE;    // Plus                N=0
        case 0xB: if (mc68k_flag_N==1)                                   return TRUE; else return FALSE;    // Minus               N=1
        case 0xC: if (mc68k_flag_N == mc68k_flag_V)                      return TRUE; else return FALSE;    // Greater or Equal    N=V
        case 0xD: if (mc68k_flag_N != mc68k_flag_V)                      return TRUE; else return FALSE;    // Less Than           N!=V
        case 0xE: if ((mc68k_flag_N==mc68k_flag_V) && mc68k_flag_Z==0)   return TRUE; else return FALSE;    // Greater Than        N=V  AND Z=0
        case 0xF: if ((mc68k_flag_N!=mc68k_flag_V) || mc68k_flag_Z==1)   return TRUE; else return FALSE;    // Less Than or Equal  N!=V AND Z=1
    }
    return 0;
}


// Push a word to the Stack
// ----------------------------------------------------------------------
void Push(uint16_t push_data) {

  if (mc68k_flag_S==0x0)
  {
    m68k_address_reg[7] = m68k_address_reg[7] - 0x2;
    BIU_Write(m68k_address_reg[7] , push_data, SIZE_WORD);
  }
  else
  {
    m68k_a7_S = m68k_a7_S - 0x2;
    BIU_Write(m68k_a7_S , push_data, SIZE_WORD);
  }
  return;
}


// Pop a word to from the Stack
// ----------------------------------------------------------------------
uint16_t Pop() {
  uint16_t temp=0;

  if (mc68k_flag_S==0x0)
  {
    temp = BIU_Read(m68k_address_reg[7] , SIZE_WORD);
    m68k_address_reg[7] = m68k_address_reg[7] + 0x2;
  }
  else
  {
    temp = BIU_Read(m68k_a7_S , SIZE_WORD);
    m68k_a7_S = m68k_a7_S + 0x2;
  }
  return temp;
}


// Runs Reset routine to put CPU back to initial conditions
// ----------------------------------------------------------------------
void Reset_routine() {
  uint32_t  pc_temp=0;

  Update_System_Flags(0x2700);                                                          // Initialize flags T=0, S=1, Mask=111

  BIU_Jump(0x000000);                                                                   // Flush prefetch queue and start fetching data at address 0x00

  m68k_a7_S = BIU_PFQ_Fetch();                                                          // Fetch upper word of the Supervisor Stack Pointer
  m68k_a7_S = (m68k_a7_S<<16) | BIU_PFQ_Fetch();                                        // Fetch lower word of the Supervisor Stack Pointer

  pc_temp = BIU_PFQ_Fetch();                                                            // Fetch upper word of the Program Counter
  mc68k_pc = (pc_temp<<16) | BIU_PFQ_Fetch();                                           // Fetch lower word of the Program Counter

  BIU_Jump(mc68k_pc);                                                                   // Jump to the new PC

  return;
}


// ----------------------------------------------------------------------
void Exception_Handler(uint16_t vector_number) {
  uint16_t  temp=0;
  uint16_t  mc68k_flags_copy=0x2700;
  uint32_t exception_address=0;

  last_exception = vector_number;                                                       // Store the value of this exception

  mc68k_flags_copy = mc68k_flags;                                                       // Store the original Flags register value
  temp = mc68k_flags | 0x2000;                                                          // Set the S flag
  temp = temp & 0x7FFF;                                                                 // Clear the T flag
  Update_System_Flags(temp);                                                            // Update the System Flags

  if (vector_number <= 3)                                                               // Additional data is stacked for Group 0
  {
    Push(BIU_Get_Failed_Access_Type());                                                 // Stack the BIU Access Type
    Push(access_address&0xFFFF);                                                        // Stack the lower portion of the failed access address
    Push(access_address>>16);                                                           // Stack the upper portion of the failed access address
    Push(first_opcode);                                                                 // Stack the Opcode
  }

  if (vector_number==0x99)                                                              // 0x99=Interrupt -- Fetch the vector number from the BIU
    {
      temp = mc68k_flags & 0xF8FF;                                                      // Clear the Interrupt Flags
      Update_System_Flags(temp | BIU_IRQ_LEVEL());                                      // Set the Interrupt Flags to the current IRQ from the BIU
      vector_number = BIU_IACK();                                                       // Fetch the vector from the BIU IACK Cycle
    }

  Push(mc68k_pc&0xFFFF);                                                                // Stack the lower PC
  Push(mc68k_pc>>16);                                                                   // Stack the upper PC
  Push(mc68k_flags_copy);                                                               // Stack the Original copy of the Flags

  exception_address = BIU_Read_32(vector_number<<2);                                    // Fetch the 32-bit exception address

  BIU_Jump(exception_address);                                                          // Jump to the exception address

  // Check again for an active Interrupt from the BIU
  if (BIU_IRQ_LEVEL() != 0)  {  Exception_Handler(99);  }

  return;
}


// ----------------------------------------------------------------------
void op_BOOL_I_TO_CCR(uint8_t bool_type) {
  if (bool_type==1) mc68k_flags = (0xFF00&mc68k_flags) |  ( (0x00FF&mc68k_flags) | (0x001F&BIU_PFQ_Fetch()) ); else
  if (bool_type==2) mc68k_flags = (0xFF00&mc68k_flags) |  ( (0x00FF&mc68k_flags) & (0x001F&BIU_PFQ_Fetch()) ); else
  if (bool_type==3) mc68k_flags = (0xFF00&mc68k_flags) |  ( (0x00FF&mc68k_flags) ^ (0x001F&BIU_PFQ_Fetch()) );
  return;
}


// ----------------------------------------------------------------------
void op_BOOL_I_TO_SR(uint8_t bool_type) {
  if (mc68k_flag_S==0x0) { Exception_Handler(8); }      // Verify that supervisor privilege is set
  else
  {
    if (bool_type==1) Update_System_Flags(mc68k_flags | BIU_PFQ_Fetch() ); else
    if (bool_type==2) Update_System_Flags(mc68k_flags & BIU_PFQ_Fetch() ); else
    if (bool_type==3) Update_System_Flags(mc68k_flags ^ BIU_PFQ_Fetch() );
  }
  return;
}


// ----------------------------------------------------------------------
uint32_t Calculate_EA(uint16_t allowed_modes) {
  uint32_t address_register;
  uint32_t temp_address;
  uint32_t ea_extension;
  uint32_t offset=0;
  uint32_t extension_displacement;
  uint32_t extension_register;
  uint8_t ea_A_bit;
  uint8_t ea_L_bit;


  ea_type=99;   // Reset ea_type value

  EA_register =(0x0007&first_opcode);

  switch( (0x0038&first_opcode)>>3 )
  {
      case 0x0: if ((0x2000&allowed_modes)==0) Exception_Handler(4); else           // Dn
                  { ea_type=DATA_REG;
                    return (EA_register);
                  } break;

      case 0x1: if ((0x1000&allowed_modes)==0) Exception_Handler(4); else           // An
                  { ea_type=ADDRESS_REG;
                    return (EA_register);
                  } break;

      case 0x2: if ((0x0800&allowed_modes)==0) Exception_Handler(4); else           // (An)
                  { ea_type=MEMORY;
                    address_register=Fetch_Address_Register(EA_register,32);
                    return address_register;
                  } break;

      case 0x3: if ((0x0400&allowed_modes)==0) Exception_Handler(4); else           // (An)+
                  { ea_type=MEMORY;
                    address_register=Fetch_Address_Register(EA_register,32);
                    temp_address = address_register;
                    address_register = (data_size==8 && EA_register==0x7) ? address_register+2 : // Special case for SSP
                                       (data_size==8)                     ? address_register+1 :
                                       (data_size==16)                    ? address_register+2 :
                                                                            address_register+4 ;
                    Store_Address_Register(EA_register, address_register , 32);
                    return temp_address;
                  } break;

      case 0x4: if ((0x0200&allowed_modes)==0) Exception_Handler(4); else           // -(An)
                  { ea_type=MEMORY;
                    address_register=Fetch_Address_Register(EA_register,32);
                    address_register = (data_size==8 && EA_register==0x7) ? address_register-2 : // Special case for SSP
                                       (data_size==8)                     ? address_register-1 :
                                       (data_size==16)                    ? address_register-2 :
                                                                            address_register-4 ;
                    Store_Address_Register(EA_register, address_register , 32);
                    return address_register;
                  } break;

      case 0x5: if ((0x0100&allowed_modes)==0) Exception_Handler(4); else           // d16(An)
                  { ea_type=MEMORY;
                    address_register=Fetch_Address_Register(EA_register,32);
                    offset=Sign_Extend(BIU_PFQ_Fetch() ,16);
                    return (0xFFFFFFFF&(address_register+offset));
                  } break;
                                                                                    // d8(An,Xn)
      case 0x6: if ((0x0080&allowed_modes)==0) Exception_Handler(4); else
                  { ea_type=MEMORY;
                    address_register=Fetch_Address_Register(EA_register,32);
                    ea_extension = BIU_PFQ_Fetch();
                    extension_displacement = Sign_Extend( (0x00FF&ea_extension) , 8);
                    extension_register = (0x7000&ea_extension)>>12;
                    ea_A_bit = ((ea_extension & 0x8000) >> 15);
                    ea_L_bit = ((ea_extension & 0x0800) >> 11);
                    if (ea_A_bit==1 && ea_L_bit==0) offset=Sign_Extend(Fetch_Address_Register(extension_register,16) ,16); else
                    if (ea_A_bit==0 && ea_L_bit==0) offset=Sign_Extend(Fetch_Data_Register(extension_register,16) ,16); else
                    if (ea_A_bit==1 && ea_L_bit==1) offset=Fetch_Address_Register(extension_register,32); else
                    if (ea_A_bit==0 && ea_L_bit==1) offset=Fetch_Data_Register(extension_register,32);
                    return (0xFFFFFFFF&(address_register+offset+extension_displacement));
                  } break;

      case 0x7:
        switch (EA_register)
        {
            case 0x0: if ((0x0040&allowed_modes)==0) Exception_Handler(4); else     // Absolute short
                      { ea_type=MEMORY;
                        temp_address = Sign_Extend(BIU_PFQ_Fetch(),16);
                        return temp_address;
                      } break;

            case 0x1: if ((0x0020&allowed_modes)==0) Exception_Handler(4); else     // Absolute long
                      { ea_type=MEMORY;
                        temp_address = BIU_PFQ_Fetch();
                        temp_address = ( (temp_address<<16) | BIU_PFQ_Fetch() );
                        return temp_address;
                      } break;

            case 0x2: if ((0x0010&allowed_modes)==0) Exception_Handler(4); else     // x(PC)
                      { ea_type=MEMORY;
                        temp_address = (0xFFFFFFFF&(mc68k_pc + Sign_Extend(BIU_PFQ_Fetch(),16)));
                        return temp_address;
                      } break;

            case 0x3: if ((0x0008&allowed_modes)==0) Exception_Handler(4); else     // d8(PC,Xn)
                      { ea_type=MEMORY;
                        original_mc68k_pc = mc68k_pc;
                        ea_extension = BIU_PFQ_Fetch();
                        extension_displacement = Sign_Extend( (0x00FF&ea_extension) , 8);
                        extension_register = (0x7000&ea_extension)>>12;
                        ea_A_bit = ((ea_extension & 0x8000) >> 15);
                        ea_L_bit = ((ea_extension & 0x0800) >> 11);
                        if (ea_A_bit==1 && ea_L_bit==0) offset=Sign_Extend(Fetch_Address_Register(extension_register,16) ,16); else
                        if (ea_A_bit==0 && ea_L_bit==0) offset=Sign_Extend(Fetch_Data_Register(extension_register,16) ,16); else
                        if (ea_A_bit==1 && ea_L_bit==1) offset=Fetch_Address_Register(extension_register,32); else
                        if (ea_A_bit==0 && ea_L_bit==1) offset=Fetch_Data_Register(extension_register,32);
                        return (0xFFFFFFFF&(original_mc68k_pc+offset+extension_displacement));
                      } break;

            case 0x4: if ((0x0040&allowed_modes)==0) Exception_Handler(4); else     // Immediate
                      {  ea_type=IMMEDIATE;
                         if (data_size==8)  { return (0x00FF&BIU_PFQ_Fetch()); }
                         if (data_size==16) { return (0xFFFF&BIU_PFQ_Fetch()); }
                         if (data_size==32) { temp_address = BIU_PFQ_Fetch();
                                              temp_address = ( (temp_address<<16) | BIU_PFQ_Fetch() );
                                              return temp_address;
                                            }
                      } break;
        }
    }
return 0;
}


// ----------------------------------------------------------------------
uint32_t Fetch_EA(uint32_t local_EA , uint8_t local_ea_type) {
    if (local_ea_type==ADDRESS_REG && data_size==8 ) return (Fetch_Address_Register(local_EA,8));    else
    if (local_ea_type==ADDRESS_REG && data_size==16) return (Fetch_Address_Register(local_EA,16));   else
    if (local_ea_type==ADDRESS_REG && data_size==32) return (Fetch_Address_Register(local_EA,32));   else

    if (local_ea_type==DATA_REG && data_size==8 )    return (Fetch_Data_Register(local_EA,8));       else
    if (local_ea_type==DATA_REG && data_size==16)    return (Fetch_Data_Register(local_EA,16));      else
    if (local_ea_type==DATA_REG && data_size==32)    return (Fetch_Data_Register(local_EA,32));      else

    if (local_ea_type==MEMORY && data_size==8)    return (0xFF&BIU_Read(local_EA , data_size));      else
    if (local_ea_type==MEMORY && data_size==16)   return BIU_Read(local_EA , data_size);             else
    if (local_ea_type==MEMORY && data_size==32)   return BIU_Read_32(local_EA);                      else

    if (local_ea_type==IMMEDIATE)  { return local_EA; } // Immediate data held in the calculated EA
    
    return 0xEEEE;
}

// ----------------------------------------------------------------------
void Writeback_EA(uint32_t local_EA , uint8_t local_ea_type , uint32_t writeback_data) {
  if (local_ea_type==ADDRESS_REG)  { Store_Address_Register(local_EA , writeback_data , data_size);  return;  }
  if (local_ea_type==DATA_REG)     { Store_Data_Register(local_EA , writeback_data , data_size);     return;  }


  if (local_ea_type==MEMORY && data_size==8)   { BIU_Write(local_EA , writeback_data , data_size); return; }
  if (local_ea_type==MEMORY && data_size==16)  { BIU_Write(local_EA , writeback_data , data_size); return; }
  if (local_ea_type==MEMORY && data_size==32)  { BIU_Write_32(local_EA , writeback_data);          return; }

  return;
}


// ----------------------------------------------------------------------
uint32_t Fetch_Immediate(uint8_t local_size) {
    uint32_t temp_data;

    if (local_size==8)  { return (0xFF&BIU_PFQ_Fetch()); }
    if (local_size==16) { return BIU_PFQ_Fetch(); }
    if (local_size==32) { temp_data = BIU_PFQ_Fetch();
                          temp_data = ( (temp_data<<16) | BIU_PFQ_Fetch() );
                          return temp_data;
                        }
    return 0xEEEE;
}


// ----------------------------------------------------------------------
void Calculate_Flag_N(uint32_t result_data) {
    if (data_size==8 && (0x80&result_data)==0)         mc68k_flags = (mc68k_flags & 0xFFF7);  else
    if (data_size==8 && (0x80&result_data)!=0)         mc68k_flags = (mc68k_flags | 0x0008);  else

    if (data_size==16 && (0x8000&result_data)==0)      mc68k_flags = (mc68k_flags & 0xFFF7);  else
    if (data_size==16 && (0x8000&result_data)!=0)      mc68k_flags = (mc68k_flags | 0x0008);  else

    if (data_size==32 && (0x80000000&result_data)==0)  mc68k_flags = (mc68k_flags & 0xFFF7);  else
    if (data_size==32 && (0x80000000&result_data)!=0)  mc68k_flags = (mc68k_flags | 0x0008);

    return ;
}


// ----------------------------------------------------------------------
void Calculate_Flag_Z(uint32_t result_data , uint8_t clear_only) {

    if (data_size==8 && (0xFF&result_data)!=0)                              mc68k_flags = (mc68k_flags & 0xFFFB);  else
    if (data_size==8 && (0xFF&result_data)==0 && clear_only==FALSE)         mc68k_flags = (mc68k_flags | 0x0004);  else

    if (data_size==16 && (0xFFFF&result_data)!=0)                           mc68k_flags = (mc68k_flags & 0xFFFB);  else
    if (data_size==16 && (0xFFFF&result_data)==0&& clear_only==FALSE)       mc68k_flags = (mc68k_flags | 0x0004);  else

    if (data_size==32 && (0xFFFFFFFF&result_data)!=0)                       mc68k_flags = (mc68k_flags & 0xFFFB);  else
    if (data_size==32 && (0xFFFFFFFF&result_data)==0 && clear_only==FALSE)  mc68k_flags = (mc68k_flags | 0x0004);

    return ;
}


// ----------------------------------------------------------------------
void op_MOVE() {
  data_size             = DATA_SIZE_TYPE_B;                                                     // Get the data size from the opcode bits[13:12]

  source_ea             = Calculate_EA(0x3FFC);                                                 // Calculate the Source EA, checking supported modes
  source_ea_type        = ea_type;

  first_opcode          = (0x0FC0&first_opcode);                                                // Re-arrange the second EA bit fields to the usual locations for the MOVE instruction
  first_opcode          = ( ((0x01C0&first_opcode)>>3) | ((0x0E00&first_opcode)>>9) );

  destination_ea        = Calculate_EA(0x3FE0);                                                 // Calculate the Destination  EA, checking supported modes
  destination_ea_type   = ea_type;


  EA_Data = Fetch_EA(source_ea , source_ea_type);                                               // Fetch the Source EA operand

  if ( (destination_ea_type==ADDRESS_REG) && data_size==16)  { Writeback_EA(destination_ea , destination_ea_type ,(Sign_Extend(EA_Data ,16)) );  } // Write-back to Destination EA
  else                                                       { Writeback_EA(destination_ea , destination_ea_type ,       EA_Data             );  }


  if (destination_ea_type != ADDRESS_REG)                                                       // Don't update flags for MOVEA
  {
    mc68k_flags = ( mc68k_flags & 0xFFFC);                                                      // Always clear V, C Flags
    Calculate_Flag_N(EA_Data);                                                                  // Calculate the N Flag
    Calculate_Flag_Z(EA_Data , FALSE);                                                          // Calculate the Z Flag - Clear_only = FALSE
  }

  return;
}


// ----------------------------------------------------------------------
void op_MOVE_TO_CCR() {
  data_size = 8;
  source_ea     = Calculate_EA(0x2FFC);                                                         // Calculate the Source EA, checking supported modes
  EA_Data       = Fetch_EA(source_ea , ea_type);                                                // Fetch the Source EA operand
  mc68k_flags   = ( (0xFF00&mc68k_flags) | (0x001F&EA_Data) );                                  // Update the CCR Flags
  return;
}


// ----------------------------------------------------------------------
void op_MOVE_TO_SR() {
  if (mc68k_flag_S==0x0) { Exception_Handler(8); }                                              // Verify that supervisor privilege is set
  else
  {
    data_size = 16;
    source_ea   = Calculate_EA(0x2FFC);                                                         // Calculate the Source EA, checking supported modes
    EA_Data     = Fetch_EA(source_ea , ea_type);                                                // Fetch the Source EA operand
    Update_System_Flags(EA_Data);                                                               // Update the SR Flags
  }
  return;
}


// ----------------------------------------------------------------------
void op_MOVE_FROM_SR() {
  data_size         = 16;
  calculated_EA     = Calculate_EA(0x2FE0);                                                     // Calculate the Source EA, checking supported modes
  EA_Data           = Fetch_EA(calculated_EA , ea_type);                                        // Unnecessary fetch the EA operand ** Unoptimized 68000 **
  Writeback_EA(calculated_EA , ea_type ,mc68k_flags);                                           // Write-back to the Destination EA
  return;
}


// ----------------------------------------------------------------------
void op_MOVE_USP() {
  if (mc68k_flag_S==0x0) { Exception_Handler(8); }                                              // Verify that supervisor privilege is set
  else
  {
    data_size = 32;
    EA_register =(0x0007&first_opcode);

    if ((0x8&first_opcode)==0) {  m68k_address_reg[7] = Fetch_Address_Register(EA_register,32);      } else    // Address Register --> USP
                               {  Store_Address_Register(EA_register , m68k_address_reg[7] , 32); }            // USP --> Address Register
  }
  return;
}


// ----------------------------------------------------------------------
void op_JMP() {
  calculated_EA = Calculate_EA(0x09F8);                                                         // Calculate the EA, checking supported modes
  BIU_Jump(calculated_EA);                                                                      // Jump to the new PC
  return;
}


// ----------------------------------------------------------------------
void op_JSR() {
  calculated_EA = Calculate_EA(0x09F8);                                                         // Calculate the EA, checking supported modes
  Push(mc68k_pc&0xFFFF);                                                                        // Stack the lower PC
  Push(mc68k_pc>>16);                                                                           // Stack the upper PC
  BIU_Jump(calculated_EA);                                                                      // Jump to the new PC
  return;
}


// ----------------------------------------------------------------------
void op_BSR() {
  uint32_t opcode_pc;
  uint32_t displacement;

  displacement = (0x00FF&first_opcode);                                                         // Isolate the displacement field of the opcode
  opcode_pc     = mc68k_pc ;                                                                    // Store the PC of the initial opcode

  if (displacement==0)  {  displacement=Sign_Extend(BIU_PFQ_Fetch() , 16);  }                   // Sign extend the 16-bit displacement held in the next opcode word
  else                  {  displacement=Sign_Extend(displacement     , 8);  }                   // Sign extend the 8-bit displacement held in the initial opcode

  Push(mc68k_pc&0xFFFF);                                                                        // Stack the lower PC of address of the next opcode
  Push(mc68k_pc>>16);                                                                           // Stack the upper PC of address of the next opcode
  BIU_Jump(opcode_pc + displacement);                                                           // Jump to the new PC calculated using the address of first byte of the opcode

  return;
}


// ----------------------------------------------------------------------
void op_RTS()
{
  mc68k_pc = Pop()<<16;                                                                         // Pop the upper PC
  mc68k_pc = mc68k_pc | Pop();                                                                  // Pop the lower PC
  BIU_Jump(mc68k_pc);                                                                           // Jump to the new PC
  return;
}


// ----------------------------------------------------------------------
void op_RTR() {
  mc68k_flags   = ( (0xFFE0&mc68k_flags) | (0x001F&Pop()) );                                    // Update the CCR Flags
  mc68k_pc = Pop()<<16;                                                                         // Pop the upper PC
  mc68k_pc = mc68k_pc | Pop();                                                                  // Pop the lower PC
  BIU_Jump(mc68k_pc);                                                                           // Jump to the new PC
  return;
}


// ----------------------------------------------------------------------
void op_BCC() {
  uint32_t opcode_pc;
  uint32_t displacement;

  displacement  = (0x00FF&first_opcode);                                                        // Isolate the displacement field of the opcode
  opcode_pc     = mc68k_pc ;                                                                    // Store the PC of the initial opcode

  if (displacement==0)  {  displacement=Sign_Extend(BIU_PFQ_Fetch() , 16);  }                   // Sign extend the 16-bit displacement held in the next opcode word
  else                  {  displacement=Sign_Extend(displacement     , 8);  }                   // Sign extend the 8-bit displacement held in the initial opcode

  if (Test_Condition(first_opcode)==TRUE) { BIU_Jump(opcode_pc + displacement); }               // Jump to the new PC
  else                                    { return;                             }
  return;
}


// ----------------------------------------------------------------------
void op_SCC() {
  data_size         = 8;
  calculated_EA     = Calculate_EA(0x2FE0);                                                     // Calculate the EA, checking supported modes
  EA_Data           = Fetch_EA(calculated_EA , ea_type);                                        // Unnecessary fetch the EA operand ** Unoptimized 68000 **

  if (Test_Condition(first_opcode)==TRUE)  {  EA_Data=0x000000FF;  }                            // Condition TRUE, so set the byte to 0xFF
  else                                     {  EA_Data=0xFFFFFF00;  }                            // Condition FALSE, so clear the byte to 0

  Writeback_EA(calculated_EA , ea_type ,EA_Data);                                               // Write-back to the EA

  return;
}


// ----------------------------------------------------------------------
void op_dBCC() {
  uint32_t opcode_pc;
  uint32_t displacement;
  uint16_t  counter;

  reg_num = (0x0007&first_opcode);                                                              // Isolate the register number from the opcode
  opcode_pc     = mc68k_pc;                                                                     // Store the PC of the initial opcode

  displacement=Sign_Extend(BIU_PFQ_Fetch() , 16);                                               // Fetch and sign-extend the 16-bit displacement from the second opcode

  counter = Fetch_Data_Register(reg_num,16);                                                    // Fetch the Loop counter value from the register - just lower 16-bits

  if ( Test_Condition(first_opcode)==FALSE )
  {
    counter = counter - 1;
    Store_Data_Register(reg_num , counter , 16);                                                // Write-back the counter value to the register

    if ((0x0000FFFF&counter)==0xFFFF)  { return;                                       }        // When the counter equals (-1), continue to next opcode, else jump to the new PC
    else                               { BIU_Jump(opcode_pc + displacement);  return;  }

  }
  return;
}


// ----------------------------------------------------------------------
void op_LEA() {
  data_size         = 32;
  calculated_EA     = Calculate_EA(0x09F8);                                                     // Calculate the EA, checking supported modes
  reg_num = (0x0E00&first_opcode)>>9;                                                           // Isolate the register number from the opcode
  Store_Address_Register(reg_num , calculated_EA , 32);                                         // Write-back the EA value to the register
  return;
}


// ----------------------------------------------------------------------
void op_PEA() {
  data_size         = 32;
  calculated_EA     = Calculate_EA(0x09F8);                                                     // Calculate the EA, checking supported modes
  Push(calculated_EA&0xFFFF);                                                                   // Stack the lower portion of the EA
  Push(calculated_EA>>16);                                                                      // Stack the upper portion of the EA
  return;
}


// ----------------------------------------------------------------------
void op_CHK()
{
  int16_t  reg_data;

  data_size         = 16;
  calculated_EA     = Calculate_EA(0x02FFC);                                                    // Calculate the EA, checking supported modes
  EA_Data = Fetch_EA(calculated_EA , ea_type);                                                  // Fetch the EA operand

  reg_num  = (0x0E00&first_opcode)>>9;                                                          // Isolate the register number from the opcode
  reg_data = Fetch_Data_Register(reg_num,16);                                                   // Fetch the lower 16-bits of the Data Register

  if ( (0x8000&reg_data) != 0)                                                                  // Is number negative?
    {
      mc68k_flags = mc68k_flags | 0x08;                                                         // Set the N flag
      Exception_Handler(6);
      return;
    }
  else if ( (int16_t)reg_data > (int16_t)EA_Data )
    {
      mc68k_flags = mc68k_flags & 0xFFF7;                                                       // Clear the N flag
      Exception_Handler(6);
      return;
    }
  else
    {
      return;
    }
}


// ----------------------------------------------------------------------
void op_TRAPV() {
  if (mc68k_flag_V==1) { Exception_Handler(7);  return;  }
  else                 { return;                         }
}


// ----------------------------------------------------------------------
void op_TRAP() {
  Exception_Handler( (0x000F&first_opcode) + 32 );                                              // Isolate the register number from the opcode and add 32 to create the vector number
  return;
}


// ----------------------------------------------------------------------
void op_LINK() {
  int32_t reg_data;

  reg_num = (0x0007&first_opcode);                                                              // Isolate the register number from the opcode
  reg_data = Fetch_Address_Register(reg_num,32);                                                // Fetch the Address Register
  Push(reg_data&0xFFFF);                                                                        // Stack the lower portion of the Address Register
  Push(reg_data>>16);                                                                           // Stack the upper portion of the Address Register

  if (mc68k_flag_S==1)
    {
      Store_Address_Register(reg_num, m68k_a7_S , 32);                                          //  Copy the active Stack Pointer to the selected Address Register
      m68k_a7_S = m68k_a7_S + Sign_Extend(BIU_PFQ_Fetch() ,16);
    }
  else
    {
      Store_Address_Register(reg_num, m68k_address_reg[7] , 32);
      m68k_address_reg[7] = m68k_address_reg[7] + Sign_Extend(BIU_PFQ_Fetch() ,16);
    }

  return;
}


// ----------------------------------------------------------------------
void op_UNLK() {
  uint32_t reg_data;

  reg_num = (0x0007&first_opcode);                                                              // Isolate the register number from the opcode
  reg_data = Fetch_Address_Register(reg_num,32);                                                   // Fetch the Address Register

  if (mc68k_flag_S==1) {  m68k_a7_S = reg_data;             }                                   // Copy Address contents to the current Stack Pointer
  else                 {  m68k_address_reg[7] = reg_data;   }

  reg_data = Pop()<<16;                                                                         // Pop the upper Address Register
  reg_data = reg_data | Pop();                                                                  // Pop the lower Address Register
  Store_Address_Register(reg_num, reg_data , 32);                                               // Write-back the stacked address to the Address Register

  return;
}


// ----------------------------------------------------------------------
void op_RTE() {
  if (mc68k_flag_S==0x0) { Exception_Handler(8); }                                              // Verify that supervisor privilege is set
  else
  {
    Update_System_Flags(Pop());                                                                 // Pop the SR Flags
    mc68k_pc    = Pop()<<16;                                                                    // Pop the upper PC
    mc68k_pc    = mc68k_pc | Pop();                                                             // Pop the lower PC
    BIU_Jump(mc68k_pc);                                                                         // Jump to the new PC
  }
  return;
}


// ----------------------------------------------------------------------
void op_RESET()
{
  if (mc68k_flag_S==0x0) {  Exception_Handler(8); }                                             // Verify that supervisor privilege is set
  else                   {  BIU_Force_Reset();    }
  return;
}


// ----------------------------------------------------------------------
void op_STOP()
{
  if (mc68k_flag_S==0x0) { Exception_Handler(8); }                                              // Verify that supervisor privilege is set
  else
  {
    Update_System_Flags(BIU_PFQ_Fetch());                                                       // Fetch the SR Flags

    while ( BIU_RESET_STATUS()==0 && BIU_IRQ_LEVEL()==0 && mc68k_flag_T==0)  {  }               // Proceed if Trace is active, or RESET/Interrupt occurs
  }
  return;
}


// ----------------------------------------------------------------------
void op_SWAP() {
  uint32_t reg_data;

  data_size     = 32;
  reg_num       = (0x0007&first_opcode);                                                        // Isolate the register number from the opcode

  reg_data = Fetch_Data_Register(reg_num,32);                                                   // Swap the upper and lower words of the register, then write it back
  reg_data = (reg_data<<16 | reg_data>>16);
  Store_Data_Register(reg_num , reg_data , 32);

  mc68k_flags = ( mc68k_flags & 0xFFFC);                                                        // Always clear V, C Flags
  Calculate_Flag_N(reg_data);                                                                   // Calculate the N Flag
  Calculate_Flag_Z(reg_data , FALSE);                                                           // Calculate the Z Flag - Clear_only = FALSE

  return;
}


// ----------------------------------------------------------------------
void op_EXT() {
  uint32_t reg_data;

  reg_num       = (0x0007&first_opcode);                                                        // Isolate the register number from the opcode

  if ( ((0x00C0&first_opcode)>>6) == 2)
  {
    reg_data = Sign_Extend(Fetch_Data_Register(reg_num,8) ,8);
    Store_Data_Register(reg_num , reg_data , 16);
    data_size = 16;                                                                             // Set size for flag calculation
  }
  else
  {
    reg_data = Sign_Extend(Fetch_Data_Register(reg_num,16) ,16);
    Store_Data_Register(reg_num , reg_data , 32);
    data_size = 32;                                                                             // Set size for flag calculation
  }

  mc68k_flags = ( mc68k_flags & 0xFFFC);                                                        // Always clear V, C Flags
  Calculate_Flag_N(reg_data);                                                                   // Calculate the N Flag
  Calculate_Flag_Z(reg_data , FALSE);                                                           // Calculate the Z Flag - Clear_only = FALSE
  return;
}


// ----------------------------------------------------------------------
void op_MOVEQ() {
  uint32_t reg_data;

  reg_num       = (0x0E00&first_opcode)>>9;                                                     // Isolate the register number from the opcode

  reg_data = Sign_Extend(first_opcode ,8);                                                      // Sign-extend the data held in the opcode and write-back to selected register
  Store_Data_Register(reg_num , reg_data , 32);

  data_size     = 32;                                                                           // Set size for flag calculation
  mc68k_flags = ( mc68k_flags & 0xFFFC);                                                        // Always clear V, C Flags
  Calculate_Flag_N(reg_data);                                                                   // Calculate the N Flag
  Calculate_Flag_Z(reg_data , FALSE);                                                           // Calculate the Z Flag - Clear_only = FALSE

  return;
}


// ----------------------------------------------------------------------
void op_MOVEM() {
  unsigned short increment_size;
  uint16_t reg_list;

  data_size      = DATA_SIZE_TYPE_C;                                                             // Get the data size from the opcode bit[6]
  increment_size = data_size>>3;                                                                 // increment_size is +2 for Word,  +4 for Long

  if ( ((0x0400&first_opcode)>>10) == 1)                                                         // ** Memory to Registers **
  {
    reg_list        = BIU_PFQ_Fetch();                                                           // Get the Register_list from the second opcode word
    calculated_EA=Calculate_EA(0x0DF8);

    if ( (0x0001&reg_list)!=0)  { Store_Data_Register   (0 , Sign_Extend(Fetch_EA(calculated_EA , ea_type) ,data_size) , 32); calculated_EA=calculated_EA+increment_size; }  // D0
    if ( (0x0002&reg_list)!=0)  { Store_Data_Register   (1 , Sign_Extend(Fetch_EA(calculated_EA , ea_type) ,data_size) , 32); calculated_EA=calculated_EA+increment_size; }  // D1
    if ( (0x0004&reg_list)!=0)  { Store_Data_Register   (2 , Sign_Extend(Fetch_EA(calculated_EA , ea_type) ,data_size) , 32); calculated_EA=calculated_EA+increment_size; }  // D2
    if ( (0x0008&reg_list)!=0)  { Store_Data_Register   (3 , Sign_Extend(Fetch_EA(calculated_EA , ea_type) ,data_size) , 32); calculated_EA=calculated_EA+increment_size; }  // D3
    if ( (0x0010&reg_list)!=0)  { Store_Data_Register   (4 , Sign_Extend(Fetch_EA(calculated_EA , ea_type) ,data_size) , 32); calculated_EA=calculated_EA+increment_size; }  // D4
    if ( (0x0020&reg_list)!=0)  { Store_Data_Register   (5 , Sign_Extend(Fetch_EA(calculated_EA , ea_type) ,data_size) , 32); calculated_EA=calculated_EA+increment_size; }  // D5
    if ( (0x0040&reg_list)!=0)  { Store_Data_Register   (6 , Sign_Extend(Fetch_EA(calculated_EA , ea_type) ,data_size) , 32); calculated_EA=calculated_EA+increment_size; }  // D6
    if ( (0x0080&reg_list)!=0)  { Store_Data_Register   (7 , Sign_Extend(Fetch_EA(calculated_EA , ea_type) ,data_size) , 32); calculated_EA=calculated_EA+increment_size; }  // D7
    if ( (0x0100&reg_list)!=0)  { Store_Address_Register(0 , Sign_Extend(Fetch_EA(calculated_EA , ea_type) ,data_size) , 32); calculated_EA=calculated_EA+increment_size; }  // A0
    if ( (0x0200&reg_list)!=0)  { Store_Address_Register(1 , Sign_Extend(Fetch_EA(calculated_EA , ea_type) ,data_size) , 32); calculated_EA=calculated_EA+increment_size; }  // A1
    if ( (0x0400&reg_list)!=0)  { Store_Address_Register(2 , Sign_Extend(Fetch_EA(calculated_EA , ea_type) ,data_size) , 32); calculated_EA=calculated_EA+increment_size; }  // A2
    if ( (0x0800&reg_list)!=0)  { Store_Address_Register(3 , Sign_Extend(Fetch_EA(calculated_EA , ea_type) ,data_size) , 32); calculated_EA=calculated_EA+increment_size; }  // A3
    if ( (0x1000&reg_list)!=0)  { Store_Address_Register(4 , Sign_Extend(Fetch_EA(calculated_EA , ea_type) ,data_size) , 32); calculated_EA=calculated_EA+increment_size; }  // A4
    if ( (0x2000&reg_list)!=0)  { Store_Address_Register(5 , Sign_Extend(Fetch_EA(calculated_EA , ea_type) ,data_size) , 32); calculated_EA=calculated_EA+increment_size; }  // A5
    if ( (0x4000&reg_list)!=0)  { Store_Address_Register(6 , Sign_Extend(Fetch_EA(calculated_EA , ea_type) ,data_size) , 32); calculated_EA=calculated_EA+increment_size; }  // A6
    if ( (0x8000&reg_list)!=0)  { Store_Address_Register(7 , Sign_Extend(Fetch_EA(calculated_EA , ea_type) ,data_size) , 32); calculated_EA=calculated_EA+increment_size; }  // A7

    immediate = Fetch_EA(calculated_EA , ea_type);                                                          // Extra fetch due to 68000 anomaly

    if ( ((0x0038&first_opcode)>>3) == 0x3 ) Store_Address_Register(EA_register, calculated_EA , 32);       // Write-back the address register for (An)+ Mode
  }


  else if ( ((0x0400&first_opcode)>>10) == 0 && ((0x0038&first_opcode)>>3) == 0x4 )                         // **  Registers to Memory    -(An) Addressing Mode **
  {
    reg_list        = BIU_PFQ_Fetch();                                                                      // Get the Register_list from the second opcode word
    calculated_EA = Calculate_EA(0x0BE0) + increment_size;                                                  // Get the Address but adjust so decrementing can start fresh below
    Store_Address_Register(EA_register, (calculated_EA) , 32);                                              // Write-back the address register to the initial value

    if ( (0x0001&reg_list)!=0)  {  calculated_EA=calculated_EA-increment_size; Writeback_EA(calculated_EA , ea_type , Fetch_Address_Register(7,32) );  }  // A7
    if ( (0x0002&reg_list)!=0)  {  calculated_EA=calculated_EA-increment_size; Writeback_EA(calculated_EA , ea_type , Fetch_Address_Register(6,32) );  }  // A6
    if ( (0x0004&reg_list)!=0)  {  calculated_EA=calculated_EA-increment_size; Writeback_EA(calculated_EA , ea_type , Fetch_Address_Register(5,32) );  }  // A5
    if ( (0x0008&reg_list)!=0)  {  calculated_EA=calculated_EA-increment_size; Writeback_EA(calculated_EA , ea_type , Fetch_Address_Register(4,32) );  }  // A4
    if ( (0x0010&reg_list)!=0)  {  calculated_EA=calculated_EA-increment_size; Writeback_EA(calculated_EA , ea_type , Fetch_Address_Register(3,32) );  }  // A3
    if ( (0x0020&reg_list)!=0)  {  calculated_EA=calculated_EA-increment_size; Writeback_EA(calculated_EA , ea_type , Fetch_Address_Register(2,32) );  }  // A2
    if ( (0x0040&reg_list)!=0)  {  calculated_EA=calculated_EA-increment_size; Writeback_EA(calculated_EA , ea_type , Fetch_Address_Register(1,32) );  }  // A1
    if ( (0x0080&reg_list)!=0)  {  calculated_EA=calculated_EA-increment_size; Writeback_EA(calculated_EA , ea_type , Fetch_Address_Register(0,32) );  }  // A0
    if ( (0x0100&reg_list)!=0)  {  calculated_EA=calculated_EA-increment_size; Writeback_EA(calculated_EA , ea_type , Fetch_Data_Register   (7,32) );  }  // D7
    if ( (0x0200&reg_list)!=0)  {  calculated_EA=calculated_EA-increment_size; Writeback_EA(calculated_EA , ea_type , Fetch_Data_Register   (6,32) );  }  // D6
    if ( (0x0400&reg_list)!=0)  {  calculated_EA=calculated_EA-increment_size; Writeback_EA(calculated_EA , ea_type , Fetch_Data_Register   (5,32) );  }  // D5
    if ( (0x0800&reg_list)!=0)  {  calculated_EA=calculated_EA-increment_size; Writeback_EA(calculated_EA , ea_type , Fetch_Data_Register   (4,32) );  }  // D4
    if ( (0x1000&reg_list)!=0)  {  calculated_EA=calculated_EA-increment_size; Writeback_EA(calculated_EA , ea_type , Fetch_Data_Register   (3,32) );  }  // D3
    if ( (0x2000&reg_list)!=0)  {  calculated_EA=calculated_EA-increment_size; Writeback_EA(calculated_EA , ea_type , Fetch_Data_Register   (2,32) );  }  // D2
    if ( (0x4000&reg_list)!=0)  {  calculated_EA=calculated_EA-increment_size; Writeback_EA(calculated_EA , ea_type , Fetch_Data_Register   (1,32) );  }  // D1
    if ( (0x8000&reg_list)!=0)  {  calculated_EA=calculated_EA-increment_size; Writeback_EA(calculated_EA , ea_type , Fetch_Data_Register   (0,32) );  }  // D0

    Store_Address_Register(EA_register, calculated_EA , 32);                                                // Write-back the address register
  }


  else if ( ((0x0400&first_opcode)>>10) == 0 && ((0x0038&first_opcode)>>3) != 0x4 )                         //  ** Registers to Memory   All except -(An) Addressing Mode **
  {
    reg_list        = BIU_PFQ_Fetch();                                                                      // Get the Register_list from the second opcode word
    calculated_EA=Calculate_EA(0x0BE0);

    if ( (0x0001&reg_list)!=0)  { Writeback_EA(calculated_EA , ea_type , Fetch_Data_Register   (0,32) );  calculated_EA=calculated_EA+increment_size; }  // D0
    if ( (0x0002&reg_list)!=0)  { Writeback_EA(calculated_EA , ea_type , Fetch_Data_Register   (1,32) );  calculated_EA=calculated_EA+increment_size; }  // D1
    if ( (0x0004&reg_list)!=0)  { Writeback_EA(calculated_EA , ea_type , Fetch_Data_Register   (2,32) );  calculated_EA=calculated_EA+increment_size; }  // D2
    if ( (0x0008&reg_list)!=0)  { Writeback_EA(calculated_EA , ea_type , Fetch_Data_Register   (3,32) );  calculated_EA=calculated_EA+increment_size; }  // D3
    if ( (0x0010&reg_list)!=0)  { Writeback_EA(calculated_EA , ea_type , Fetch_Data_Register   (4,32) );  calculated_EA=calculated_EA+increment_size; }  // D4
    if ( (0x0020&reg_list)!=0)  { Writeback_EA(calculated_EA , ea_type , Fetch_Data_Register   (5,32) );  calculated_EA=calculated_EA+increment_size; }  // D5
    if ( (0x0040&reg_list)!=0)  { Writeback_EA(calculated_EA , ea_type , Fetch_Data_Register   (6,32) );  calculated_EA=calculated_EA+increment_size; }  // D6
    if ( (0x0080&reg_list)!=0)  { Writeback_EA(calculated_EA , ea_type , Fetch_Data_Register   (7,32) );  calculated_EA=calculated_EA+increment_size; }  // D7
    if ( (0x0100&reg_list)!=0)  { Writeback_EA(calculated_EA , ea_type , Fetch_Address_Register(0,32) );  calculated_EA=calculated_EA+increment_size; }  // A0
    if ( (0x0200&reg_list)!=0)  { Writeback_EA(calculated_EA , ea_type , Fetch_Address_Register(1,32) );  calculated_EA=calculated_EA+increment_size; }  // A1
    if ( (0x0400&reg_list)!=0)  { Writeback_EA(calculated_EA , ea_type , Fetch_Address_Register(2,32) );  calculated_EA=calculated_EA+increment_size; }  // A2
    if ( (0x0800&reg_list)!=0)  { Writeback_EA(calculated_EA , ea_type , Fetch_Address_Register(3,32) );  calculated_EA=calculated_EA+increment_size; }  // A3
    if ( (0x1000&reg_list)!=0)  { Writeback_EA(calculated_EA , ea_type , Fetch_Address_Register(4,32) );  calculated_EA=calculated_EA+increment_size; }  // A4
    if ( (0x2000&reg_list)!=0)  { Writeback_EA(calculated_EA , ea_type , Fetch_Address_Register(5,32) );  calculated_EA=calculated_EA+increment_size; }  // A5
    if ( (0x4000&reg_list)!=0)  { Writeback_EA(calculated_EA , ea_type , Fetch_Address_Register(6,32) );  calculated_EA=calculated_EA+increment_size; }  // A6
    if ( (0x8000&reg_list)!=0)  { Writeback_EA(calculated_EA , ea_type , Fetch_Address_Register(7,32) );  calculated_EA=calculated_EA+increment_size; }  // A7
  }
  return;
}


// ----------------------------------------------------------------------
void op_TAS() {
  data_size     = 8;
  calculated_EA = Calculate_EA(0x2FE0);                                                         // Calculate the EA, checking supported modes

  BIU_RMW(TRUE);                                                                                // Signal the BIU to perform an atomic R-M-W Cycle

  EA_Data = Fetch_EA(calculated_EA , ea_type);                                                  // Fetch the data
  Writeback_EA(calculated_EA , ea_type , (0x80|EA_Data));                                       // Write-back the data with bit[7] set to 1

  BIU_RMW(FALSE);                                                                               // Debounce R-M-W Cycle

  mc68k_flags = ( mc68k_flags & 0xFFFC);                                                        // Always clear V, C Flags
  Calculate_Flag_N(EA_Data);                                                                    // Calculate the N Flag
  Calculate_Flag_Z(EA_Data , FALSE);                                                            // Calculate the Z Flag - Clear_only = FALSE

  return;
}


// ----------------------------------------------------------------------
void op_TST() {

  data_size     = DATA_SIZE_TYPE_A;                                                             // Get the data size from the opcode bits[7:6]
  calculated_EA = Calculate_EA(0x2FE0);                                                         // Calculate the EA, checking supported modes

  EA_Data = Fetch_EA(calculated_EA , ea_type);                                                  // Fetch the data

  mc68k_flags = ( mc68k_flags & 0xFFFC);                                                        // Always clear V, C Flags
  Calculate_Flag_N(EA_Data);                                                                    // Calculate the N Flag
  Calculate_Flag_Z(EA_Data , FALSE);                                                            // Calculate the Z Flag - Clear_only = FALSE

  return;
}

// op_type  0=ADD  1=SUB
// ----------------------------------------------------------------------
void Calculate_Flags_C(uint32_t operand0 , uint32_t operand1 , uint8_t op_type) {
  uint32_t carry=0;

  if (op_type==0)
  {
    if      (data_size==8)   {  carry  = ( ((operand0&0x000000FF)+(operand1&0x000000FF)) &  0x00000100 );  }
    else if (data_size==16)  {  carry  = ( ((operand0&0x0000FFFF)+(operand1&0x0000FFFF)) &  0x00010000 );  }
    else if (data_size==32)  {  carry  = ( operand0 + operand1 ) < operand0; }
  }
  else
  {
    if      (data_size==8)   {  carry  = ( ((operand0&0x000000FF)-(operand1&0x000000FF)) &  0x00000100 );  }
    else if (data_size==16)  {  carry  = ( ((operand0&0x0000FFFF)-(operand1&0x0000FFFF)) &  0x00010000 );  }
    else if (data_size==32)  {  carry  = ( operand0 - operand1 ) > operand0;  }
  }

  if (carry!=0)   { mc68k_flags = ( mc68k_flags | 0x0001);  }                                   //Set C Flag
  return;
}


void Calculate_Flags_V(uint32_t operand0 , uint32_t operand1 , uint32_t result , uint8_t op_size , uint8_t op_type) {
  uint8_t overflow=0;

  if      (op_size==8)    {  operand0 = (operand0&0x00000080);  operand1 = (operand1&0x00000080);  result = (result&0x00000080); }
  else if (op_size==16)   {  operand0 = (operand0&0x00008000);  operand1 = (operand1&0x00008000);  result = (result&0x00008000); }
  else if (op_size==32)   {  operand0 = (operand0&0x80000000);  operand1 = (operand1&0x80000000);  result = (result&0x80000000); }

  if (op_type==0 )
  {
    if      (operand0==0 && operand1==0 && result!=0)   { overflow=1;   }      //Set V Flag for ADD
    else if (operand0!=0 && operand1!=0 && result==0)   { overflow=1;   }
  }
  else
  {
    if      (operand0==0 && operand1!=0 && result!=0)   {  overflow=1;   }      //Set V Flag for SUB
    else if (operand0!=0 && operand1==0 && result==0)   {  overflow=1;   }
  }

  if  (overflow==1) {  mc68k_flags = ( mc68k_flags | 0x0002);  }        // Set V Flag
  else              {  mc68k_flags = ( mc68k_flags & 0xFFFD);  }          // Clear V Flag


  return;
}



// op_type  1=NOT  2=NEG  3=NEGX  4=CLR
// ----------------------------------------------------------------------
void op_NEGS(uint8_t op_type) {

  data_size     = DATA_SIZE_TYPE_A;                                                             // Get the data size from the opcode bits[7:6]
  calculated_EA = Calculate_EA(0x2FE0);                                                         // Calculate the EA, checking supported modes
  mc68k_flags = ( mc68k_flags & 0xFFFC);                                                        // Pre-clear V, C Flags

  EA_Data = Fetch_EA(calculated_EA , ea_type);                                                  // Fetch the data

  if (op_type==1) { EA_Data=(~EA_Data);                                                         // NOT
                    Calculate_Flag_Z(EA_Data , FALSE);
                  } else

  if (op_type==2) { Calculate_Flags_C(0 , EA_Data , 1);                                         // NEG
                    Calculate_Flags_V(0 , EA_Data , -EA_Data , data_size , 1);
                    EA_Data= -EA_Data;
                    Calculate_Flag_Z(EA_Data , FALSE);
                    if (EA_Data==0) { mc68k_flags=(mc68k_flags & 0xFFEE); } else                // Set/Clear X and C Flags
                                    { mc68k_flags=(mc68k_flags | 0x0011); }
                  } else

  if (op_type==3) { Calculate_Flags_C(0 , EA_Data , 1);                                         // NEGX   ** Z Flag - Clear_only = TRUE
                    Calculate_Flags_V(0 , EA_Data , (0-EA_Data) , data_size , 1);
                    EA_Data= -EA_Data;
                    Calculate_Flags_C(EA_Data , mc68k_flag_X , 1);
                    Calculate_Flags_V(EA_Data , mc68k_flag_X , (EA_Data+mc68k_flag_X) , data_size , 1);
                    EA_Data=(EA_Data - mc68k_flag_X);
                    Calculate_Flag_Z(EA_Data , TRUE);
                    if (mc68k_flag_C != 0)  { mc68k_flags=(mc68k_flags | 0x0010);  } else       // Copy C flag to X flag
                                            { mc68k_flags=(mc68k_flags & 0xFFEF);  }
                  } else

  if (op_type==4) { EA_Data=(0x0);                                                              // CLR
                    Calculate_Flag_Z(EA_Data , FALSE);
                  }

  Writeback_EA(calculated_EA,ea_type,EA_Data);

  Calculate_Flag_N(EA_Data);
  return;
}


// ----------------------------------------------------------------------
void op_EXG() {
  uint8_t regX;
  uint8_t regY;
  uint32_t tempX;
  uint32_t tempY;

  regX = (0x0E00&first_opcode)>>9;                                                              // Isolate register numbers from the opcode
  regY = (0x0007&first_opcode);

  if ( (0x01F8&first_opcode) == 0x0140)       {  tempX = Fetch_Data_Register(regX,32);             // Exchange the registers
                                                 tempY = Fetch_Data_Register(regY,32);
                                                 Store_Data_Register(regX , tempY , 32);
                                                 Store_Data_Register(regY , tempX , 32);
                                              }
  else if ( (0x01F8&first_opcode) == 0x0148)  {  tempX = Fetch_Address_Register(regX,32);
                                                 tempY = Fetch_Address_Register(regY,32);
                                                 Store_Address_Register(regX , tempY , 32);
                                                 Store_Address_Register(regY , tempX , 32);
                                              }
  else if ( (0x01F8&first_opcode) == 0x0188)  {  tempX = Fetch_Data_Register(regX,32);
                                                 tempY = Fetch_Address_Register(regY,32);
                                                 Store_Data_Register(regX , tempY , 32);
                                                 Store_Address_Register(regY , tempX , 32);
                                              }
  return;
}


// op_type  00=BSET_Dynamic  10=BCLR_Dynamic  20=BCHG_Dynamic  30=BTST_Dynamic
//          01=BSET_Static   11=BCLR_Static   21=BCHG_Static   31=BTST_Static
// ------------------------------------------------------------------------------
void op_BMOD(uint8_t op_type) {
  uint32_t bit_num;

  if ( (0x0F&op_type)==0x0)                                                                     // ** Bit-number Dynamic **
  {
    reg_num = (0x0E00&first_opcode)>>9;                                                         // Isolate the register number from the opcode

    data_size = 8;    // Force EA Calculation to use BYTE mode so pre-decrement and post-increment address modifications as well as immediate fetching are correct
    if ((0xF0&op_type)==0x30)  calculated_EA = Calculate_EA(0x2FFC); else calculated_EA = Calculate_EA(0x2FE0);  //  BTST allowed EA types different from rest

    if (ea_type==MEMORY) { bit_num = (0x7&Fetch_Data_Register(reg_num,8));                      // Isolate a byte's worth of bit addressing
                           EA_Data = (0xFF&Fetch_EA(calculated_EA , ea_type));                  // Fetch the byte data from the Memory EA
                         }
    else                 { bit_num = (0x1F&Fetch_Data_Register(reg_num,8));                     // Isolate 32-bits worth of bit addressing
                           data_size = 32;                                                      // Force data size to 32-bit for non-memory locations
                           EA_Data = (Fetch_EA(calculated_EA , ea_type));                       // Fetch the register or immediate data
                         }
  }

  else                                                                                          // ** Bit-number Static **
  {
    bit_num = Fetch_Immediate(8);                                                               // Fetch the Bit Number field first

    data_size = 8;    // Force EA Calculation to use BYTE mode so pre-decrement and post-increment address modifications as well as immediate fetching are correct
    if ((0xF0&op_type)==0x30)  calculated_EA = Calculate_EA(0x2FF8); else calculated_EA = Calculate_EA(0x2FE0);  // BTST allowed EA types different from rest

    if (ea_type==MEMORY) { bit_num = (0x7&bit_num);                                             // Isolate a byte's worth of bit addressing from the second opcode
                           EA_Data = (0xFF&Fetch_EA(calculated_EA , ea_type));                  // Fetch the byte data from the Memory EA
                         }
    else                 { bit_num = (0x1F&bit_num);                                            // Isolate 32-bits worth of bit addressing from the second opcode
                           data_size = 32;                                                      // Force data size to 32-bit for non-memory locations
                           EA_Data = (Fetch_EA(calculated_EA , ea_type));                       // Fetch the register or immediate data

                         }
  }

  bit_num = 0x1<<bit_num;                                                                       // Convert encoded bit number to a one-bit-active vector
  if ( (EA_Data & bit_num) == 0) {  mc68k_flags = ( mc68k_flags | 0x0004);  }                   // Calculate Z Flag
  else                           {  mc68k_flags = ( mc68k_flags & 0xFFFB);  }


  if ( (0xF0&op_type)==0x00) { EA_Data = (EA_Data | bit_num);  Writeback_EA(calculated_EA , ea_type , EA_Data); } else      // BSET - Set the selected bit in the EA's Data
  if ( (0xF0&op_type)==0x10) { EA_Data = (EA_Data & ~bit_num); Writeback_EA(calculated_EA , ea_type , EA_Data); } else      // BCLR - Clear the selected bit in the EA's Data
  if ( (0xF0&op_type)==0x20) { EA_Data = (EA_Data ^ bit_num);  Writeback_EA(calculated_EA , ea_type , EA_Data); }           // BCHG - Change the selected bit in the EA's Data

  return;
}


// op_type   1=ASL  2=LSL
// ----------------------------------------------------------------------
void op_xSL(uint8_t dst_is_register , uint8_t op_type) {
  uint32_t data;
  uint8_t shift_count;

  mc68k_flags=(mc68k_flags&0xFFFD);                                                             // Pre-clear the V flag

  if (dst_is_register==1) { data_size  = DATA_SIZE_TYPE_A;                                      // Get the data size from the opcode bits[7:6]
                            immediate = (0x0E00&first_opcode)>>9;                               // Isolate the Immediate number from the opcode
                            reg_num   = (0x0007&first_opcode);                                  // Isolate the register number from the opcode
                            data = Fetch_Data_Register(reg_num,data_size);                      // Fetch the register we want to shift
                            if ((0x0020&first_opcode) == 0)   //   T=0 for Immediate field contains the shift_count   T=1 for shift_count contained in a register - Modulo-64
                              {  if(immediate==0)  shift_count=8; else shift_count = immediate;
                              }
                            else
                              { shift_count = (0x3F&Fetch_Data_Register(immediate,8));
                              }
                          }

  else                    { data_size  = 16;                                                    // Size is always 16 for memory
                            shift_count = 1;                                                    // Shift count is always 1 for memory
                            calculated_EA = Calculate_EA(0x0FE0);                               // Calculate the EA, checking supported modes
                            data = (Fetch_EA(calculated_EA , ea_type));                         // Fetch the word data we want to shift from the EA
                          }

  while (shift_count!=0)
  {
    if (data_size==8)  { if( (0x00000080&data)==0) mc68k_flags=(mc68k_flags&0xFFEE); else mc68k_flags=(mc68k_flags|0x0011); }       // Copy data[MSB] to the X and C flags
    if (data_size==16) { if( (0x00008000&data)==0) mc68k_flags=(mc68k_flags&0xFFEE); else mc68k_flags=(mc68k_flags|0x0011); }
    if (data_size==32) { if( (0x80000000&data)==0) mc68k_flags=(mc68k_flags&0xFFEE); else mc68k_flags=(mc68k_flags|0x0011); }

    if (data_size==8)  { if( (0x000000C0&data)==0x00000080 || (0x000000C0&data)==0x00000040 ) mc68k_flags=(mc68k_flags|0x0002); }   //  Set V if the MSB ever changes
    if (data_size==16) { if( (0x0000C000&data)==0x00008000 || (0x0000C000&data)==0x00004000 ) mc68k_flags=(mc68k_flags|0x0002); }
    if (data_size==32) { if( (0xC0000000&data)==0x80000000 || (0xC0000000&data)==0x40000000 ) mc68k_flags=(mc68k_flags|0x0002); }

    data = data << 1;                                                                           // Shift data to the left by one bit
    shift_count--;                                                                              // Decrement the shift_counter
  }

  if (dst_is_register==1) { Store_Data_Register(reg_num , data , data_size);  }                 // Write-back the results to the register
  else                    { Writeback_EA(calculated_EA , ea_type , data);     }                 // Write-back the results to the EA

  Calculate_Flag_Z(data , FALSE);                                                               // Calculate the Z Flag - Clear_only = FALSE
  Calculate_Flag_N(data);                                                                       // Calculate the N Flag
  if (op_type==2) { mc68k_flags=(mc68k_flags&0xFFFD);  }                                        // Always clear the V flag for LSL
  return;
}


// ----------------------------------------------------------------------
void op_ROL(uint8_t dst_is_register) {
  uint32_t data;
  uint8_t shift_count;

  if (dst_is_register==1) { data_size  = DATA_SIZE_TYPE_A;                                      // Get the data size from the opcode bits[7:6]
                            immediate = (0x0E00&first_opcode)>>9;                               // Isolate the Immediate number from the opcode
                            reg_num   = (0x0007&first_opcode);                                  // Isolate the register number from the opcode
                            data = Fetch_Data_Register(reg_num,data_size);                      // Fetch the register we want to shift
                            if ((0x0020&first_opcode) == 0)   //   T=0 for Immediate field contains the shift_count   T=1 for shift_count contained in a register - Modulo-64
                              {  if(immediate==0)  shift_count=8; else shift_count = immediate;
                              }
                            else
                              { shift_count = (0x3F&Fetch_Data_Register(immediate,8));
                              }
                            }
  else                    { data_size  = 16;
                            calculated_EA = Calculate_EA(0x0FE0);                               // Calculate the EA, checking supported modes
                            data = (Fetch_EA(calculated_EA , ea_type));                         // Fetch the word data we want to shift from the EA
                            shift_count = 1;
                          }


  while (shift_count!=0)
  {
    if (data_size==8)  { if( (0x00000080&data)==0) mc68k_flags=(mc68k_flags&0xFFFE); else mc68k_flags=(mc68k_flags|0x0001); }   // Copy data[MSB] to the C flag
    if (data_size==16) { if( (0x00008000&data)==0) mc68k_flags=(mc68k_flags&0xFFFE); else mc68k_flags=(mc68k_flags|0x0001); }
    if (data_size==32) { if( (0x80000000&data)==0) mc68k_flags=(mc68k_flags&0xFFFE); else mc68k_flags=(mc68k_flags|0x0001); }

    data = data << 1;                                                                           // Shift data to the left by one bit
    data = ( (mc68k_flags&0x0001) | data );                                                     // Or the Carry Flag into bit[0] of the new data
    shift_count--;                                                                              // Decrement the shift_counter
  }

  if (dst_is_register==1) { Store_Data_Register(reg_num , data , data_size);  }                 // Write-back the results to the register
  else                    { Writeback_EA(calculated_EA , ea_type , data);     }                 // Write-back the results to the EA
  mc68k_flags=(mc68k_flags&0xFFFD);                                                             // Always clear the V flag
  Calculate_Flag_Z(data , FALSE);                                                               // Calculate the Z Flag - Clear_only = FALSE
  Calculate_Flag_N(data);                                                                       // Calculate the N Flag
  return;
}


// ----------------------------------------------------------------------
void op_ROXL(uint8_t dst_is_register) {
  uint32_t data;
  uint16_t old_X_Flag;
  uint8_t shift_count;

  if (dst_is_register==1) { data_size  = DATA_SIZE_TYPE_A;                                      // Get the data size from the opcode bits[7:6]
                            immediate = (0x0E00&first_opcode)>>9;                               // Isolate the Immediate number from the opcode
                            reg_num   = (0x0007&first_opcode);                                  // Isolate the register number from the opcode
                            data = Fetch_Data_Register(reg_num,data_size);                                // Fetch the register we want to shift
                            if ((0x0020&first_opcode) == 0)   //   T=0 for Immediate field contains the shift_count   T=1 for shift_count contained in a register - Modulo-64
                              {  if(immediate==0)  shift_count=8; else shift_count = immediate;
                              }
                            else
                              { shift_count = (0x3F&Fetch_Data_Register(immediate,8));
                              }
                            }
  else                    { data_size  = 16;
                            calculated_EA = Calculate_EA(0x0FE0);                               // Calculate the EA, checking supported modes
                            data = (Fetch_EA(calculated_EA , ea_type));                         // Fetch the word data we want to shift from the EA
                            shift_count = 1;
                          }


  while (shift_count!=0)
  {
    old_X_Flag = mc68k_flag_X;
    if (data_size==8)  { if( (0x00000080&data)==0) mc68k_flags=(mc68k_flags&0xFFEE); else mc68k_flags=(mc68k_flags|0x0011); }   // Copy data[MSB] to the X and C flags
    if (data_size==16) { if( (0x00008000&data)==0) mc68k_flags=(mc68k_flags&0xFFEE); else mc68k_flags=(mc68k_flags|0x0011); }
    if (data_size==32) { if( (0x80000000&data)==0) mc68k_flags=(mc68k_flags&0xFFEE); else mc68k_flags=(mc68k_flags|0x0011); }

    data = data << 1;                                                                           // Shift data to the left by one bit
    data = (old_X_Flag | data);                                                                 // Or the original  X Flag into bit[0] of the new data
    shift_count--;                                                                              // Decrement the shift_counter
  }

  if (dst_is_register==1) { Store_Data_Register(reg_num , data , data_size);  }                 // Write-back the results to the register
  else                    { Writeback_EA(calculated_EA , ea_type , data);     }                 // Write-back the results to the EA
  mc68k_flags=(mc68k_flags&0xFFFD);                                                             // Always clear the V flag
  Calculate_Flag_Z(data , FALSE);                                                               // Calculate the Z Flag - Clear_only = FALSE
  Calculate_Flag_N(data);                                                                       // Calculate the N Flag
  return;
}



// op_type  1=ASR  2=LSR
// ----------------------------------------------------------------------
void op_xSR(uint8_t op_type , uint8_t dst_is_register) {
  uint32_t data;
  uint8_t shift_count;

  mc68k_flags=(mc68k_flags&0xFFFD);                                                             // Pre-clear the V flag

  if (dst_is_register==1) { data_size  = DATA_SIZE_TYPE_A;                                      // Get the data size from the opcode bits[7:6]
                            immediate = (0x0E00&first_opcode)>>9;                               // Isolate the Immediate number from the opcode
                            reg_num   = (0x0007&first_opcode);                                  // Isolate the register number from the opcode
                            data = Fetch_Data_Register(reg_num,data_size);                                // Fetch the register we want to shift
                            if ((0x0020&first_opcode) == 0)   //   T=0 for Immediate field contains the shift_count   T=1 for shift_count contained in a register - Modulo-64
                              {  if(immediate==0)  shift_count=8; else shift_count = immediate;
                              }
                            else
                              { shift_count = (0x3F&Fetch_Data_Register(immediate,8));
                              }
                            }
  else                    { data_size  = 16;
                            calculated_EA = Calculate_EA(0x0FE0);                               // Calculate the EA, checking supported modes
                            data = (Fetch_EA(calculated_EA , ea_type));                         // Fetch the word data we want to shift from the EA
                            shift_count = 1;
                          }


  while (shift_count!=0)
  {
    if( (0x1&data)==0) mc68k_flags=(mc68k_flags&0xFFEE); else mc68k_flags=(mc68k_flags|0x0011);                             // Copy  current data[0] to the X and C flags

    if (data_size==8)  { data = (0x000000FF&data)>>1;  if (op_type==1 && (0x00000040&data)!=0) data=(0x00000080|data);  }   // Shift right.
    if (data_size==16) { data = (0x0000FFFF&data)>>1;  if (op_type==1 && (0x00004000&data)!=0) data=(0x00008000|data);  }   //  For ASR: Copy old MSBit to new MSbit
    if (data_size==32) { data = data >> 1;             if (op_type==1 && (0x40000000&data)!=0) data=(0x80000000|data);  }

    shift_count--;                                                                                                          // Decrement the shift_counter
  }

  if (dst_is_register==1) { Store_Data_Register(reg_num , data , data_size);  }                 // Write-back the results to the register
  else                    { Writeback_EA(calculated_EA , ea_type , data);     }                 // Write-back the results to the EA
  Calculate_Flag_Z(data , FALSE);                                                               // Calculate the Z Flag - Clear_only = FALSE
  Calculate_Flag_N(data);                                                                       // Calculate the N Flag
  return;
}


// ----------------------------------------------------------------------
void op_ROR(uint8_t dst_is_register) {
  uint32_t data;
  uint8_t shift_count;

  if (dst_is_register==1) { data_size  = DATA_SIZE_TYPE_A;                                      // Get the data size from the opcode bits[7:6]
                            immediate = (0x0E00&first_opcode)>>9;                               // Isolate the Immediate number from the opcode
                            reg_num   = (0x0007&first_opcode);                                  // Isolate the register number from the opcode
                            data = Fetch_Data_Register(reg_num,data_size);                      // Fetch the register we want to shift
                            if ((0x0020&first_opcode) == 0)   //   T=0 for Immediate field contains the shift_count   T=1 for shift_count contained in a register - Modulo-64
                              {  if(immediate==0)  shift_count=8; else shift_count = immediate;
                              }
                            else
                              { shift_count = (0x3F&Fetch_Data_Register(immediate,8));
                              }
                            }
  else                    { data_size  = 16;
                            calculated_EA = Calculate_EA(0x0FE0);                               // Calculate the EA, checking supported modes
                            data = (Fetch_EA(calculated_EA , ea_type));                         // Fetch the word data we want to shift from the EA
                            shift_count = 1;
                          }


  while (shift_count!=0)
  {
    if( (0x1&data)==0) mc68k_flags=(mc68k_flags&0xFFFE); else mc68k_flags=(mc68k_flags|0x0001); // Copy data[0] to the C flag

    data = data >> 1;
    if (data_size==8)  { if (mc68k_flag_C==1) data=(0x00000080 | data); else  data=(0xFFFFFF7F & data); }    // Shift right. Copy data[0] to the MSbit
    if (data_size==16) { if (mc68k_flag_C==1) data=(0x00008000 | data); else  data=(0xFFFF7FFF & data); }
    if (data_size==32) { if (mc68k_flag_C==1) data=(0x80000000 | data); else  data=(0x7FFFFFFF & data); }

    shift_count--;                                                                              // Decrement the shift_counter
  }

  if (dst_is_register==1) { Store_Data_Register(reg_num , data , data_size);  }                 // Write-back the results to the register
  else                    { Writeback_EA(calculated_EA , ea_type , data);     }                 // Write-back the results to the EA
  mc68k_flags=(mc68k_flags&0xFFFD);                                                             // Always clear the V flag
  Calculate_Flag_Z(data , FALSE);                                                               // Calculate the Z Flag - Clear_only = FALSE
  Calculate_Flag_N(data);                                                                       // Calculate the N Flag
  return;
}


// ----------------------------------------------------------------------
void op_ROXR(uint8_t dst_is_register) {
  uint32_t data;
  uint16_t old_X_Flag;
  uint8_t shift_count;

  if (dst_is_register==1) { data_size  = DATA_SIZE_TYPE_A;                                      // Get the data size from the opcode bits[7:6]
                            immediate = (0x0E00&first_opcode)>>9;                               // Isolate the Immediate number from the opcode
                            reg_num   = (0x0007&first_opcode);                                  // Isolate the register number from the opcode
                            data = Fetch_Data_Register(reg_num,data_size);                                // Fetch the register we want to shift
                            if ((0x0020&first_opcode) == 0)   //   T=0 for Immediate field contains the shift_count   T=1 for shift_count contained in a register - Modulo-64
                              {  if(immediate==0)  shift_count=8; else shift_count = immediate;
                              }
                            else
                              { shift_count = (0x3F&Fetch_Data_Register(immediate,8));
                              }
                            }
  else                    { data_size  = 16;
                            calculated_EA = Calculate_EA(0x0FE0);                               // Calculate the EA, checking supported modes
                            data = (Fetch_EA(calculated_EA , ea_type));                         // Fetch the word data we want to shift from the EA
                            shift_count = 1;
                          }

  while (shift_count!=0)
  {
    old_X_Flag = mc68k_flag_X;
    if( (0x1&data)==0) mc68k_flags=(mc68k_flags&0xFFEE); else mc68k_flags=(mc68k_flags|0x0011); // Copy data[0] to the C and X flags

    data = data >> 1;
    if (data_size==8)  { if (old_X_Flag==1) data=(0x00000080 | data); else  data=(0xFFFFFF7F & data); } else   // Shift right. Copy X flag to the MSbit
    if (data_size==16) { if (old_X_Flag==1) data=(0x00008000 | data); else  data=(0xFFFF7FFF & data); } else
    if (data_size==32) { if (old_X_Flag==1) data=(0x80000000 | data); else  data=(0x7FFFFFFF & data); }

    shift_count--;                                                                              // Decrement the shift_counter
  }

  if (dst_is_register==1) { Store_Data_Register(reg_num , data , data_size);  }                 // Write-back the results to the register
  else                    { Writeback_EA(calculated_EA , ea_type , data);     }                 // Write-back the results to the EA
  mc68k_flags=(mc68k_flags&0xFFFD);                                                             // Always clear the V flag
  Calculate_Flag_Z(data , FALSE);                                                               // Calculate the Z Flag - Clear_only = FALSE
  Calculate_Flag_N(data);                                                                       // Calculate the N Flag
  return;
}


// bool_type:  1=OR  2=AND  3=EOR  4=CMP
// ----------------------------------------------------------------------
void op_BOOL(uint8_t bool_type) {
  uint32_t Reg_Data;

  data_size     = DATA_SIZE_TYPE_A;                                                             // Get the data size from the opcode bits[7:6]
  reg_num = (0x0E00&first_opcode)>>9;
  Reg_Data = Fetch_Data_Register(reg_num,data_size);
  mc68k_flags = ( mc68k_flags & 0xFFFC);                                                        // Pre-clear V, C Flags -- Will be calculated for CMP opcode


  if ( (0x0100&first_opcode)==0)                                                                // EA is the source
    {
      if (bool_type==1) { calculated_EA=Calculate_EA(0x2FFC); EA_Data=Fetch_EA(calculated_EA,ea_type); result=EA_Data | Reg_Data;  Store_Data_Register(reg_num,result,data_size); } else
      if (bool_type==2) { calculated_EA=Calculate_EA(0x2FFC); EA_Data=Fetch_EA(calculated_EA,ea_type); result=EA_Data & Reg_Data;  Store_Data_Register(reg_num,result,data_size); } else
      if (bool_type==4) { calculated_EA=Calculate_EA(0x3FFC); EA_Data=Fetch_EA(calculated_EA,ea_type); result=Reg_Data - EA_Data;  Calculate_Flags_C(Reg_Data , EA_Data , 1);  Calculate_Flags_V(Reg_Data , EA_Data , result , data_size , 1); }
    }

  else                                                                                          // Register is the source
    {
      if (bool_type==1) { calculated_EA=Calculate_EA(0x0FE0); EA_Data=Fetch_EA(calculated_EA,ea_type); result = EA_Data | Reg_Data;  Writeback_EA(calculated_EA , ea_type , result); }  else
      if (bool_type==2) { calculated_EA=Calculate_EA(0x0FE0); EA_Data=Fetch_EA(calculated_EA,ea_type); result = EA_Data & Reg_Data;  Writeback_EA(calculated_EA , ea_type , result); }  else
      if (bool_type==3) { calculated_EA=Calculate_EA(0x2FE0); EA_Data=Fetch_EA(calculated_EA,ea_type); result = EA_Data ^ Reg_Data;  Writeback_EA(calculated_EA , ea_type , result); }
    }

  Calculate_Flag_N(result);                                                                     // Calculate the N Flag
  Calculate_Flag_Z(result , FALSE);                                                             // Calculate the Z Flag - Clear_only = FALSE

  return;
}


// bool_type:  1=ORI  2=ANDI  3=EORI  4=CMPI
// ----------------------------------------------------------------------
void op_BOOL_I(uint8_t bool_type) {
  data_size     = DATA_SIZE_TYPE_A;                                                             // Get the data size from the opcode bits[7:6]
  immediate     = Fetch_Immediate(data_size);                                                   // Fetch the immediate operand of the correct data size

  calculated_EA = Calculate_EA(0x2FE3);                                                         // Calculate the EA, checking supported modes
  mc68k_flags = ( mc68k_flags & 0xFFFC);                                                        // Pre-clear V, C Flags

  EA_Data = Fetch_EA(calculated_EA , ea_type);                                                  // Fetch the EA operand

  // Perform the operation then write-back the result to the EA
  if (bool_type==1) { result = EA_Data | immediate;  Writeback_EA(calculated_EA , ea_type , result); }  else
  if (bool_type==2) { result = EA_Data & immediate;  Writeback_EA(calculated_EA , ea_type , result); }  else
  if (bool_type==3) { result = EA_Data ^ immediate;  Writeback_EA(calculated_EA , ea_type , result); }  else
  if (bool_type==4) { result = EA_Data - immediate;  Calculate_Flags_C(EA_Data , immediate , 1); Calculate_Flags_V(EA_Data , immediate , result , data_size , 1);     }  // Calculate the V and C Flags for CMPI
  Calculate_Flag_N(result);                                                                     // Calculate the N Flag
  Calculate_Flag_Z(result , FALSE);                                                             // Calculate the Z Flag - Clear_only = FALSE

  return;
}


// ----------------------------------------------------------------------
void op_CMPA() {
  uint32_t Reg_Data;

  data_size     = DATA_SIZE_TYPE_D;                                                             // Get the data size from the opcode bits8]
  calculated_EA=Calculate_EA(0x3FFC);                                                           // Calculate the EA, checking supported modes
  EA_Data=Fetch_EA(calculated_EA,ea_type);                                                      // Fetch the EA data

  reg_num = (0x0E00&first_opcode)>>9;                                                           // Isolate the register number from the opcode
  Reg_Data = Fetch_Address_Register(reg_num,32);                                                // Fetch the Address Register data

  if (data_size==16) { EA_Data=Sign_Extend(EA_Data,16);  }                                      // Sign extend source operand for word

  result = Reg_Data - EA_Data ;

  data_size     = 32;                                                                           // Force the size to 32-bit for flag calculations
  mc68k_flags = ( mc68k_flags & 0xFFFC);                                                        // Pre-clear V, C Flags
  Calculate_Flags_C(Reg_Data , EA_Data , 1);
  Calculate_Flags_V(Reg_Data , EA_Data , result , data_size , 1);                                                       // Calculate the V and C Flags
  Calculate_Flag_N(result);                                                                     // Calculate the N Flag
  Calculate_Flag_Z(result , FALSE);                                                             // Calculate the Z Flag - Clear_only = FALSE
  return;
}


// ----------------------------------------------------------------------
void op_CMPM() {
  uint8_t regX;
  uint8_t regY;
  uint32_t AddrX;
  uint32_t AddrY;
  uint32_t DataX;
  uint32_t DataY;

  data_size     = DATA_SIZE_TYPE_A;                                                             // Get the data size from the opcode bits[7:6]

  regX = (0x0E00&first_opcode)>>9;                                                              // Isolate the register X number from the opcode
  regY = (0x0007&first_opcode);                                                                 // Isolate the register Y number from the opcode


  AddrX = Fetch_Address_Register(regX,32);                                                      // Fetch Address Register X data
  AddrY = Fetch_Address_Register(regY,32);                                                      // Fetch Address Register Y data

  if (data_size==8)  {  Store_Address_Register(regX,AddrX+0x01,32);   Store_Address_Register(regY,AddrY+0x01,32);  }      // Post-increment both addresses  (An)+
  if (data_size==16) {  Store_Address_Register(regX,AddrX+0x02,32);   Store_Address_Register(regY,AddrY+0x02,32);  }
  if (data_size==32) {  Store_Address_Register(regX,AddrX+0x04,32);   Store_Address_Register(regY,AddrY+0x04,32);  }


  if (data_size==32) { DataX=BIU_Read_32(AddrX);        DataY=BIU_Read_32(AddrY);         }     // Fetch data from both addresses
  else               { DataX=BIU_Read(AddrX,data_size); DataY=BIU_Read(AddrY,data_size);  }

  result = DataX - DataY ;

  Calculate_Flag_N(result);                                                                     // Calculate the N Flag
  Calculate_Flag_Z(result , FALSE);                                                             // Calculate the Z Flag - Clear_only = FALSE
  mc68k_flags = ( mc68k_flags & 0xFFFC);                                                        // Pre-clear V, C Flags
  Calculate_Flags_C(DataX , DataY , 1);
  Calculate_Flags_V(DataX , DataY , result , data_size , 1);                                    // Calculate the V and C Flags
  return;
}


// math_type  0=SUB   1=ADD
// ----------------------------------------------------------------------
void op_ADDSUB(uint8_t math_type) {
  int32_t  Reg_Data;

  if ( (0x0100&first_opcode)==0)                                                                // ** EA is the source **
  {
    data_size     = DATA_SIZE_TYPE_A;                                                           // Get the data size from the opcode bits[7:6]
    calculated_EA = Calculate_EA(0x3FFF);                                                       // Calculate the EA, checking supported modes
    EA_Data       = Fetch_EA(calculated_EA , ea_type);                                          // Fetch the EA data
    mc68k_flags = ( mc68k_flags & 0xFFFC);                                                      // Pre-clear V, C Flags

    reg_num = (0x0E00&first_opcode)>>9;                                                         // Isolate the register number from the opcode
    Reg_Data = Fetch_Data_Register(reg_num,data_size);                                          // Fetch the Register data

    if (math_type==0) { result = Reg_Data - EA_Data;  Calculate_Flags_C(Reg_Data , EA_Data , 1);  Calculate_Flags_V(Reg_Data , EA_Data , result , data_size , 1);  } // Calculate the results and the C,V Flags
    if (math_type==1) { result = Reg_Data + EA_Data;  Calculate_Flags_C(Reg_Data , EA_Data , 0);  Calculate_Flags_V(Reg_Data , EA_Data , result , data_size , 0);  }

    Store_Data_Register(reg_num,result,data_size);                                              // Write-back the results to the Data Register
  }

  else                                                                                          // ** Register is the source **
  {
    data_size     = DATA_SIZE_TYPE_A;                                                           // Get the data size from the opcode bits[7:6]
    calculated_EA = Calculate_EA(0x0FE0);                                                       // Calculate the EA, checking supported modes
    EA_Data       = Fetch_EA(calculated_EA , ea_type);                                          // Fetch the EA data

    reg_num = (0x0E00&first_opcode)>>9;                                                         // Isolate the register number from the opcode
    Reg_Data = Fetch_Data_Register(reg_num,data_size);                                          // Fetch the Register data

    if (math_type==0) { result = EA_Data - Reg_Data;   Calculate_Flags_C(EA_Data , Reg_Data , 1); Calculate_Flags_V(EA_Data , Reg_Data , result , data_size , 1);  } // Calculate the results and the C,V Flags
    if (math_type==1) { result = EA_Data + Reg_Data;   Calculate_Flags_C(EA_Data , Reg_Data , 0); Calculate_Flags_V(EA_Data , Reg_Data , result , data_size , 0);  }

    Writeback_EA(calculated_EA , ea_type , result);                                             // Write-back the results to the EA
  }

  Calculate_Flag_N(result);                                                                     // Calculate the N Flag
  Calculate_Flag_Z(result , FALSE);                                                             // Calculate the Z Flag - Clear_only = FALSE
  if (mc68k_flag_C != 0)  { mc68k_flags=(mc68k_flags | 0x0010);  }                              // Copy C flag to X flag
  else                    { mc68k_flags=(mc68k_flags & 0xFFEF);  }


  return;
}


// math_type  0=SUB   1=ADD
// ----------------------------------------------------------------------
void op_ADDSUBA(uint8_t math_type) {
  uint32_t Reg_Data;
  uint32_t result=0;


  data_size     = DATA_SIZE_TYPE_D;                                                             // Get the data size from the opcode bit[8]
  calculated_EA = Calculate_EA(0x3FFC);                                                         // Calculate the EA, checking supported modes
  EA_Data       = Fetch_EA(calculated_EA , ea_type);                                            // Fetch the EA data

  reg_num = (0x0E00&first_opcode)>>9;                                                           // Isolate the register number from the opcode
  Reg_Data = Fetch_Address_Register(reg_num,data_size);                                         // Fetch the Register data

  if (math_type==0) result = Reg_Data - EA_Data;                                                // Calculate the results
  if (math_type==1) result = Reg_Data + EA_Data;

  Store_Address_Register(reg_num,result,32);                                                    // Write-back the results to the 32-bits of the Address Register
  return;
}


// math_type  0=SUB   1=ADD
// ----------------------------------------------------------------------
void op_ADDSUBI(uint8_t math_type) {
  uint32_t result=0;

  data_size     = DATA_SIZE_TYPE_A;                                                             // Get the data size from the opcode bits[7:6]
  immediate     = Fetch_Immediate(data_size);                                                   // Fetch the immediate operand of the correct data size

  calculated_EA = Calculate_EA(0x2FE0);                                                         // Calculate the EA, checking supported modes
  EA_Data       = Fetch_EA(calculated_EA , ea_type);                                            // Fetch the EA data
  mc68k_flags = ( mc68k_flags & 0xFFFC);                                                        // Pre-clear V, C Flags


  if (math_type==0) { result = EA_Data - immediate; Calculate_Flags_C(EA_Data , immediate , 1);  Calculate_Flags_V(EA_Data , immediate , result , data_size , 1);  }  // Calculate the results and the C,V Flags
  if (math_type==1) { result = EA_Data + immediate; Calculate_Flags_C(EA_Data , immediate , 0);  Calculate_Flags_V(EA_Data , immediate , result , data_size , 0);  }

  Writeback_EA(calculated_EA , ea_type , result);                                               // Write-back the results to the EA

  Calculate_Flag_N(result);                                                                     // Calculate the N Flag
  Calculate_Flag_Z(result , FALSE);                                                             // Calculate the Z Flag - Clear_only = FALSE
  if (mc68k_flag_C != 0)  { mc68k_flags=(mc68k_flags | 0x0010);  }                              // Copy C flag to X flag
  else                    { mc68k_flags=(mc68k_flags & 0xFFEF);  }

  return;
}


// math_type  0=SUB   1=ADD
// ----------------------------------------------------------------------
void op_ADDSUBQ(uint8_t math_type) {
  uint32_t result=0;
  uint32_t opcode_data;

  data_size     = DATA_SIZE_TYPE_A;                                                             // Get the data size from the opcode bits[7:6]
  calculated_EA = Calculate_EA(0x3FE0);                                                         // Calculate the EA, checking supported modes
  EA_Data       = Fetch_EA(calculated_EA , ea_type);                                            // Fetch the EA data
  mc68k_flags = ( mc68k_flags & 0xFFFC);                                                        // Pre-clear V, C Flags

  opcode_data = (0x0E00&first_opcode)>>9;                                                       // Isolate the immediate "Q" data from the opcode
  if (opcode_data==0) opcode_data=8;

  if (math_type==0) { result = EA_Data - opcode_data;  }                      // Calculate the results
  if (math_type==1) { result = EA_Data + opcode_data;  }

  Writeback_EA(calculated_EA , ea_type , result);                                               // Write-back the results to the EA

  if (ea_type != ADDRESS_REG)                                                                   // Don't set flags if destination is an Address register
  {
    if (math_type==0) { Calculate_Flags_C(EA_Data , opcode_data , 1); Calculate_Flags_V(EA_Data , opcode_data , result , data_size , 1 );    }
    if (math_type==1) { Calculate_Flags_C(EA_Data , opcode_data , 0); Calculate_Flags_V(EA_Data , opcode_data , result , data_size , 0 );    }
    Calculate_Flag_N(result);                                                                   // Calculate the N Flag
    Calculate_Flag_Z(result , FALSE);                                                           // Calculate the Z Flag - Clear_only = FALSE
    if (mc68k_flag_C != 0)  { mc68k_flags=(mc68k_flags | 0x0010);  }                            // Copy C flag to X flag
    else                    { mc68k_flags=(mc68k_flags & 0xFFEF);  }
  }

  return;
}


// math_type  0=SUBX   1=ADDX
// ----------------------------------------------------------------------
void op_ADDSUBX(uint8_t math_type) {
  uint8_t regX;
  uint8_t regY;
  uint32_t AddrX;
  uint32_t AddrY;
  uint32_t DataX;
  uint32_t DataY;
  uint64_t result=0;

  data_size     = DATA_SIZE_TYPE_A;                                                             // Get the data size from the opcode bits[7:6]

  regX = (0x0E00&first_opcode)>>9;                                                              // Isolate the register X number from the opcode
  regY = (0x0007&first_opcode);                                                                 // Isolate the register Y number from the opcode
  mc68k_flags = ( mc68k_flags & 0xFFFC);                                                        // Pre-clear V, C Flags


  if ( (0x0008&first_opcode)==0)                                                                // ** Register to Register **
  {
    DataX = Fetch_Data_Register(regX,data_size);                                                // Fetch Data Register X data
    DataY = Fetch_Data_Register(regY,data_size);                                                // Fetch Data Register Y data


    if (math_type==0) { Calculate_Flags_C(DataX , DataY , 1);
                        result = DataX - DataY;                                                 // Subtract the two operands first and calculate the V,C Flags
                        Calculate_Flags_C(result , mc68k_flag_X , 1);
                        result = result - mc68k_flag_X;
                        Calculate_Flags_V(DataX , DataY , result , data_size , 1);
                      }

    if (math_type==1) { Calculate_Flags_C(DataX , DataY , 0);
                        result = DataX + DataY;                                                 // Add the two operands first and calculate the V,C Flags
                        Calculate_Flags_C(result , mc68k_flag_X , 0);
                        result = result + mc68k_flag_X;
                        Calculate_Flags_V(DataX , DataY , result , data_size , 0);
                      }

    Store_Data_Register(regX,result,data_size);                                                 // Write-back the results to the Data Register
  }

  else                                                                                          // ** Memory to Memory **
  {

    AddrX = Fetch_Address_Register(regX,32);                                                    // Fetch Address Register X data
    AddrY = Fetch_Address_Register(regY,32);                                                    // Fetch Address Register Y data

    if (data_size==8)   {  AddrX = AddrX - 0x1;  AddrY = AddrY - 0x1;  }                        // Pre-decrement both addresses  -(An)
    if (data_size==16)  {  AddrX = AddrX - 0x2;  AddrY = AddrY - 0x2;  }
    if (data_size==32)  {  AddrX = AddrX - 0x4;  AddrY = AddrY - 0x4;  }

    Store_Address_Register(regX,AddrX,32);                                                      // Write-back the -(An) address both Address Registers
    Store_Address_Register(regY,AddrY,32);


    if (data_size==32) { DataX=BIU_Read_32(AddrX);        DataY=BIU_Read_32(AddrY);         }   // Fetch data from both addresses
    else               { DataX=BIU_Read(AddrX,data_size); DataY=BIU_Read(AddrY,data_size);  }

    if (math_type==0) { Calculate_Flags_C(DataX , DataY , 1);
                        result = DataX - DataY;                                                 // Subtract the two operands first and calculate the V,C Flags
                        Calculate_Flags_C(result , mc68k_flag_X , 1);
                        result = result - mc68k_flag_X;
                        Calculate_Flags_V(DataX , DataY , result , data_size , 1);
                      }

    if (math_type==1) { Calculate_Flags_C(DataX , DataY , 0);
                        result = DataX + DataY;                                                 // Add the two operands first and calculate the V,C Flags
                        Calculate_Flags_C(result , mc68k_flag_X , 0);
                        result = result + mc68k_flag_X;
                        Calculate_Flags_V(DataX , DataY , result , data_size , 0);
                      }

    if (data_size==8)   { BIU_Write(AddrX ,    result  , 8);  }
    if (data_size==16)  { BIU_Write(AddrX ,    result , 16);  }
    if (data_size==32)  { BIU_Write_32(AddrX , result );      }


  }
  Calculate_Flag_N(result);                                                                     // Calculate the N Flag
  Calculate_Flag_Z(result , TRUE);                                                              // Calculate the Z Flag - Clear_only = TRUE
  if (mc68k_flag_C != 0)  { mc68k_flags=(mc68k_flags | 0x0010);  }                              // Copy C flag to X flag
  else                    { mc68k_flags=(mc68k_flags & 0xFFEF);  }

  return;
}

#include <math.h>

// ----------------------------------------------------------------------
void op_DIVS() {
  int32_t  dividend;
  int16_t  divisor;
  int32_t  quotient;
  int16_t  remainder;
  uint32_t result;

  data_size     = 16;                                              // Get the data size from the opcode bits[7:6]
  calculated_EA = Calculate_EA(0x2FFC);                            // Calculate the EA, checking supported modes
  divisor       = (int16_t) (Fetch_EA(calculated_EA , ea_type) );  // Fetch the EA data and convert to a signed 16-bit number

  reg_num       = (0x0E00&first_opcode)>>9;                        // Isolate the register number from the opcode
  dividend      = (int32_t) Fetch_Data_Register(reg_num,32);       // Fetch the Register data and convert to a signed 32-bit number

  if (divisor==0)  { Exception_Handler(5);  return;  }             // Check for division by zero. Trap if true
  else {
    quotient  = dividend / divisor;                                // Calculate the results
    remainder = dividend % divisor;
    result = ( (remainder<<16) | (0x0000FFFF&quotient) );

    if ( (quotient > 32767) || (quotient < -32768)  )  {
      mc68k_flags = (mc68k_flags | 0x0002);                        // If overflow, set the V Flag and don't update the register
      mc68k_flags = (mc68k_flags & 0xFFFE);                        // Always clear the C Flag

    } else {
      Store_Data_Register(reg_num,result,32);                      // Else, write-back the 32-bit results to the Data Register
      mc68k_flags = (mc68k_flags & 0xFFFC);                        // Clear the V and C flag
      Calculate_Flag_N((0x0000FFFF&quotient));
      Calculate_Flag_Z((0x0000FFFF&quotient) , FALSE);
    }
  }
  return;
}


// ----------------------------------------------------------------------
void op_DIVU() {
  uint32_t dividend;
  uint16_t divisor;
  uint32_t quotient;
  uint16_t remainder;
  uint32_t result;

  data_size     = 16;                                                                           // Get the data size from the opcode bits[7:6]
  calculated_EA = Calculate_EA(0x2FFC);                                                         // Calculate the EA, checking supported modes
  divisor       = Fetch_EA(calculated_EA , ea_type);                                            // Fetch the EA data

  reg_num = (0x0E00&first_opcode)>>9;                                                           // Isolate the register number from the opcode
  dividend = Fetch_Data_Register(reg_num,32);                                                   // Fetch the Register data

  if (divisor==0)  { Exception_Handler(5);  return;  }                                          // Check for division by zero. Trap if true
  else
  {
    quotient  = dividend / divisor;                                                             // Calculate the results
    remainder = dividend % divisor;
    result = ( (remainder<<16) | (0x0000FFFF&quotient) );

    if ( (quotient>0xFFFF) )  {  mc68k_flags = (mc68k_flags | 0x0002);              // If overflow, set the V Flag and don't update the register
                                 mc68k_flags=(mc68k_flags&0xFFFE);                // Always clear the C Flag
                }

    else                      {  Store_Data_Register(reg_num,result,32);              // Else, write-back the 32-bit results to the Data Register
                 mc68k_flags=(mc68k_flags&0xFFFC);                // Clear the V and C flag
                   Calculate_Flag_N((0x0000FFFF&quotient));
                   Calculate_Flag_Z((0x0000FFFF&quotient) , FALSE);
                }

  }
  return;
}


// ----------------------------------------------------------------------
void op_MULS() {
  int16_t reg_data_s;
  int16_t ea_data_s;
  int32_t result;
  uint32_t wb_result;

  data_size     = 16;                                                                           // Force size to 16
  calculated_EA = Calculate_EA(0x2FFC);                                                         // Calculate the EA, checking supported modes
  ea_data_s     = (int16_t) Fetch_EA(calculated_EA , ea_type);                         // Fetch the EA data and convert to a signed 16-bit number

  reg_num    = (0x0E00&first_opcode)>>9;                                                        // Isolate the register number from the opcode
  reg_data_s = (int16_t) Fetch_Data_Register(reg_num,16);                              // Fetch the Register data and convert to a signed 16-bit number

  result  = (int32_t)ea_data_s * (int32_t)reg_data_s;                                   // Calculate the results

  wb_result = (uint32_t ) result;

  Store_Data_Register(reg_num,result,32);                                                       // Write-back the full 32-bits of the results to the Data Register

  data_size     = 32;                                                                           // Force size to 32-bits so flags are calculated correctly
  Calculate_Flag_N(wb_result);                                                                  // Calculate the N Flag
  Calculate_Flag_Z(wb_result , FALSE);                                                          // Calculate the Z Flag - Clear_only = FALSE
  mc68k_flags = ( mc68k_flags & 0xFFFC);                                                        // Always clear V, C Flags
  return;
}


// ----------------------------------------------------------------------
void op_MULU() {
  uint32_t reg_data;
  uint32_t result;

  data_size     = 16;                                                                           // Force size to 16
  calculated_EA = Calculate_EA(0x2FFC);                                                         // Calculate the EA, checking supported modes
  EA_Data       = Fetch_EA(calculated_EA , ea_type);                                            // Fetch the EA data

  reg_num = (0x0E00&first_opcode)>>9;                                                           // Isolate the register number from the opcode
  reg_data = Fetch_Data_Register(reg_num,16);                                                   // Fetch the Register data

  result  = (uint32_t)EA_Data * (uint32_t)reg_data;                                   // Calculate the results

  Store_Data_Register(reg_num,result,32);                                                       // Write-back the full 32-bits of the results to the Data Register

  data_size     = 32;                                                                           // Force size to 32-bits so flags are calculated correctly
  Calculate_Flag_N(result);                                                                     // Calculate the N Flag
  Calculate_Flag_Z(result , FALSE);                                                             // Calculate the Z Flag - Clear_only = FALSE
  mc68k_flags = ( mc68k_flags & 0xFFFC);                                                        // Always clear V, C Flags
  return;
}


// bcd_type  0=ABCD   1=SBCD
// ----------------------------------------------------------------------
void op_xBCD(uint8_t bcd_type) {
  uint8_t regX;
  uint8_t regY;
  uint32_t AddrX;
  uint32_t AddrY;
  int32_t result;
  int32_t DataX;
  int32_t DataY;
  int16_t result_lower;
  int16_t result_upper;
  int8_t carry=0;

  data_size     = 8;                                                                            // Byte only

  regX = (0x0E00&first_opcode)>>9;                                                              // Isolate the register X number from the opcode
  regY = (0x0007&first_opcode);                                                                 // Isolate the register Y number from the opcode

  if ( (0x0008&first_opcode)==0)                                                                // ** Opcode[3]=0 for Register to Register **
  {
    DataX = Fetch_Data_Register(regX,data_size);                                                // Fetch Data Register X data
    DataY = Fetch_Data_Register(regY,data_size);                                                // Fetch Data Register Y data


    if (bcd_type==0)                                                                            // ## ABCD##
    {
      carry = 0;
    result_lower = (0x0F&DataX) + (0x0F&DataY) + mc68k_flag_X;                                // Lower BCD digit
      if (result_lower>9) { result_lower = result_lower - 10;    carry = 1; }

      result_upper = ((0xF0&DataX)>>4) + ((0xF0&DataY)>>4) + carry;                             // Upper BCD digit
      carry = 0;
      if (result_upper>9) { result_upper = result_upper - 10;  carry = 1; }

      result = (result_upper<<4) + result_lower;
    }

    else                                                                                        // ## SBCD ##
    {
      carry = 0;
    result_lower = (0x0F&DataX) - (0x0F&DataY) - mc68k_flag_X;                                // Lower BCD digit
      if (result_lower<0) { result_lower = result_lower + 10;    carry = 1; }

      result_upper = ((0xF0&DataX)>>4) - ((0xF0&DataY)>>4) - carry;                             // Upper BCD digit
      carry = 0;
      if (result_upper<0) { result_upper = result_upper + 10;  carry = 1; }

      result = (result_upper<<4) + result_lower;
    }

    Store_Data_Register(regX,result,data_size);                                                 // Write-back the results to the Data Register
  }

  else                                                                                          // ** Memory to Memory **
  {
    AddrX = Fetch_Address_Register(regX,32);                                                    // Fetch Address Register X data
    AddrY = Fetch_Address_Register(regY,32);                                                    // Fetch Address Register Y data

    AddrX = AddrX - 0x1;                                                                        // Pre-decrement both addresses  -(An)
    AddrY = AddrY - 0x1;

    Store_Address_Register(regX,AddrX,32);                                                      // Write-back the -(An) address both Address Registers
    Store_Address_Register(regY,AddrY,32);

    DataX = BIU_Read(AddrX , data_size);                                                        // Fetch data from both addresses
    DataY = BIU_Read(AddrY , data_size);


    if (bcd_type==0)                                                                            // ## ABCD##
    {
      carry = 0;
      result_lower = (0x0F&DataX) + (0x0F&DataY) + mc68k_flag_X;                                // Lower BCD digit
      if (result_lower>9) { result_lower = result_lower - 10;  carry = 1; }

      result_upper = ((0xF0&DataX)>>4) + ((0xF0&DataY)>>4) + carry;                             // Upper BCD digit
      carry = 0;
      if (result_upper>9) { result_upper = result_upper - 10;  carry = 1; }

      result = (result_upper<<4) + result_lower;
    }

    else                                                                                        // ## SBCD ##
    {
      carry = 0;
      result_lower = (0x0F&DataX) - (0x0F&DataY) - mc68k_flag_X;                                // Lower BCD digit
      if (result_lower<0) { result_lower = result_lower + 10;  carry = 1; }

      result_upper = ((0xF0&DataX)>>4) - ((0xF0&DataY)>>4) - carry;                             // Upper BCD digit
      carry = 0;
      if (result_upper<0) { result_upper = result_upper + 10;  carry = 1; }

      result = (result_upper<<4) + result_lower;
    }

    Writeback_EA(AddrX , MEMORY , result);                                                      // Write-back the results to Memory
  }

  if (carry==1)  mc68k_flags=(mc68k_flags|0x0011); else mc68k_flags=(mc68k_flags&0xFFEE);       // Set X and C Flags to the Carry bit
  Calculate_Flag_Z(result , TRUE);                                                              // Calculate the Z Flag - Clear_only = TRUE

  return;
}


// ----------------------------------------------------------------------
void op_NBCD() {
  int16_t result_lower;
  int16_t result_upper;
  int32_t result;
  int8_t carry=0;

  data_size         = 8;
  calculated_EA     = Calculate_EA(0x2FE0);                                                     // Calculate the Source EA, checking supported modes
  EA_Data           = Fetch_EA(calculated_EA , ea_type);                                        // Fetch the EA data


  result_lower = (0x0) - (0x0F&EA_Data) - mc68k_flag_X;                                         // Lower BCD digit
  if (result_lower<0) { result_lower = result_lower + 10;  carry = 1; }

  result_upper = (0x0) - ((0xF0&EA_Data)>>4) - carry;                                           // Upper BCD digit
  carry = 0;
  if (result_upper<0) { result_upper = result_upper + 10;  carry = 1; }

  result = (result_upper<<4) + result_lower;

  Writeback_EA(calculated_EA , ea_type ,result);                                                // Write-back data to the EA

  if (carry==1)  mc68k_flags=(mc68k_flags|0x0011); else mc68k_flags=(mc68k_flags&0xFFEE);       // Set X and C Flags to the Carry bit
  Calculate_Flag_Z(result , TRUE);                                                              // Calculate the Z Flag - Clear_only = TRUE

  return;
}


// ----------------------------------------------------------------------
void op_MOVEP() {
  uint8_t regX;
  uint32_t DataX;
  uint32_t result=0;

  if ( (0x0040&first_opcode) == 0)  data_size = 16;  else  data_size = 32;                      // Isolate opcode[6] to set the size

  first_opcode   = first_opcode | 0x20;                                                         // Force the EA field to d16(An) mode
  calculated_EA  = Calculate_EA(0x0100);                                                        // Calculate the EA with the forced EA mode
  regX = (0x0E00&first_opcode)>>9;                                                              // Isolate the register X number from the opcode


  if ( (0x0080&first_opcode) == 0)                                                              // ## Memory to Register ##
  {
                         result = (0xFF&BIU_Read(calculated_EA+0x0 , SIZE_BYTE));            result = result<<8;    // Byte #0
                         result = result | (0xFF&BIU_Read(calculated_EA+0x2 , SIZE_BYTE));                          // Byte #1
                         Store_Data_Register(regX,result,16);                                                       // Write-back the results to 16 bits of the Data Register
                         result = result<<8;                                                                        // Keep shifting for possible word operation
    if (data_size==32) { result = result | (0xFF&BIU_Read(calculated_EA+0x4 , SIZE_BYTE));   result = result<<8;    // Byte #2 for Long data
                         result = result | (0xFF&BIU_Read(calculated_EA+0x6 , SIZE_BYTE));                          // Byte #3 for Long data
                         Store_Data_Register(regX,result,32);                                                       // Write-back the results to the full Data Register
                       }
  }

  else                                                                                          // ## Register to Memory ##
  {
    DataX = Fetch_Data_Register(regX,32);                                                          // Fetch Data Register X data

    if (data_size==16) {  BIU_Write(calculated_EA+0x0 , ( (0x0000FF00&DataX)>>8) , SIZE_BYTE);  // Byte #0
                          BIU_Write(calculated_EA+0x2 , ( (0x000000FF&DataX)>>0) , SIZE_BYTE);  // Byte #1
                       }

    else               {  BIU_Write(calculated_EA+0x0 , ( (0xFF000000&DataX)>>24) , SIZE_BYTE); // Byte #0
                          BIU_Write(calculated_EA+0x2 , ( (0x00FF0000&DataX)>>16) , SIZE_BYTE); // Byte #1
                          BIU_Write(calculated_EA+0x4 , ( (0x0000FF00&DataX)>>8 ) , SIZE_BYTE); // Byte #2
                          BIU_Write(calculated_EA+0x6 , ( (0x000000FF&DataX)>>0 ) , SIZE_BYTE); // Byte #3
                       }
  }
  return;
}

// Main MCL68 loop
// ----------------------------------------------------------------------
int16_t run_mcl68k(void* start) {
  memory_base = (uint32_t)start;

  Reset_routine();

  while (1) {
     char  temp;

      // Handle a RESET from the BIU
      // Hold here while BIU being held in RESET
      while (BIU_RESET_STATUS()!=0)  { reset_status_d = 1;  }
      // Run the reset sequence on falling edge of RESET
      if (reset_status_d==1)  {  reset_status_d = 0; Reset_routine(); }

      // Check for an active Interrupt from the BIU
      if (BIU_IRQ_LEVEL() != 0)  {  Exception_Handler(99);  }

      first_opcode = BIU_PFQ_Fetch();

      switch (first_opcode&0xF000) {
        case (0x0000):
          if ((first_opcode&0x0FFF)==0x003C) { op_BOOL_I_TO_CCR(1);       break; }// Checked
          if ((first_opcode&0x0FFF)==0x007C) { op_BOOL_I_TO_SR(1);        break; }// Checked
          if ((first_opcode&0x0FFF)==0x0A3C) { op_BOOL_I_TO_CCR(3);       break; }// Checked
          if ((first_opcode&0x0FFF)==0x0A7C) { op_BOOL_I_TO_SR(3);        break; }// Checked
          if ((first_opcode&0x0FFF)==0x023C) { op_BOOL_I_TO_CCR(2);       break; }// Checked
          if ((first_opcode&0x0FFF)==0x027C) { op_BOOL_I_TO_SR(2);        break; }// Checked
          if ((first_opcode&0x0FC0)==0x0800) { op_BMOD(0x31);             break; }// Checked
          if ((first_opcode&0x0FC0)==0x0840) { op_BMOD(0x21);             break; }// Checked
          if ((first_opcode&0x0FC0)==0x0880) { op_BMOD(0x11);             break; }// Checked
          if ((first_opcode&0x0FC0)==0x08C0) { op_BMOD(0x01);             break; }// Checked
          if ((first_opcode&0x0138)==0x0108) { op_MOVEP();                break; }// Checked
          if ((first_opcode&0x0F00)==0x0A00) { op_BOOL_I(3);              break; }// Checked
          if ((first_opcode&0x0F00)==0x0000) { op_BOOL_I(1);              break; }// Checked
          if ((first_opcode&0x0F00)==0x0200) { op_BOOL_I(2);              break; }// Checked
          if ((first_opcode&0x0F00)==0x0400) { op_ADDSUBI(0);             break; }// Checked
          if ((first_opcode&0x0F00)==0x0600) { op_ADDSUBI(1);             break; }// Checked
          if ((first_opcode&0x0F00)==0x0C00) { op_BOOL_I(4);              break; }// Checked
          if ((first_opcode&0x01C0)==0x0100) { op_BMOD(0x30);             break; }// Checked
          if ((first_opcode&0x01C0)==0x0140) { op_BMOD(0x20);             break; }// Checked
          if ((first_opcode&0x01C0)==0x0180) { op_BMOD(0x10);             break; }// Checked
          if ((first_opcode&0x01C0)==0x01C0) { op_BMOD(0x00);             break; }// Checked
           Exception_Handler(4);               
           break;                               
                                               
         case (0x1000):  case (0x2000):  case (0x3000):
                                             {   op_MOVE();               break; }// Checked
                                               
                                               
         case (0x4000):                         
           if ((first_opcode&0x0FC0)==0x00C0) { op_MOVE_FROM_SR();          break; }// Checked
           if ((first_opcode&0x0FC0)==0x04C0) { op_MOVE_TO_CCR();           break; }// Checked
           if ((first_opcode&0x0FC0)==0x06C0) { op_MOVE_TO_SR();            break; }// Checked
           if ((first_opcode&0x0F00)==0x0000) { op_NEGS(3);                 break; }// Checked
           if ((first_opcode&0x0F00)==0x0200) { op_NEGS(4);                 break; }// Checked
           if ((first_opcode&0x0F00)==0x0400) { op_NEGS(2);                 break; }// Checked
           if ((first_opcode&0x0F00)==0x0600) { op_NEGS(1);                 break; }// Checked
           if ((first_opcode&0x0FB8)==0x0880) { op_EXT();                   break; }// Checked
           if ((first_opcode&0x0FC0)==0x0800) { op_NBCD();                  break; }// Checked
           if ((first_opcode&0x0FF8)==0x0840) { op_SWAP();                  break; }// Checked
           if ((first_opcode&0x0FC0)==0x0840) { op_PEA();                   break; }// Checked
           if ((first_opcode&0x0FFF)==0x0AFC) { Exception_Handler(4);       break; }// Checked
           if ((first_opcode&0x0FC0)==0x0AC0) { op_TAS();                   break; }// Checked
           if ((first_opcode&0x0F00)==0x0A00) { op_TST();                   break; }// Checked
           if ((first_opcode&0x0FF0)==0x0E40) { op_TRAP();                  break; }// Checked
           if ((first_opcode&0x0FF8)==0x0E50) { op_LINK();                  break; }// Checked
           if ((first_opcode&0x0FF8)==0x0E58) { op_UNLK();                  break; }// Checked
           if ((first_opcode&0x0FF0)==0x0E60) { op_MOVE_USP();              break; }// Checked
           if ((first_opcode&0x0FFF)==0x0E71) { /* Do Nothing */            break; }// Checked
           if ((first_opcode&0x0FFF)==0x0E70) { op_RESET();                 break; }// Checked
           if ((first_opcode&0x0FFF)==0x0E72) { op_STOP();                  break; }// Checked
           if ((first_opcode&0x0FFF)==0x0E73) { op_RTE();                   break; }// Checked
           if ((first_opcode&0x0FFF)==0x0E75) { op_RTS();                   break; }// Checked
           if ((first_opcode&0x0FFF)==0x0E76) { op_TRAPV();                 break; }// Checked
           if ((first_opcode&0x0FFF)==0x0E77) { op_RTR();                   break; }// Checked
           if ((first_opcode&0x0FC0)==0x0E80) { op_JSR();                   break; }// Checked
           if ((first_opcode&0x0FC0)==0x0EC0) { op_JMP();                   break; }// Checked
           if ((first_opcode&0x0B80)==0x0880) { op_MOVEM();                 break; }// Checked
           if ((first_opcode&0x01C0)==0x01C0) { op_LEA();                   break; }// Checked
           if ((first_opcode&0x01C0)==0x0180) { op_CHK();                   break; }// Checked
           Exception_Handler(4);               
           break;                               
                                               
         case (0x5000):                         
           if ((first_opcode&0x00F8)==0x00C8) { op_dBCC();                  break; }// Checked
           if ((first_opcode&0x00C0)==0x00C0) { op_SCC();                   break; }// Checked
           if ((first_opcode&0x0100)==0x0000) { op_ADDSUBQ(1);              break; }// Checked
           if ((first_opcode&0x0100)==0x0100) { op_ADDSUBQ(0);              break; }// Checked
           Exception_Handler(4);
           break;                               
                                               
         case (0x6000):                         
           if ((first_opcode&0x0F00)==0x0100) { op_BSR();                   break; }// Checked
           if ((first_opcode&0x0000)==0x0000) { op_BCC();                   break; }// Checked
           Exception_Handler(4);               
           break;                               
                                               
         case (0x7000):                         
           if ((first_opcode&0x0100)==0x0000) { op_MOVEQ();                 break; }// Checked
           Exception_Handler(4);               
           break;                               
                                               
         case (0x8000):                         
           if ((first_opcode&0x01C0)==0x00C0) { op_DIVU();                  break; }// Checked
           if ((first_opcode&0x01C0)==0x01C0) { op_DIVS();                  break; }// Checked
           if ((first_opcode&0x01F0)==0x0100) { op_xBCD(1);                 break; }// Checked
           if ((first_opcode&0x0000)==0x0000) { op_BOOL(1);                 break; }// Checked
           Exception_Handler(4);               
           break;                               
                                               
         case (0x9000):                         
           if ((first_opcode&0x00C0)==0x00C0) { op_ADDSUBA(0);              break; }// Checked
           if ((first_opcode&0x0130)==0x0100) { op_ADDSUBX(0);              break; }// Checked
           if ((first_opcode&0x0000)==0x0000) { op_ADDSUB(0);               break; }// Checked
           Exception_Handler(4);               
           break;                               
                                                 
         case (0xA000):   {  Exception_Handler(10);                         break; }// Checked
                                               
         case (0xB000):                         
           if ((first_opcode&0x00C0)==0x00C0) { op_CMPA();                  break; }// Checked
           if ((first_opcode&0x0138)==0x0108) { op_CMPM();                  break; }// Checked
           if ((first_opcode&0x0100)==0x0100) { op_BOOL(3);                 break; }// Checked
           if ((first_opcode&0x0100)==0x0000) { op_BOOL(4);                 break; }// Checked
           Exception_Handler(4);
           break;

         case (0xC000):
           if ((first_opcode&0x01C0)==0x00C0) { op_MULU();                  break; }// Checked
           if ((first_opcode&0x01C0)==0x01C0) { op_MULS();                  break; }// Checked
           if ((first_opcode&0x01F0)==0x0100) { op_xBCD(0);                 break; }// Checked
           if ((first_opcode&0x0130)==0x0100) { op_EXG();                   break; }// Checked
           if ((first_opcode&0x0000)==0x0000) { op_BOOL(2);                 break; }// Checked
           Exception_Handler(4);               
           break;                               
                                               
         case (0xD000):                         
           if ((first_opcode&0x00C0)==0x00C0) { op_ADDSUBA(1);              break; }// Checked
           if ((first_opcode&0x0130)==0x0100) { op_ADDSUBX(1);              break; }// Checked
           if ((first_opcode&0x0000)==0x0000) { op_ADDSUB(1);               break; }// Checked
           Exception_Handler(4);
           break;

         case (0xE000):
           if ((first_opcode&0x0FC0)==0x01C0) { op_xSL(0,1);                break; }// Checked  // MEMORY Left
           if ((first_opcode&0x0FC0)==0x03C0) { op_xSL(0,2);                break; }// Checked
           if ((first_opcode&0x0FC0)==0x05C0) { op_ROXL(0);                 break; }// Checked
           if ((first_opcode&0x0FC0)==0x07C0) { op_ROL(0);                  break; }// Checked

           if ((first_opcode&0x0FC0)==0x00C0) { op_xSR(1,2);                break; }// Checked  // MEMORY Right
           if ((first_opcode&0x0FC0)==0x02C0) { op_xSR(2,2);                break; }// Checked
           if ((first_opcode&0x0FC0)==0x04C0) { op_ROXR(0);                 break; }// Checked
           if ((first_opcode&0x0FC0)==0x06C0) { op_ROR(0);                  break; }// Checked

           if ((first_opcode&0x0118)==0x0100) { op_xSL(1,1);                break; }// Checked  // REGISTER Left
           if ((first_opcode&0x0118)==0x0108) { op_xSL(1,2);                break; }// Checked
           if ((first_opcode&0x0118)==0x0110) { op_ROXL(1);                 break; }// Checked
           if ((first_opcode&0x0118)==0x0118) { op_ROL(1);                  break; }// Checked

           if ((first_opcode&0x0118)==0x0000) { op_xSR(1,1);                break; }// Checked  // REGISTER Right
           if ((first_opcode&0x0118)==0x0008) { op_xSR(2,1);                break; }// Checked
           if ((first_opcode&0x0118)==0x0010) { op_ROXR(1);                 break; }// Checked
           if ((first_opcode&0x0118)==0x0018) { op_ROR(1);                  break; }// Checked
           Exception_Handler(4);
           break;

         case (0xF000):                       { Exception_Handler(11);      break; }// Checked

         default: ;
     }


     // Process Trace if flag is set, but don't allow if last opcode caused ILLEGAL or PIVILEGE exception
     // Also don't allow Trace if it was just set/restored. This allows one instruction to be executed between Traces
     //
     if ( (last_mc68k_flag_T==1 && mc68k_flag_T==1) && last_exception!=4 && last_exception!=8)   { Exception_Handler(9); }
     else                                                                                        { last_exception = 0;   } // Debounce

     last_mc68k_flag_T = mc68k_flag_T;
   }
}