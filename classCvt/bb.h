#ifndef _BB_H_
#define _BB_H_

struct BB;

#include "common.h"

#define INSTR_TYPE_WIDE		0xC4
#define INSTR_TYPE_TABLESWITCH	0xAA
#define INSTR_TYPE_LOOKUPSWITCH	0xAB
#define INSTR_TYPE_PUSH_RAW	0xFE

typedef struct{

	uint8_t type;
	uint8_t wide;

	uint32_t numBytes;
	uint8_t bytes[8];

	uint32_t destLen;		//0 2 or 4 for no, short, and long pointers respectively
	uint32_t destOrigAddr;	//original address of destination

	struct{

		uint32_t first; 		//unused for lookup
		uint32_t numCases;
		uint32_t* matchVals;	//unused for table

		uint32_t* initialDestAddrs;

		//this gets resolved later and cannot be used by the pass function!
		int32_t* destOffsets; //numCases+1, where least is the default addr
		struct BB** dests;

	}switches;

	//these gets resolved later and cannot be used by the pass function!

	int32_t finalOffset;	//resolved to be correct
	struct BB* jmpDest;	//calculated at end (final address of destination)

}Instr;



//parse code into blocks (code pointer must remain valid after this call!)
void bbInit(uint8_t* code, uint32_t len);

//add exception info
void bbAddExc(uint16_t startpc, uint16_t endpc, uint16_t handler);

//final checks before optimization passes (initial code pointer guaranteed not used after this)
void bbFinishLoading();

//replacement func type
typedef void (*BbPassF)(Instr* instrs, uint32_t numInstrs, void* userData);
//do a pass over all blocks of this method
void bbPass(BbPassF pF, void* userData);

//finalization
void bbFinalizeChanges();

//export
uint32_t bbExport(uint8_t* buf);	//resutn size, buf may be NULL
void bbGetExc(uint32_t idx, uint16_t* startP, uint16_t* endP, uint16_t* handlerP);

//cleanup
void bbDestroy();



#endif
