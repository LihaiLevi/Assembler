
/*
This file gets the data structures from the first file read, and convert them into bits.
*/

/* ======== Includes ======== */
#include "assembler.h"

/* ====== Externs ====== */

/* Use the commands list from firstRead.c */
extern const command g_cmdArr[];

/* Use the data from firstRead.c */
extern labelInfo g_labelArr[MAX_LABELS_NUM];
extern int g_labelNum;
extern lineInfo *g_entryLines[MAX_LABELS_NUM];
extern int g_entryLabelsNum;
extern int g_dataArr[MAX_DATA_NUM];

/* ====== Methods ====== */

/* Updates the addresses of all the data labels in g_labelArr. */
void updateDataLabelsAddress(int IC)
{
    int i;

    /* Search in the array for label with isData flag */
    for (i = 0; i < g_labelNum; i++)
    {
        if (g_labelArr[i].isData)
        {
            /* Increase the address */
            g_labelArr[i].address += IC;
        }
    }
}

/* Returns if there is an illegal entry line in g_entryLines. */
int countIllegalEntries()
{
    int i, errors = 0;
    labelInfo *label;

    for (i = 0; i < g_entryLabelsNum; i++)
    {
        label = getLabel(g_entryLines[i]->lineStr);
        if (label)
        {
            if (label->isExtern)
            {
                printError(g_entryLines[i]->lineNum, "The parameter for .entry can't be an external label.");
                errors++;
            }
        }
        else
        {
            printError(g_entryLines[i]->lineNum, "No such label as \"%s\".", g_entryLines[i]->lineStr);
            errors++;
        }
    }

    return errors;
}



/* If the op is a label, this method is updating the value of it to be the address of the label. */
/* Returns FALSE if there is an error, or TRUE otherwise. */
bool updateLableOpAddress(operandInfo *op, int lineNum)
{
    if (op->type == LABEL)
    {
            labelInfo *label = getLabel(op->str);

            /* Check if op.str is a real label name */
            if (label == NULL)
            {
                /* Print errors (legal name is illegal or not exists yet) */
                if (isLegalLabel(op->str, lineNum, TRUE))
                {
                    printError(lineNum, "No such label as \"%s\"", op->str);
                }
                return FALSE;
            }
            op->value = label->address;
    }
    return TRUE;
}

/* Returns the int value of a memory word. */
int getNumFromMemoryWord(memoryWord memory)
{
    /* Create an int of "MEMORY_WORD_LENGTH" times '1', and all the rest are '0' */
    unsigned int mask = ~0;
    mask >>= (((sizeof(int)) * BYTE_SIZE) - MEMORY_WORD_LENGTH);

    /* The mask makes sure we only use the first "MEMORY_WORD_LENGTH" bits */
    return mask & (memory.valueBits.value ) ;
}


/* Returns a memory word which represents the command in a line (according to the required instruction). */
memoryWord getCmdMemoryWord(lineInfo line)
{
    memoryWord memory = { 0 };
    if (line.cmd->typeCmd == typeRcmd)
    {
        if (line.cmd->opcode == 0)
        {
            memory.valueBits.RcmdBits.unusedBit = 0 ;
            memory.valueBits.RcmdBits.funct = line.cmd->funct ;
            memory.valueBits.RcmdBits.rd = line.operand3.value ;
            memory.valueBits.RcmdBits.rt = line.operand2.value ;
            memory.valueBits.RcmdBits.rs = line.operand1.value ;
            memory.valueBits.RcmdBits.opcode = 0 ;
        }
        else
        {
            memory.valueBits.RcmdBits.unusedBit = 0 ;
            memory.valueBits.RcmdBits.funct = line.cmd->funct ;
            memory.valueBits.RcmdBits.rd = line.operand2.value ;
            memory.valueBits.RcmdBits.rt = 0 ;
            memory.valueBits.RcmdBits.rs = line.operand1.value ;
            memory.valueBits.RcmdBits.opcode = 1 ;
        }
    }
    else if (line.cmd->typeCmd == typeIcmd)
    {
        if(line.cmd->opcode <= 14)
        {
            memory.valueBits.IcmdBits.immed = line.operand2.value ;
            memory.valueBits.IcmdBits.rt = line.operand3.value ;
            memory.valueBits.IcmdBits.rs = line.operand1.value ;
            memory.valueBits.IcmdBits.opcode = line.cmd->opcode ;

        }
        else if ((line.cmd->opcode <= 18)&&(line.cmd->opcode >= 15))
        {
            labelInfo *label = getLabel(line.operand3.str);
            memory.valueBits.IcmdBits.immed = ((label->address) - (line.address)) ;
            memory.valueBits.IcmdBits.rt = line.operand2.value ;
            memory.valueBits.IcmdBits.rs = line.operand1.value ;
            memory.valueBits.IcmdBits.opcode = line.cmd->opcode ;
        }
        else
        {
            memory.valueBits.IcmdBits.immed = line.operand2.value ;
            memory.valueBits.IcmdBits.rt = line.operand3.value ;
            memory.valueBits.IcmdBits.rs = line.operand1.value ;
            memory.valueBits.IcmdBits.opcode = line.cmd->opcode ;
        }
    }
    else /*command from type J*/
    {
        if (line.cmd->opcode == 30) /* jmp command */
            {
                if (line.operand1.type == LABEL )
                {
                    labelInfo *label = getLabel(line.operand1.str);
                    if(label->isExtern)
                    {
                        memory.valueBits.JcmdBits.address = 0 ;
                    }
                    else
                    {
                        memory.valueBits.JcmdBits.address = label->address ;
                    }
                    memory.valueBits.JcmdBits.reg = 0 ;
                    memory.valueBits.JcmdBits.opcode = line.cmd->opcode ;
                }
                else
                {
                    memory.valueBits.JcmdBits.address = line.operand1.value ;
                    memory.valueBits.JcmdBits.reg = 1 ;
                    memory.valueBits.JcmdBits.opcode = line.cmd->opcode ;
                }
            }
        else if (line.cmd->opcode == 31||line.cmd->opcode == 32)/*la or call command*/
                {
                    labelInfo *label = getLabel(line.operand1.str);
                    if(label->isExtern)
                    {
                        memory.valueBits.JcmdBits.address = 0 ;
                        memory.valueBits.JcmdBits.opcode = line.cmd->opcode ;
                    }
                    else
                    {
                        memory.valueBits.JcmdBits.address = label->address ;
                        memory.valueBits.JcmdBits.opcode = line.cmd->opcode ;
                    }
                    memory.valueBits.JcmdBits.reg = 0 ;
                }
         else   /*stop command*/
                 {
                     memory.valueBits.JcmdBits.address = 0 ;
                     memory.valueBits.JcmdBits.reg = 0 ;
                     memory.valueBits.JcmdBits.opcode = line.cmd->opcode ;
                 }
    }

    return memory;
}


