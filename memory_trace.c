#include <stdio.h>
#include "pin.H"

// Print a memory read record
VOID outputRead(VOID* ip, VOID* addr) { fprintf(file, "%p\n", addr); }

// Print a memory write record
VOID outputWrite(VOID* ip, VOID* addr) { fprintf(file, "%p\n", addr); }

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID* v)
{
    UINT32 memOperands = INS_MemoryOperandCount(ins);
    
    for(UINT32 memOp = 0; memOp < memOperands; memOp++){
      if(INS_MemoryOperandIsRead(inst, memOp)){
        INST_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)outputRead, IARG_INST_PTR, IARG_MEMORYOP_EA, memOp, IARG_END);
      }
      if (INS_MemoryOperandIsWritten(ins, memOp)){
          INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)outputWrite, IARG_INST_PTR, IARG_MEMORYOP_EA, memOp, IARG_END);
      }

    }
}

VOID Fini(INT32 code, VOID* v)
{
    fprintf(file, "#eof\n");
    fclose(file);
}

int main(int argc, char* argv[])
{

    file = fopen("memory_trace", "w");

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
