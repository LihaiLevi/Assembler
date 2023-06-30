
/*
 This file analysis a specific assembly code ,
 Its save the data from an assembly code file in data structure , and check for errors .
 */


/* ======== Includes ======== */
#include <ctype.h>
#include <stdlib.h>
#include "assembler.h"

/* ====== Directives List ====== */

void analysisDataDirective(lineInfo *line, int *IC, int *DC);
void analysisAscizDirective(lineInfo *line, int *IC, int *DC);
void analysisExternDirective(lineInfo *line);
void analysisEntryDirective(lineInfo *line);


const directive g_directiveArr[] =
        {	/* Name | analysis Function */
                {"dh",analysisDataDirective},
                {"dw",analysisDataDirective},
                {"db",analysisDataDirective},
                {"asciz", analysisAscizDirective } ,
                {"extern",analysisExternDirective },
                {"entry", analysisEntryDirective },
                {NULL } /* represent the end of the array */
        };

/* ====== Commands List ====== */
const command g_cmdArr[] =
        {	/* Name | typeOfCommand | funct | Opcode | numOfParams */
                { "add", 'R', 1 , 0,3 } ,
                { "sub", 'R', 2,0 ,3} ,
                { "and", 'R', 3,0,3} ,
                { "or", 'R', 4,0,3} ,
                { "nor", 'R', 5,0,3} ,
                { "move", 'R', 1,1,2} ,
                { "mvhi", 'R', 2 ,1,2} ,
                { "mvlo", 'R', 3,1,2} ,
                { "addi", 'I', 1,10,3} ,
                { "subi", 'I', 1,11,3 } ,
                { "andi", 'I', 1 ,12,3} ,
                { "ori", 'I', 1 ,13,3} ,
                { "nori", 'I', 1,14,3} ,
                { "bne", 'I', 1 ,15,3} ,
                { "beq", 'I', 0,16,3 } ,
                { "blt", 'I', 0 ,17,3} ,
                { "bgt", 'I', 0 ,18,3} ,
                { "lb", 'I', 0 ,19,3} ,
                { "sb", 'I', 0 ,20,3} ,
                { "lw", 'I', 0 ,21,3,} ,
                { "sw", 'I', 0 ,22,3} ,
                { "lh", 'I', 0 ,23,3} ,
                { "sh", 'I', 0 ,24,3} ,
                { "jmp", 'J', 0 ,30,1} ,
                { "la", 'J', 0 ,31,1} ,
                { "call", 'J', 0 ,32,1} ,
                { "stop", 'J', 0 ,63,0} ,
                { NULL } /* represent the end of the array */
        };

/* ====== Externs ====== */
extern labelInfo g_labelArr[MAX_LABELS_NUM];
extern int g_labelNum;
extern lineInfo *g_entryLines[MAX_LABELS_NUM];
extern int g_entryLabelsNum;
extern int g_dataArr[MAX_DATA_NUM];

/* ====== Methods ====== */

/* Adds the label to the labelArr and increases labelNum. Returns a pointer to the label in the array. */
labelInfo *addLabelToArr(labelInfo label, lineInfo *line)
{
    /* Check if label is legal */
    if (!isLegalLabel(line->lineStr, line->lineNum, TRUE))
    {
        /* Illegal label name */
        line->isError = TRUE;
        return NULL;
    }

    /* Check if the label is already exist */
    if (isExistingLabel(line->lineStr))
    {
        printError(line->lineNum, "Label already exists.");
        line->isError = TRUE;
        return NULL;
    }

    /* Add the name to the label */
    strcpy(label.name, line->lineStr);

    /* Add the label to g_labelArr and to the lineInfo */
    if (g_labelNum < MAX_LABELS_NUM)
    {
        g_labelArr[g_labelNum] = label;
        return &g_labelArr[g_labelNum++];
    }

    /* there is too many labels */
    printError(line->lineNum, "Too many labels - max is %d.", MAX_LABELS_NUM, TRUE);
    line->isError = TRUE;
    return NULL;
}



