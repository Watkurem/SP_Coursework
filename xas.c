# include "xas.h"
# include <stdio.h>
# include <stdlib.h>
# include <ctype.h>
# include <string.h>

# define ERROR_DUCK(description) {    \
    printf(description ": \""); \
    printf("%s\"\n", currentLine->origin); \
    errquan++; \
    continue; \
  } struct dummy

extern Section section_Data[];
extern Instruction id_Data[];

int Lexer(char *line, Input_Line *where){
  Lexeme tmp_lex;
  int test = 1;
  float range_check = 0;
  size_t range_check2 = 0;

  line[strlen(line) - 1] = '\0';
  where->origin = strdup(line);
  for(size_t i = 0; '\0' != line[i]; i++)
    line[i] = toupper(line[i]);

  for(size_t i=0; line[i] != '\0'; i++){
    if(isspace(line[i]))
      continue;
    if(isdigit(line[i])){
      if('0' != line[i]){
        sscanf(line+i, "%f", &range_check);
        test = sscanf(line+i, "%ld", &tmp_lex.numdata);
        if(tmp_lex.numdata != range_check)
          test = 0;
        while(isdigit(line[i+1]))
          i++;
      } else {

        switch(toupper(line[i+1])){
        case 'X':
          test = sscanf(line+i, "%li", &tmp_lex.numdata);
          range_check2 = ++i;
          while(isxdigit(line[i+1]))
            i++;
          range_check2 = i-range_check2;
          if(range_check2 > 2*8)
            test = 0;
          break;
        case 'B':
          if(test = sscanf(line+i+2, "%ld", &tmp_lex.numdata))
            tmp_lex.numdata = Parse_Binary(tmp_lex.numdata);
          range_check2 = ++i;
          while('0' == line[i+1] || '1' == line[i+1])
            i++;
          range_check2 = i-range_check2;
          if(range_check2 > 8*8)
            test = 0;
          break;
        default:
          sscanf(line+i, "%f", &range_check);
          test = sscanf(line+i, "%ld", &tmp_lex.numdata);
          if(tmp_lex.numdata != range_check)
            test = 0;
          while(isdigit(line[i+1]))
            i++;
        }
      }
      if(!test)
        return test;
      tmp_lex.type = NUM_CONST;
      New_Element(where, tmp_lex);
      continue;
    }
    if(isalpha(line[i]) || '_' == line[i] || '.' == line[i]){
      size_t start_i = i;
      for(; isalnum(line[i+1]) || '_' == line[i+1] || '.' == line[i+1]; i++);
      tmp_lex.cdata = malloc(i-start_i+2);
      tmp_lex.cdata[i-start_i+1] = '\0';
      memcpy(tmp_lex.cdata, &line[start_i], i-start_i+1);
      if(test = IdIsInData(tmp_lex.cdata)){
        tmp_lex.type = MNEMONIC;
        free(tmp_lex.cdata);
        tmp_lex.numdata = test;
      }
      else{
        tmp_lex.type = IDENTIFIER;
      }
      New_Element(where, tmp_lex);
      continue;
    }
    if('"' == line[i]){
      i++;
      size_t start_i = i;
      for(; '"' != line[i]; i++){
        if('\\' == line[i]){
          i++;
        }
      }
      tmp_lex.cdata = malloc(i-start_i+1);
      tmp_lex.cdata[i-start_i] = '\0';
      memcpy(tmp_lex.cdata, &line[start_i], i-start_i);
      tmp_lex.type = CHAR_CONST;
      New_Element(where, tmp_lex);
      continue;
    }
    switch(line[i]){
    case '(':
    case ')':
    case '+':
    case '-':
    case '*':
    case '/':
    case ',':
    case '%':
    case '$':
    case ':':
      tmp_lex.numdata = line[i];
      tmp_lex.type = SINGULAR;
      New_Element(where, tmp_lex);
      continue;
    }
    return 0;
  }
  return 1;
}

long Parse_Binary(long binary){
  long tmp = 0;
  for(int i = 1; binary > 0; i *= 2, binary /= 10){
    tmp += binary%10 * i;
  }
  return tmp;
}

