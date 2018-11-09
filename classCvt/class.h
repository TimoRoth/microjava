#ifndef _CLASS_H_
#define _CLASS_H_

#include "common.h"

#define JAVA_CONST_TYPE_INVALID		0xEE	//used for second slot of double/long

#define JAVA_CONST_TYPE_STRING		1	//2 bytes + string
#define JAVA_CONST_TYPE_INT		3	//4 bytes
#define JAVA_CONST_TYPE_FLOAT		4	//4 bytes
#define JAVA_CONST_TYPE_LONG		5	//8 bytes
#define JAVA_CONST_TYPE_DOUBLE		6	//8 bytes
#define JAVA_CONST_TYPE_CLASS		7	//2 bytes (index back into constant pool pointing to class name string)
#define JAVA_CONST_TYPE_STR_REF		8	//2 bytes (index back into constant pool pointing to string)
#define JAVA_CONST_TYPE_FIELD		9	//4 bytes (2x index back into constant pool: class & name&type info)
#define JAVA_CONST_TYPE_METHOD		10	//4 bytes (2x index back into constant pool: class & name&type info)
#define JAVA_CONST_TYPE_INTERFACE	11	//4 bytes (2x index back into constant pool: class & name&type info) (interfce METHOD)
#define JAVA_CONST_TYPE_NAME_TYPE_INFO	12	//4 bytes (2x index back into constant pool: method name string & type info string)

#define JAVA_TYPE_BYTE		'B'
#define JAVA_TYPE_CHAR		'C'
#define JAVA_TYPE_DOUBLE	'D'
#define JAVA_TYPE_FLOAT		'F'
#define JAVA_TYPE_INT		'I'
#define JAVA_TYPE_LONG		'J'
#define JAVA_TYPE_SHORT		'S'
#define JAVA_TYPE_BOOL		'Z'
#define JAVA_TYPE_ARRAY		'['
#define JAVA_TYPE_OBJ		'L'
#define JAVA_TYPE_OBJ_END	';'

#define J_ATTR_TYPE_GENERIC	0
#define J_ATTR_TYPE_CODE	1

typedef struct{

	UInt16 start_pc;
	UInt16 end_pc;
	UInt16 handler_pc;
	UInt16 catchType;

}JavaExceptionTableEntry;

typedef struct {
		
	UInt16 maxStack;	//in elements
	UInt16 maxLocals;	//in elements
	
	UInt16 numExceptions;	//num entries in exception table
	JavaExceptionTableEntry* exceptions;
	
	UInt16 numAttributes;
	struct JavaAttribute** attributes;
	
	UInt32 codeLen;
	UInt8 code[];

}JavaCodeAttr;

typedef struct JavaAttribute{

	UInt16 nameIdx;
	UInt8 type;
	union{
		struct{
			
			UInt32 len;
			UInt8 data[];
		}generic;
		
		JavaCodeAttr code;
	}data;

}JavaAttribute;

typedef struct{

	UInt8 type;			//see JAVA_CONST_TYPE_NAME_TYPE_* constants
	Boolean used;
	Boolean directUsed;
	//...
	
}JavaConstant;

typedef struct{

	UInt16 accessFlags;
	UInt16 nameIdx;
	UInt16 descrIdx;
	UInt16 numAttr;
	JavaAttribute** attributes;

	UInt16 offset;
	char type;

}JavaMethodOrField;

typedef struct{

	UInt16 len;
	UInt24 addr;	//used later
	char data[];	//not null-terminated

}JavaString;


#define ACCESS_FLAG_PUBLIC		0x0001
#define ACCESS_FLAG_PRIVATE		0x0002
#define ACCESS_FLAG_PROTECTED		0x0004
#define ACCESS_FLAG_STATIC		0x0008
#define ACCESS_FLAG_FINAL		0x0010
#define ACCESS_FLAG_SUPER		0x0020	//for classes: older compilers generate code with this flag clear. we *do not* support that
#define ACCESS_FLAG_SYNCHRONIZED	0x0020	//for non-classes
#define ACCESS_FLAG_NATIVE 		0x0100
#define ACCESS_FLAG_INTERFACE		0x0200
#define ACCESS_FLAG_ABSTARCT		0x0400
#define ACCESS_FLAG_STRICT		0x0800	//floating point is strict mode

typedef struct{

	UInt16 constantPoolSz;
	
	/* if these are not equal to the above, STRICT ordering of cunstant must exist: first addressable, then just placed, then neither */
	UInt16 addressblConstantPoolSz;	//export constant address as if we had only this many
	UInt16 placedConstantPoolSz;	//export constant data as if we had only this many
	
	JavaConstant** constantPool;	//constantPoolSz - 1 items (index starts at 1)
	
	UInt16 accessFlags;		//see ACCESS_FLAG_* constants
	
	UInt16 thisClass;		//index into constant pool
	UInt16 superClass;		//index into constant pool or 0 (if this class is Object class only)
	
	UInt16 numInterfaces;
	UInt16* interfaces;		//numInterfaces items
	
	UInt16 numFields;
	JavaMethodOrField** fields;	//numFields items
	
	UInt16 numMethods;
	JavaMethodOrField** methods;	//numMethods items
	
	UInt16 numAttributes;
	JavaAttribute** attributes;	//numAttributes items

}JavaClass;


#define CLASS_CONSTANT(class,idx)		(class->constantPool[idx - 1])


#endif

