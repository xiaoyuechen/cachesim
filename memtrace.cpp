#include <stdio.h>
#include "pin.H"

#define WARMUP 1000000000
#define RUN 2000000000

FILE* file;
unsigned long int insts_count;
unsigned long int accesses;

// Print a memory read record
VOID outputRead(VOID* ip, VOID* addr) { 
  accesses++;
//  if(accesses>WARMUP){
    fprintf(file, "%lu\n", (long unsigned int) addr); 
//  }
//  if(accesses>RUN){
//    exit(0);
//  }
//  fprintf(file, "accesses: %lu\n", (long unsigned int) accesses); 
}

// Print a memory write record
VOID outputWrite(VOID* ip, VOID* addr) { 
  accesses++;
//  if(accesses>WARMUP){
    fprintf(file, "%lu\n", (long unsigned int) addr); 
//  }
//  if(accesses>RUN){
    exit(0);
//  }
}

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID* v)
{
  insts_count++;  
  UINT32 memOperands = INS_MemoryOperandCount(ins);
  
  for(UINT32 memOp = 0; memOp < memOperands; memOp++){
    if(INS_MemoryOperandIsRead(ins, memOp)){
      INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)outputRead, IARG_INST_PTR, IARG_MEMORYOP_EA, memOp, IARG_END);
    }
    if(INS_MemoryOperandIsWritten(ins, memOp)){
      INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)outputWrite, IARG_INST_PTR, IARG_MEMORYOP_EA, memOp, IARG_END);
    }

  }
}

VOID Fini(INT32 code, VOID* v)
{
//    fprintf(file, "#eof\n");
    fclose(file);
}
INT32 Usage()
{
    PIN_ERROR("This Pintool prints a trace of memory addresses\n" + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv)) return Usage();
    insts_count=0;
    accesses=0;

    file = fopen("memory_trace_sphinx", "w");

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