int Parser(Input_Line *head){
  Lexeme *i;
  char inparentheses = 0;
  int errquan = 0;

  Input_Line *prevcurrentLine = head, *currentLine = head->next;
  for(; NULL != currentLine->element;
      prevcurrentLine = currentLine, currentLine = currentLine->next){
    if(0 == currentLine->element->prev->id){
      Remove_One_Line(currentLine, prevcurrentLine);
      currentLine = prevcurrentLine;
    }
  }
  for(currentLine = head->next;
      head != currentLine;
      currentLine = currentLine->next){
    for(i = currentLine->element->next; i != currentLine->element; i = i->next){
      if(SINGULAR == i->type){
        switch((i->numdata)){
        case ':':
          if(2 == i->id && IDENTIFIER == i->prev->type){
            currentLine->semantics.label = i->prev;
          }
          else ERROR_DUCK("Erroneous label here");
          break;
        case ',':
          if(!inparentheses){
            if(0 != i->next->id){
              currentLine->semantics.operand2 = i->next;
            }
            else ERROR_DUCK("Erroneous comma here");
          }
          break;
        case '(':
          if(!inparentheses)
            inparentheses = 1;
          else ERROR_DUCK("Erroneous parenthese here");
          break;
        case ')':
          if(inparentheses)
            inparentheses = 0;
          else ERROR_DUCK("Erroneous parenthese here");
          break;
        }
      }
      if(MNEMONIC == i->type){
        switch(i->id){
        case 3:
          if(!currentLine->semantics.label)
            ERROR_DUCK("Line messed up");
        case 1:
          currentLine->semantics.mnemonic = i;
          break;
        default:
          ERROR_DUCK("Line messed up");
        }
      }
    }
    if(inparentheses)
      ERROR_DUCK("Erroneous parenthese here");
    if(currentLine->semantics.mnemonic &&
       currentLine->semantics.mnemonic->next != currentLine->element)
      currentLine->semantics.operand1=currentLine->semantics.mnemonic->next;
    if((currentLine->semantics.operand2 && !currentLine->semantics.mnemonic) ||
       (currentLine->semantics.operand1 &&
        currentLine->semantics.operand1->type == NUM_CONST &&
        currentLine->semantics.operand1->next != currentLine->element) ||
       !(currentLine->semantics.mnemonic || currentLine->semantics.label))
      ERROR_DUCK("Line messed up");
  }
  return errquan;
}

int GetInstructionSize(Input_Line * line){
  int opcode = id_Data[line->semantics.mnemonic->numdata].code_length,
    immediate = 0,
    displacement = 0,
    mod_rm = 0,
    sib = 0,
    prefix = 0;
  Lexeme * i = line->semantics.mnemonic->next;
  while(i->id != 0){
    switch(i->type){
    case NUM_CONST:
      if(i->next->type == SINGULAR &&
         i->next->numdata == '(')
        displacement = GetConstantSize(i);
      else if(!(i->next->type == SINGULAR &&
                i->next->numdata == ')'))
        return -1;
      break;
    case IDENTIFIER:
      displacement = 4;
      mod_rm = 1;
      if(!(i->next->type == SINGULAR && i->next->numdata == '('))
        sib = 1;
      break;
    case SINGULAR:
      switch(i->numdata){
      case '%':
        mod_rm = 1;
        i = i->next;
        break;
      case '$':
        immediate = GetConstantSize(i->next);
        if(!immediate)
          return -1;
        i = i->next;
        break;
      case '(':
        mod_rm = 1;
        if(!(i->next->next->next->type == SINGULAR &&
             i->next->next->next->numdata == ')')){
          sib = 1;
        }
        while(i->id != 0 && !(i->type == SINGULAR && i->numdata == ')')){
          i = i->next;
          if(i->type == SINGULAR &&
             i->numdata == '%' &&
             i->next->type == IDENTIFIER &&
             i->next->cdata[0] == 'E'){
            prefix = 1;
          }
        }
        break;
      }
      break;
    default:
      break;
    }
    i = i->next;
  }
  return opcode+immediate+displacement+mod_rm+sib+prefix;
}

