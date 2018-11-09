#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include "common.h"
#include "uj.h"




UInt8 ujReadClassByte(UInt32 userData, UInt32 offset){
	
	int i;
	UInt8 v;
	FILE* f = (FILE*)userData;
	
	if((UInt32)ftell(f) != offset){
	
		i = fseek(f, offset, SEEK_SET);
		if(i == -1){
		
			fprintf(stderr, "Failed to seek to offset %lu, errno=%d\n", offset, errno);
			exit(-2);	
		}
	}
	
	i = fread(&v, 1, 1, f);
	if(i == -1){
	
		fprintf(stderr, "Failed to read\n");
		exit(-2);	
	}
	
	return v;
}

#ifdef UJ_LOG
	void ujLog(const char* fmtStr, ...){
	
		va_list va;
		
		va_start(va, fmtStr);
		vfprintf(stdout, fmtStr, va);
		fflush(stdout);
		va_end(va);	
	}
#endif

int main(int argc, char** argv){

	UInt32 threadH;
	Boolean done;
	Boolean remaining;
	UInt8 ret;
	struct UjClass* mainClass = NULL;
	int i;
	
	if(argc == 1){
		
		fprintf(stderr, "%s: No classes given\n", argv[0]);
		return -1;
	}
	
	ret = ujInit(NULL);
	if(ret != UJ_ERR_NONE){
		fprintf(stderr, "ujInit() fail\n");
		return -1;	
	}
	
	//load provided classes now
	
	argc--;
	argv++;
	do{
		done = false;
		remaining = false;
		for(i = 0; i < argc; i++){
		
			if(argv[i]){
				remaining = true;
				FILE* f = fopen(argv[i], "rb");
				if(!f){
					fprintf(stderr," Failed to open file\n");
					return -1;
				}
				
				ret = ujLoadClass((UInt32)f, (i == 0) ? &mainClass : NULL);
				if(ret == UJ_ERR_NONE){				//success
				
					done = true;
					argv[i] = NULL;
				}
				else if(ret == UJ_ERR_DEPENDENCY_MISSING){	//fail: we'll try again later
				
					//nothing to do here	
				}
				else{
					
					fprintf(stderr, "Failed to load class %d: %d\n", i, ret);
					exit(-4);
				}
			}
		}
	}while(done);
	
	for(i = 0; i < argc; i++) if(argv[i]){
		
		fprintf(stderr, "Completely failed to load class %d (%s)\n", i, argv[i]);
		exit(-8);
	}
	
	ret = ujInitAllClasses();
	if(ret != UJ_ERR_NONE){
		fprintf(stderr, "ujInitAllClasses() fail\n");
		return -1;	
	}
	
	//now classes are loaded, time to call the entry point
	
	threadH = ujThreadCreate(0);
	if(!threadH){
		fprintf(stderr, "ujThreadCreate() fail\n");
		return -1;	
	}
	
	i = ujThreadGoto(threadH, mainClass, "main", "()V");
	if(i == UJ_ERR_METHOD_NONEXISTENT){
	
		fprintf(stderr, "Main method not found!\n");
		exit(-9);	
	}
	while(ujCanRun()){
		
		i = ujInstr();
		if(i != UJ_ERR_NONE){
		
			fprintf(stderr, "Ret %d @ instr right before 0x%08lX\n", i, ujThreadDbgGetPc(threadH));
			exit(-10);
		}
	}
	
	return 0;
}