/* Adds the value of memory word to the memoryArr, and increase the memory counter. */
void addWordToMemory(int *memoryArr, int *memoryCounter, memoryWord memory)
{
    /* Check if memoryArr isn't full yet */
    if (*memoryCounter < MAX_DATA_NUM)
    {
        /* Add the memory word and increase memoryCounter */
        memoryArr[(*memoryCounter)++] = getNumFromMemoryWord(memory);
    }
}


/* Adds a whole line into the memoryArr, and increase the memory counter. */
bool addLineToMemory(int *memoryArr, int *memoryCounter, lineInfo *line)
 {
    bool foundError = FALSE;

    /* Don't do anything if the line is error or if it's not a command line */
    if (!line->isError && line->cmd != NULL )
    {
        /* Update the label operands value */
        if (!updateLableOpAddress(&line->operand1, line->lineNum) ||
              !updateLableOpAddress(&line->operand2, line->lineNum)||
                !updateLableOpAddress(&line->operand3, line->lineNum))
        {
            line->isError = TRUE;
            foundError = TRUE;
        }

        /* Add the word to the memory */
        addWordToMemory(memoryArr, memoryCounter, getCmdMemoryWord(*line));
    }
    return !foundError;

}



/* Adds the data from g_dataArr to the end of memoryArr. */
void addDataToMemory(int *memoryArr, int *memoryCounter, int DC)
{
    int i;
    /* Create an int of "MEMORY_WORD_LENGTH" times '1', and all the rest are '0' */
    unsigned int mask = ~0;
    mask >>= (sizeof(int) * BYTE_SIZE - MEMORY_WORD_LENGTH);

    /* Add each int from g_dataArr to the end of memoryArr */
    for (i = 0; i < DC; i++)
    {
        if (*memoryCounter < MAX_DATA_NUM)
        {
            /* The mask makes sure we only use the first "MEMORY_WORD_LENGTH" bits */
            memoryArr[(*memoryCounter)++] = mask & g_dataArr[i];
        }
        else
        {
            /* No more space in memoryArr */
            return;
        }
    }
}

/* Reads the data from the first read for the second time. */
/* It converts all the lines into the memory. */
int secondFileRead(int *memoryArr, lineInfo *linesArr, int lineNum, int IC, int DC)
{
    int errorsFound = 0;
    int memoryCounter = 0 ;
    int i;

    /* Update the data labels */
    updateDataLabelsAddress(IC);

    /* Check if there are illegal entries */
    errorsFound += countIllegalEntries();

    /* Add each line in linesArr to the memoryArr */
    for (i = 0; i < lineNum; i++)
    {
        if (!addLineToMemory(memoryArr, &memoryCounter, &linesArr[i]))
        {
            /* An error was found while adding the line to the memory */
            errorsFound++;
        }
    }

    /* Add the data from g_dataArr to the end of memoryArr */
    addDataToMemory(memoryArr, &memoryCounter, DC);

    return errorsFound;
}