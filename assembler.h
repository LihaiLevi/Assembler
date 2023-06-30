/*
Header file for the assembly .
contain macros , data structures and methods declaration.
*/

#include <stdio.h>
#include <string.h>

/* ======== Macros ======== */

/* Utilities */
#define FOR_LOOP_UNTIL_EXIT				for(;;)
#define BYTE_SIZE			8
#define FALSE				0
#define TRUE				1

/* Given Constants */
#define MAX_DATA_NUM		1000
#define MAX_LINE_LENGTH		80
#define MAX_LABEL_LENGTH	31
#define MEMORY_WORD_LENGTH	32

/* Defining Constants */
#define MAX_LINES_NUM		700
#define MAX_LABELS_NUM		MAX_LINES_NUM
#define typeRcmd  'R'
#define typeJcmd  'J'
#define typeIcmd  'I'

/* ======== Data Structures ======== */


typedef unsigned int bool; /* Only get TRUE or FALSE values */


/* Labels Management */
typedef struct
{
    int address;					/* The value it contains */
    char name[MAX_LABEL_LENGTH];	/* The name of the label */
    bool isExtern;					/* Extern flag */
    bool isData;					/* Data flag (.db or .dw or.dh or .asciz) */
} labelInfo;

/* Directive And Commands */
typedef struct
{
    char *name;
    void (*analysisFunc)();
} directive;


typedef struct
{
    char *name;
    char typeCmd;
    int funct ;
    unsigned int opcode : 6;
    int numOfParams ;
} command;


/* Operands */

typedef enum { NUMBER = 0, LABEL = 1,REGISTER = 2, INVALID = -1 } operandType;

typedef struct
{
    int value;				/* Value */
    char *str;				/* String */
    operandType type;	    /* Type */
    int address;			/* The address of the operand in the memory */
} operandInfo;


/* Line */
typedef struct
{
    int lineNum;				/* The number of the line in the file */
    int address;				/* The address of the first word in the line */
    char *originalString;		/* The original pointer, allocated by malloc */
    char *lineStr;				/* The text it contains (changed while using analysisLine) */
    bool isError;				/* Represent whether there is an error or not */
    labelInfo *label;			/* A pointer to the lines label in labelArr */

    char *commandStr;			/* The string of the command or directive */

    /* Command line */
    const command *cmd;			/* A pointer to the command in g_cmdArr */
    operandInfo operand1;			/* The 1st operand */
    operandInfo operand2;			/* The 2nd operand */
    operandInfo operand3;			/* The 3nd operand */

} lineInfo;

/* === Second Read  === */


/* Memory Word */

typedef struct /* 32 bits */
{

    union  /* 32 bits */
    {
        /* Commands (32 bits) */
        struct
        {
            unsigned int unusedBit: 6;	/* Unused Bit */
            unsigned int funct : 5;		/* unique code for similar opcode */
            unsigned int rd : 5;
            unsigned int rt : 5;
            unsigned int rs : 5;
            unsigned int opcode : 6;    /*command code */
        } RcmdBits;
        struct
        {
            unsigned int immed : 16;    /* immediate value */
            unsigned int rt : 5;
            unsigned int rs : 5;
            unsigned int opcode : 6;    /*command code */
        } IcmdBits;

        struct
        {
            unsigned int address : 25;
            unsigned int reg : 1;/* if its label reg = 0 anf if its register reg = 1 */
            unsigned int opcode : 6;    /*command code */
        } JcmdBits;

        int value : 32 ;/*32 bits*/

    } valueBits; /* End of 32 bits union */

} memoryWord;









/* ======== Methods Declaration ======== */


/* utility.c methods */
int isStrCommand(char *cmdName);
labelInfo *getLabel(char *labelName);
void ignoreLeftSpaceStr(char **ptStr);
void trimString(char **ptStr);
char *getFirstTok(char *str, char **endOfTok);
bool isSingleWord(char *str);
bool isWhiteSpaces(char *str);
bool isLegalLabel(char *label, int lineNum, bool printErrors);
bool isExistingLabel(char *label);
bool isExistingEntryLabel(char *labelName);
bool isRegister(char *str, int *value);
bool isCommentOrEmpty(lineInfo *line);
char *getOperand(char *line, char **endOfOp, bool *foundComma);
bool isDirective(char *cmd);
bool isAscizInFormat(char **ascizStr, int lineNum);
bool isLegalNum(char *numStr, int numOfBits, int lineNum, int *value);

/* firstRead.c methods */
int firstFileRead(FILE *file, lineInfo *linesArr, int *linesFound, int *IC, int *DC);

/* secondRead.c methods */
int secondFileRead(int *memoryArr, lineInfo *linesArr, int lineNum, int IC, int DC);

/* main.c methods */
void printError(int lineNum, const char *format, ...);

