# include "xas.h"
# include <stdio.h>
# include <stdlib.h>
# include <ctype.h>
# include <string.h>

extern Instruction id_Data[];

Input_Line * New_Line(Input_Line *head, Input_Line *last){
  Input_Line *tmp = calloc(sizeof(Input_Line), 1);
  tmp->next = head;
  tmp->element = calloc(sizeof(Lexeme), 1);
  tmp->element->next = tmp->element;
  tmp->element->prev = tmp->element;
  tmp->origin = NULL;
  tmp->assembly = NULL;
  last->next = tmp;
  return tmp;
}

Lexeme * New_Element(Input_Line *line, const Lexeme new_el){
  Lexeme *tmp = malloc(sizeof(Lexeme));
  tmp->id = line->element->prev->id + 1;
  tmp->type = new_el.type;

  switch(tmp->type){
  case NOTHING:
    tmp->cdata = NULL;
    break;
  case IDENTIFIER:
  case CHAR_CONST:
    tmp->cdata = new_el.cdata;
    break;
  default:
    tmp->numdata = new_el.numdata;
  }

  tmp->next = line->element;
  tmp->prev = line->element->prev;
  line->element->prev = line->element->prev->next = tmp;
  return tmp;
}

void First_Pass_Print(Input_Line *head){
  if(head->next == head){
    printf("No input was received\n");
    return;
  }
  Line_Recur_Print(head->next);
}

void Line_Recur_Print(Input_Line *current){
  if(current->element == NULL)
    return;
  Line_Print(current->element);
  printf("Label: %d\n",current->semantics.label?current->semantics.label->id:0);
  printf("Mnemonic: %d\n",
         current->semantics.mnemonic?current->semantics.mnemonic->id:0);
  printf("Op1: %d\n",
         current->semantics.operand1?current->semantics.operand1->id:0);
  printf("Op2: %d\n",
         current->semantics.operand2?current->semantics.operand2->id:0);
  printf("Source: %s\n",
         current->origin?current->origin:0);
  putchar('\n');
  Line_Recur_Print(current->next);
}

void Line_Print(Lexeme *head){
  if(head->next == head){
    printf("<Empty line>\n");
    return;
  }
  Lexeme_Recur_Print(head->next);
}

void Lexeme_Recur_Print(Lexeme *current){
  if(current->id == 0)
    return;
  printf("%-2d|", current->id);

  switch(current->type){
  case NOTHING:
    printf("I TRIED TO PRINT NOTHING! THIS SHOULDN'T HAVE HAPPENED!");
    break;
  case NUM_CONST:
    printf("  Numeric constant:%ld", current->numdata);
    break;
  case CHAR_CONST:
    printf("Character constant:%s", current->cdata);
    break;
  case IDENTIFIER:
    printf("        Identifier:%s", current->cdata);
    break;
  case MNEMONIC:
    printf("          Mnemonic:%s", id_Data[current->numdata].mnemonic);
    break;
  case SINGULAR:
    printf("   Singular lexeme:");
    putchar((current->numdata));
    break;
  default:
    printf("SOMETHING WENT HORRIBLY WRONG!");
  }

  putchar('\n');
  Lexeme_Recur_Print(current->next);
}

void Purge_List(Input_Line *head){
  Line_Recur_Delete(head->next);
}

void Line_Recur_Delete(Input_Line *current){
  if(NULL != current->element){
    Line_Recur_Delete(current->next);
    Purge_Elements(current->element);
    free(current->element);
    free(current->origin);
    free(current->assembly);
    free(current);
  }
}

void Purge_Elements(Lexeme *head){
  Lexeme_Recur_Delete(head->next);
  head->prev = head;
}

void Lexeme_Recur_Delete(Lexeme *current){
  if(NOTHING != current->type){
    Lexeme_Recur_Delete(current->next);
    if(CHAR_CONST == current->type || IDENTIFIER == current->type)
      free(current->cdata);
    free(current);
  }
}

void Remove_One_Line(Input_Line *this_one, Input_Line *previous){
  previous->next = this_one->next;
  Purge_Elements(this_one->element);
  free(this_one->element);
  free(this_one->origin);
  free(this_one);
}

Identifier * NewID(Identifier *tail, char *newID, unsigned long _value, Section * _section){
  Identifier *head = tail->next;
  tail->next = malloc(sizeof(Identifier));
  tail->next->next = head;
  tail->next->name = strdup(newID);
  tail->next->value = _value;
  tail->next->section = _section;
  return tail->next;
}

void ID_Print(Identifier *head){
  head = head->next;
  if(!head->name)
    return;
  printf("%-10s  %-5s:%016lx\n", head->name, head->section?head->section->name:"*ABS*", head->value);
  ID_Print(head);
}

int IdIsInData(char *id){
  for(size_t i = 0; id_Data[i].mnemonic != NULL; i++)
    if(strcmp(id, id_Data[i].mnemonic) == 0)
      return i;
  return 0;
}

Identifier * IdIsInList(char *id, Identifier *head){
  for(head = head->next; head->name != NULL; head = head->next){
    if(strcmp(head->name, id) == 0){
      return head;
    }
  }
  return NULL;
}

void Purge_Identifiers(Identifier *head){
  for(Identifier *current = head->next->next, *marked = head->next;
      marked != head;
      marked = current, current = current->next){
    free(marked->name);
    free(marked);
  }
  head->next = head;
}

int GetConstantSize(Lexeme * constant){
  if(constant->type == NUM_CONST){
    if(constant->numdata <= 255)
      return 1;
    else
      if(constant->numdata <= 4294967295)
        return 4;
  } else
    if(constant->type == IDENTIFIER){
      return 4;
    }
  return 0;
}

void Listing_Print(Input_Line *head){
  for(head = head->next; head->element != NULL; head = head->next){
    if(head->assembly){
      if(head->assembly->section)
        printf("%04lX:", head->assembly->section_point);
      else
        printf("     ");
      printf("%04X:%-*s:%s\n",
             head->assembly->code_length,
             CODE_LENGTH,
             head->assembly->code,
             head->origin);
    } else {
      for(int i = CODE_LENGTH + 10; i > 0; i--)
        putchar(' ');
      printf(":%s\n", head->origin);
    }
  }
}
