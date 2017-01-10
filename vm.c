/*Jason Wheeler
  PID: j3051796
  NID: ja599505
  Date: 9/25/2015
  Program: vm.c
  Systems Software
  Assignment #1
*/

#include<stdio.h>
#include<stdlib.h>

//Constants
#define MAX_STACK_HEIGHT 2000
#define MAX_CODE_LENGTH 500
#define MAX_LEXI_LEVELS 3


//Globals

typedef struct instruction {
        int Line;
        int OP;
        int L;
        int M;
        struct instruction *next;
} Instruction;

//Opcodes
enum opCodes{
  LIT = 1,
  OPR = 2,
  LOD = 3,
  STO = 4,
  CAL = 5,
  INC = 6,
  JMP = 7,
  JPC = 8,
  SIO_1 = 9,
  SIO_2 = 10,
  SIO_3 = 11
};

//ALU operations 
enum operators{
  RET,
  NEG,
  ADD,
  SUB,
  MUL,
  DIV,
  ODD,
  MOD,
  EQL,
  NEQ,
  LSS,
  LEQ,
  GTR,
  GEQ
};

//function declaration
Instruction ** ReadFile(char * fileName);
Instruction * NewInstruction(int line,int op, int l, int m);
void PrintStore(Instruction ** store);
void ExecStore(Instruction ** store, int pc, int bp, int ir, int sp);
char * PrintInstructionType(int type);
void PrintStack(int line, int op, int l, int m, int pc, int bp, int sp, int * stack);
int Base(int level, int b, int *s);
void PrintStackLabels(int pc, int bp, int sp);
void sio(int *stack, int sp);

//Main function
int main (){

    //Initialize variables
    int pc = 0;
    int bp = 1;//Base pointer
    int ir = 0;//Instruction register
    int sp = 0;//Stack pointer
    Instruction ** codeStore = NULL;
    
    //Read file and initialize codeStore with the store returned from ReadFile
    codeStore = ReadFile("mcode.txt");

    //Print instructions to stacktrace.txt
    PrintStore(codeStore);

    //Execute instructions
    ExecStore(codeStore, pc, bp, ir, sp);

    return 0;
}

//Reads the instructions from the file while storing them in a code store then returns the code store back to main
Instruction ** ReadFile(char * fileName){
   //Initialize file pointer
    FILE * fileInput = fopen(fileName, "r");

    //Allocate space for code store
    Instruction ** store = malloc(sizeof(Instruction) * MAX_CODE_LENGTH);
    //Number of instructions in the code store
    int numInst = 0;
    
    //Temp holding vars
    int op, l, m;

    if(fileInput != NULL)
        //Scan in the instructions
        while(fscanf(fileInput, "%d %d %d", &op, &l, &m) != EOF){
            store[numInst] = NewInstruction(numInst, op, l, m);
            numInst++;
        }

    else{
        printf("Error opening file.\n");
        exit(1);
    }

    fclose(fileInput);

    return store;
}

//Creates a new struct Instruction 
Instruction * NewInstruction(int line, int op, int l, int m){

    Instruction *newInstruction = (Instruction*) malloc(sizeof(Instruction));

    newInstruction -> Line = line;
    newInstruction -> OP = op;
    newInstruction -> L = l;
    newInstruction -> M = m;
    newInstruction -> next = NULL;

  return newInstruction;
}

//Prints all instructions read from file
void PrintStore(Instruction ** store){
    Instruction ** tempStore = NULL;
    tempStore = store;
    int i = 0;

    printf("Line\t OP\t L\t M\n");
    for(i = 0; tempStore[i] != NULL ; i++){
      printf(" %d\t %s\t %d\t %d\n", i
      , PrintInstructionType(tempStore[i]->OP), tempStore[i]->L
      ,tempStore[i]->M);

    }
    printf("\n");

    free(tempStore);
}

