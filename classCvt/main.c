#include "classOptimizer.h"
#include "classAccess.h"
#include "common.h"
#include "class.h"

#include <stdio.h>
#include <stdlib.h>





//requitred stuff

#define ALLOC_SPECIAL_VAL_ZERO_SZ_CHUNK	((void*)-1)

void* natAlloc(uint16_t sz){

	return sz ? malloc(sz) : ALLOC_SPECIAL_VAL_ZERO_SZ_CHUNK;
}

void natFree(void* ptr){

	if(ptr && ptr != ALLOC_SPECIAL_VAL_ZERO_SZ_CHUNK) free(ptr);
}

void natMemZero(void* ptr, uint16_t len){

	uint8_t* p = ptr;

	while(len--) *p++ = 0;
}

void err(const char* str){

	fprintf(stderr, "%s\n", str);
}


uint16_t classReadF(_UNUSED_ void* ptr){

	int c = getchar();

	return (c == EOF) ? CLASS_IMPORT_READ_F_FAIL : (uint16_t)(uint8_t)c;
}

int main(_UNUSED_ int argc, _UNUSED_ char** argv){

	JavaClass* cls;


	if(sizeof(uint64_t) != 8 || sizeof(uint32_t) != 4 || sizeof(uint16_t) != 2 || sizeof(uint8_t) != 1){

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