Assembly_type AnalyzeInstruction(Input_Line * line){
  Assembly_type tmp = PLAIN;
  Lexeme *i = line->semantics.operand1;
  if(!i)
    return tmp;

  switch(i->type){
  case SINGULAR:
    switch(i->numdata){
    case '%':
      tmp = REG;
      break;
    case '$':
      tmp = IMM;
      break;
    case '(':
      tmp = MEM;
      break;
    default:
      return DOES_NOT_ASSEMBLE;
    }
    break;
  case NUM_CONST:
  case IDENTIFIER:
    tmp = MEM;
    break;
  default:
    return DOES_NOT_ASSEMBLE;
  }

  i = line->semantics.operand2;
  if(!i)
    return tmp;
  else
    switch(tmp){
    case REG:
      tmp = REG_MEM;
      break;
    case IMM:
      tmp = IMM_MEM;
      break;
    default:
      tmp = MEM_REG;
      break;
    }

  switch(i->type){
  case NUM_CONST:
  case IDENTIFIER:
    if(tmp != MEM_REG)
      break;
  case CHAR_CONST:
  case MNEMONIC:
    return DOES_NOT_ASSEMBLE;
    break;
  case SINGULAR:
    switch(i->numdata){
    case '%':
      switch(tmp){
      case REG_MEM:
        tmp = REG_REG;
        break;
      case IMM_MEM:
        tmp = IMM_REG;
        break;
      default:
        break;
      }
      break;
    case '(':
      if(tmp != MEM_REG)
        break;
    default:
      return DOES_NOT_ASSEMBLE;
    }
    break;
  default:
    return DOES_NOT_ASSEMBLE;
  }

  return tmp;
}

