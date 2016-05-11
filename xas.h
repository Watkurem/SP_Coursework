# ifndef MAIN_HEADER
# define MAIN_HEADER

# include <stddef.h>
# include <assert.h>

# define D_SECTION 26
# define D_ASCII    1
# define D_SHORT    2
# define D_GLOBL    3
# define D_ENDIF    4
# define D_BYTE     5
# define D_LONG     6
# define D_ELSE     7
# define D_EQU      9
# define D_IF      22
# define CMPSD      8
# define MULQ      10
# define MULL      11
# define DECQ      12
# define DECL      13
# define ADDQ      14
# define ADDL      15
# define CMPQ      16
# define CMPL      17
# define ANDQ      18
# define ANDL      19
# define SHLQ      20
# define SHLL      21
# define JBE       23
# define ORQ       24
# define ORL       25

# define TEXT 0
# define DATA 1

# define CODE_LENGTH 60

typedef enum {NOTHING,
              NUM_CONST,
              CHAR_CONST,
              IDENTIFIER,
              SINGULAR,
              MNEMONIC
} Element_type;

typedef enum {NOT_INST,
              NO_OP,
              ONE_OP,
              TWO_OP
} Instruction_type;

typedef enum {DOES_NOT_ASSEMBLE,
              PLAIN,
              IMM,
              REG,
              MEM,
              REG_REG,
              REG_MEM,
              MEM_REG,
              IMM_REG,
              IMM_MEM,
              ACCUMULATOR_SPECIAL,
              JUMP,
              SHIFT
} Assembly_type;

typedef struct lexeme {
  int id;
  union {
    char *cdata;
    long numdata;
  };
  Element_type type;
  struct lexeme *next;
  struct lexeme *prev;
} Lexeme;

typedef struct segm {
  char *name;
  unsigned long point;
} Section;

typedef struct instr {
  char *mnemonic;
  int code_length;
  Instruction_type type;
  char *basic;
  char *imm_rm;
  char *reg_rm;
  char *rm_reg;
  long extra;
} Instruction;

typedef struct idnt {
  char *name;
  unsigned long value;
  Section * section;
  struct idnt *next;
} Identifier;

typedef struct ass {
  char code[CODE_LENGTH];
  int code_length;
  unsigned long section_point;
  Section * section;
  Assembly_type type;
} Assembly;

typedef struct in_line {
  Lexeme *element;
  /* Lexeme *latest; */
  struct in_line *next;
  struct {
    Lexeme *label;
    Lexeme *mnemonic;
    Lexeme *operand1;
    Lexeme *operand2;
  } semantics;
  char *origin;
  Assembly *assembly;
} Input_Line;

void Listing_Print(Input_Line *head);
int GetConstantSize(Lexeme * constant);
void Purge_Identifiers(Identifier *head);
Identifier * IdIsInList(char *id, Identifier *head);
int IdIsInData(char *id);
void ID_Print(Identifier *head);
Identifier * NewID(Identifier *tail, char *newID, unsigned long value, Section * _section);
void Remove_One_Line(Input_Line *this_one, Input_Line *previous);
int  Parser(Input_Line *head);
void Lexeme_Recur_Delete(Lexeme *current);
void Purge_Elements(Lexeme *head);
void Line_Recur_Delete(Input_Line *current);
void Purge_List(Input_Line *head);
long Parse_Binary(long binary);
int  Lexer(char *line, Input_Line *where);
void Lexeme_Recur_Print(Lexeme *current);
void Line_Print(Lexeme *head);
void Line_Recur_Print(Input_Line *current);
void First_Pass_Print(Input_Line *head);
Lexeme * New_Element(Input_Line *line, const Lexeme new_el);
Input_Line * New_Line(Input_Line *head, Input_Line *last);

# endif
