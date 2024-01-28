/* d6502 v0.4 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int main(int argc, char **argv) {

  FILE *file;
  uint8_t *buffer;
  unsigned long fileLen;
  int address = 0;
  int i;
  unsigned int currentbyte;
  unsigned int previousbyte;
  unsigned int opcodebyte;
  int paramcount;
  int addrmode;
  char *opcode;
  char *pad;
  char *pre;
  char *post;

  // Padding for 1,2 & 3 byte instructions
  char *padding[3] = {"      ","   ",""};

  // 57 Instructions + Undefined ("???")
  char *instruction[73] = {
    //0     1     2     3     4     5     6     7     8     9
    "ADC","AND","ASL","BCC","BCS","BEQ","BIT","BMI","BNE","BPL",  // 0
    "BRK","BVC","BVS","CLC","CLD","CLI","CLV","CMP","CPX","CPY",  // 1
    "DEC","DEX","DEY","EOR","INC","INX","INY","JMP","JSR","LDA",  // 2
    "LDX","LDY","LSR","NOP","ORA","PHA","PHP","PLA","PLP","ROL",  // 3
    "ROR","???","RTI","RTS","SBC","SEC","SED","SEI","STA","STX",  // 4
    "STY","TAX","TAY","TSX","TXA","TXS","TYA","???","BRA","STZ",  // 5
    "TRB","TSB","PHY","PLY","PHX","PLX","RMB","SMB","BBR","BBS",  // 6
    "NOP8","SKP1","SKP2"};

  // Mnemonic > 57 => 65SC02
  // Mnemonic > 65 => 65C02

  // This is a lookup of the text formating required for mode output, plus one entry to distinguish relative mode
  char *modes[10][2]={{"",""},{"#",""},{"",",X"},{"",",Y"},{"(",",X)"},{"(","),Y"},{"(",")"},{"A",""},{"",""},{"",""}};
  // Addressing mode 7 not used, as it does not comply with lyxass (inc A => inc)

  // Opcode Properties for 256 opcodes {length_in_bytes, mnemonic_lookup, mode_chars_lookup}
  int opcode_props[256][3] = {
//  LSN    0        1        2        3        4        5        6        7        8        9        A        B        C        D        E        F
//  ******** -------- ******** -------- ******** -------- ******** -------- ******** -------- ******** -------- ******** -------- ******** --------
    {2,10,1},{2,34,4},{1,57,0},{1,57,0},{2,60,0},{2,34,0},{2,2,0}, {2,66,0},{1,36,0},{2,34,1},{1, 2,0},{1,57,0},{3,60,0},{3,34,0},{3,2,0}, {3,68,9}, // 0
    {2, 9,8},{2,34,5},{2,34,6},{1,57,0},{2,61,0},{2,34,2},{2,2,2}, {2,66,0},{1,13,0},{3,34,3},{1,24,0},{1,57,0},{3,61,0},{3,34,2},{3,2,2}, {3,68,9}, // 1
    {3,28,0},{2, 1,4},{1,57,0},{1,57,0},{2, 6,0},{2, 1,0},{2,39,0},{2,66,0},{1,38,0},{2, 1,1},{1,39,0},{1,57,0},{3, 6,0},{3,1,0}, {3,39,0},{3,68,9}, // 2
    {2, 7,8},{2, 1,5},{2, 1,6},{1,57,0},{2, 6,2},{2, 1,2},{2,39,2},{2,66,0},{1,45,0},{3, 1,3},{1,20,0},{1,57,0},{3, 6,2},{3,1,2}, {3,39,2},{3,68,9}, // 3
    {1,42,0},{2,23,4},{1,57,0},{1,57,0},{1,57,0},{2,23,0},{2,32,0},{2,66,0},{1,35,0},{2,23,1},{1,32,0},{1,57,0},{3,27,0},{3,23,0},{3,32,0},{3,68,9}, // 4
    {2,11,8},{2,23,5},{2,23,6},{1,57,0},{1,57,0},{2,23,2},{2,32,2},{2,66,0},{1,15,0},{3,23,3},{1,62,0},{1,57,0},{3,70,0},{3,23,2},{3,32,2},{3,68,9}, // 5
    {1,43,0},{2, 0,4},{2,57,0},{1,57,0},{2,59,0},{2, 0,0},{2,40,0},{2,66,0},{1,37,0},{2, 0,1},{1,40,0},{1,57,0},{3,27,6},{3,0,0}, {3,40,0},{3,68,9}, // 6
    {2,12,8},{2, 0,5},{2, 0,6},{1,57,0},{2,59,2},{2, 0,2},{2,40,2},{2,66,0},{1,47,0},{3, 0,3},{1,63,0},{1,57,0},{3,27,2},{3,0,2}, {3,40,2},{3,68,9}, // 7
    {2,58,8},{2,48,4},{1,57,0},{1,57,0},{2,50,0},{2,48,0},{2,49,0},{2,67,0},{1,22,0},{2, 6,1},{1,54,0},{1,57,0},{3,50,0},{3,48,0},{3,49,0},{3,68,9}, // 8
    {2,3,8}, {2,48,5},{2,48,6},{1,57,0},{2,50,2},{2,48,2},{2,49,3},{2,67,0},{1,56,0},{3,48,3},{1,55,0},{1,57,0},{3,59,0},{3,48,2},{3,59,2},{3,69,9}, // 9
    {2,31,1},{2,29,4},{2,30,1},{1,57,0},{2,31,0},{2,29,0},{2,30,0},{2,67,0},{1,52,0},{2,29,1},{1,51,0},{1,57,0},{3,31,0},{3,29,0},{3,30,0},{3,69,9}, // A
    {2,4,8}, {2,29,5},{2,29,6},{1,57,0},{2,31,2},{2,29,2},{2,30,3},{2,67,0},{1,16,0},{3,29,3},{1,53,0},{1,57,0},{3,31,2},{3,29,2},{3,30,3},{3,69,9}, // B
    {2,19,1},{2,17,4},{1,57,0},{1,57,0},{2,19,0},{2,17,0},{2,20,0},{2,67,0},{1,26,0},{2,17,1},{1,21,0},{1,57,0},{3,19,0},{3,17,0},{3,20,0},{3,69,9}, // C
    {2,8,8}, {2,17,5},{2,17,6},{1,57,0},{1,57,0},{2,17,2},{2,20,2},{2,67,0},{1,14,0},{3,17,3},{1,64,0},{1,57,0},{1,72,0},{3,17,2},{3,20,2},{3,69,9}, // D
    {2,18,1},{2,44,4},{1,71,0},{1,57,0},{2,18,0},{2,44,0},{2,24,0},{2,67,0},{1,25,0},{2,44,1},{1,33,0},{1,57,0},{3,18,0},{3,44,0},{3,24,0},{3,69,9}, // E
    {2,5,8}, {2,44,5},{2,44,6},{1,57,0},{1,57,0},{2,44,2},{2,24,2},{2,67,0},{1,46,0},{3,44,3},{1,65,0},{1,57,0},{1,57,0},{3,44,2},{3,24,2},{3,69,9}  // F
  };

  if (argc < 2) {                                           //If no parameters given, display usage instructions and exit.
    fprintf(stderr, "Usage: %s filename address\n\n", argv[0]);
    fprintf(stderr, "Example: %s dump.rom E000\n", argv[0]);
    exit(1);
  }

  if (argc == 3) {
    address = strtol(argv[2], NULL, 16);                    //If second parameter, accept it as HEX address for start of dissasembly.
  }

  file = fopen(argv[1], "rb");                              //Open file
  if (!file) {
    fprintf(stderr, "Can't open file %s", argv[1]);         //Error if file not found
    exit(1);
  }

  fseek(file, 0, SEEK_END);                                 //Seek to end of file to find length
  fileLen = ftell(file);
  fseek(file, 0, SEEK_SET);                                 //And back to the start

  buffer = (uint8_t * ) malloc(fileLen + 1);                //Set up file buffer

  if (!buffer) {                                            //If memory allocation error...
    fprintf(stderr, "Memory allocation error!");            //...display message...
    fclose(file);                                           //...and close file
    exit(1);
  }

  fread((char *)buffer, fileLen, 1, file);                  //Read entire file into buffer and...
  fclose(file);                                             //...close file

  int skip = 0;
  if ( buffer[0] == 0x80U && buffer[1] == 0x08 ){
    skip = 10;
    if ( !address ){
      address = buffer[2]<<8|buffer[3];
    }
  }

  paramcount = 0;
  printf("                  RUN $%04X \n", address);         //Display org address

  for (i = skip; i < fileLen; ++i) {                        //Start proccessing loop.
    previousbyte = currentbyte;
    currentbyte = ((unsigned char * ) buffer)[i];
    if (paramcount == 0) {
      opcodebyte = currentbyte;
      printf("%04X   ", address);                          //Display current address at beginning of line
      paramcount = opcode_props[opcodebyte][0];            //Get instruction length
      opcode = instruction[opcode_props[opcodebyte][1]];   //Get opcode name
      addrmode = opcode_props[opcodebyte][2];              //Get info required to display addressing mode
      pre = modes[addrmode][0];                            //Look up pre-operand formatting text
      post = modes[addrmode][1];                           //Look up post-operand formatting text
      pad = padding[(paramcount - 1)];                     //Calculate correct padding for output alignment
      address = address + paramcount;                      //Increment address
    }
    if (paramcount != 0)                                   //Keep track of possition within instruction
      paramcount = paramcount - 1;
    printf("%02X ", currentbyte);                          //Display the current byte in HEX
    if (paramcount == 0) {
      if ( (opcodebyte & 0xf)  == 7 || (opcodebyte & 0xf) == 0xf ){
        printf(" %s %s%X %s", pad, opcode, (opcodebyte >> 4)&0x7, pre);  //Pad text, display instruction name and pre-operand chars
      } else {
        printf(" %s %s %s", pad, opcode, pre);              //Pad text, display instruction name and pre-operand chars
      }
      if(!strcmp (pad,"   " )) {                            //Check if single operand instruction
        if ( addrmode == 8 ) {
          printf("$%04X", (address + ((currentbyte < 128) ? currentbyte : currentbyte - 256))); //...display relative address.
        } else {                                //If not using relative addressing ...
          printf("$%02X", currentbyte);                     //...display operand
        }
      }
      if(!strcmp (pad,"" )) {                                //Check if two operand instruction and if so...
        if ( addrmode == 9 ){
          printf("$%02X,$%04x", previousbyte, (address + ((currentbyte < 128) ? currentbyte : currentbyte - 256)));
        } else {
          printf("$%02X%02X", currentbyte, previousbyte);     //...display operand
        }
      }
      printf("%s\n", post);                                 //Display post-operand chars
    }
  }
  printf("                  END\n");                        //Add  END directive to end of output
  free(buffer);                                             //Return buffer memory to the system
  return 0;                                                 //All done, exit to the OS
}
