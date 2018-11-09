#ifndef _BB_H_
#define _BB_H_

struct BB;

#include "common.h"

#define INSTR_TYPE_WIDE		0xC4
#define INSTR_TYPE_TABLESWITCH	0xAA
#define INSTR_TYPE_LOOKUPSWITCH	0xAB
#define INSTR_TYPE_PUSH_RAW	0xFE

typedef struct{

	UInt8 type;
	UInt8 wide;
	
	UInt32 numBytes;
	UInt8 bytes[8];

	UInt32 destLen;		//0 2 or 4 for no, short, and long pointers respectively
	UInt32 destOrigAddr;	//original address of destination
	
	struct{
	
		UInt32 first; 		//unused for lookup
		UInt32 numCases;
		UInt32* matchVals;	//unused for table
			
		UInt32* initialDestAddrs;
		
		//this gets resolved later and cannot be used by the pass function!
		Int32* destOffsets; //numCases+1, where least is the default addr
		struct BB** dests;
		
	}switches;

	//these gets resolved later and cannot be used by the pass function!
	
	Int32 finalOffset;	//resolved to be correct
	struct BB* jmpDest;	//calculated at end (final address of destination)

}Instr;



//parse code into blocks (code pointer must remain valid after this call!)
void bbInit(UInt8* code, UInt32 len);

//add exception info
void bbAddExc(UInt16 startpc, UInt16 endpc, UInt16 handler);

//final checks before optimization passes (initial code pointer guaranteed not used after this)
void bbFinishLoading();

//replacement func type
typedef void (*BbPassF)(Instr* instrs, UInt32 numInstrs, void* userData);
//do a pass over all blocks of this method
void bbPass(BbPassF pF, void* userData);

//finalization
void bbFinalizeChanges();

//export
UInt32 bbExport(UInt8* buf);	//resutn size, buf may be NULL
void bbGetExc(UInt32 idx, UInt16* startP, UInt16* endP, UInt16* handlerP);

//cleanup
void bbDestroy();



#endif
