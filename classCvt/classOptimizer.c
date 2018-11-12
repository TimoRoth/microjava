#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "classOptimizer.h"
#include "classAccess.h"
#include "bb.h"



static void dumpcode(const char* name, const uint8_t* code, uint32_t codeLen,
		JavaExceptionTableEntry* excs, uint32_t numExcs){

	uint32_t i;
	
	fprintf(stderr, "%s:", name);
	
	for(i = 0; i < codeLen; i++){
		
		if(!(i % 16)) fprintf(stderr, "\n%04" PRIX32, i);
		fprintf(stderr," %02X", code[i]);
	}
	fprintf(stderr, "\n");
	if(numExcs){
		
		fprintf(stderr, "EXCs(%" PRIu32 "):\n", numExcs);
		while(numExcs--){
		
			fprintf(stderr, "  type_%05u %04X-%04X -> %04X\n", excs->catchType, excs->start_pc, excs->end_pc, excs->handler_pc);
			excs++;	
		}
	}
	fprintf(stderr, "\n");
}


bool classOptimizerPrvHandleLocals(JavaClass* c, bool isField, uint8_t instr, uint16_t idx, uint8_t* dstP){	///aaa

	static const char* invokeType[] = {"virtual", "special", "static", "interface", "dynamic"};
	uint16_t clsIdx, nameTypeIdx, nameIdx, typeIdx, num;
	JavaMethodOrField** pp;
	uint16_t* t;
	
	
	t = (uint16_t*)(c->constantPool[idx - 1] + 1);
	clsIdx = t[0];
	nameTypeIdx = t[1];
	t = (uint16_t*)(c->constantPool[nameTypeIdx - 1] + 1);
	nameIdx = t[0];
	typeIdx = t[1];


	if(clsIdx == c->thisClass){	//this field access
	
		uint16_t i;
		
		if(isField){
			
			num = c->numFields;
			pp = c->fields;
		}
		else{
			
			num = c->numMethods;
			pp = c->methods;
		}
		
		
		for(i = 0; i < num; i++){
			
			if(pp[i]->nameIdx == nameIdx && pp[i]->descrIdx == typeIdx) break;
		}
		
		if(i == num){
		
			fprintf(stderr, "failed to find field in local class\n");
			exit (-6);	
		}

		if(isField){
			
			dstP[0] = pp[i]->type;
			dstP[1] = pp[i]->offset >> 8;
			dstP[2] = pp[i]->offset;
		}
		else{

			JavaString* js;
			uint16_t len;
			const char* cp;
			bool inArray = false;
			uint16_t nParams = 0;
			
			js = (JavaString*)(c->constantPool[typeIdx - 1] + 1);
			len = js->len;
			cp = js->data;

			if(len < 3){
			
				fprintf(stderr, "invalid type in method call (len=%d)\n", len);
				exit(-6);
			}
			if(*cp++ != '('){
			
				fprintf(stderr, "not open parens in first byte of func type\n");
				exit(-6);
			}
			while(*cp != ')'){
			
				switch (*cp++){
					
					case JAVA_TYPE_DOUBLE:
					case JAVA_TYPE_LONG:
						
						if(inArray) inArray = false;
						else nParams += 2;
						break;
					
					case JAVA_TYPE_ARRAY:
						
						nParams++;
						inArray = true;
						break;
					
					case JAVA_TYPE_OBJ:
						
						while(*cp++ != ';');
						//fallthrough
					
					default:
					
						if(inArray) inArray = false;
						else nParams++;
						break;
				}
			}		
						
			if(!(pp[i]->accessFlags & ACCESS_FLAG_STATIC)) nParams++;
		
			if(nParams >= 255) return false;	//too many param slots to encode in a byte
		
			dstP[0] = nParams + 1;
			dstP[1] = i >> 8;
			dstP[2] = i;
		}
		
		if(DEBUG) fprintf(stderr, "%s%s %d (ref is %d.%d.%d)\n",
				isField ? ((instr == 0xB2 || instr == 0xB4) ? "get" : "put") : "invoke",
				isField ? ((instr == 0xB2 || instr == 0xB3) ? "static" : "field") : invokeType[instr - 0xB6],
				idx, clsIdx, nameIdx, typeIdx);
		
		return true;
	}
	return false;
}