//Executes the instruction given by the store
//performs IO and stack operations
void ExecStore (Instruction ** store, int pc, int bp, int ir, int sp){

    Instruction ** tempInstructionStore = NULL;
    tempInstructionStore = store;
    int stack[MAX_STACK_HEIGHT] = {0};
    int i = 0;
    Instruction * currentInstruction = NULL;

    //Prints initial labels
    PrintStackLabels(pc, bp, sp);

    while(bp > 0){

      printf("%d %s %d %d", pc, PrintInstructionType(tempInstructionStore[pc]->OP), tempInstructionStore[pc]->L, tempInstructionStore[pc]->M);

      currentInstruction = tempInstructionStore[pc];

      pc++;

      //Big switch to perform all operations
      switch(currentInstruction->OP){
        case LIT:
            sp++;
            stack[sp] = currentInstruction->M;
            break;

        case OPR:
            if(currentInstruction->M == RET){
              sp = bp - 1;
              pc = stack[sp + 4];
              bp = stack[sp + 3];
            }
            else if (currentInstruction->M == NEG){
              stack[sp] = -stack[sp];
            }
            else if(currentInstruction->M == ADD){
              sp--;
              stack[sp] = stack[sp] + stack[sp + 1];
            }
            else if(currentInstruction->M == SUB){
              sp--;
              stack[sp] = stack[sp] - stack[sp + 1];
            }
            else if (currentInstruction->M == MUL){
              sp--;
              stack[sp] = (stack[sp]) * (stack[sp + 1]);
            }
            else if (currentInstruction->M == DIV){
              sp--;
              stack[sp] = (stack[sp]) / (stack[sp + 1]);
            }
            else if (currentInstruction->M == ODD){
              stack[sp] = stack[sp] % 2;
            }
            else if (currentInstruction->M == MOD){
              sp--;
              stack[sp] = (stack[sp]) % (stack[sp + 1]);
            }
            else if (currentInstruction->M == EQL){
              sp--;
              stack[sp] = (stack[sp]) == (stack[sp + 1]);
            }
            else if (currentInstruction->M == NEQ){
              sp--;
              stack[sp] = (stack[sp]) != (stack[sp + 1]);
            }
            else if (currentInstruction->M == LSS){
              sp--;
              stack[sp] = (stack[sp]) < (stack[sp + 1]);
            }
            else if (currentInstruction->M == LEQ){
              sp--;
              stack[sp] = (stack[sp]) <= (stack[sp + 1]);
            }
            else if (currentInstruction->M == GTR){
              sp--;
              stack[sp] = (stack[sp]) > (stack[sp + 1]);
            }
            else if (currentInstruction->M == GEQ){
              sp--;
              stack[sp] = (stack[sp]) >= (stack[sp + 1]);
            }
            break;

        case LOD:
            sp++;
            stack[sp] = stack[Base(currentInstruction->L, bp, stack) + currentInstruction->M];
            break;

        case STO:
            stack[Base(currentInstruction->L, bp, stack) + currentInstruction->M] = stack[sp];
            sp--;
            break;

        case CAL:
            stack[sp + 1 ] = 0;
            stack[sp + 2] = Base(currentInstruction->L,bp, stack);
            stack[sp + 3] = bp;
            stack[sp + 4] = pc;
            bp = sp + 1;
            pc = currentInstruction->M;
            break;

        case INC:
            sp = sp + currentInstruction->M;
            break;

        case JMP:
            pc = currentInstruction->M;
            break;

        case JPC:
            if(stack[sp] == 0){
              pc = currentInstruction->M;
            }
            sp--;
            break;

        case SIO_1:
            printf("%d\n", stack[sp]);
            sp--;
            break;

        case SIO_2:
            sp++;
            sio(stack, sp);
            break;

        case SIO_3:
            exit(0);
            break;

        default: 
            printf("\nError occured on execution\n");
            exit(1);
            break;
      }//End switch statement

      PrintStack(tempInstructionStore[pc] -> Line,
      tempInstructionStore[pc] -> OP, tempInstructionStore[pc] -> L,
      tempInstructionStore[pc] -> M, pc, bp, sp, stack);

      free(currentInstruction);
    }

    return;
}

//prints initial values of pc, bp, and sp
void PrintStackLabels(int pc, int bp, int sp){
      printf("\t\t\t\t\tpc\tbp\tsp\tstack\n");
      //initial values of pc bp sp ...
      printf("Initial Values\n");
      printf("\t\t\t\t\t%d\t%d\t%d\n", pc, bp, sp);
      printf("\n");
}

//Returns the base of the stack, L levels down
int Base(int level, int b, int *s) {

  while (level > 0){
    b = s[b + 2];
    level--;
  }

  return b;
}

//Given an enum type, print the instruction opcode name
char * PrintInstructionType (int type){

    switch(type){
      case LIT: return "lit";
      case OPR: return "opr";
      case LOD: return "lod";
      case STO: return "sto";
      case CAL: return "cal";
      case INC: return "inc";
      case JMP: return "jmp";
      case JPC: return "jpc";
      case SIO_1: return "sio";
      case SIO_2: return "sio";
      case SIO_3: return "sio";
      default: return "NULL";
    }
}

void PrintStack(int line, int op, int l, int m, int pc, int bp, int sp, int * stack){
    int i = 0;
    int bpOffset = 0;

    printf("\t\t\t\t%d\t%d\t%d", pc, bp, sp);

    int *tempStack = stack;
    printf("\t");
    //print stacks
    if(sp!= 0){
      for(i = 0; i <= sp; i++){

        //check for activation records
        if(bp > 1){
          bpOffset = bp - i;
          //prints bar if diff == 0, handles all cases
          if(bpOffset == 0)
            printf("| ");
        }

        printf("%d ", tempStack[i]);
      }
    }

    printf("\n");
}

//Asks user for input and pushes that on the stack 
void sio(int *stack, int sp){
    int val = 0;
    printf("Enter value to push onto stack:\n");
    scanf("%d", &val);
    stack[sp] = val;
}