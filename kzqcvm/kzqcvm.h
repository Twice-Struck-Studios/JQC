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
kzqcvm/kzqcvm.h
*/

//-----------------------------------------------------------------------------
#ifndef KZQCVM_KZQCVM_H
#define KZQCVM_KZQCVM_H
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <string>
#include <map>
#include <sstream>

#include "structs.h"
#include "errors.h"
#include "stringmanager.h"
#include "entitymanager.h"

//-----------------------------------------------------------------------------
namespace kzqcvm {
	using std::string;
	using std::map;
	using std::ostringstream;
//-----------------------------------------------------------------------------

/*
TYPES

Kzqcvm provides types for pointers into global and entity data, and
the values that they represent. This enables us to strictly type everything and
prevent arithmetic from being performed on data for which it makes no sense.

All types overload the cast to bool to represent a 'null' value, because the
types are passed around by value rather than reference and sometimes they may
not be able to return valid values or pointers.


VALUE TYPES
*/
class String;
class Entity;
class Field;
class Function;
/*
The value types provide a few methods:

const char *String::GetValue()
  Returns the C string that the String object represents.

ProgsDefinitionType Field::Type()
  Returns the type of a field. This is our best guess at a field type as the
  field values stored internally don't keep type data. Vectors share the same
  offset as their first float component, so the type may return vector when
  a float is expected. It's perfectly safe to accept in this situation, and
  other than this it's accurate.

bool Function::Run()
  Runs the function, returning when execution has finished. The return value
  is false if there was an error which halted execution, otherwise true.
  Function parameters can be set prior to running the function, and the return
  value can be read afterward. It is possible to run a builtin function this
  way.


POINTER TYPES
*/
class FloatPointer;
class VectorPointer;
class StringPointer;
class EntityPointer;
class FunctionPointer;
class FieldPointer;
/*
All pointers implement Get and Set functions to set or return their
corresponding value type, with the exception of VectorPointer which overloads
the [] operator to access the components. Use of invalid indices is truncated
into valid ones, though the data will likely then be erroneous.

Note:
* When setting we check that the values come from the same QCVM as the pointer
  we're writing to. Values from other QCVMs might not make sense.
  Functions and Fields would only be valid if the progs had an identical
  structure but this is currently not checked.


BUILTINS

Builtins use a simple function pointer callback. The qcvm is passed to the
builtin when the QC code calls it, along with the builtin number. This means
multiple builtin numbers could use the same callback if so desired.
Additionally, if a builtin isn't found then the interpreter will look for a
builtin numbered zero. This allows all builtins to be routed through a single
external function if so desired.
*/
class Kzqcvm;
typedef bool(*BuiltinCallback)(Kzqcvm *qcvm, int32_t builtinNum);

/*
The Kzqcvm is the main class. An instance of this represents any QCVM
regardless of its structure (in contrast to the original Quake interpreter
which was hard coded to one structure). It is down to the app itself to reject
files which do not meet its required structure, or to implement some kind of
flexibility.
*/
class Kzqcvm {
public:
	/*
	Constructs with a filename. It will try to load and validate the file.
	*/
	Kzqcvm(string filename);
	~Kzqcvm();

	/*
	Returns true if a valid QCVM is loaded, else false.
	*/
	bool IsLoaded() { return mQcvmData != NULL; }
	int32_t GetCRC() { if (!IsLoaded()) return 0; return mHeader->crc; }

	// DO NOT USE ANY OTHER FUNCTIONS IF IsLoaded() RETURNS FALSE

	/*
	This void pointer is for attaching your own data to the qcvm, which can
	be cast and accessed by builtins, for example.
	*/
	void* dataObject;

	// ---- GLOBALS -----------------------------------------------------------

	/*
	Global values are accessed with the following functions. They all return
	either a valid pointer, or a null pointer if no global could be found with
	the given name and the requested type.
	*/
	FloatPointer    GetFloatPointer   (string name);
	VectorPointer   GetVectorPointer  (string name);
	StringPointer   GetStringPointer  (string name);
	EntityPointer   GetEntityPointer  (string name);
	FunctionPointer GetFunctionPointer(string name);
	FieldPointer    GetFieldPointer   (string name);

	// ---- ENTITIES ----------------------------------------------------------

	/*
	Entities can be created and deleted with the following functions. The time
	value is used to specify the current game time. This is so that entity
	slots which are deleted cannot be reused within a given length of time.
	That period of time is specified by the const float ENTITY_REUSE_DELAY.
	To circumvent this mechanism, specify ENTITY_REUSE_DELAY when creating and
	zero when deleting, or set the constant to zero.
	*/
	Entity CreateEntity(int64_t time);
	void DeleteEntity(Entity entity, int64_t time);

	static constexpr float ENTITY_REUSE_DELAY = 2.0f;

	/*
	Field offsets can be retrieved with the following functions. They return a
	null field if none could be found with that name. If a type is given, then
	the field is also null if the type did not match.
	*/
	Field GetEntityField(string name);
	Field GetEntityField(string name, QcvmDefinitionType type);

	/*
	These are for iterating through all entities that are currently allocated.
	(i.e. have been created and not deleted. First is the lowest numbered
	entity which has been allocated. If there are no entities then
	GetFirstEntity will return a null Entity. NextEntity on the 'last' entity
	will loop round to the first again.
	*/
	Entity GetFirstEntity();
	Entity NextEntity(Entity entity);

	// Get a fields's type (Using Field.GetType is prefered)
	QcvmDefinitionType GetFieldType(Field f);