static void bbOptimizationPassF(Instr* instrs, uint32_t numInstr, void *userData){

	uint32_t i;
	uint16_t idx;
	Instr* instr = instrs;
	JavaClass* c = (JavaClass*)userData;
	
	for(i = 0; i < numInstr; i++, instr++){
		
		idx = 0;
		switch(instr->type){
		
			case 0x12:			//ldc
			
				idx = instr->bytes[0];
				goto handle_ldc;
			
			case 0x13:			//ldc_w
			
				idx = instr->bytes[0];
				idx <<= 8;
				idx += instr->bytes[1];
		handle_ldc:
				if(c->constantPool[idx - 1]->type != JAVA_CONST_TYPE_STR_REF){
					
					uint32_t d = *(uint32_t*)(c->constantPool[idx - 1] + 1);
					
					instr->type = INSTR_TYPE_PUSH_RAW;
					instr->numBytes = 4;
					instr->bytes[0] = d >> 24;
					instr->bytes[1] = d >> 16;
					instr->bytes[2] = d >> 8;
					instr->bytes[3] = d;
				
					if(DEBUG) fprintf(stderr, "LDC%s %d (val is 0x%08" PRIu32 ")\n", (instr->type == 0x12) ? "" : "_w", idx, d);
				}
				break;
				
		
			case 0xB2:			//getstatic
			case 0xB3:			//putstatic
			case 0xB4:			//getfield
			case 0xB5:			//putfield
			
				idx = instr->bytes[0];
				idx <<= 8;
				idx += instr->bytes[1];
		
				if(classOptimizerPrvHandleLocals(c, true, instr->type, idx, instr->bytes)){
				
					instr->wide = true;
					instr->numBytes = 3;
				}
				break;
			
			case 0xB7:			//invokespecial
			case 0xB8:			//invokestatic
				
				idx = instr->bytes[0];
				idx <<= 8;
				idx += instr->bytes[1];
				
				if(classOptimizerPrvHandleLocals(c, false, instr->type, idx, instr->bytes)){
					
					instr->wide = true;
					instr->numBytes = 3;
				}
				break;
		}
	}
}

static void bbMarkConstantsPassF(Instr* instrs, uint32_t numInstr, void *userData){

	uint32_t i;
	uint16_t idx;
	Instr* instr = instrs;
	JavaClass* c = (JavaClass*)userData;
	
	for(i = 0; i < numInstr; i++, instr++){
		
		idx = 0;
		switch(instr->type){
		
			case 0x12:			//ldc

				idx = instr->bytes[0];
				break;

			case 0xB2:			//getstatic
			case 0xB3:			//putstatic
			case 0xB4:			//getfield
			case 0xB5:			//putfield
			case 0xB7:			//invokespecial
			case 0xB8:			//invokestatic
			
				if(instr->wide) break;	//no actual references are used in the optimized version(s)
				//fallthrough
				
			case 0x13:			//ldc_w
			case 0x14:			//ldc2_w
			case 0xB6:			//invokevirtual
			case 0xB9:			//invokeinterface
			case 0xBA:			//invokedynamic
			case 0xBB:			//new
			case 0xBD:			//anewarray
			case 0xC0:			//checkcast
			case 0xC1:			//instanceof
			case 0xC5:			//multinewarray

		
				idx = instr->bytes[0];
				idx <<= 8;
				idx += instr->bytes[1];
				break;
		}
		if(idx){
		
			c->constantPool[idx - 1]->used = true;
			c->constantPool[idx - 1]->directUsed = 1;
		}
	}
}

