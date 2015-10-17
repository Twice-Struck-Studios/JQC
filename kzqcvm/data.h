/*
Kzqcvm QuakeC VM Interpreter
Copyright (c) 2010 David Laurie

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
/*
kzqcvm/data.h
*/

//-----------------------------------------------------------------------------
#ifndef KZQCVM_DATA_H
#define KZQCVM_DATA_H
//-----------------------------------------------------------------------------

#include <assert.h>
#include <string>
#include <iostream>

#include "kzqcvm.h"
#include "structs.h"

//-----------------------------------------------------------------------------
namespace kzqcvm {
	using std::string;
//-----------------------------------------------------------------------------

// Note: Cast to bool is overridden, and represents invalid values. It doesn't
// bounds check, it's just to be sure a function didn't intentionally return
// a null value or pointer.

//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------

class String {
	friend class Kzqcvm;
	friend class StringPointer;
public:
	const char* GetValue() { return qcvm->GetStringValue(*this); }
	operator bool() { return stringNum != 0; }
	String() : qcvm(0), stringNum(0) { }
private:
	String(Kzqcvm *vm, int32_t str) : qcvm(vm), stringNum(str) { }
	Kzqcvm  *qcvm;
	int32_t  stringNum;
};

class Entity {
	friend class Kzqcvm;
	friend class EntityPointer;
public:
	Entity Next() { return qcvm->NextEntity(*this); }
	operator bool() { return entNum >= 0; }
	Entity() : qcvm(0), entNum(-1) { }
private:
	Entity(Kzqcvm *vm, int32_t ent) : qcvm(vm), entNum(ent) { }
	Kzqcvm  *qcvm;
	int32_t  entNum;
};

class Field {
	friend class Kzqcvm;
	friend class FieldPointer;
public:
	QcvmDefinitionType Type() { return qcvm->GetFieldType(*this); }
	operator bool() { return offset >= 0; }
	Field() : qcvm(0), offset(0) { }
private:
	Field(Kzqcvm *vm, int32_t ofs) : qcvm(vm), offset(ofs) { }
	Kzqcvm  *qcvm;
	int32_t  offset;
};

class Function {
	friend class Kzqcvm;
	friend class FunctionPointer;
public:
	bool Run() { return qcvm->RunFunction(*this); }
	operator bool() { return number > 0; }
	Function() : qcvm(0), number(0) { }
private:
	Function(Kzqcvm *vm, int32_t num) : qcvm(vm), number(num) { }
	Kzqcvm  *qcvm;
	int32_t  number;
};

//-----------------------------------------------------------------------------
// Pointers
//-----------------------------------------------------------------------------

// Don't access this. It's just so the inline Pointer functions know if
// they're null.
extern float null_data[3];

class FloatPointer {
	friend class Kzqcvm;
public:
	float Get() { return *value; }
	void Set(float f) { *value = f; }
	operator bool() const { return value != null_data; }
	FloatPointer() : qcvm(0), value(null_data) { }
private:
    FloatPointer(Kzqcvm *vm, float *v) : qcvm(vm), value(v) { }
	Kzqcvm *qcvm;
	float  *value;
};

class VectorPointer {
	friend class Kzqcvm;
public:
	float Get(int i) { return value[i&3]; }
	void  Set(int i, float f) { value[i&3] = f; }
	float &operator[](const unsigned int i) { return value[i&3]; }
	operator bool() const { return value != null_data; }
	VectorPointer() : qcvm(0), value(null_data) { }
private:
    VectorPointer(Kzqcvm *vm, float *v) : qcvm(vm), value(v) { }
	Kzqcvm *qcvm;
	float  *value;
};

// We check that String and Entity data values come from within the same qcvm.
#define QCVM_CHECK(a) assert(qcvm == a.qcvm)
// We check that Field and Function data values come from qcvms with the same
// structure.
#define QCVM_CHECK_CRC(a) assert(qcvm->GetCRC() == a.qcvm->GetCRC())

class StringPointer {
	friend class Kzqcvm;
public:
	String Get() { return String(qcvm, *value); }
	void Set(String s) { QCVM_CHECK(s); *value = s.stringNum; }
	operator bool() const { return value != (int*)null_data; }
	StringPointer() : qcvm(0), value((int*)null_data) { }
private:
    StringPointer(Kzqcvm *vm, int32_t *v) : qcvm(vm), value(v) { }
	Kzqcvm  *qcvm;
	int32_t *value;
};

class EntityPointer {
	friend class Kzqcvm;
public:
	Entity Get() { return Entity(qcvm, *value); }
	void Set(Entity e) { QCVM_CHECK(e); *value = e.entNum; }
	operator bool() const { return value != (int*)null_data; }
	EntityPointer() : qcvm(0), value((int*)null_data) { }
private:
    EntityPointer(Kzqcvm *vm, int32_t *v) : qcvm(vm), value(v) { }
	Kzqcvm  *qcvm;
	int32_t *value;
};

class FieldPointer {
	friend class Kzqcvm;
public:
	Field Get() { return Field(qcvm, *value); }
	void Set(Field f) { QCVM_CHECK(f); *value = f.offset; }
	operator bool() const { return value != (int*)null_data; }
	FieldPointer() : qcvm(0), value((int*)null_data) { }
private:
    FieldPointer(Kzqcvm *vm, int32_t *v) : qcvm(vm), value(v) { }
	Kzqcvm  *qcvm;
	int32_t *value;
};

class FunctionPointer {
	friend class Kzqcvm;
public:
	Function Get() { return Function(qcvm, *value); }
	void Set(Function f) { QCVM_CHECK(f); *value = f.number; }
	operator bool() const { return value != (int*)null_data; }
	FunctionPointer() : qcvm(0), value((int*)null_data) { }
private:
    FunctionPointer(Kzqcvm *vm, int32_t *v) : qcvm(vm), value(v) { }
	Kzqcvm  *qcvm;
	int32_t *value;
};

//-----------------------------------------------------------------------------
} // namespace
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