int First_Pass(Identifier *ids, Input_Line *sourceHead, int empty){
  Input_Line *currentLine = sourceHead->next;
  Identifier *currentID = ids;
  Section *currentSec = NULL;
  Identifier * tmpId;
  char skipping = 0;
  char in_if = 0;
  int code_length;
  Assembly_type assembly_type;
  int errquan = 0;

  for(; currentLine->element != NULL; currentLine = currentLine->next){
    assembly_type = DOES_NOT_ASSEMBLE;
    code_length = 0;
    if(!skipping){
      if(currentLine->semantics.label){
        if(IdIsInData(currentLine->semantics.label->cdata) ||
           (empty && IdIsInList(currentLine->semantics.label->cdata, ids)) ||
           !currentSec){
          ERROR_DUCK("First pass error");
        }
        if(empty)
          currentID = NewID(currentID, currentLine->semantics.label->cdata,
                            currentSec->point, currentSec);
        else
          IdIsInList(currentLine->semantics.label->cdata, ids)->value = currentSec->point;
      }
      if(currentLine->semantics.mnemonic){
        switch(currentLine->semantics.mnemonic->numdata){
        case D_SECTION:
          if(!currentLine->semantics.operand1 ||
             currentLine->semantics.operand2 ||
             currentLine->semantics.operand1->type != IDENTIFIER)
            ERROR_DUCK("First pass error");
          if(!strcmp(currentLine->semantics.operand1->cdata, ".TEXT"))
            currentSec = &section_Data[TEXT];
          else if(!strcmp(currentLine->semantics.operand1->cdata, ".DATA"))
            currentSec = &section_Data[DATA];
          else
            ERROR_DUCK("First pass error");
          currentSec->point = 0;
          break;
        case D_EQU:
          if(!currentLine->semantics.operand1 ||
             !currentLine->semantics.operand2 ||
             currentLine->semantics.operand1->type != IDENTIFIER ||
             currentLine->semantics.operand2->type != NUM_CONST)
            ERROR_DUCK("First pass error");
          tmpId = IdIsInList(currentLine->semantics.operand1->cdata,
                             ids);
          if(!tmpId)
            currentID = NewID(currentID,
                              currentLine->semantics.operand1->cdata,
                              currentLine->semantics.operand2->numdata,
                              NULL);
          else
            tmpId->value = currentLine->semantics.operand2->numdata;
          break;
        case D_BYTE:
          if(!currentLine->semantics.operand1 ||
             currentLine->semantics.operand2 ||
             NUM_CONST != currentLine->semantics.operand1->type){
            ERROR_DUCK("First pass error");
          }
          code_length = 1;
          assembly_type = IMM;
          break;
        case D_SHORT:
          if(!currentLine->semantics.operand1 ||
             currentLine->semantics.operand2 ||
             NUM_CONST != currentLine->semantics.operand1->type){
            ERROR_DUCK("First pass error");
          }
          code_length = 2;
          assembly_type = IMM;
          break;
        case D_LONG:
          if(!currentLine->semantics.operand1 ||
             currentLine->semantics.operand2 ||
             NUM_CONST != currentLine->semantics.operand1->type){
            ERROR_DUCK("First pass error");
          }
          code_length = 4;
          assembly_type = IMM;
          break;
        case D_ASCII:
          if(!currentLine->semantics.operand1 ||
             currentLine->semantics.operand2 ||
             CHAR_CONST != currentLine->semantics.operand1->type){
            ERROR_DUCK("First pass error");
          }
          code_length = strlen(currentLine->semantics.operand1->cdata);
          for(size_t i = 0;
              currentLine->semantics.operand1->cdata[i] != '\0';
              i++)
            if(currentLine->semantics.operand1->cdata[i] == '\\')
              code_length--;
          assembly_type = IMM;
          break;
        case D_IF:
          if(in_if ||
             !currentLine->semantics.operand1 ||
             currentLine->semantics.operand2)
            ERROR_DUCK("First pass error");
          in_if = 1;
          switch(currentLine->semantics.operand1->type){
          case NUM_CONST:
            if(!currentLine->semantics.operand1->numdata)
              skipping = 1;
            break;
          case IDENTIFIER:
            tmpId = IdIsInList(currentLine->semantics.operand1->cdata, ids);
            if(tmpId && !tmpId->value)
              skipping = 1;
            break;
          default:
            ;
          }
          break;
        case D_ELSE:
          if(!in_if)
            ERROR_DUCK("First pass error");
          skipping = 1;
          break;
        case D_ENDIF:
          if(!in_if)
            ERROR_DUCK("First pass error");
          in_if = 0;
          break;
        case D_GLOBL:
          break;
        case ADDL:
        case ADDQ:
        case CMPL:
        case CMPQ:
        case ANDL:
        case ANDQ:
        case ORL:
        case ORQ:
          if(!currentLine->semantics.operand1 ||
             !currentLine->semantics.operand2){
            ERROR_DUCK("First pass error");
          }
          else
            if(currentLine->semantics.operand1->type == SINGULAR &&
               currentLine->semantics.operand1->numdata == '$' &&
               currentLine->semantics.operand2->type == SINGULAR &&
               currentLine->semantics.operand2->numdata == '%' &&
               currentLine->semantics.operand1->next->type == NUM_CONST &&
               currentLine->semantics.operand2->next->type == IDENTIFIER &&
               0 == strcmp(&(currentLine->semantics.operand2->next->cdata[1]),
                           "AX")){
              code_length =
                id_Data[currentLine->semantics.mnemonic->numdata].code_length +
                GetConstantSize(currentLine->semantics.operand1->next);
              assembly_type = ACCUMULATOR_SPECIAL;
            }
            else
              goto nothing_special;
          break;
        case JBE:
          if(!currentLine->semantics.operand1 ||
             currentLine->semantics.operand2 ||
             currentLine->semantics.operand1->type != IDENTIFIER){
            ERROR_DUCK("First pass error");
          }else{
            assembly_type = JUMP;
            if(!empty){
              if(!(tmpId = IdIsInList(currentLine->semantics.operand1->cdata, ids)))
                ERROR_DUCK("First pass error, undefined symbol");
              if(currentSec == tmpId->section &&
                 (tmpId->value - currentSec->point - 2) < 256){
                code_length = 2;
                break;
              }
            }
            code_length = 6;
          }
          break;
        case SHLL:
        case SHLQ:
          if(!(currentLine->semantics.operand1 &&
               currentLine->semantics.operand2) ||
             (currentLine->semantics.operand2->type == SINGULAR &&
              currentLine->semantics.operand2->numdata == '$') ||
             !(currentLine->semantics.operand1->type == SINGULAR &&
               (currentLine->semantics.operand1->numdata == '$' ||
                (currentLine->semantics.operand1->numdata == '%' &&
                 currentLine->semantics.operand1->next->type == IDENTIFIER &&
                 0 == strcmp(currentLine->semantics.operand1->next->cdata, "CL")))))
            ERROR_DUCK("First pass error");
          assembly_type = SHIFT;
          code_length = GetInstructionSize(currentLine);
          break;
        default:
        nothing_special:
          code_length = GetInstructionSize(currentLine);
          assembly_type = AnalyzeInstruction(currentLine);
          if(code_length < 0)
            ERROR_DUCK("First pass error");
          break;
        }
      }
      if(!empty && assembly_type != DOES_NOT_ASSEMBLE){
        currentLine->assembly = calloc(sizeof(Assembly), 1);
        currentLine->assembly->code_length = code_length;
        currentLine->assembly->type = assembly_type;
        if(currentSec){
          currentLine->assembly->section_point = currentSec->point;
          currentLine->assembly->section = currentSec;
        }
      }
      if(currentSec)
        currentSec->point+=code_length;
    } else {                          assert(in_if == 1);
      if(currentLine->semantics.mnemonic)
        switch(currentLine->semantics.mnemonic->numdata){
        case D_IF:
          ERROR_DUCK("First pass error");
        case D_ENDIF:
          in_if = 0;
        case D_ELSE:
          skipping = 0;
          break;
        }
    }
  }
  return errquan;
}