static void bbFixConstantsPassF(Instr* instrs, uint32_t numInstr, void *userData){

	uint16_t* constantFMap = userData;
	Instr* instr = instrs;
	uint16_t idx;
	uint32_t i;
	
	for(i = 0; i < numInstr; i++, instr++){
		
		idx = 0;
		switch(instr->type){
		
			case 0x12:			//ldc

				instr->bytes[0] = constantFMap[instr->bytes[0]];
				break;

			case 0xB2:			//getstatic
			case 0xB3:			//putstatic
			case 0xB4:			//getfield
			case 0xB5:			//putfield
			case 0xB7:			//invokespecial
			case 0xB8:			//invokestatic
			
				if(instr->wide) break;	//no actual references are used in the optimized version(s)
				//fallthrough
				
			case 0x13:			//ldc_w
			case 0x14:			//ldc2_w
			case 0xB6:			//invokevirtual
			case 0xB9:			//invokeinterface
			case 0xBA:			//invokedynamic
			case 0xBB:			//new
			case 0xBD:			//anewarray
			case 0xC0:			//checkcast
			case 0xC1:			//instanceof
			case 0xC5:			//multinewarray

		
				idx = instr->bytes[0];
				idx <<= 8;
				idx += instr->bytes[1];
				
				idx = constantFMap[idx];
				
				instr->bytes[0] = idx >> 8;
				instr->bytes[1] = idx;
				break;
		}
	}	
}

static void classOptPrvRemapAttrs(JavaAttribute** attributes,uint16_t numAttr, const uint16_t* constantFMap){
	
	uint16_t j;
	
	for(j = 0; j < numAttr; j++){
		
		attributes[j]->nameIdx = constantFMap[attributes[j]->nameIdx];
		
		if(attributes[j]->type == J_ATTR_TYPE_CODE){
			
			classOptPrvRemapAttrs(attributes[j]->data.code.attributes, attributes[j]->data.code.numAttributes, constantFMap);
		}
	}
}

