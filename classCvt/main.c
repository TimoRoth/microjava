#include "classOptimizer.h"
#include "classAccess.h"
#include "common.h"
#include "class.h"

#include <stdio.h>
#include <stdlib.h>





//requitred stuff

#define ALLOC_SPECIAL_VAL_ZERO_SZ_CHUNK	((void*)-1)

void* natAlloc(UInt16 sz){

	return sz ? malloc(sz) : ALLOC_SPECIAL_VAL_ZERO_SZ_CHUNK;
}

void natFree(void* ptr){

	if(ptr && ptr != ALLOC_SPECIAL_VAL_ZERO_SZ_CHUNK) free(ptr);	
}

void natMemZero(void* ptr, UInt16 len){

	UInt8* p = ptr;
	
	while(len--) *p++ = 0;	
}

void err(const char* str){

	fprintf(stderr, "%s\n", str);
}


UInt16 classReadF(_UNUSED_ void* ptr){
	
	int c = getchar();

	return (c == EOF) ? CLASS_IMPORT_READ_F_FAIL : (UInt16)(UInt8)c;
}

int main(_UNUSED_ int argc, _UNUSED_ char** argv){

	JavaClass* cls;
	
	
	if(sizeof(UInt64) != 8 || sizeof(UInt32) != 4 || sizeof(UInt16) != 2 || sizeof(UInt8) != 1){
	
		fprintf(stderr, "Type megafail!\n");
		return -1;
	}
	
	cls = classImport(&classReadF, NULL);
	if(cls){	

		classDump(cls);
		classOptimize(cls);
		classDump(cls);
		classExport(cls);
		classFree(cls);
		return 0;
	}
	else{

		fprintf(stderr, "Failed to laid class\n");
		return -1;
	}
}



	



