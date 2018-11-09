#ifndef _CLASS_IMPORTER_H_
#define _CLASS_IMPORTER_H_

#include "common.h"
#include "class.h"

//notes: all integers stored in class files are big-endian

typedef UInt16 classImporterReadF(void* paramP);
#define CLASS_IMPORT_READ_F_FAIL	0x1000

JavaClass* classImport(classImporterReadF readF, void* readD);
void classDump(JavaClass* c);
void classExport(JavaClass* c);
void classFree(JavaClass* c);

//lower-level but still used externally
void classFreeConstant(JavaConstant* f);

#endif

