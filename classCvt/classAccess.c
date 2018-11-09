#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "classAccess.h"
#include "../UJC.h"


#define ERR(str)	do{			\
				err(str);	\
				goto fail;	\
			}while(0);

#if (DEBUG)

	#include <string.h>
	#define LOG(...)	fprintf(stderr, __VA_ARGS__)
	void logUtf8Str(const char* before, JavaString* s, const char* after){
	
		int i;
		
		LOG("%s", before);
		for(i = 0; i < s->len; i++) if(s->data[i] == '\n') LOG("\\n"); else LOG("%c",s->data[i]);
		LOG("%s", after);
			
	}
#else

	#define LOG(...)
	#define logUtf8Str(b, s, a)

#endif

#define GETBYTE	do{										\
			readFret = readF(readD);						\
			if(readFret == CLASS_IMPORT_READ_F_FAIL) ERR("class read byte error");	\
			byte = readFret;							\
		}while(0)



typedef struct{

	const UInt8* ptr;
	UInt32 left;
	
}FakeReaderData;


UInt16 fakeReaderReadF(void* paramP){

	FakeReaderData* d = paramP;
	
	if(d->left){
		
		d->left--;
		return *d->ptr++;
	}
	return CLASS_IMPORT_READ_F_FAIL;
}

Boolean classImportPrvReadU16(classImporterReadF readF, void* readD, UInt16* ptrP){

	UInt16 readFret;
	UInt8 byte, t;
	
	GETBYTE;
	t = byte;
	GETBYTE;
	*ptrP = (((UInt16)t) << 8) | byte;
	return true;

fail:
	return false;
}

Boolean classImportPrvReadU32(classImporterReadF readF, void* readD, UInt32* ptrP){

	UInt16 readFret;
	UInt8 byte, i;
	
	*ptrP = 0;
	
	for(i = 0; i < 4; i++){
		
		GETBYTE;
		*ptrP = ((*ptrP) << 8) | byte;
	}
	return true;

fail:
	return false;
}

UInt16 readBE16(const UInt8* p){

	UInt16 ret = 0;
	
	ret = *p++;
	ret <<= 8;
	return ret + *p;
}

UInt32 readBE32(const UInt8* p){

	UInt32 ret = 0;
	UInt8 i;

	for(i = 0; i < 4; i++){
		
		ret <<= 8;
		ret += *p++;
	}
	
	return ret;
}

static JavaConstant* classImportPrvReadConstant(classImporterReadF readF, void* readD){
	JavaConstant* ret = NULL;
	UInt16 readFret, tmp16 = 0, tmp16b;
	UInt8 type, byte, i;
	
	
	GETBYTE;	//ret the tag
	switch((type = byte)){
		
		case JAVA_CONST_TYPE_STRING:	//2 bytes + string
		
			if(!classImportPrvReadU16(readF, readD, &tmp16)) ERR("Failed to read string length");
			//we now have length -> allocate it
			ret = natAlloc(sizeof(JavaConstant) + sizeof(JavaString) + tmp16);
			if(!ret) ERR("Failed to alloc string");
			((JavaString*)(ret + 1))->len = tmp16;
			((JavaString*)(ret + 1))->addr = 0xFFFFFF;
			for(tmp16b = 0; tmp16b < tmp16; tmp16b++){
				
				GETBYTE;
				((JavaString*)(ret + 1))->data[tmp16b] = byte;
			}
			break;
		
		case JAVA_CONST_TYPE_INT:
			
			ret = natAlloc(sizeof(JavaConstant) + sizeof(UInt32));
			if(!ret) ERR("Failed to alloc int");
			if(!classImportPrvReadU32(readF, readD, (UInt32*)(ret + 1))) ERR("Failed to read int val");
			break;

		case JAVA_CONST_TYPE_LONG:
			
			ret = natAlloc(sizeof(JavaConstant) + sizeof(UInt64));
			if(!ret) ERR("Failed to alloc long");
			*(UInt64*)(ret + 1) = 0;
			for(i = 0; i < 8; i++){
				
				GETBYTE;
				(*(UInt64*)(ret + 1)) <<= 8;
				(*(UInt64*)(ret + 1)) += byte;
			}
			break;
		
		case JAVA_CONST_TYPE_FLOAT:
			
			ret = natAlloc(sizeof(JavaConstant) + 4);
			if(!ret) ERR("Failed to alloc float");
			for(i = 0; i < 4; i++){
				
				GETBYTE;
				((UInt8*)(ret + 1))[3-i] = byte;
			}
			break;
			
		case JAVA_CONST_TYPE_DOUBLE:
			
			ret = natAlloc(sizeof(JavaConstant) + 8);
			if(!ret) ERR("Failed to alloc double");
			for(i = 0; i < 8; i++){
				
				GETBYTE;
				((UInt8*)(ret + 1))[7 - i] = byte;
			}
			break;
			
		case JAVA_CONST_TYPE_CLASS:
		case JAVA_CONST_TYPE_STR_REF:
			
			ret = natAlloc(sizeof(JavaConstant) + sizeof(UInt16));
			if(!ret) ERR("Failed to alloc class/strRef");
			if(!classImportPrvReadU16(readF, readD, (UInt16*)(ret + 1))) ERR("Failed to read class/strRef ref");
			break;

		case JAVA_CONST_TYPE_FIELD:
		case JAVA_CONST_TYPE_METHOD:
		case JAVA_CONST_TYPE_INTERFACE:
		case JAVA_CONST_TYPE_NAME_TYPE_INFO:
			
			ret = natAlloc(sizeof(JavaConstant) + sizeof(UInt16[2]));
			if(!ret) ERR("Failed to alloc field/method/interface/typenameinfo");
			if(!classImportPrvReadU16(readF, readD, ((UInt16*)(ret + 1)) + 0)) ERR("Failed to read field/method/interface/typenameinfo ref1");
			if(!classImportPrvReadU16(readF, readD, ((UInt16*)(ret + 1)) + 1)) ERR("Failed to read field/method/interface/typenameinfo ref2");
			break;

		default:
			fprintf(stderr,"$$ %d $$\n", type);
			ERR("Unknown constant type");
	}
	
	ret->used = false;
	ret->directUsed = false;
	ret->type = type;
	return ret;
fail:
	natFree(ret);
	return NULL;	
}

