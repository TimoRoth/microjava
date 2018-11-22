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

	uint16_t start_pc;
	uint16_t end_pc;
	uint16_t handler_pc;
	uint16_t catchType;

}JavaExceptionTableEntry;

typedef struct {

	uint16_t maxStack;	//in elements
	uint16_t maxLocals;	//in elements

	uint16_t numExceptions;	//num entries in exception table
	JavaExceptionTableEntry* exceptions;

	uint16_t numAttributes;
	struct JavaAttribute** attributes;

	uint32_t codeLen;
	uint8_t code[];

}JavaCodeAttr;

typedef struct JavaAttribute{

	uint16_t nameIdx;
	uint8_t type;
	union{
		struct{

			uint32_t len;
			uint8_t data[];
		}generic;

		JavaCodeAttr code;
	}data;

}JavaAttribute;

typedef struct{

	uint8_t type;			//see JAVA_CONST_TYPE_NAME_TYPE_* constants
	bool used;
	bool directUsed;
	//...

}JavaConstant;

typedef struct{

	uint16_t accessFlags;
	uint16_t nameIdx;
	uint16_t descrIdx;
	uint16_t numAttr;
	JavaAttribute** attributes;

	uint16_t offset;
	char type;

}JavaMethodOrField;

typedef struct{

	uint16_t len;
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

	uint16_t constantPoolSz;

	/* if these are not equal to the above, STRICT ordering of cunstant must exist: first addressable, then just placed, then neither */
	uint16_t addressblConstantPoolSz;	//export constant address as if we had only this many
	uint16_t placedConstantPoolSz;	//export constant data as if we had only this many

	JavaConstant** constantPool;	//constantPoolSz - 1 items (index starts at 1)

	uint16_t accessFlags;		//see ACCESS_FLAG_* constants

	uint16_t thisClass;		//index into constant pool
	uint16_t superClass;		//index into constant pool or 0 (if this class is Object class only)

	uint16_t numInterfaces;
	uint16_t* interfaces;		//numInterfaces items

	uint16_t numFields;
	JavaMethodOrField** fields;	//numFields items

	uint16_t numMethods;
	JavaMethodOrField** methods;	//numMethods items

	uint16_t numAttributes;
	JavaAttribute** attributes;	//numAttributes items

}JavaClass;


#define CLASS_CONSTANT(class,idx)		(class->constantPool[idx - 1])


#endif

