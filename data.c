# include "xas.h"
# include <stdlib.h>

Instruction id_Data[] = {
  {"NULL", 0, NOT_INST, NULL, NULL, NULL, NULL, 0},        
  {".ASCII", 0, ONE_OP, "", "", "", "", 0},                
  {".SHORT", 0, ONE_OP, "", "", "", "", 0},                
  {".GLOBL", 0, NOT_INST, "", "", "", "", 0},              
  {".ENDIF", 0, NOT_INST, "", "", "", "", 0},              
  {".BYTE", 0, ONE_OP, "", "", "", "", 0},                 
  {".LONG", 0, ONE_OP, "", "", "", "", 0},                 
  {".ELSE", 0, NOT_INST, "", "", "", "", 0},               
  {"CMPSD", 1, NO_OP, "A7", "", "", "", 0},                
  {".EQU", 0, NOT_INST, "", "", "", "", 0},                
  {"MULQ", 2, ONE_OP, "48F7", "", "", "", 100000},         
  {"MULL", 1, ONE_OP, "F7", "", "", "", 100000},           
  {"DECQ", 2, ONE_OP, "48FF", "", "", "", 0},              
  {"DECL", 1, ONE_OP, "FF", "", "", "", 0},                
  {"ADDQ", 2, TWO_OP, "4805", "4881", "4801", "4803", 0},  
  {"ADDL", 1, TWO_OP, "05", "81", "01", "03", 0},          
  {"CMPQ", 2, TWO_OP, "483D", "4881", "4839", "483B", 0},  
  {"CMPL", 1, TWO_OP, "3D", "81", "39", "3B", 0},          
  {"ANDQ", 2, TWO_OP, "4825", "4881", "4821", "4823", 0},  
  {"ANDL", 1, TWO_OP, "25", "81", "21", "23", 0},          
  {"SHLQ", 2, TWO_OP, "", "48D1", "48D3", "48C1", 0},      
  {"SHLL", 1, TWO_OP, "", "D1", "D3", "C1", 0},            
  {".IF", 0, NOT_INST, "", "", "", "", 0},                 
  {"JBE", 1, ONE_OP, "76", "0F86", "", "", 0},             
  {"ORQ", 2, TWO_OP, "480D", "4881", "4809", "480B", 1000},
  {"ORL", 1, TWO_OP, "0D", "81", "09", "0B", 1000},        
  {".SECTION", 0, NOT_INST, "", "", "", "", 0},            
  {NULL, 0, NOT_INST, NULL, NULL, NULL, NULL, 0}
};

// For shift imm_rm == by 1, reg_rm == by CL, rm_reg == by imm8
// For jump basic == 8 bit version, imm_rm = 32 bit version

Section section_Data[] = {
  {".TEXT", 0},
  {".DATA", 0}
};