void classOptimize(JavaClass* c){

	JavaMethodOrField* m;
	JavaAttribute* attrib;
	uint16_t i, j, k, clsOfst = 0, instOfst = 0, newNumConsts, newDirectConsts = 1;
	uint16_t* constantFMap = NULL;
	uint16_t* constantRMap = NULL;
	bool codeFound;
	uint8_t sz;

	//calculate fields' offsets
	{
		for(i = 0; i < c->numFields; i++){
	
			JavaString* s;
			uint16_t* ofstP;
	
			ofstP = (c->fields[i]->accessFlags & ACCESS_FLAG_STATIC) ? &clsOfst : &instOfst;
			
			s = (JavaString*)(c->constantPool[c->fields[i]->descrIdx - 1] + 1);
			
			switch(s->data[0]){
				
				case JAVA_TYPE_BYTE:
				case JAVA_TYPE_BOOL:
				
					sz = 1;
					break;
				
				case JAVA_TYPE_CHAR:
				case JAVA_TYPE_SHORT:
					
					sz = 2;
					break;
				
				case JAVA_TYPE_DOUBLE:
				case JAVA_TYPE_LONG:
					
					sz = 8;
					break;
				
				default:
					
					sz = 4;
					break;
			}
			
			c->fields[i]->offset = *ofstP;
			c->fields[i]->type = s->data[0];
			
			(*ofstP) += sz;
		}
	}
	
	//perform code optimization pass(es) and mark all used constants
	{
		for(i = 0; i < c->numMethods; i++){
	
			m = c->methods[i];
			codeFound = false;
	
	
			for(j = 0; j < m->numAttr; j++){
	
				attrib = m->attributes[j];
				if(attrib->type == J_ATTR_TYPE_CODE){
	
					uint16_t idx;
					JavaAttribute* newCode;
					uint32_t len;
						
					if(codeFound){
						fprintf(stderr, "Multiple code attributes not supported in a single method\n");
						exit(-2);
					}
					codeFound = true;
					
					if(DEBUG) dumpcode("BEFORE", attrib->data.code.code, attrib->data.code.codeLen,
							attrib->data.code.exceptions, attrib->data.code.numExceptions);
					
					bbInit(attrib->data.code.code, attrib->data.code.codeLen);
					
					for(k = 0; k < attrib->data.code.numExceptions; k++){
						
						bbAddExc(attrib->data.code.exceptions[k].start_pc,
								attrib->data.code.exceptions[k].end_pc,
								attrib->data.code.exceptions[k].handler_pc);
					}
					
					bbFinishLoading();
					
					bbPass(bbOptimizationPassF, c);
					bbPass(bbMarkConstantsPassF, c);
					bbFinalizeChanges();
					
					len = bbExport(NULL);
					newCode = natAlloc(sizeof(JavaAttribute) + len);
					if(!newCode){
					
						fprintf(stderr, "fail to alloc new code (sz=%" PRIu32 ")\n", len);
						exit(-50);	
					}
					memcpy(newCode, attrib, sizeof(JavaAttribute));
					free(attrib);
					attrib = newCode;
					m->attributes[j] = attrib;
					attrib->data.code.codeLen = len;
					bbExport(attrib->data.code.code);
					
					if(DEBUG) dumpcode("AFTER", attrib->data.code.code, attrib->data.code.codeLen,
							attrib->data.code.exceptions, attrib->data.code.numExceptions);
					
					for(len = 0; len < attrib->data.code.numExceptions; len++){
					
						bbGetExc(len, &attrib->data.code.exceptions[len].start_pc,
								&attrib->data.code.exceptions[len].end_pc,
								&attrib->data.code.exceptions[len].handler_pc);	
					}
					
					bbDestroy();
					
					for(k = 0; k < attrib->data.code.numExceptions; k++){
						
						idx = attrib->data.code.exceptions[k].catchType;
						if(idx){
							c->constantPool[idx - 1]->used = true;
							c->constantPool[idx - 1]->directUsed = 1;
						}
					}
				}
			}
		}
	}
	
	//mark other used resources as such
	{
		//account for "this", "supr", interfaces's STRING names (not class refs themselves)
		c->constantPool[*(uint16_t*)(c->constantPool[c->thisClass - 1] + 1) - 1]->used = true;
		c->constantPool[*(uint16_t*)(c->constantPool[c->thisClass - 1] + 1) - 1]->directUsed = true;
	
		if(c->superClass){
	
			c->constantPool[*(uint16_t*)(c->constantPool[c->superClass - 1] + 1) - 1]->used = true;
			c->constantPool[*(uint16_t*)(c->constantPool[c->superClass - 1] + 1) - 1]->directUsed = true;
		}
		
		//accound for interfaces
		for(i = 0; i < c->numInterfaces; i++){
		
			c->constantPool[c->interfaces[i] - 1]->used = true;
			c->constantPool[c->interfaces[i] - 1]->directUsed = true;
		}
		
		//account for fields
		for(i = 0; i < c->numFields; i++){
		
			c->constantPool[c->fields[i]->nameIdx - 1]->used = true;
			c->constantPool[c->fields[i]->nameIdx - 1]->directUsed = true;
			c->constantPool[c->fields[i]->descrIdx - 1]->used = true;
			c->constantPool[c->fields[i]->descrIdx - 1]->directUsed = true;
		}
		
		//account for methods
		for(i = 0; i < c->numMethods; i++){
		
			c->constantPool[c->methods[i]->nameIdx - 1]->used = true;
			c->constantPool[c->methods[i]->nameIdx - 1]->directUsed = true;
			c->constantPool[c->methods[i]->descrIdx - 1]->used = true;
			c->constantPool[c->methods[i]->descrIdx - 1]->directUsed = true;
		}
	}
	
	//account for const -> const references and mark them as used
	{
		for(i = 0; i < c->constantPoolSz - 1; i++){
				
			JavaConstant* cn;
			
			if(!c->constantPool[i]->directUsed) continue;
			
			if(c->constantPool[i]->type == JAVA_CONST_TYPE_CLASS || c->constantPool[i]->type == JAVA_CONST_TYPE_STR_REF){
			
				cn = c->constantPool[((uint16_t*)(c->constantPool[i] + 1))[0] - 1];
				
				if(!cn->used){
					cn->used = true;
				}
			}
			if(c->constantPool[i]->type == JAVA_CONST_TYPE_FIELD || c->constantPool[i]->type == JAVA_CONST_TYPE_METHOD ||
				c->constantPool[i]->type == JAVA_CONST_TYPE_INTERFACE || c->constantPool[i]->type == JAVA_CONST_TYPE_NAME_TYPE_INFO){
			
				uint16_t t;
				
				cn = c->constantPool[((uint16_t*)(c->constantPool[i] + 1))[0] - 1];	//class ref
				cn = c->constantPool[((uint16_t*)(cn + 1))[0] - 1];			//utf8 str (class)
				if(!cn->used){
					cn->used = true;
				}
				
				cn = c->constantPool[((uint16_t*)(c->constantPool[i] + 1))[1] - 1];	//name type info
				t = ((uint16_t*)(cn + 1))[1];
				
				cn = c->constantPool[((uint16_t*)(cn + 1))[0] - 1];			//utf8 str (name)
				if(!cn->used){
					cn->used = true;
				}
				
				cn = c->constantPool[t - 1];						//utf8 str (type)
				if(!cn->used){
					cn->used = true;
				}
			}
		}
	}
	
	//DBG: list unused constants
	for(i = 0; i < c->constantPoolSz - 1; i++){
	
		if(!c->constantPool[i]->used){
			
			if(DEBUG) fprintf(stderr, "unused constant: %u\n", i + 1);
		}
	}
	
	//reorder constants
	{
		uint16_t j;
		
		constantFMap = malloc(sizeof(uint16_t[c->constantPoolSz]));
		constantRMap = malloc(sizeof(uint16_t[c->constantPoolSz]));
		if(!constantFMap || !constantRMap){
		
			fprintf(stderr," failed to allocate constant map(s)\n");
			exit(-29);	
		}
		
		constantFMap[0] = 0;
		constantRMap[0] = 0;
		
		//first come constants tha are directly used (and thus need to have addresses in UJC file
		for(i = 0; i < c->constantPoolSz - 1; i++){
		
			if(c->constantPool[i]->directUsed){
			
				constantFMap[i + 1] = newDirectConsts;
				constantRMap[newDirectConsts] = i + 1;
				newDirectConsts++;
				if(DEBUG) fprintf(stderr, "(D) CONST: %5u -> %5u\n", i + 1, constantFMap[i + 1]);
			}
		}
		
		newNumConsts = newDirectConsts;
		//then come constants that are indirectly used used (and thus do not need to have addresses in UJC file)
		for(i = 0; i < c->constantPoolSz - 1; i++){
		
			if(c->constantPool[i]->used && !c->constantPool[i]->directUsed){
			
				constantFMap[i + 1] = newNumConsts;
				constantRMap[newNumConsts] = i + 1;
				newNumConsts++;
				if(DEBUG) fprintf(stderr, "(.) CONST: %5u -> %5u\n", i + 1, constantFMap[i + 1]);
			}
		}
		
		//the useful part of the map is done - finish the other part since it is needed to complete the sort
		j = newNumConsts;
		for(i = 0; i < c->constantPoolSz - 1; i++){
		
			if(!c->constantPool[i]->used){
			
				constantFMap[i + 1] = j;
				constantRMap[j] = i + 1;
				j++;
				if(DEBUG) fprintf(stderr, "( ) CONST: %5u -> %5u\n", i + 1, constantFMap[i + 1]);
			}
		}
	}
	
	//correct code for the reordered constants
	{
		for(i = 0; i < c->numMethods; i++){
	
			m = c->methods[i];
			codeFound = false;
	
			for(j = 0; j < m->numAttr; j++){
	
				attrib = m->attributes[j];
				if(attrib->type == J_ATTR_TYPE_CODE){
	
					JavaAttribute* newCode;
					uint32_t len;
						
					if(DEBUG) dumpcode("BEFORE", attrib->data.code.code, attrib->data.code.codeLen,
							attrib->data.code.exceptions, attrib->data.code.numExceptions);
					
					bbInit(attrib->data.code.code, attrib->data.code.codeLen);
					
					for(k = 0; k < attrib->data.code.numExceptions; k++){
						
						bbAddExc(attrib->data.code.exceptions[k].start_pc,
								attrib->data.code.exceptions[k].end_pc,
								attrib->data.code.exceptions[k].handler_pc);
					}
					
					bbFinishLoading();
					
					bbPass(bbFixConstantsPassF, constantFMap);
					bbFinalizeChanges();
					
					len = bbExport(NULL);
					newCode = natAlloc(sizeof(JavaAttribute) + len);
					if(!newCode){
					
						fprintf(stderr, "fail to alloc new code (sz=%" PRIu32 ")\n", len);
						exit(-50);	
					}
					memcpy(newCode, attrib, sizeof(JavaAttribute));
					free(attrib);
					attrib = newCode;
					m->attributes[j] = attrib;
					attrib->data.code.codeLen = len;
					bbExport(attrib->data.code.code);
					
					if(DEBUG) dumpcode("AFTER", attrib->data.code.code, attrib->data.code.codeLen,
							attrib->data.code.exceptions, attrib->data.code.numExceptions);
					
					for(len = 0; len < attrib->data.code.numExceptions; len++){
					
						bbGetExc(len, &attrib->data.code.exceptions[len].start_pc,
								&attrib->data.code.exceptions[len].end_pc,
								&attrib->data.code.exceptions[len].handler_pc);
						
						//remap types here too :)
						attrib->data.code.exceptions[len].catchType = constantFMap[attrib->data.code.exceptions[len].catchType];
					}
					
					bbDestroy();
				}
			}
		}
	}
	
	//correct other constant sources for constant reordering
	{
		//account for "this", "supr" (the strings they indirectly point to, not the ClassRefs they directly point to)
		// in this process we re-point them to no longer be indirect
		c->thisClass = constantFMap[*(uint16_t*)(c->constantPool[c->thisClass - 1] + 1)];
		if(c->superClass){
	
			c->superClass = constantFMap[*(uint16_t*)(c->constantPool[c->superClass - 1] + 1)];
		}
		
		//account for interfaces
		for(i = 0; i < c->numInterfaces; i++){
		
			c->interfaces[i] = constantFMap[c->interfaces[i]];
		}
		
		//account for fields
		for(i = 0; i < c->numFields; i++){
		
			c->fields[i]->nameIdx = constantFMap[c->fields[i]->nameIdx];
			c->fields[i]->descrIdx = constantFMap[c->fields[i]->descrIdx];
			
			classOptPrvRemapAttrs(c->fields[i]->attributes, c->fields[i]->numAttr, constantFMap);
		}
		
		//account for methods
		for(i = 0; i < c->numMethods; i++){
		
			c->methods[i]->nameIdx = constantFMap[c->methods[i]->nameIdx];
			c->methods[i]->descrIdx = constantFMap[c->methods[i]->descrIdx];
		
			classOptPrvRemapAttrs(c->methods[i]->attributes, c->methods[i]->numAttr, constantFMap);
		}
		
		//handle class attributes themselves
		{
			classOptPrvRemapAttrs(c->attributes, c->numAttributes, constantFMap);
		}
	}
	
	//correct constant->constant refs
	{
		for(i = 0; i < c->constantPoolSz - 1; i++){
				
			if(c->constantPool[i]->type == JAVA_CONST_TYPE_CLASS || c->constantPool[i]->type == JAVA_CONST_TYPE_STR_REF){
			
				*(uint16_t*)(c->constantPool[i] + 1) = constantFMap[*(uint16_t*)(c->constantPool[i] + 1)];
			}
			if(c->constantPool[i]->type == JAVA_CONST_TYPE_FIELD || c->constantPool[i]->type == JAVA_CONST_TYPE_METHOD ||
				c->constantPool[i]->type == JAVA_CONST_TYPE_INTERFACE || c->constantPool[i]->type == JAVA_CONST_TYPE_NAME_TYPE_INFO){
			
				((uint16_t*)(c->constantPool[i] + 1))[0] = constantFMap[((uint16_t*)(c->constantPool[i] + 1))[0]];
				((uint16_t*)(c->constantPool[i] + 1))[1] = constantFMap[((uint16_t*)(c->constantPool[i] + 1))[1]];
			}
		}	
	}
	
	//move actual constants around
	{
		JavaConstant** cl = malloc(sizeof(JavaConstant*[c->constantPoolSz - 1]));
		if(!cl){
		
			fprintf(stderr, "failed to alloc new constant list\n");
			exit(-20);	
		}
		
		for(i = 0; i < c->constantPoolSz - 1; i++){
		
			cl[i] = c->constantPool[constantRMap[i + 1] - 1];
		}
		
		for(i = 0; i < c->constantPoolSz - 1; i++){
		
			c->constantPool[i] = cl[i];
		}
		
		free(cl);
		free(constantRMap);
		free(constantFMap);
		
		c->addressblConstantPoolSz = newDirectConsts;
		c->placedConstantPoolSz = newNumConsts;
	}
}