/* Adds the number to the g_dataArr and increases DC. Returns if it succeeded. */
bool  addNumberToData(int num, int *IC, int *DC, int lineNum,int numOfBytesParam)
{
    /* Check if there is enough space in g_dataArr for the data */
    if (*DC + *IC < MAX_DATA_NUM)
    {
        int i ;
        int startSetDfcValueArr = (*DC)+ numOfBytesParam ;
        for( i = numOfBytesParam - 1  ; i >= 0 ; i--)
        {

            g_dataArr[--startSetDfcValueArr] = num >> 8*i & 0xff ;
        }
        *DC = *DC + numOfBytesParam ;
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}



/* Adds the asciz to the g_dataArr and increases DC. Returns if it succeeded. */
bool addAscizToData(char *ascizStr, int *IC, int *DC, int lineNum)
{
    int numOfBytesEachParam = 1 ;/*as requested*/
    do
    {
        if (!addNumberToData((int)*ascizStr, IC, DC, lineNum,numOfBytesEachParam))
        {
            return FALSE;
        }
    } while (*ascizStr++);

    return TRUE;
}



/* Finds the label in line->lineStr and add it to the label list. */
/* Returns a pointer to the next char after the label, or NULL is there isn't a legal label. */
char *findLabel(lineInfo *line, int IC)
{
    char *labelEnd = strchr(line->lineStr, ':');
    labelInfo label = { 0 };
    label.address = IC;

    /* check if the func strchr find the label (or return NULL if there isn't) */
    if (!labelEnd)
    {
        return NULL;
    }
    *labelEnd = '\0';

    /* Check if the ':' came after the first word */
    if (!isSingleWord(line->lineStr))
    {
        *labelEnd = ':'; /* Fix the change in line->lineStr */
        return NULL;
    }

    /* Check if the label is legal and add it to the labelList */
    line->label = addLabelToArr(label, line);

    return labelEnd + 1; /* +1 to make it point at the next char after the \0 */
}



/* ignore the last label in labelArr by updating g_labelNum. */
/* Used to remove the label from an entry/extern line. */
void removeLastLabel(int lineNum)
{
    g_labelNum--;
    printf("[Warning] At line %d: The assembler ignored the label before the directive.\n", lineNum);
}

/* analysis a data directive (.db or .dw or.dh). */
void analysisDataDirective(lineInfo *line, int *IC, int *DC)
{
    char *operandTok = line->lineStr;
    char *endOfOp = line->lineStr;
    int operandValue;
    bool foundComma;
    int numOfBytesEachParam;
    int memoryWordLengthTemp; /*each directive has different size word  */

    /*check which data directive it is db or dw or dh*/
    bool  isDh = (* ((line ->commandStr)+1) =='h')  ? TRUE : FALSE ;
    bool  isDw = (* ((line ->commandStr)+1) =='w')  ? TRUE : FALSE ;

    /* Make the label a data label (is there is one) */
    if (line->label)
    {
        line->label->isData = TRUE;
        line->label->address = *DC;
    }

    /* Check if there are params */
    if (isWhiteSpaces(line->lineStr))
    {
        /* No parameters */
        printError(line->lineNum, "No parameter.");
        line->isError = TRUE;
        return;
    }

    /*set the memory word length temp according to the directive */
    if(isDh)
    {
        memoryWordLengthTemp = (MEMORY_WORD_LENGTH / 2) ; /*dh require 2 byte (32/2 = 16 bits ) */
        numOfBytesEachParam = 2;
    }
    else if (isDw)
    {
        memoryWordLengthTemp = MEMORY_WORD_LENGTH  ; /*dw require 4 byte (32 bits)*/
        numOfBytesEachParam = 4 ;
    }
    else/*in case its db*/
    {
       memoryWordLengthTemp = (MEMORY_WORD_LENGTH / 4 ) ; /*db require 1 byte (32/4 = 8 bits)*/
        numOfBytesEachParam = 1 ;
    }

    /* Find all the params and add them to g_dataArr */
    FOR_LOOP_UNTIL_EXIT
    {
        /* Get next param or break if there isn't */
        if (isWhiteSpaces(line->lineStr))
        {
            break;
        }
        operandTok = getOperand(line->lineStr, &endOfOp, &foundComma);

        /* Add the param to g_dataArr */
        if (isLegalNum(operandTok, memoryWordLengthTemp, line->lineNum, &operandValue))
        {

            if (!addNumberToData(operandValue, IC, DC, line->lineNum,numOfBytesEachParam))
            {
                /* Not enough memory */
                line->isError = TRUE;
                return;
            }
        }
        else
        {
            /* Illegal number */
            line->isError = TRUE;
            return;
        }

        /* Change the line to start after the parameter */
        line->lineStr = endOfOp;
    }

    if (foundComma)
    {
        /* Comma after the last param */
        printError(line->lineNum, "Do not write a comma after the last parameter.");
        line->isError = TRUE;
        return;
    }
}

/* analysis a .asciz directive. */
void analysisAscizDirective(lineInfo *line, int *IC, int *DC)
{
    /* Make the label a data label (is there is one) */
    if (line->label)
    {
        line->label->isData = TRUE;
        line->label->address = *DC;
    }

    trimString(&line->lineStr);

    if (isAscizInFormat(&line->lineStr, line->lineNum))
    {
        if (!addAscizToData(line->lineStr, IC, DC, line->lineNum))
        {
            /* Not enough memory */
            line->isError = TRUE;
            return;
        }
    }
    else
    {
        /* Illegal string */
        line->isError = TRUE;
        return;
    }
}


/* analysis .extern directive. */
void analysisExternDirective(lineInfo *line)
{
    labelInfo label = { 0 } ;
    labelInfo *labelPointer;

    /* If there is a label in the line, remove it from labelArr */
   if (line->label)
    {
        removeLastLabel(line->lineNum);
    }

    trimString(&line->lineStr);
    labelPointer = addLabelToArr(label, line);
    /* Make the label an extern label */
    if (!line->isError)
    {
        labelPointer->address = 0 ;
        labelPointer->isExtern = TRUE;
    }
}

/* analysis .entry directive. */
void analysisEntryDirective(lineInfo *line)
{
    /* If there is a label in the line, remove it from labelArr */
    if (line->label)
    {
        removeLastLabel(line->lineNum);
    }

    /* Add the label to the entry labels list */
    trimString(&line->lineStr);

    if (isLegalLabel(line->lineStr, line->lineNum, TRUE))
    {
        if (isExistingEntryLabel(line->lineStr))
        {
            printError(line->lineNum, "Label already defined as an entry label.");
            line->isError = TRUE;
        }
        else if (g_entryLabelsNum < MAX_LABELS_NUM)
        {
            g_entryLines[g_entryLabelsNum++] = line;
        }
    }
}


/* analysis the directive and in a directive line. */
void analysisDirective(lineInfo *line, int *IC, int *DC)
{
    int i = 0;
    while (g_directiveArr[i].name)
    {
        if (!strcmp(line->commandStr, g_directiveArr[i].name))
        {
            /* Call the analysis function for this type of directive */
            g_directiveArr[i].analysisFunc(line, IC, DC);
            return;
        }
        i++;
    }

    /* line->commandStr isn't a real directive */
    printError(line->lineNum, "No such directive as \"%s\".", line->commandStr);
    line->isError = TRUE;
}



/* Returns if the operands' types are legal (depending on the command). */
bool areLegalOperandTypes(const command *cmd, operandInfo op1, operandInfo op2, operandInfo op3, int lineNum)
{
    /* --- Check First Operand --- */

    /*command from type R */
    if(cmd->typeCmd == typeRcmd)
    {
        if((cmd->opcode==0)&&(cmd->funct>=1)&&(cmd->funct<=5))/*type of R that get 3 operands all must be register */
            {
                if( (op1.type != REGISTER) || (op2.type != REGISTER) || (op3.type != REGISTER))
                {
                    printError(lineNum, "The command \"%s\" the three operand must be register ",cmd->name);
                    return FALSE;
                }
            }
        else /*command from type R that opcode 1 and funct is between 1 and 3 all 2 operand must be register "*/
            {
                if( (op1.type != REGISTER) || (op2.type != REGISTER) || (op3.type != INVALID) )
                {
                    printError(lineNum, "The command \"%s\" the two operand must be register ",cmd->name);
                    return FALSE;
                }
            }
    }
    else if(cmd->typeCmd == typeIcmd)
    {
        if ((cmd->opcode >=10)&&(cmd->opcode<=14))/*type of I that get 3 operands must be 2 register and 1 number */
            {
                if( ((op1.type != REGISTER) || (op2.type != NUMBER)) || (op3.type != REGISTER))
                {
                    printError(lineNum, "The command \"%s\" must have first op is register second op is number and the third op is register",cmd->name);
                    return FALSE;
                }
            }
        else if ((cmd->opcode >=15)&&(cmd->opcode<=18)) /*command from type I that opcode between 15 and 18*/
            {
                if( ((op1.type != REGISTER) || (op2.type != REGISTER)) || (op3.type != LABEL))
                {
                    printError(lineNum, "The command \"%s\" must have first op is register second op is register and the third op is label",cmd->name);
                    return FALSE;
                }
            }
        else /*command from type I that opcode between 19 and 24 */
             {
                if( ((op1.type != REGISTER) || (op2.type != NUMBER)) || (op3.type != REGISTER))
                {
                    printError(lineNum, "The command \"%s\" must have first op is register second op is number and the third op is register",cmd->name);
                    return FALSE;
                }
            }
    }
    else/*command from type J*/
    {
        if (cmd->opcode == 30)
        {
            if ((op1.type != LABEL ) && (op1.type != REGISTER))
            {
                printError(lineNum, "The command jmp must get operand label or register only");
                return FALSE;
            }
        }
        else if (cmd->opcode == 31)
        {
            if (op1.type != LABEL)
            {
                printError(lineNum, "The command la must get operand label ");
                return FALSE;
            }
        }
        else if (cmd->opcode == 32)
        {
            if (op1.type != LABEL)
            {
                printError(lineNum, "The command call must get operand label ");
                return FALSE;
            }
        }
        else /*stop command*/
        {
            if( (op1.type != INVALID) || (op2.type != INVALID) || (op3.type != INVALID))
            {
                printError(lineNum, "The command stop dont get any operands ");
                return FALSE;
            }
        }

    }
    return TRUE ;
}


/* Updates the type and value of operand. */
void analysisOperandInfo(operandInfo *operand, int lineNum,char typeOfCommand)
{
    int value = 0;
    int numOfBitsAllowedByType ;

    if (isWhiteSpaces(operand->str))
    {
        printError(lineNum, "Empty parameter.");
        operand->type = INVALID;
        return;
    }

    /* Check if the type is NUMBER stat with a sign*/
    if ((*operand->str == '-') || (*operand->str == '+'))
    {
        /* Check if the number is legal */
        if (isspace(*operand->str))
        {
            printError(lineNum, "There is a white space after the number sign.");
            operand->type = INVALID;
        }
        else
        {
            if(typeOfCommand==typeIcmd)
            {
                numOfBitsAllowedByType = (MEMORY_WORD_LENGTH / 2 );/* there is limit of 16 bits on I command*/
            }
            else
            {
                numOfBitsAllowedByType = MEMORY_WORD_LENGTH ;
            }
            if (isLegalNum(operand->str, numOfBitsAllowedByType, lineNum, &value))
            {
                operand->type =  NUMBER ;
            }
            else
            {
                operand->type = INVALID;
            }
        }
    }
    /* Check if the type is NUMBER start without a sign*/
    else if( (((*operand->str) - '0') >= 0) && (((*operand->str) - '0') <=9))
    {
        if(typeOfCommand==typeIcmd)
        {
            numOfBitsAllowedByType = (MEMORY_WORD_LENGTH / 2 );/* there is limit of 16 bits on I command*/
        }
        else
        {
            numOfBitsAllowedByType = MEMORY_WORD_LENGTH ;
        }
        operand->type = isLegalNum(operand->str, numOfBitsAllowedByType, lineNum, &value) ? NUMBER : INVALID;
    }


    /* Check if the type is REGISTER */
    else if (isRegister(operand->str, &value))
    {
        operand->type = REGISTER;
    }
    /* Check if the type is a label */
    else if (isLegalLabel(operand->str, lineNum, FALSE))
    {
        operand->type = LABEL;
    }
        /* The type is INVALID */
    else
    {
        printError(lineNum, "\"%s\" is an invalid parameter.", operand->str);
        operand->type = INVALID;
        value = -1;
    }

    operand->value = value;
}



/* analysis the operands in a command line. */
void analysisCmdOperands(lineInfo *line,int *IC, int *DC)
{
    char *startOfNextPart = line->lineStr;
    bool foundComma = FALSE;
    int numOfOperandsFound = 0;
    char typeOfCommand = line->cmd->typeCmd ;


    /* Reset the op types */
    line->operand1.type = INVALID;
    line->operand2.type = INVALID;
    line->operand3.type = INVALID;

    /* Get the parameters */
    FOR_LOOP_UNTIL_EXIT
    {
        /* Check if there is enough memory */
        if (*IC + *DC > MAX_DATA_NUM)
        {
            line ->isError = TRUE ;
            return;
        }

        /* Check if there are still more operands to read */
        if (isWhiteSpaces(line->lineStr) || numOfOperandsFound > 3)
        {
            /* If there are more than 3 operands it's already illegal */
            break;
        }

        if (numOfOperandsFound == 0)
        {
            line->operand1.str = getOperand(line->lineStr, &startOfNextPart, &foundComma);
            analysisOperandInfo(&line->operand1, line->lineNum,typeOfCommand);
        }
        else if (numOfOperandsFound == 1)
        {
            line->operand2.str = getOperand(line->lineStr, &startOfNextPart, &foundComma);
            analysisOperandInfo(&line->operand2, line->lineNum,typeOfCommand);
        }
        else if (numOfOperandsFound == 2)
        {
            line->operand3.str = getOperand(line->lineStr, &startOfNextPart, &foundComma);
            analysisOperandInfo(&line->operand3, line->lineNum,typeOfCommand);
        }

        numOfOperandsFound++;
        line->lineStr = startOfNextPart;

    } /* End of loop */


    /* Check if there are enough operands */
    if (numOfOperandsFound != line->cmd->numOfParams)
    {
        /* There are more or less operands than needed */
        if (numOfOperandsFound <  line->cmd->numOfParams)
        {
            printError(line->lineNum, "Not enough operands.", line->commandStr);
        }
        else
        {
            printError(line->lineNum, "Too many operands.", line->commandStr);
        }

        line->isError = TRUE;
        return;
    }

    /* Check if there is a comma after the last param */
    if (foundComma)
    {
        printError(line->lineNum, "Don't write a comma after the last parameter.");
        line->isError = TRUE;
        return;
    }
    /* Check if the operands' types are legal */
    if (!areLegalOperandTypes(line->cmd, line->operand1, line->operand2, line->operand3, line->lineNum))
    {
        line->isError = TRUE;
        return;
    }
    *IC = *IC + 4 ;
}



/* analysis the command in a command line. */
void analysisCommand(lineInfo *line, int *IC, int *DC)
{
    int cmdId = isStrCommand(line->commandStr);
    if (cmdId == -1)
    {
        line->cmd = NULL;
        if (*line->commandStr == '\0')
        {
            /* The command is empty, but the line isn't empty , it's only a label. */
            printError(line->lineNum, "Can't write a label to an empty line.", line->commandStr);
        }
        else
        {
            /* Illegal command. */
            printError(line->lineNum, "No such command as \"%s\".", line->commandStr);
        }
        line->isError = TRUE;
        return;
    }

    line->cmd = &g_cmdArr[cmdId];
    analysisCmdOperands(line,IC, DC);
}



/* Returns the same string in a different part of the memory by using malloc. */
char *allocString(const char *str)
{
    char *newString = (char *)malloc(strlen(str) + 1);
    if (newString)
    {
        strcpy(newString, str);
    }

    return newString;
}



/* analysis a line, and print errors. */
void analysisLine(lineInfo *line, char *lineStr, int lineNum, int *IC, int *DC)
{
    char *startOfNextPart = lineStr;

    line->lineNum = lineNum;
    line->address = *IC;
    line->originalString = allocString(lineStr);
    line->lineStr = line->originalString;
    line->isError = FALSE;
    line->label = NULL;
    line->commandStr = NULL;
    line->cmd = NULL;

    if (!line->originalString)/*check if there is enough space */
    {
        printf("[Error] Not enough memory - malloc failed.");
        return;
    }

    /* Check if the line is a comment or empty line */
    if (isCommentOrEmpty(line))
    {
        return;
    }

    /* Find label and add it to the label list */
    startOfNextPart = findLabel(line, *IC);
    if (line->isError)
    {
        return;
    }
    /* Update the line if startOfNextPart isn't NULL */
    if (startOfNextPart)
    {
        line->lineStr = startOfNextPart;
    }

    /* Find the command token */
    line->commandStr = getFirstTok(line->lineStr, &startOfNextPart);
    line->lineStr = startOfNextPart;

    /* analysis the command / directive */
    if (isDirective(line->commandStr))
    {
        line->commandStr++; /* Remove the '.' from the command */
        analysisDirective(line, IC, DC);
    }
    else
    {
        analysisCommand(line, IC, DC);
    }

    if (line->isError)
    {
        return;
    }
}

/* Puts a line from 'file' in 'buf'. Returns if the line is shorter than maxLength. */
bool checkLineLength(FILE *file, char *buf, size_t maxLength)
{
    char *endOfLine;

    if (!fgets(buf, maxLength, file))
    {
        return FALSE;
    }

    /* Check if the line os too long (no '\n' was present). */
    endOfLine = strchr(buf, '\n');
    if (endOfLine)
    {
        *endOfLine = '\0';
    }
    else
    {
        char c;
        bool ret = (feof(file)) ? TRUE : FALSE; /* Return FALSE, unless it's the end of the file */

        /* Keep reading chars until you reach the end of the line ('\n') or EOF */
        do
        {
            c = fgetc(file);
        } while (c != '\n' && c != EOF);

        return ret;
    }
    return TRUE;
}

/* Reading the file for the first time, line by line, and analysis it. */
/* Returns how many errors were found. */
int firstFileRead(FILE *file, lineInfo *linesArr, int *linesFound, int *IC, int *DC)
{
    char lineStr[MAX_LINE_LENGTH + 2]; /* +2 for the \n and \0 at the end */
    int errorsFound = 0;
    *linesFound = 0;

    /* Read lines and analysis them */
     while (!feof(file))
    {
        if (checkLineLength(file, lineStr, MAX_LINE_LENGTH + 2))
        {
            /* Check if the file is too long */
            if (*linesFound >= MAX_LINES_NUM)
            {
                printf("[Error] File is too long. Max lines number in file is %d.\n", MAX_LINES_NUM);
                return ++errorsFound;
            }

            /* analysis a line */
            analysisLine(&linesArr[*linesFound], lineStr, *linesFound + 1, IC, DC);

            /* Update errorsFound */
            if (linesArr[*linesFound].isError)
            {
                errorsFound++;
            }

            /* Check if the number of memory words needed is small enough */
            if (*IC + *DC >= MAX_DATA_NUM)
            {
                /* dataArr is full . Stop reading the file. */
                printError(*linesFound + 1, "Too much data and code. Max memory words is %d.", MAX_DATA_NUM);
                printf("[Info] Memory is full. stop reading the file.\n");
                return ++errorsFound;
            }
            ++*linesFound;
        }
        else if (!feof(file))
        {
            /* Line is too long */
            printError(*linesFound + 1, "Line is too long. Max line length is %d.", MAX_LINE_LENGTH);
            errorsFound++;
            ++*linesFound;
        }
    }


    return errorsFound;
}