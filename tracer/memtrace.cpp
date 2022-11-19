#include "pin.H"

#include "filter.H"
#include "instlib.H"

#include <cstdio>
#include <stdio.h>

INSTLIB::FILTER filter;

FILE *file;

// Print a memory read record
VOID outputRead(VOID *ip, VOID *addr) {
  fprintf(file, "%lu\n", (long unsigned int)addr);
}

// Print a memory write record
VOID outputWrite(VOID *ip, VOID *addr) {
  fprintf(file, "%lu\n", (long unsigned int)addr);
}

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID *v) {
  UINT32 memOperands = INS_MemoryOperandCount(ins);

  for (UINT32 memOp = 0; memOp < memOperands; memOp++) {
    if (INS_MemoryOperandIsRead(ins, memOp)) {
      INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)outputRead,
                               IARG_INST_PTR, IARG_MEMORYOP_EA, memOp,
                               IARG_END);
    }
    if (INS_MemoryOperandIsWritten(ins, memOp)) {
      INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)outputWrite,
                               IARG_INST_PTR, IARG_MEMORYOP_EA, memOp,
                               IARG_END);
    }
  }
}

VOID Trace(TRACE trace, VOID *v) {
  if (!filter.SelectTrace(trace))
    return;

  for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
    for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins)) {
      Instruction(ins, 0);
    }
  }
}

VOID Fini(INT32 code, VOID *v) { fclose(file); }

INT32 Usage() {
  PIN_ERROR("This Pintool prints a trace of memory addresses\n" +
            KNOB_BASE::StringKnobSummary() + "\n");
  return -1;
}

int main(int argc, char *argv[]) {
  if (PIN_Init(argc, argv))
    return Usage();

  file = stdout;

  filter.Activate();
  TRACE_AddInstrumentFunction(Trace, 0);
  PIN_AddFiniFunction(Fini, 0);

  // Never returns
  PIN_StartProgram();

  return 0;
}
