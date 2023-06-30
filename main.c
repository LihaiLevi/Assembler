

/*
This is main file.
This file manages the assembling process. 
It calls the first and second read methods, and then creates the output files that required.
*/

/* ======== Includes ======== */
#include "assembler.h"
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

/* ====== Global Data Structures ====== */

/* Labels */
labelInfo g_labelArr[MAX_LABELS_NUM];
int g_labelNum = 0;

/* Entry Lines */
lineInfo *g_entryLines[MAX_LABELS_NUM];
int g_entryLabelsNum = 0;

/* Data */
int g_dataArr[MAX_DATA_NUM];

/* ====== Methods ====== */


/* Print an error with the line number. */
void printError(int lineNum, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printf("[Error] At line %d: ", lineNum);
    vprintf(format, args);
    printf("\n");
    va_end(args);
}


/* Creates a file (for writing) from a given name and ending, and returns a pointer to it. */
FILE *openFile(char *name, char *ending, const char *mode)
{
    FILE *file;
    char *mallocStr = (char *)malloc(strlen(name) + strlen(ending) + 1);
    char *fileName = mallocStr;
    sprintf(fileName, "%s%s", name, ending);

    file = fopen(fileName, mode);
    free(mallocStr);

    return file;
}



/* Creates the .obj file, which contains the assembled lines in format */
void createObjectFile(char *name, int ICF, int DCF, int *memoryArr)
{
    int i ; /*index for print instruction*/
    int j ; /*index for print data  */
    int addressForPrint = 100 ;
    int first8Bits ;/* represent the first byte of a word */
    int second8Bits ;/* represent the second byte of a word */
    int third8bits; /* represent the third byte of a word */
    int last8Bits ;/* represent the last byte of a word */
    FILE *file;
    file = openFile(name, ".ob", "w");

    /* Print ICF and DCF */
    fprintf(file,"  %d %d   ",ICF - 100,DCF);


    /* Print all of memoryArr */

    /*loop for print instruction*/
    for (i = 0; i < ((ICF -100)/4 ) ; i++)
    {
        first8Bits = memoryArr[i] & 0xff ;
        second8Bits = (memoryArr[i] >> 8) & 0xff;
        third8bits = (memoryArr[i] >> 16) & 0xff ;
        last8Bits =   (memoryArr[i] >> 24) & 0xff;

        fprintf(file,"\n%04d ",addressForPrint);
        fprintf(file, "%02X %02X %02X %02X", first8Bits, second8Bits, third8bits,last8Bits);
        addressForPrint = addressForPrint + 4 ;
    }
    /*loop for print data in format*/
    for ( j = ((ICF -100)/4 ) ; j < +( (ICF -100)/4 )+ DCF ; j++)
    {
        if(j%4 == 1)
        {
            fprintf(file,"\n%04d ",addressForPrint);
            addressForPrint += 4; /*4 represent block of 4 in memory */
        }
        fprintf(file, "%02X ", memoryArr[j]);
    }

    fclose(file);
}


/* Creates the .ent file, which contains the addresses for the .entry labels  . */
void createEntriesFile(char *name)
{
    int i;
    FILE *file;

    /* Don't create the entries file if there aren't entry lines */
    if (!g_entryLabelsNum)
    {
        return;
    }

    file = openFile(name, ".ent", "w");

    for (i = 0; i < g_entryLabelsNum; i++)
    {
        fprintf(file, "%s\t\t", g_entryLines[i]->lineStr);
        fprintf(file,"%04d" ,getLabel(g_entryLines[i]->lineStr)->address);

        if (i != g_entryLabelsNum - 1)
        {
            fprintf(file, "\n");
        }
    }

    fclose(file);
}