void GetRegisterCode(char *op, unsigned char *byte){
  switch(op[1]){
  case 'A':
    break; // R/M = 000
  case 'C':
    *byte += (char) Parse_Binary(1);
    break;
  case 'D':
    if(op[2] == 'X')
      *byte += (char) Parse_Binary(10);
    else
      *byte += (char) Parse_Binary(111);
    break;
  case 'B':
    if(op[2] == 'X')
      *byte += (char) Parse_Binary(11);
    else
      *byte += (char) Parse_Binary(101);
    break;
  case 'S':
    if(op[2] == 'P')
      *byte += (char) Parse_Binary(100);
    else
      *byte += (char) Parse_Binary(110);
    break;
  default:
    break;
  }
}

int Second_Pass(Identifier *ids, Input_Line *sourceHead){
  Input_Line *currentLine = sourceHead->next;
  char * code;
  char * mod_rm = calloc(sizeof(char), 3);
  char * sib = calloc(sizeof(char), 3);
  char * disp = calloc(sizeof(char), 9);
  char * imm = calloc(sizeof(char), 9);

  unsigned char mod_rm_byte;
  unsigned char sib_byte;

  for(; currentLine->element != NULL; currentLine = currentLine->next){
    if(currentLine->assembly){
      mod_rm_byte = 0;
      sib_byte = 0;
      code = currentLine->assembly->code;
      strcat(code, id_Data[currentLine->semantics.mnemonic->numdata].basic);
      switch(currentLine->assembly->type){
      case IMM:
        if(currentLine->semantics.operand1 && currentLine->semantics.operand1->type == CHAR_CONST){
          // If you read all the way here, please accept my respect and
          // sincerest apologies for the s&$tcode that follows. Deadlines
          // are horrible stuff.
          for(int i = 0; i < CODE_LENGTH && currentLine->semantics.operand1->cdata[i] != '\0'; i++ ){
            if(currentLine->semantics.operand1->cdata[i] == '\\' &&
               currentLine->semantics.operand1->cdata[i+1] == '0'){
              for(int k = i; currentLine->semantics.operand1->cdata[k] != '\0'; k++)
                currentLine->semantics.operand1->cdata[k] = currentLine->semantics.operand1->cdata[k+1];
              currentLine->semantics.operand1->cdata[i] = '\0';
            }
            sprintf(&code[i*2], "%02hhX", currentLine->semantics.operand1->cdata[i]);
          }
          break;
        } else
          if(currentLine->semantics.operand1->type == NUM_CONST)
            switch(GetConstantSize(currentLine->semantics.operand1)){
            case 1:
              sprintf(code, "%02lX", currentLine->semantics.operand1->numdata);
              break;
            case 4:
              for(int i = 0; i < currentLine->assembly->code_length; i++)
                sprintf(&code[i*2], "%02hhX", *((char *) &(currentLine->semantics.operand1->numdata) + i));
              break;
            default:
              return 0;
            }
        break;
      case REG:
        GetRegisterCode(currentLine->semantics.operand1->next->cdata, &mod_rm_byte);
        mod_rm_byte += (char) Parse_Binary(11000000);
        mod_rm_byte += (char) Parse_Binary(id_Data[currentLine->semantics.mnemonic->numdata].extra);
        sprintf(mod_rm, "%02hhX", mod_rm_byte);
        break;
      case MEM:
        if(currentLine->semantics.operand1->type == NUM_CONST){
          for(int i = 0; i < 4; i++)
            sprintf(&disp[i*2], "%02hhX", *((char *) &(currentLine->semantics.operand1->numdata) + i));
          break;
        }
        if(currentLine->semantics.operand1->type == IDENTIFIER){
          if(IdIsInList(currentLine->semantics.operand1->cdata, ids)->section == currentLine->assembly->section &&
             IdIsInList(currentLine->semantics.operand1->cdata, ids)->value > currentLine->assembly->section_point)
            sprintf(disp, "%lX", IdIsInList(currentLine->semantics.operand1->cdata, ids)->value - currentLine->assembly->section_point - currentLine->assembly->code_length);
          else
            strcat(disp, "00000000");
        }
        Lexeme *tmplx = NULL;
        if(currentLine->semantics.operand1->next->type == SINGULAR &&
           currentLine->semantics.operand1->next->numdata == '(')
          tmplx = currentLine->semantics.operand1->next->next;
        if(currentLine->semantics.operand1->type == SINGULAR &&
           currentLine->semantics.operand1->numdata == '(')
          tmplx = currentLine->semantics.operand1->next;
        if(tmplx){
          if(*disp == 0)
            strcat(mod_rm, "0C");
          else
            strcat(mod_rm, "8C");
          GetRegisterCode(tmplx->next->next->next->next->cdata, &sib_byte);
          sib_byte<<=3;
          GetRegisterCode(tmplx->next->cdata, &sib_byte);
          sprintf(sib, "%02hhX", sib_byte);
          break;
        } else{
          strcat(mod_rm, "0C");
          strcat(sib, "25");
        }
        break;
      case ACCUMULATOR_SPECIAL:
      case PLAIN:
        break;
      case REG_REG:
        *code = 0;
        strcat(code, id_Data[currentLine->semantics.mnemonic->numdata].reg_rm);
        GetRegisterCode(currentLine->semantics.operand1->next->cdata, &mod_rm_byte);
        mod_rm_byte<<=3;
        GetRegisterCode(currentLine->semantics.operand2->next->cdata, &mod_rm_byte);
        mod_rm_byte += (char) Parse_Binary(11000000);
        sprintf(mod_rm, "%02hhX", mod_rm_byte);
        break;
      case REG_MEM:
        *code = 0;
        strcat(code, id_Data[currentLine->semantics.mnemonic->numdata].reg_rm);
        GetRegisterCode(currentLine->semantics.operand1->next->cdata, &mod_rm_byte);
        mod_rm_byte<<=3;
        mod_rm_byte += (char) Parse_Binary(100); // There'll be SIB
        sprintf(mod_rm, "%02hhX", mod_rm_byte);
        strcat(sib, "25");
        if(IdIsInList(currentLine->semantics.operand2->cdata, ids)->section == currentLine->assembly->section &&
           IdIsInList(currentLine->semantics.operand2->cdata, ids)->value > currentLine->assembly->section_point)
          sprintf(disp, "%lX", IdIsInList(currentLine->semantics.operand2->cdata, ids)->value - currentLine->assembly->section_point - currentLine->assembly->code_length);
        else
          strcat(disp, "00000000");
        break;
      case MEM_REG:
        *code = 0;
        strcat(code, id_Data[currentLine->semantics.mnemonic->numdata].rm_reg);
        GetRegisterCode(currentLine->semantics.operand2->next->cdata, &mod_rm_byte);
        mod_rm_byte<<=3;
        mod_rm_byte += (char) Parse_Binary(100); // There'll be SIB
        sprintf(mod_rm, "%02hhX", mod_rm_byte);
        strcat(sib, "25");
        if(IdIsInList(currentLine->semantics.operand1->cdata, ids)->section == currentLine->assembly->section &&
           IdIsInList(currentLine->semantics.operand1->cdata, ids)->value > currentLine->assembly->section_point)
          sprintf(disp, "%lX", IdIsInList(currentLine->semantics.operand1->cdata, ids)->value - currentLine->assembly->section_point - currentLine->assembly->code_length);
        else
          strcat(disp, "00000000");
        break;
      case IMM_REG:
        break;
      case IMM_MEM:
        *code = 0;
        strcat(code, id_Data[currentLine->semantics.mnemonic->numdata].imm_rm);
        for(int i = 0; i < 4; i++)
          sprintf(&imm[i*2], "%02hhX", *((char *) &(currentLine->semantics.operand1->next->numdata) + i));
        if(IdIsInList(currentLine->semantics.operand2->cdata, ids)->section == currentLine->assembly->section &&
           IdIsInList(currentLine->semantics.operand2->cdata, ids)->value > currentLine->assembly->section_point)
          sprintf(disp, "%lX", IdIsInList(currentLine->semantics.operand2->cdata, ids)->value - currentLine->assembly->section_point - currentLine->assembly->code_length);
        else
          strcat(disp, "00000000");
        mod_rm_byte += (char) Parse_Binary(id_Data[currentLine->semantics.mnemonic->numdata].extra);
        mod_rm_byte += (char) Parse_Binary(100); // there'll be SIB
        strcat(sib, "25");
        sprintf(mod_rm, "%02hhX", mod_rm_byte);
        break;

      case JUMP:
        *code = 0;
        if(currentLine->assembly->code_length == 2)
          strcat(code, id_Data[currentLine->semantics.mnemonic->numdata].basic);
        else
          if(currentLine->assembly->code_length == 6)
            strcat(code, id_Data[currentLine->semantics.mnemonic->numdata].imm_rm);
          else
            return 0;
        if(IdIsInList(currentLine->semantics.operand1->cdata, ids)->section == currentLine->assembly->section &&
           IdIsInList(currentLine->semantics.operand1->cdata, ids)->value > currentLine->assembly->section_point)
          sprintf(disp, "%lX", IdIsInList(currentLine->semantics.operand1->cdata, ids)->value - currentLine->assembly->section_point - currentLine->assembly->code_length);
        else
          strcat(disp, "00000000");
        break;

      case SHIFT:
        *code = 0;
        if(currentLine->semantics.operand1->numdata == '$')
          if(currentLine->semantics.operand1->next->numdata == 1)
            strcat(code, id_Data[currentLine->semantics.mnemonic->numdata].imm_rm);
          else{
            if(GetConstantSize(currentLine->semantics.operand1->next) > 1)
              return 0;
            else{
              strcat(code, id_Data[currentLine->semantics.mnemonic->numdata].rm_reg);
              sprintf(imm, "%02lx", currentLine->semantics.operand1->next->numdata);
            }
          }
        else
          strcat(code, id_Data[currentLine->semantics.mnemonic->numdata].reg_rm);
        strcat(mod_rm,"E1");
      default: break;
      }

      strcat(code, mod_rm);
      strcat(code, sib);
      strcat(code, disp);
      strcat(code, imm);
      mod_rm_byte = 0;
      sib_byte = 0;
      *mod_rm = 0;
      *sib = 0;
      *disp = 0;
      *imm = 0;
    }
  }
  return 1;
}