	/*
	These return pointers to the fields of specific entities. The entity and
	field must be from this QCVM.
	*/
	FloatPointer    GetFloatPointer   (Entity &entity, Field &field);
	VectorPointer   GetVectorPointer  (Entity &entity, Field &field);
	StringPointer   GetStringPointer  (Entity &entity, Field &field);
	EntityPointer   GetEntityPointer  (Entity &entity, Field &field);
	FunctionPointer GetFunctionPointer(Entity &entity, Field &field);
	FieldPointer    GetFieldPointer   (Entity &entity, Field &field);

	// ---- STRINGS -----------------------------------------------------------

	/*
	Create a temporary string. This is for builtins to return string data. All
	temporary strings should be cleared when the QCVM finishes running.
	*/
	String TempString(string s);
	void ClearTempStrings();

	/*
	Allocate and free permanent Strings. Allocated Strings persist until Freed.
	*/
	String Alloc(String s);
	bool   Free(String s);

	// Get a string's value (Using String.GetValue is prefered)
	char *GetStringValue(String s);

	// ---- FUNCTIONS & PARAMETERS --------------------------------------------

	/*
	Returns a Function which can then be Run().
	*/
	Function GetFunction(string name);

	// Run a function (Using Function.Run is prefered)
	bool RunFunction(Function &func);

	/*
	These return pointers to the return value and the eight parameter values
	respectively. The parameter value is always truncated to a value from 0
	to 7. They are guaranteed never to return a null pointer.
	*/
	FloatPointer    GetReturnFloatPointer   ();
	VectorPointer   GetReturnVectorPointer  ();
	StringPointer   GetReturnStringPointer  ();
	EntityPointer   GetReturnEntityPointer  ();
	FunctionPointer GetReturnFunctionPointer();
	FieldPointer    GetReturnFieldPointer   ();
	FloatPointer    GetParameterFloatPointer   (int parm);
	VectorPointer   GetParameterVectorPointer  (int parm);
	StringPointer   GetParameterStringPointer  (int parm);
	EntityPointer   GetParameterEntityPointer  (int parm);
	FunctionPointer GetParameterFunctionPointer(int parm);
	FieldPointer    GetParameterFieldPointer   (int parm);

	// ---- BUILTINS ----------------------------------------------------------

	/*
	Add and remove builtin callbacks. The number given is
	positive, the same as specified in QuakeC code.
	Adding a builtin with a number already used will replace the builtin.
	Attempting to remove a builtin which is not present will do nothing.
	*/
	void AddBuiltin(BuiltinCallback builtin, int number);
	void RemoveBuiltin(int number);
	/*
	Gets the number of a named builtin, or zero if no such name was found.
	*/
	int FindBuiltinNumber(string name);
	/*
	From within a BuiltinCallback, this will return the number of parameters
	which the builtin was called with.
	*/
	int NumCallParameters() { return mNumCallParameters; }

	// ---- REFLECTION --------------------------------------------------------

	/*
	These allow iterating through all entity fields to read data about them.
	*/
	int NumEntityFields();
	string GetFieldName(int i);
	QcvmDefinitionType GetFieldType(int i);
	Field GetField(int i);

	/*
	These allow iterating through all functions to read data about them.
	*/
	int NumFunctions();
	string GetFunctionName(int i);
	Function GetFunction(int i);

	// ---- ERROR REPORTING ---------------------------------------------------

	QcvmError GetLastError();
	string    GetErrorMessages();
	void      ClearErrors();

	/*
	Allow builtins to set errors.
	*/
	void BuiltinError(string errorName);
	void AddErrorLine(string message);

	// ------------------------------------------------------------------------

	// debug dump the contents to console, csv
	void Dump();

private:
	void Load();
	void Unload();
	bool RunFunction(int functionNum, int *instructionCount);

	static const int16_t OFS_RETURN = 1;
	static const int16_t OFS_PARM0  = 4;
	static const int16_t OFS_PARM1  = 7;
	static const int16_t OFS_PARM2  = 10;
	static const int16_t OFS_PARM3  = 13;
	static const int16_t OFS_PARM4  = 16;
	static const int16_t OFS_PARM5  = 19;
	static const int16_t OFS_PARM6  = 22;
	static const int16_t OFS_PARM7  = 25;

	string           mFilename;

	int32_t          mQcvmSize;
	char            *mQcvmData;

	QcvmHeader      *mHeader;
	QcvmStatement   *mStatements;
	QcvmDefinition  *mGlobalDefs;
	QcvmDefinition  *mFieldDefs;
	QcvmFunction    *mFunctions;
	char            *mStringData;
	float           *mGlobalData;

	static const int  GLOBALDEF_TYPE_MASK = 0x07;

	static const char GLOBAL_DEF_SYSTEM  = 1 << 0;
	static const char GLOBAL_DEF_FIELD   = 1 << 1;
	static const char GLOBAL_DEF_LOCAL   = 1 << 2;
	static const char GLOBAL_DEF_SPECIAL = 1 << 3;
	char            *mGlobalDefData;

	QcvmDefinitionType *mFieldOffsetTypes;

	string NameForGlobalOffset(int16_t ofs);
	string NameForGlobalOffset(int16_t ofs, QcvmDefinitionType type);
	string NameForFieldOffset(int16_t ofs);
	string NameForFieldOffset(int16_t ofs, QcvmDefinitionType type);

	StringManager    mStringManager;
	EntityManager    mEntityManager;

	// builtins
	map <int, BuiltinCallback> mBuiltins;
	int                        mNumCallParameters;
	bool RunBuiltin(int builtinNum);

	// errors
	QcvmError     mError;
	ostringstream mErrorLog;

	void StartError(QcvmError errorType, string errorName);
	void TraceFunction(QcvmFunction *func, QcvmStatement *programCounter);
};

//-----------------------------------------------------------------------------
} // namespace
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