/* Creates the .ext file, which contains the addresses for the extern labels operands */
void createExternFile(char *name, lineInfo *linesArr, int linesFound)
{
    int i;
    labelInfo *label;
    bool firstPrint = TRUE; /* This bool meant to prevent the creation of the file if there aren't any externals */
    FILE *file = NULL;

    for (i = 0; i < linesFound; i++)
    {
        /* Check if the 1st operand is extern label, and print it. */
        if (linesArr[i].cmd && linesArr[i].cmd->numOfParams >= 1 && linesArr[i].operand1.type == LABEL)
        {
            label = getLabel(linesArr[i].operand1.str);
            if (label && label->isExtern)
            {
                if (firstPrint)
                {
                    /* Create the file only if there is at least 1 extern */
                    file = openFile(name, ".ext", "w");
                }
                else
                {
                    fprintf(file, "\n");
                }

                fprintf(file, "%s\t\t", label->name);
                fprintf(file,"%04d" ,linesArr[i].address);
                firstPrint = FALSE;
            }
        }

        /* Check if the 2nd operand is extern label, and print it. */
        if (linesArr[i].cmd && linesArr[i].cmd->numOfParams >= 1 && linesArr[i].operand2.type == LABEL)
        {
            label = getLabel(linesArr[i].operand2.str);
            if (label && label->isExtern)
            {
                if (firstPrint)
                {
                    /* Create the file only if there is at least 1 extern */
                    file = openFile(name, ".ext", "w");
                }
                else
                {
                    fprintf(file, "\n");
                }

                fprintf(file, "%s\t\t", label->name);
                fprintf(file,"%04d" ,linesArr[i].address);
                firstPrint = FALSE;
            }
        }
        /* Check if the 3rd operand is extern label, and print it. */
        if (linesArr[i].cmd && linesArr[i].cmd->numOfParams >= 1 && linesArr[i].operand3.type == LABEL)
        {
            label = getLabel(linesArr[i].operand3.str);
            if (label && label->isExtern)
            {
                if (firstPrint)
                {
                    /* Create the file only if there is at least 1 extern */
                    file = openFile(name, ".ext", "w");
                }
                else
                {
                    fprintf(file, "\n");
                }

                fprintf(file, "%s\t\t", label->name);
                fprintf(file,"%04d" ,linesArr[i].address);
                firstPrint = FALSE;
            }
        }
    }
    if (file)
    {
        fclose(file);
    }
}

/* Resets all the globals and free all the malloc blocks. */
void clearData(lineInfo *linesArr, int linesFound, int dataCount)
{
    int i;

    /* --- Reset Globals --- */

    /* Reset global labels */
    for (i = 0; i < g_labelNum; i++)
    {
        g_labelArr[i].address = 0;
        g_labelArr[i].isData = 0;
        g_labelArr[i].isExtern = 0;
    }
    g_labelNum = 0;

    /* Reset global entry lines */
    for (i = 0; i < g_entryLabelsNum; i++)
    {
        g_entryLines[i] = NULL;
    }
    g_entryLabelsNum = 0;

    /* Reset global data */
    for (i = 0; i < dataCount; i++)
    {
        g_dataArr[i] = 0;
    }

    /* Free malloc blocks */
    for (i = 0; i < linesFound; i++)
    {
        free(linesArr[i].originalString);
    }
}

/* analysis a file, and creating the output files. */
void analysisFile(char *fileName)
{
    FILE *file = openFile(fileName, ".as", "r");
    lineInfo linesArr[MAX_LINES_NUM];
    int memoryArr[MAX_DATA_NUM] = { 0 };
    int IC = 100 ; /*Instruction Counter */
    int DC = 0 ; /* Data Counter */
    int numOfErrors = 0 ;
    int linesFound = 0;
    int ICF ;
    int DCF ;
    /* Open File */
    if (file == NULL)
    {
        printf("[Info] Can't open the file \"%s.as\".\n", fileName);
        return;
    }
    printf("[Info] Successfully opened the file \"%s.as\".\n", fileName);

    /* First Read */
    numOfErrors += firstFileRead(file, linesArr, &linesFound, &IC, &DC);
    ICF = IC ;
    DCF = DC ;

    /* Second Read */
    numOfErrors += secondFileRead(memoryArr, linesArr, linesFound, IC, DC);

    /* Create Output Files */
    if (numOfErrors == 0)
    {
        /* Create all the output files */
        createObjectFile(fileName, ICF, DCF , memoryArr);
        createExternFile(fileName, linesArr, linesFound);
        createEntriesFile(fileName);
        printf("[Info] Created output files for the file \"%s.as\".\n", fileName);
    }
    else
    {
        /* print the number of errors. */
        printf("[Info] A total of %d error%s found throughout \"%s.as\".\n", numOfErrors, (numOfErrors > 1) ? "s were" : " was", fileName);
    }

    /* Free all malloc pointers, and reset the globals. */
    clearData(linesArr, linesFound, ICF + DCF);

    /* Close File */
    fclose(file);
}

/* Main method. Calls the "analysisFile" method for each file name in argv. */
int main(int argc, char *argv[])
{
    int i;

    if (argc < 2)
    {
        printf("[Info] no file names were observed.\n");
        return 1;
    }

    for (i = 1; i < argc; i++)
    {
        analysisFile(argv[i]);
        printf("\n");
    }

    return 0;
}