#define READ_Xex(xc, xn, xt, xf, obj)									\
													\
		do{											\
			UInt8 i;									\
			if(!classImportPrvReadU16(readF, readD, &obj xc)) ERR("Failed to read " #xc);	\
			obj xn = natAlloc(obj xc * sizeof(xt*));					\
			if(!obj xn) ERR("failed to allocate " #xn" array");				\
			for(i = 0; i < obj xc; i++){							\
													\
				obj xn[i] = xf(readF, readD);						\
				if(!obj xn[i]) ERR("failed to read " #xn);				\
			}										\
		}while(0)


#define READ_X(xc, xn, xt, xf) READ_Xex(xc, xn, xt, xf, ret->)

static JavaAttribute* classImportPrvReadAttribute(classImporterReadF readF, void* readD){

	JavaAttribute* ret = NULL;
	UInt16 readFret;
	UInt8 byte;
	UInt16 name;
	UInt32 len;
	UInt8* ptr;
	
	if(!classImportPrvReadU16(readF, readD, &name)) ERR("Failed to read attribute name");
	if(!classImportPrvReadU32(readF, readD, &len)) ERR("Failed to read attribute length");
	
	ret = natAlloc(sizeof(JavaAttribute) + len);
	if(!ret) ERR("Failed to alloc attribute");
	
	ret->nameIdx = name;
	ret->data.generic.len = len;
	ptr = ret->data.generic.data;
	
	while(len--){
		
		GETBYTE;
		*ptr++ = byte;
	}
	
	ret->type = J_ATTR_TYPE_GENERIC;
	return ret;
fail:
	natFree(ret);
	return NULL;		
}

static UInt16 classImportPrvReadInterface(classImporterReadF readF, void* readD){

	UInt16 ret = 0;
	
	if(!classImportPrvReadU16(readF, readD, &ret)) ERR("Failed to read attribute name");
	
	return ret;
	
fail:
	return 0;
}

static JavaMethodOrField* classImportPrvReadMethodOrField(classImporterReadF readF, void* readD){

	JavaMethodOrField* ret = natAlloc(sizeof(JavaMethodOrField));
	if(!ret) ERR("Failed to alloc method");
	
	if(!classImportPrvReadU16(readF, readD, &ret->accessFlags)) ERR("Failed to read mathod accessFlags");
	if(!classImportPrvReadU16(readF, readD, &ret->nameIdx)) ERR("Failed to read mathod nameIdx");
	if(!classImportPrvReadU16(readF, readD, &ret->descrIdx)) ERR("Failed to read mathod descrIdx");
	
	READ_X(numAttr, attributes, JavaAttribute, classImportPrvReadAttribute);
	
	return ret;
fail:
	natFree(ret);
	return NULL;		
}

static JavaMethodOrField* classImportPrvReadMethod(classImporterReadF readF, void* readD){

	return classImportPrvReadMethodOrField(readF, readD);
}

static JavaMethodOrField* classImportPrvReadField(classImporterReadF readF, void* readD){

	return classImportPrvReadMethodOrField(readF, readD);
}


void classProcessAttributes(JavaClass* c, JavaAttribute** attribP){

	JavaAttribute* attrib = *attribP;
	FakeReaderData rd;
	classImporterReadF* readF = fakeReaderReadF;
	void* readD = &rd;
	JavaConstant* jc;
	JavaString* js;
	UInt32 t;
	UInt16 i;
	

	jc = CLASS_CONSTANT(c, attrib->nameIdx);
	if(jc->type != JAVA_CONST_TYPE_STRING) ERR("attribute name does not point to a string");
	js = (JavaString*)(jc + 1);
	
	if(js->len == 4 && js->data[0] == 'C' && js->data[1] == 'o' && js->data[2] == 'd' && js->data[3] == 'e'){
	
		JavaAttribute* na;
		UInt32 codeLen;
		
		codeLen = readBE32(attrib->data.generic.data + 4);
		na = natAlloc(sizeof(JavaAttribute) + codeLen);
		if(!na) ERR("Failed to alloc new code attribute struct");
		
		na->data.code.maxStack = readBE16(attrib->data.generic.data + 0);
		na->data.code.maxLocals = readBE16(attrib->data.generic.data + 2);
		na->data.code.codeLen = codeLen;
		
		for(t = 0; t < codeLen; t++) na->data.code.code[t] = attrib->data.generic.data[t + 8];
		
		na->data.code.numExceptions = readBE16(attrib->data.generic.data + 8 + codeLen);
		na->data.code.exceptions = natAlloc(na->data.code.numExceptions * sizeof(JavaExceptionTableEntry));
		if(!na->data.code.exceptions) ERR("Failed to alloc new code attribute struct's exceptions table");
		
		for(i = 0; i < na->data.code.numExceptions; i++){
		
			na->data.code.exceptions[i].start_pc = readBE16(attrib->data.generic.data + 8 + codeLen + 2 + 8 * i + 0);
			na->data.code.exceptions[i].end_pc = readBE16(attrib->data.generic.data + 8 + codeLen + 2 + 8 * i + 2);
			na->data.code.exceptions[i].handler_pc = readBE16(attrib->data.generic.data + 8 + codeLen + 2 + 8 * i + 4);
			na->data.code.exceptions[i].catchType = readBE16(attrib->data.generic.data + 8 + codeLen + 2 + 8 * i + 6);
		}
		
		t = 8 + codeLen + 2 + 8 * na->data.code.numExceptions;	//where attributes begin
		rd.ptr = attrib->data.generic.data + t;
		rd.left = attrib->data.generic.len - t;
		READ_Xex(numAttributes, attributes, JavaAttribute, classImportPrvReadAttribute, na->data.code.);
	
		na->nameIdx = attrib->nameIdx;
		na->type = J_ATTR_TYPE_CODE;
		natFree(attrib);
		attrib = na;
	}
	
	
fail:
	*attribP = attrib;
}


JavaClass* classImport(classImporterReadF readF, void* readD){

	static const UInt8 classMagic[] = {0xCA, 0xFE, 0xBA, 0xBE};
	JavaClass* ret = NULL;
	UInt16 readFret;
	UInt8 byte;
	UInt16 i, j;
	
	for(i = 0; i < 4; i++){
		
		GETBYTE;
		if(byte!= classMagic[i]) err("Magic error");
	}
	
	for(i = 0; i < 4; i++){
		
		GETBYTE;	//minor version & major version
	}
	
	ret = natAlloc(sizeof(JavaClass));
	if(!ret) ERR("Failed to allocate class");
	natMemZero(ret, sizeof(JavaClass));
	if(!classImportPrvReadU16(readF, readD, &ret->constantPoolSz)) ERR("Failed to read constant pool length");
	ret->constantPool = natAlloc((ret->constantPoolSz - 1) * sizeof(JavaConstant*));
	if(!ret->constantPool) ERR("failed to allocate constant pool array");
	for(i = 0; i < ret->constantPoolSz - 1; i++){
	
		ret->constantPool[i] = classImportPrvReadConstant(readF, readD);
		if(!ret->constantPool[i]) ERR("failed to read constant");
		if(ret->constantPool[i]->type == JAVA_CONST_TYPE_LONG || ret->constantPool[i]->type == JAVA_CONST_TYPE_DOUBLE){
		
			i++;
			ret->constantPool[i] = natAlloc(sizeof(JavaConstant));
			if(!ret->constantPool[i]) ERR("failed alloc empty constant slot");
			ret->constantPool[i]->type = JAVA_CONST_TYPE_INVALID;
			ret->constantPool[i]->used = false;
			ret->constantPool[i]->directUsed = false;
		}
	}
	
	if(!classImportPrvReadU16(readF, readD, &ret->accessFlags)) ERR("Failed to read access flags");
	if(!classImportPrvReadU16(readF, readD, &ret->thisClass)) ERR("Failed to read thisClass");
	if(!classImportPrvReadU16(readF, readD, &ret->superClass)) ERR("Failed to read superClass");
	
	READ_X(numInterfaces, interfaces, UInt16, classImportPrvReadInterface);
	READ_X(numFields, fields, JavaMethodOrField, classImportPrvReadField);
	READ_X(numMethods, methods, JavaMethodOrField, classImportPrvReadMethod);
	READ_X(numAttributes, attributes, JavaAttribute, classImportPrvReadAttribute);
	
	
	for(i = 0; i < ret->numFields; i++){
	
		for(j = 0; j < ret->fields[i]->numAttr; j++) classProcessAttributes(ret, ret->fields[i]->attributes + j);
	}
	
	for(i = 0; i < ret->numMethods; i++){
	
		for(j = 0; j < ret->methods[i]->numAttr; j++) classProcessAttributes(ret, ret->methods[i]->attributes + j);
	}
	
	for(i = 0; i < ret->numAttributes; i++){
	
		classProcessAttributes(ret, ret->attributes + i);
		if(ret->attributes[i]->type == J_ATTR_TYPE_CODE){	//some types have subattributes
		
			for(j = 0; j < ret->attributes[i]->data.code.numAttributes; j++) classProcessAttributes(ret, ret->attributes[i]->data.code.attributes + j);
		}
	}
	
	return ret;
	
fail:
	if(ret){
		natFree(ret->constantPool);
		natFree(ret->interfaces);
		natFree(ret->fields);
		natFree(ret->methods);
	}
	natFree(ret);
	return NULL;
}

static void classDumpAttribute(JavaClass* c, JavaAttribute* attrib, UInt8 indents){

	JavaConstant* jc;
	UInt16 i;
	UInt32 j;

	for(i = 0; i < indents; i++) LOG("\t");
	LOG("{\n");
	
	for(i = 0; i <= indents; i++) LOG("\t");
	jc = CLASS_CONSTANT(c, attrib->nameIdx);
	if(jc->type != JAVA_CONST_TYPE_STRING) ERR("attribute name does not point to a string");
	logUtf8Str("Name: '" , (JavaString*)(jc + 1), "'\n");
	
	if(attrib->type == J_ATTR_TYPE_GENERIC){
		
		for(i = 0; i <= indents; i++) LOG("\t");
		LOG("Size: %lu\n", attrib->data.generic.len);
		
		for(i = 0; i <= indents; i++) LOG("\t");
		LOG("Data:");
		
		for(j = 0; j < attrib->data.generic.len; j++){
			if(j % 16) LOG(" ");
			else{
				LOG("\n");
				for(i = 0; i <= indents + 1; i++) LOG("\t");
			}
			LOG("%02X", attrib->data.generic.data[j]);
		}
		LOG("\n");
	}
	else if(attrib->type == J_ATTR_TYPE_CODE){
	
		for(i = 0; i <= indents; i++) LOG("\t");
		LOG("Stack: %u\n", attrib->data.code.maxStack);
		
		for(i = 0; i <= indents; i++) LOG("\t");
		LOG("Locals: %u\n", attrib->data.code.maxLocals);
		
		for(i = 0; i <= indents; i++) LOG("\t");
		LOG("Exceptions(%u):\n", attrib->data.code.numExceptions);
		
		for(j = 0; j < attrib->data.code.numExceptions; j++){
			for(i = 0; i <= indents + 1; i++) LOG("\t");
			LOG("{\n");
			
			for(i = 0; i <= indents + 2; i++) LOG("\t");
			LOG("PC range: 0x%04X - 0x%04X\n", attrib->data.code.exceptions[j].start_pc, attrib->data.code.exceptions[j].end_pc);
			
			for(i = 0; i <= indents + 2; i++) LOG("\t");
			LOG("Handler PC: 0x%04X\n", attrib->data.code.exceptions[j].handler_pc);
			
			for(i = 0; i <= indents + 2; i++) LOG("\t");
			LOG("catchType: 0x%04X\n", attrib->data.code.exceptions[j].catchType);
			
			for(i = 0; i <= indents + 1; i++) LOG("\t");
			LOG("}\n");
		}
		
		if(attrib->data.code.numAttributes){
		
			for(i = 0; i <= indents; i++) LOG("\t");
			LOG("Attributes(%u):\n", attrib->data.code.numAttributes);
			
			for(j = 0; j < attrib->data.code.numAttributes; j++){
				classDumpAttribute(c, attrib->data.code.attributes[j], indents + 2);
			}
		}
		for(i = 0; i <= indents; i++) LOG("\t");
		LOG("Code Size: %lu\n", attrib->data.code.codeLen);
		
		for(i = 0; i <= indents; i++) LOG("\t");
		LOG("Code:");
		
		for(j = 0; j < attrib->data.code.codeLen; j++){
			if(j % 16) LOG(" ");
			else{
				LOG("\n");
				for(i = 0; i <= indents + 1; i++) LOG("\t");
			}
			LOG("%02X", attrib->data.code.code[j]);
		}
		LOG("\n");
	}
	
fail:
	for(i = 0; i < indents; i++) LOG("\t");
	LOG("}\n");
}

static void classDumpConstant(JavaClass* c, UInt16 idx, UInt8 indent){

	JavaConstant* jc = c->constantPool[idx];
	UInt16 t;
	UInt8 i;
	
	if(jc->type == JAVA_CONST_TYPE_INVALID) return;
	
	for(i = 0; i < indent; i++) LOG("\t");
	LOG("const %u (%s %s): ", idx + 1, jc->used ? "u" : " ", jc->directUsed ? "du" : "  ");
	
	switch(jc->type){
	
		case JAVA_CONST_TYPE_STRING:		//2 bytes + string
			
			logUtf8Str("UTF8_string '" , (JavaString*)(jc + 1), "'\n");
			break;
			
		case JAVA_CONST_TYPE_INT:		//4 bytes
			
			LOG("INT %ld\n", *(Int32*)(jc + 1));
			break;
		
		case JAVA_CONST_TYPE_FLOAT:		//4 bytes
			
			LOG("FLOAT %f\n", *(float*)(jc + 1));
			break;
		
		case JAVA_CONST_TYPE_LONG:		//8 bytes
			
			LOG("LONG %lld\n", *(signed long long*)(jc + 1));
			break;
		
		case JAVA_CONST_TYPE_DOUBLE:		//8 bytes
			
			LOG("DOUBLE %f\n", *(double*)(jc + 1));
			break;
			
		case JAVA_CONST_TYPE_CLASS:		//2 bytes (index back into constant pool pointing to class name string)
			
			LOG("CLASS ");
			goto single_ref;
			
		case JAVA_CONST_TYPE_STR_REF:		//2 bytes (index back into constant pool pointing to string)
			
			LOG("STR_REF ");
		
	single_ref:
			LOG(" {%u} ", *(UInt16*)(jc + 1));
			jc = CLASS_CONSTANT(c, *(UInt16*)(jc + 1));
			if(jc->type != JAVA_CONST_TYPE_STRING) ERR("class/strref does not point to a string type");
			logUtf8Str("'" , (JavaString*)(jc + 1), "'\n");
			break;
			
		case JAVA_CONST_TYPE_FIELD:		//4 bytes (2x index back into constant pool: class & name&type info)
			
			LOG("FIELD ");
			goto tripple_ref;
		
		case JAVA_CONST_TYPE_METHOD:		//4 bytes (2x index back into constant pool: class & name&type info)
			
			LOG("METHOD ");
			goto tripple_ref;
		
		case JAVA_CONST_TYPE_INTERFACE:		//4 bytes (2x index back into constant pool: class & name&type info)
			
			LOG("INTERFACE METHOD ");
			
	tripple_ref:
			LOG("{%u, %u} ", ((UInt16*)(jc + 1))[0], ((UInt16*)(jc + 1))[1]);
			t = ((UInt16*)(jc + 1))[1];
			
			jc = CLASS_CONSTANT(c, ((UInt16*)(jc + 1))[0]);
			if(jc->type != JAVA_CONST_TYPE_CLASS) ERR("tripple ref[0] does not point to a class type");
			
			jc = CLASS_CONSTANT(c, ((UInt16*)(jc + 1))[0]);
			if(jc->type != JAVA_CONST_TYPE_STRING) ERR("class[0] does not point to a string type");
			
			logUtf8Str("" , (JavaString*)(jc + 1), ".");
			
			jc = CLASS_CONSTANT(c, t);
			if(jc->type != JAVA_CONST_TYPE_NAME_TYPE_INFO) ERR("tripple ref[1] does not point to a name/type info");
			
			goto double_ref;
		
		case JAVA_CONST_TYPE_NAME_TYPE_INFO:	//4 bytes (jc + 1)
			
			LOG("NAME/TYPE info ");
			LOG("{%u, %u} ", ((UInt16*)(jc + 1))[0], ((UInt16*)(jc + 1))[1]);
			
		double_ref:
		
			t = ((UInt16*)(jc + 1))[1];
			
			jc = CLASS_CONSTANT(c, ((UInt16*)(jc + 1))[0]);
			if(jc->type != JAVA_CONST_TYPE_STRING) ERR("nametypeinfo[0] does not point to a string type");
			logUtf8Str("" , (JavaString*)(jc + 1), " ");
			
			jc = CLASS_CONSTANT(c, t);
			if(jc->type != JAVA_CONST_TYPE_STRING) ERR("nametypeinfo[1] does not point to a string type");
			logUtf8Str("" , (JavaString*)(jc + 1), "\n");
			break;
	}
fail:;
}

static void classDumpInterface(JavaClass* c, UInt16 iface, UInt8 indents){

	JavaConstant* jc;
	UInt16 i;
	
	jc = CLASS_CONSTANT(c, iface);
	if(jc->type != JAVA_CONST_TYPE_CLASS) ERR("iface does not point to a class type");
	i = *(UInt16*)(jc + 1);
	jc = CLASS_CONSTANT(c, i);
	if(jc->type != JAVA_CONST_TYPE_STRING) ERR("iface class name does not point to a string");
	
	for(i = 0; i < indents; i++) LOG("\t");
	
	logUtf8Str("Interface: '" , (JavaString*)(jc + 1), "'\n");
	
fail:;
}

static void classDumpMethodOrField(JavaClass* c, JavaMethodOrField* method, UInt8 indents, const char* type){

	JavaConstant* jc;
	UInt16 i;
	
	for(i = 0; i < indents; i++) LOG("\t");
	LOG("%s{\n", type);
	
	for(i = 0; i <= indents; i++) LOG("\t");
	jc = CLASS_CONSTANT(c, method->nameIdx);
	if(jc->type != JAVA_CONST_TYPE_STRING) ERR("method name does not point to a string");
	logUtf8Str("Name: '" , (JavaString*)(jc + 1), "'\n");
	
	for(i = 0; i <= indents; i++) LOG("\t");
	jc = CLASS_CONSTANT(c, method->descrIdx);
	if(jc->type != JAVA_CONST_TYPE_STRING) ERR("method descriptor does not point to a string");
	logUtf8Str("Descriptor: '" , (JavaString*)(jc + 1), "'\n");
	
	for(i = 0; i <= indents; i++) LOG("\t");
	LOG("Flags: 0x%04X\n", method->accessFlags);
	
	if(method->numAttr){
		
		for(i = 0; i <= indents; i++) LOG("\t");
		LOG("Attributes (%d):\n", method->numAttr);
		for(i = 0; i < method->numAttr; i++) classDumpAttribute(c, method->attributes[i], indents + 2);
	}
	
fail:
	for(i = 0; i < indents; i++) LOG("\t");
	LOG("}\n");
}


static void classDumpField(JavaClass* c, JavaMethodOrField* field, UInt8 indents){

	classDumpMethodOrField(c, field, indents, "field");
}

static void classDumpMethod(JavaClass* c, JavaMethodOrField* method, UInt8 indents){

	classDumpMethodOrField(c, method, indents, "method");
}


void classDump(JavaClass* c){

	JavaConstant* jc;
	UInt16 idx;
	
	LOG("Class dump:\n");
	
	jc = CLASS_CONSTANT(c, c->thisClass);
	if(jc->type == JAVA_CONST_TYPE_CLASS){
		
		idx = *(UInt16*)(jc + 1);
		jc = CLASS_CONSTANT(c, idx);	
	}
	if(jc->type == JAVA_CONST_TYPE_STRING){
	
		logUtf8Str("\tName: '", (JavaString*)(jc + 1),"'\n");	
	}
	else ERR("this object does not point to a class type/class_direct_name");
	
	jc = CLASS_CONSTANT(c, c->superClass);
	if(jc->type == JAVA_CONST_TYPE_CLASS){
		
		idx = *(UInt16*)(jc + 1);
		jc = CLASS_CONSTANT(c, idx);	
	}
	if(jc->type == JAVA_CONST_TYPE_STRING){
	
		logUtf8Str("\tSuper: '", (JavaString*)(jc + 1),"'\n");	
	}
	else ERR("this object does not point to a class type/class_direct_name");
	
	LOG("\tFlags: 0x%04X\n", c->accessFlags);
	
	LOG("\tconstants (%d):\n", c->constantPoolSz);
	for(idx = 0; idx < c->constantPoolSz - 1; idx++) classDumpConstant(c, idx, 2);
	
	LOG("\tInterfaces (%d):\n", c->numInterfaces);
	for(idx = 0; idx < c->numInterfaces; idx++) classDumpInterface(c, c->interfaces[idx], 2);
	
	LOG("\tFields (%d):\n", c->numFields);
	for(idx = 0; idx < c->numFields; idx++) classDumpField(c, c->fields[idx], 2);
	
	LOG("\tMethods (%d):\n", c->numMethods);
	for(idx = 0; idx < c->numMethods; idx++) classDumpMethod(c, c->methods[idx], 2);
	
	if(c->numAttributes){
		
		LOG("\tAttributes (%d):\n", c->numAttributes);
		for(idx = 0; idx < c->numAttributes; idx++) classDumpAttribute(c, c->attributes[idx], 2);
	}
	
fail:;
}

void classFreeAttribute(JavaAttribute* f){

	if(!f) return;
	if(f->type == J_ATTR_TYPE_CODE){
	
		UInt16 i;
		natFree(f->data.code.exceptions);
		
		for(i = 0; i < f->data.code.numAttributes; i++){
		
			classFreeAttribute(f->data.code.attributes[i]);
		}
		natFree(f->data.code.attributes);
	}
	
	natFree(f);
}

void classFreeConstant(JavaConstant* f){

	if(!f) return;
	
	
	natFree(f);	
}


void classFreeMethodOrField(JavaMethodOrField* f){

	UInt16 i;
	if(!f) return;
	
	
	for(i = 0; i < f->numAttr; i++){
	
		classFreeAttribute(f->attributes[i]);	
	}
	natFree(f);
}

void classFree(JavaClass* c){

	UInt16 i;
	
	
	for(i = 0; i < c->constantPoolSz - 1; i++){
		
		classFreeConstant(c->constantPool[i]);
	}
	natFree(c->constantPool);
	
	natFree(c->interfaces);
	
	for(i = 0; i < c->numFields; i++){
		
		classFreeMethodOrField(c->fields[i]);
	}
	natFree(c->fields);
	
	for(i = 0; i < c->numMethods; i++){
		
		classFreeMethodOrField(c->methods[i]);
	}
	natFree(c->methods);
	
	for(i = 0; i < c->numAttributes; i++){
		
		classFreeAttribute(c->attributes[i]);
	}
	natFree(c->attributes);
	
	natFree(c);	
}



static UInt8 ujStrHash(JavaString* str){

	UInt8 c = 0xCC;
	
	UInt16 L = str->len;


	while(L){

		L--;
		c = (c << 1) ^ ((c & 0x80) ? 0x41 : 0x00) ^ str->data[L];
	}

	if(0){
		UInt16 t;
		
		fprintf(stderr,"HASH('");
		for(t = 0; t < str->len; t++) fprintf(stderr, "%c", str->data[t]);
		fprintf(stderr, "') = 0x%02X\n", c);	
	}

	return c;
}

static UInt24 gFileSz = 0;
static UInt32 gLastVal = 0;

void putU8(UInt8 v){

	putchar(v);
	gFileSz++;
	gLastVal = v;
}

void putU16(UInt16 v){

	putchar(v >> 8);
	putchar(v);
	gFileSz += 2;
	gLastVal = v;
}

void putU24(UInt24 v){

	putchar(v >> 16);
	putchar(v >> 8);
	putchar(v);
	gFileSz += 3;
	gLastVal = v;
}
void putU32(UInt32 v){

	putchar(v >> 24);
	putchar(v >> 16);
	putchar(v >> 8);
	putchar(v);
	gFileSz += 4;
	gLastVal = v;
}

void classExport(JavaClass* c){

	UInt24 hdrsz = 18, crefs = 0, interfaces = 0, methods = 0, fields = 0, consts = 0, code = 0, addr;
	JavaConstant* jc;
	JavaAttribute* ja;
	UInt16 i, j;
	Int16 sz;
	
	
	//precalculate sizes
	{
		
		//precalc. size of constant refs table
		crefs = 2 + (UInt32)(c->addressblConstantPoolSz - 1) * 3;
	
		//precalc. size of constants
		for(i = 0; i < c->placedConstantPoolSz - 1; i++){
		
			jc = c->constantPool[i];
			if(!jc->used) continue;
			
			sz = -1;
			switch(jc->type){
				
				case JAVA_CONST_TYPE_STRING:
					
					sz = 2 + ((JavaString*)(jc + 1))->len;
					break;
					
				case JAVA_CONST_TYPE_INT:
				case JAVA_CONST_TYPE_FLOAT:
					
					if(!jc->directUsed) break;
					sz = 4;
					break;
				
				case JAVA_CONST_TYPE_LONG:
				case JAVA_CONST_TYPE_DOUBLE:
					
					if(!jc->directUsed) break;
					sz = 8;
					break;
				
				case JAVA_CONST_TYPE_CLASS:
				case JAVA_CONST_TYPE_STR_REF:
					
					if(!jc->directUsed) break;
					//no size increase since we reference string directly form crefs
					break;
				
				case JAVA_CONST_TYPE_FIELD:
				case JAVA_CONST_TYPE_METHOD:
				case JAVA_CONST_TYPE_INTERFACE:
				
					if(!jc->directUsed) break;
					sz = 9;	//class, name, type
					break;
				
				case JAVA_CONST_TYPE_NAME_TYPE_INFO:
					
					fprintf(stderr, "name type info marked as used\n");
					break;
				
				default:
					
					fprintf(stderr, "weird constant type %d used\n", jc->type);
					break;
			}	
			sz++;/* for type */
			consts += sz;
		}
		
		//precalc interfaces size
		interfaces = 2 + (UInt32)c->numInterfaces * 3;
		
		//precalc methods size
		methods = 2 + (UInt32)c->numMethods * 13;
		
		//precalc fields
		fields = 2 + (UInt32)c->numFields * 10;
		
		//code sizes
		for(i = 0; i < c->numMethods; i++){
		
			for(j = 0; j < c->methods[i]->numAttr; j++){
			
				ja = c->methods[i]->attributes[j];
				
				if(ja->type != J_ATTR_TYPE_CODE) continue;
				
				code += ja->data.code.codeLen + 4 /*locals, stask sizes*/ + 2 /*num exceptions */ + (UInt32)ja->data.code.numExceptions * 8;
			}
		}
		
	}
	LOG("Expecting to produce:\n\t%lu bytes of header\n\t%lu bytes of constant refs\n\t%lu bytes of constants"
		"\n\t%lu bytes of interface refs\n\t%lu bytes of method infos\n\t%lu bytes of field infos"
		"\n\t%lu bytes of code\n\t = %lu bytes total\n", hdrsz, crefs, consts, interfaces, methods, fields, code,
		hdrsz + crefs + interfaces + methods + fields + consts + code);
	
	//produce the header
	{
		putU16(UJC_MAGIC);
		putU16(c->thisClass);
		putU16(c->superClass);
		putU16(c->accessFlags);
		putU24(hdrsz + crefs + consts + 2);					//interfaces	(+2 is a claver hack, see vm code)
		putU24(hdrsz + crefs + consts + interfaces);				//methods
		putU24(hdrsz + crefs + consts + interfaces + methods + 2);		//fields  	(+2 is a claver hack, see vm code)
		putU8(ujStrHash((JavaString*)(c->constantPool[c->thisClass - 1] + 1)));	//name hash
	}
	
	//calculate constant positions and write crefs
	{
		addr = hdrsz + crefs;
		for(i = 0; i < c->placedConstantPoolSz - 1; i++){
		
			jc = c->constantPool[i];
			
			if(!jc->used) continue;
			if(jc->type != JAVA_CONST_TYPE_STRING) continue;
			
			((JavaString*)(jc + 1))->addr = addr;
			addr += 1 /* for type */ + 2 + ((JavaString*)(jc + 1))->len;
		}
		
		putU16(c->addressblConstantPoolSz);
		for(i = 0; i < c->addressblConstantPoolSz - 1; i++){
		
			jc = c->constantPool[i];
			
			if(!jc->directUsed) putU24(0);
			else switch(jc->type){

				case JAVA_CONST_TYPE_STRING:
					
					if(((JavaString*)(jc + 1))->addr == 0xFFFFFF) fprintf(stderr, "string %d not ready (1)\n", i);
					putU24(((JavaString*)(jc + 1))->addr);
					break;
					
				case JAVA_CONST_TYPE_INT:
				case JAVA_CONST_TYPE_FLOAT:
					
					putU24(addr);
					addr += 4 + 1 /* for type */;
					break;
				
				case JAVA_CONST_TYPE_LONG:
				case JAVA_CONST_TYPE_DOUBLE:
					
					putU24(addr);
					addr += 8 + 1 /* for type */;
					break;
				
				case JAVA_CONST_TYPE_CLASS:
				case JAVA_CONST_TYPE_STR_REF:
					
					//for single refs, we just the actual string addr
					jc = c->constantPool[*(UInt16*)(c->constantPool[i] + 1) - 1];
					putU24(((JavaString*)(jc + 1))->addr);
					break;
				
				case JAVA_CONST_TYPE_FIELD:
				case JAVA_CONST_TYPE_METHOD:
				case JAVA_CONST_TYPE_INTERFACE:
				
					putU24(addr);
					addr += 9 + 1 /* for type */;
					break;
				
				case JAVA_CONST_TYPE_NAME_TYPE_INFO:
					
					fprintf(stderr, "cannot produce ref to nti\n");
					exit(-5);
					break;
				
				default:
					
					fprintf(stderr, "cannot produce ref to weird constant type %d used\n", jc->type);
					exit(-5);
					break;
			}
		//	LOG(" pointing const %u to addr 0x%06X using ptr loc 0x%06X \n", i + 1, gLastVal, gFileSz - 3);
		}
	}

	//write constants themselves
	{
		addr = hdrsz + crefs;
		
		//pass 1: strings with assigned addresses
		for(i = 0; i < c->placedConstantPoolSz - 1; i++){
		
			jc = c->constantPool[i];
			
			if(!jc->used) continue;
			if(jc->type == JAVA_CONST_TYPE_STRING){
			
				JavaString* str = (JavaString*)(jc + 1);
				
				if(str->addr == 0xFFFFFF) fprintf(stderr, "string %d not ready (2)\n", i);
				
				if(addr != str->addr){
				
					fprintf(stderr, "address fail on const string %u (expected 0x%lX, got 0x%lX)\n", i + 1, str->addr, addr);
					exit(-5);
				}
			//	LOG(" writing const %u at addr 0x%06X\n", i + 1, gFileSz);
				putU8(jc->type);
				putU16(str->len);
				for(j = 0; j < str->len; j++) putU8(str->data[j]);
				addr += 1 /* for type */ + 2 + str->len;
			}
		}
		
		//pass2: all other things
		for(i = 0; i < c->placedConstantPoolSz - 1; i++){
		
			jc = c->constantPool[i];
			
			if(!jc->directUsed) continue;
		//	LOG(" writing const %u at addr 0x%06X\n", i + 1, gFileSz);
			switch(jc->type){

				case JAVA_CONST_TYPE_STRING:
					
					//done already
					break;
				
				case JAVA_CONST_TYPE_INT:
				case JAVA_CONST_TYPE_FLOAT:
					
					putU8(jc->type);
					putU32(((UInt32*)(jc + 1))[0]);
					addr += 4;
					break;
				
				case JAVA_CONST_TYPE_LONG:
				case JAVA_CONST_TYPE_DOUBLE:
					
					putU8(jc->type);
					putU32(((UInt32*)(jc + 1))[1]);
					putU32(((UInt32*)(jc + 1))[0]);
					addr += 8;
					break;
				
				case JAVA_CONST_TYPE_CLASS:
				case JAVA_CONST_TYPE_STR_REF:
					
					//nothing to do
					break;
				
				case JAVA_CONST_TYPE_FIELD:
				case JAVA_CONST_TYPE_METHOD:
				case JAVA_CONST_TYPE_INTERFACE:
				
					putU8(jc->type);
					
					j = ((UInt16*)(jc + 1))[1];				//const idx for name type info
					
					jc = c->constantPool[((UInt16*)(jc + 1))[0] - 1];	//class ref
					if(jc->type != JAVA_CONST_TYPE_CLASS){
						
						fprintf(stderr," mfi[8] doesn't refer to class\n");
						exit(-5);
					}
					jc = c->constantPool[((UInt16*)(jc + 1))[0] - 1];	//utf8: classname
					if(jc->type != JAVA_CONST_TYPE_STRING){
						
						fprintf(stderr," 2 cls/strref doesn't refer to utf8 str\n");
						exit(-5);
					}
					if(((JavaString*)(jc + 1))->addr == 0xFFFFFF) fprintf(stderr, "string %d not ready (4)\n", i);
					putU24(((JavaString*)(jc + 1))->addr + 1); //+ 1 to point direct to string, not const type - helps the RT
					
					jc = c->constantPool[j - 1];				//name type info
					j = ((UInt16*)(jc + 1))[1];				//const idx for type info
					jc = c->constantPool[((UInt16*)(jc + 1))[0] - 1];	//utf8: name
					if(jc->type != JAVA_CONST_TYPE_STRING){
						
						fprintf(stderr,"  nti[0] doesn't refer to utf8 str\n");
						exit(-5);
					}
					if(((JavaString*)(jc + 1))->addr == 0xFFFFFF) fprintf(stderr, "string not ready (5)\n");
					putU24(((JavaString*)(jc + 1))->addr + 1); //+ 1 to point direct to string, not const type - helps the RT
					
					jc = c->constantPool[j - 1];	//utf8: type
					if(jc->type != JAVA_CONST_TYPE_STRING){
						
						fprintf(stderr,"  nti[1] doesn't refer to utf8 str\n");
						exit(-5);
					}
					if(((JavaString*)(jc + 1))->addr == 0xFFFFFF) fprintf(stderr, "string %d not ready (6)\n", j);
					putU24(((JavaString*)(jc + 1))->addr + 1); //+ 1 to point direct to string, not const type - helps the RT
					addr += 9;
					break;
				
				case JAVA_CONST_TYPE_NAME_TYPE_INFO:
					
					fprintf(stderr, "2 cannot produce ref to nti\n");
					exit(-5);
					break;
				
				default:
					
					fprintf(stderr, "2 cannot produce ref to weird constant type %d used\n", jc->type);
					exit(-5);
					break;
			}
		}
	}
	
	//write interfaces
	{
		putU16(c->numInterfaces);
		for(i = 0; i < c->numInterfaces; i++){
		
			jc = c->constantPool[c->interfaces[i] - 1];
			if(jc->type != JAVA_CONST_TYPE_CLASS){
					
				fprintf(stderr," mfi[8] doesn't refer to class\n");
				exit(-5);
			}
			jc = c->constantPool[((UInt16*)(jc + 1))[0] - 1];	//utf8: classname
			if(jc->type != JAVA_CONST_TYPE_STRING){
				
				fprintf(stderr," 2 cls/strref doesn't refer to utf8 str\n");
				exit(-5);
			}
			if(((JavaString*)(jc + 1))->addr == 0xFFFFFF) fprintf(stderr, "string %d not ready (7) (iface %d)\n", c->interfaces[i], i);
			putU24(((JavaString*)(jc + 1))->addr);
		}
	}
	
	//write methods
	{
		addr = hdrsz + crefs + interfaces + methods + fields + consts;
		putU16(c->numMethods);
		for(i = 0; i < c->numMethods; i++){
		
			UInt24 codeAddr = 0, nameAddr, typeAddr;
			UInt8 nameHash, typeHash;
			
			for(j = 0; j < c->methods[i]->numAttr; j++){
			
				ja = c->methods[i]->attributes[j];
				if(ja->type == J_ATTR_TYPE_CODE) break;
			}
			
			jc = c->constantPool[c->methods[i]->nameIdx - 1];	//utf8: method name
			if(jc->type != JAVA_CONST_TYPE_STRING){
				
				fprintf(stderr," 3 method name not uts8 string\n");
				exit(-5);
			}
			if(((JavaString*)(jc + 1))->addr == 0xFFFFFF) fprintf(stderr, "string %d not ready (8)\n", c->methods[i]->nameIdx);
			nameAddr = ((JavaString*)(jc + 1))->addr;
			nameHash = ujStrHash((JavaString*)(jc + 1));
			
			jc = c->constantPool[c->methods[i]->descrIdx - 1];	//utf8: method type
			if(jc->type != JAVA_CONST_TYPE_STRING){
				
				fprintf(stderr," 3 method type not uts8 string\n");
				exit(-5);
			}
			if(((JavaString*)(jc + 1))->addr == 0xFFFFFF) fprintf(stderr, "string %d not ready (9)\n", c->methods[i]->descrIdx);
			typeAddr = ((JavaString*)(jc + 1))->addr;
			typeHash = ujStrHash((JavaString*)(jc + 1));
			
			if(j != c->methods[i]->numAttr){	//have code
			
				codeAddr = addr + 4 + 2 + 8 * (UInt32)ja->data.code.numExceptions;
				addr += ja->data.code.codeLen + 4 + 2 + 8 * (UInt32)ja->data.code.numExceptions;
			}
		
			putU16(c->methods[i]->accessFlags);
			putU8(nameHash);
			putU8(typeHash);
			putU24(nameAddr);
			putU24(typeAddr);
			putU24(codeAddr);
		}
	}
	
	//write: fields
	{
	
		putU16(c->numFields);
		for(i = 0; i < c->numFields; i++){
		
			UInt8 nameHash, typeHash;
			UInt24 nameAdr, typeAdr;
			
			jc = c->constantPool[c->fields[i]->nameIdx - 1];	//utf8: method name
			if(jc->type != JAVA_CONST_TYPE_STRING){
				
				fprintf(stderr," 3 field name not uts8 string\n");
				exit(-5);
			}
			if(((JavaString*)(jc + 1))->addr == 0xFFFFFF) fprintf(stderr, "string %d not ready (10)\n", c->fields[i]->nameIdx);
			nameAdr = ((JavaString*)(jc + 1))->addr;
			nameHash = ujStrHash((JavaString*)(jc + 1));
			
			jc = c->constantPool[c->fields[i]->descrIdx - 1];	//utf8: method type
			if(jc->type != JAVA_CONST_TYPE_STRING){
				
				fprintf(stderr," 3 field type not uts8 string\n");
				exit(-5);
			}
			if(((JavaString*)(jc + 1))->addr == 0xFFFFFF) fprintf(stderr, "string %d not ready (11)\n", c->fields[i]->descrIdx);
			typeAdr = ((JavaString*)(jc + 1))->addr;
			typeHash = ujStrHash((JavaString*)(jc + 1));
			
			putU16(c->fields[i]->accessFlags);
			putU8(nameHash);
			putU8(typeHash);
			
			putU24(nameAdr);
			putU24(typeAdr);
		}
	}
	
	//write: code
	{
		for(i = 0; i < c->numMethods; i++){
			
			for(j = 0; j < c->methods[i]->numAttr; j++){
			
				ja = c->methods[i]->attributes[j];
				if(ja->type == J_ATTR_TYPE_CODE) break;
			}
			if(j == c->methods[i]->numAttr) continue;	//no code -> nothing to do
			
			//exception table first
			for(addr = 0; addr < ja->data.code.numExceptions; addr++){
			
				putU16(ja->data.code.exceptions[addr].start_pc);
				putU16(ja->data.code.exceptions[addr].end_pc);
				putU16(ja->data.code.exceptions[addr].handler_pc);
				putU16(ja->data.code.exceptions[addr].catchType);
			}
			
			//number of exceptions next
			putU16(ja->data.code.numExceptions);
			putU16(ja->data.code.maxLocals);
			putU16(ja->data.code.maxStack);
			
			//code finally
			for(addr = 0; addr < ja->data.code.codeLen; addr++){
			
				putU8(ja->data.code.code[addr]);	
			}
		}
	}
}

