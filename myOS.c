#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE 100 // bytes = character
#define BUFFER_SIZE 40

// Structure for OS state
typedef struct
{
    char M[MEMORY_SIZE][4];   // Physical Memory
    char IR[4];               // Instruction Register (4 bytes)
    char R[4];                // General Purpose Register (4 bytes)
    int IC;                   // Instruction Counter Register (2 bytes) {0-99}
    int SI;                   // Interrupt {1,2,3}
    bool C;                   // Toggle (1 byte)
    char buffer[BUFFER_SIZE]; // Buffer for input/output
    FILE *inputFile;
    FILE *outputFile;
} OS;

bool loadingData = false;
int x = 0; // To iterate the memory block bytewise from 0 to 399
// To get the word no (00-99): do x / 4,
// to get byte no. (0-3) : do x % 4

OS os;

// Function declarations
void LOAD();               // Loads from input.txt to Memory
void init();               // Resets everything to " "
void startExecution();     // Initiates the execution the commands
void executeUserProgram(); // Executes the commands {Commands definition} Slave Mode
void MOS();                // Definition of GD, PD, H - Master Mode

void printMemory();
void clearBuffer();
int getIntAddress();

// Function to initialize the system (reset memory, registers, etc.)

void LOAD()
{

    do
    {
        clearBuffer();

        // fgets gets n-1 (39) chars and a \n
        fgets(os.buffer, BUFFER_SIZE + 1, os.inputFile); // Gets a line (or 40 chars+\n)
        // printf("%s|\n", os.buffer);

        if (strncmp(os.buffer, "$AMJ", strlen("$AMJ")) == 0)
        { // To create a JOB
            printf("\n\n---Initializing OS---\n");
            init();
        }
        else if (strncmp(os.buffer, "$DTA", strlen("$DTA")) == 0)
        {
            printMemory();

            printf("\n---Starting Execution---\n");
            startExecution();
        }
        else if (strncmp(os.buffer, "$END", strlen("$END")) == 0)
        {
            printf("\n---The program card is executed successfully!---\n\n");
            printf("Final state of Main Memory : \n");
            printMemory();
            os.IC = 0;
        }
        // Loading of instruction
        else
        {
            // {fetch instruction to memory}
            int k = 0; // Buffer iterator

            while (k < 40 && os.buffer[k] != '\n')
            {
                // printf("------------\n");
                for (int j = 0; j < 4 && k < 40; ++j) // when k=37 , it runs for 3 more times .. thus k=38,39,40 but k=40 not allowed since buffer[40] doesnt exists
                {
                    // printf("k = %d | x = %d | Writing : M[%d][%d] | %c\n ", k, x, x / 4, x % 4, os.buffer[k]);

                    if (os.buffer[k] == 'H')
                    {
                        os.M[x / 4][x % 4] = os.buffer[k];
                        x += 3; // leave 3 blank spaces
                        j = 4;  //(break) So that the loop does not run 3 more times as all 4 bytes are captured already
                        k++;
                    }
                    else
                    {
                        os.M[x / 4][x % 4] = os.buffer[k];
                        k++;
                    }
                    x++; // Increment the byte {Goes to new word}
                }

                // All instructions are loaded (GD10PD10H)
            }
        }

    } while (!feof(os.inputFile)); // Continue till end of file
}

void init()
{
    for (int i = 0; i < MEMORY_SIZE; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            os.M[i][j] = ' ';
        }
    }

    for (int i = 0; i < 4; i++)
    {
        os.IR[i] = ' ';
        os.R[i] = ' ';
    }

    os.C = false;
    x = 0;

    loadingData = false;
    printf("\n---OS Initialization Complete---\n");
}

void startExecution()
{
    os.IC = 0;
    executeUserProgram();
}

void executeUserProgram() // Executes each the instructions
{

    while (1)
    {

        // Load 4 byte Instruction pointed by IC into IR from Main Memory
        // IR <- M[IC]
        for (int j = 0; j < 4; j++)
        {
            os.IR[j] = os.M[os.IC][j]; // os.IC = 00
        }

        // IR = LR30

        os.IC++;

        // //decoding the instruction

        if (os.IR[0] == 'G' && os.IR[1] == 'D')
        {
            // printf("GD instruction decoding\n");
            os.SI = 1;
            MOS();
        }
        else if (os.IR[0] == 'P' && os.IR[1] == 'D')
        {
            // printf("PD instruction decoding\n");
            os.SI = 2;
            MOS();
        }
        else if (os.IR[0] == 'H')
        {
            // pri0tf("H instruction decoding\n");
            os.SI = 3;
            MOS();
            break;
        }
        else if (os.IR[0] == 'L' && os.IR[1] == 'R')
        {
            int memLoc = getIntAddress(); // 30
            for (int j = 0; j < 4; j++)
            {
                os.R[j] = os.M[memLoc][j];
            }
        }
        else if (os.IR[0] == 'S' && os.IR[1] == 'R')
        {
            int memLoc = getIntAddress();
            for (int j = 0; j < 4; j++)
            {
                os.M[memLoc][j] = os.R[j];
            }
        }
        else if (os.IR[0] == 'C' && os.IR[1] == 'R')
        {
            int memLoc = getIntAddress();
            os.C = 1;
            for (int j = 0; j < 4; j++)
            {
                if (os.M[memLoc][j] != os.R[j])
                {
                    os.C = 0;
                    break;
                }
            }
        }
        else if (os.IR[0] == 'B' && os.IR[1] == 'T')
        {
            if(os.C){

                int memLoc = getIntAddress();
                os.IC = memLoc;
            }
        }
    }
}

void MOS()
{
    // printf("In MOS\n");
    int k = 0;
    switch (os.SI)
    {
    case 1:
        // GD
        clearBuffer();
        fgets(os.buffer, BUFFER_SIZE + 1, os.inputFile); // Gets a Data line

        k = 0;

        for (int i = getIntAddress(); i < getIntAddress() + 10; i++) // i = 10,11,12...19
        {
            for (int j = 0; j < 4; j++) // j = 1,2,3,4
            {
                if (os.buffer[k] == '\n')
                {
                    return;
                }
                os.M[i][j] = os.buffer[k];
                k++;
            }
        }

        break;
    case 2:
        // PD

        clearBuffer();

        k = 0;

        for (int i = getIntAddress(); i < getIntAddress() + 10; i++) // i = 10,11,12...
        {
            for (int j = 0; j < 4; j++) // j = 1,2,3,4
            {
                os.buffer[k] = os.M[i][j];
                k++;
            }
        }
        fputs(os.buffer, os.outputFile); // Prints a Data line into Output.txt
        fputs("\n", os.outputFile);

        break;
    case 3:
        // H {Halt}

        fputs("\n\n", os.outputFile);

        break;
    default:
        break;
    }
}

void printMemory()
{
    for (int a = 0; a < 100; a++)
    {
        printf("M[%d]\t -\t |", a);

        for (int j = 0; j < 4; ++j)
        {
            printf(" %c |", os.M[a][j]);
        }
        printf("\n");
    }
}

void clearBuffer()
{
    // Clear buffer
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        os.buffer[i] = ' ';
    }
};

int getIntAddress()
{
    return (((int)os.IR[2] - 48) * 10 + (int)os.IR[3] - 48);
}

int main()
{
    os.inputFile = fopen("input.txt", "r");
    os.outputFile = fopen("output.txt", "w");

    if (os.inputFile == NULL)
    {
        printf("File Not Found\n");
        return 1;
    }

    LOAD();

    fclose(os.inputFile);
    fclose(os.outputFile);

    return 0;
}