int main(){
  Input_Line source = {NULL, &source, {NULL, NULL, NULL, NULL}, NULL, NULL};
  Input_Line *current = &source;
  int errquan = 0;

  char *source_line = NULL;
  size_t length_of_source_line;

  printf("\n\n\n\n\n");
  while(-1 != getline(&source_line, &length_of_source_line, stdin)){
    current = New_Line(&source, current);
    if(!Lexer(source_line, current)){
      printf("Lexer found an error on this line: \"%s\"\n", current->origin);
      errquan++;
      Purge_Elements(current->element);
    }
  }
  free(source_line);
  errquan += Parser(&source);

  Identifier userDefinedIDs = {NULL, 0, NULL, &userDefinedIDs};

  First_Pass_Print(&source); // Uncomment to get lexeme and sentence structure listing

  errquan += First_Pass(&userDefinedIDs, &source, 1);
  errquan += First_Pass(&userDefinedIDs, &source, 0);
  if(errquan){
    printf("\n%d erroneous lines were found in input\n", errquan);
    return 1;
  }

  if(!Second_Pass(&userDefinedIDs, &source)){
    printf("!!SECOND PASS FAILURE!!\n");
    Purge_List(&source);
    Purge_Identifiers(&userDefinedIDs);
    return 1;
  }

  Listing_Print(&source);
  printf("\n\n");
  ID_Print(&userDefinedIDs);

  Purge_List(&source);
  Purge_Identifiers(&userDefinedIDs);
  return 0;
}
