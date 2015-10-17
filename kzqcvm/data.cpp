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
kzqcvm/data.cpp
*/

#include <stdio.h>

#include "kzqcvm.h"
#include "data.h"

#include <assert.h>
#include <map>
#include <iostream>

//-----------------------------------------------------------------------------
namespace kzqcvm {
	using std::pair;
	using std::cout;
	using std::endl;
//-----------------------------------------------------------------------------

float null_data[3] = { 0.0f, 0.0f, 0.0f };

//-----------------------------------------------------------------------------
// Pointers to globals
//-----------------------------------------------------------------------------

// Search through globaldefs, return one with an offset matching the name and
// type, else return a null pointer.
#define GLOBALPOINTERFUNC(returnType, cast, typeNum)\
returnType Kzqcvm::Get ## returnType(string name) {\
	for (int i=0; i<mHeader->globaldefs_num; ++i) {\
		if ((mGlobalDefs[i].type & GLOBALDEF_TYPE_MASK) == typeNum) {\
			if ((mGlobalDefData[i] & (GLOBAL_DEF_SPECIAL | GLOBAL_DEF_LOCAL)) == 0) {\
				if (name.compare(&mStringData[mGlobalDefs[i].nameOffset]) == 0) {\
					return returnType(this, (cast)&mGlobalData[mGlobalDefs[i].offset]);\
				}\
			}\
		}\
	}\
	return returnType(this, (cast)null_data);\
}

GLOBALPOINTERFUNC(FloatPointer,    float*, FLOAT)
GLOBALPOINTERFUNC(VectorPointer,   float*, VECTOR)
GLOBALPOINTERFUNC(StringPointer,   int*,   STRING)
GLOBALPOINTERFUNC(EntityPointer,   int*,   ENTITY)
GLOBALPOINTERFUNC(FunctionPointer, int*,   FUNCTION)
GLOBALPOINTERFUNC(FieldPointer,    int*,   FIELD)

//-----------------------------------------------------------------------------
// Fields and entities
//-----------------------------------------------------------------------------

// Note: We could have different types of field, but there might make the
// interface too confusing. Just check type before you get a pointer to a
// field of an entity.

Entity Kzqcvm::CreateEntity(int64_t time)
{
	return Entity(this, mEntityManager.CreateEntity(time));
}

void Kzqcvm::DeleteEntity(Entity entity, int64_t time)
{
	assert(entity.qcvm == this);
	mEntityManager.DeleteEntity(entity.entNum, time);
}

Field Kzqcvm::GetEntityField(string name)
{
	for (int i=0; i<mHeader->fielddefs_num; ++i)
	{
		if (name.compare(&mStringData[mFieldDefs[i].nameOffset]) == 0)
		{
			return Field(this, mFieldDefs[i].offset);
		}
	}
	return Field(this, -1);
}

Field Kzqcvm::GetEntityField(string name, QcvmDefinitionType type)
{
	// we can match type exactly because we know the name of a field
	for (int i=0; i<mHeader->fielddefs_num; ++i)
	{
		if (mFieldOffsetTypes[mFieldDefs[i].offset] == type)
		{
			if (name.compare(&mStringData[mFieldDefs[i].nameOffset]) == 0)
			{
				return Field(this, mFieldDefs[i].offset);
			}
		}
	}
	return Field(this, -1);
}

Entity Kzqcvm::GetFirstEntity()
{
	return Entity(this, mEntityManager.GetFirstEntity());
}

Entity Kzqcvm::NextEntity(Entity entity)
{
	return Entity(this, mEntityManager.GetNextEntity(entity.entNum));
}

// Note: We can't tell if we have the first component of a vector or the vector
// itself, so we'll always return float in that situation.
QcvmDefinitionType Kzqcvm::GetFieldType(Field f)
{
	if (f.offset < mHeader->entity_size || f.offset <= 0)
		return NOTYPE;
	return mFieldOffsetTypes[f.offset];
}

//-----------------------------------------------------------------------------
// Addressing entity data
//-----------------------------------------------------------------------------

#define ENTITYPOINTERFUNC(returnType, cast)\
returnType Kzqcvm::Get ## returnType(Entity &entity, Field &field) {\
	assert(field.qcvm->GetCRC() == GetCRC());\
	assert(entity.qcvm == this);\
	cast value = (cast)mEntityManager.GetPointer(entity.entNum, field.offset);\
	if (!value) return returnType(this, (cast)null_data);\
	return returnType(this, value);\
}

ENTITYPOINTERFUNC(FloatPointer,    float*)
ENTITYPOINTERFUNC(VectorPointer,   float*)
ENTITYPOINTERFUNC(StringPointer,   int*)
ENTITYPOINTERFUNC(EntityPointer,   int*)
ENTITYPOINTERFUNC(FunctionPointer, int*)
ENTITYPOINTERFUNC(FieldPointer,    int*)

//-----------------------------------------------------------------------------
// Strings
//-----------------------------------------------------------------------------

String Kzqcvm::TempString(string s)
{
	return String(this, mStringManager.TempString(s));
}

void Kzqcvm::ClearTempStrings()
{
	mStringManager.ClearTempStrings();
}

// Allows cross-progs zoning - i.e. we zone a string from another progs, and
// get a string valid for this progs.
String Kzqcvm::Alloc(String s)
{
	return String(this, mStringManager.Zone(string(s.GetValue())));
}

bool Kzqcvm::Free(String s)
{
	assert(s.qcvm == this);
	return mStringManager.Unzone(s.stringNum);
}

char *Kzqcvm::GetStringValue(String s)
{
	assert(s.qcvm == this);
	return mStringManager.GetString(s.stringNum);
}

//-----------------------------------------------------------------------------
// Parameter and return value access
//-----------------------------------------------------------------------------

#define RETURNFUNC(type, cast)\
type Kzqcvm::GetReturn ## type()\
{ return type(this, (cast)&mGlobalData[OFS_RETURN]); }

RETURNFUNC(FloatPointer,    float*)
RETURNFUNC(VectorPointer,   float*)
RETURNFUNC(StringPointer,   int*)
RETURNFUNC(EntityPointer,   int*)
RETURNFUNC(FunctionPointer, int*)
RETURNFUNC(FieldPointer,    int*)

#define PARAMETERFUNC(type, cast)\
type Kzqcvm::GetParameter ## type(int parm)\
{ return type(this, (cast)&mGlobalData[OFS_PARM0+(parm&7)*3]); }

PARAMETERFUNC(FloatPointer,    float*)
PARAMETERFUNC(VectorPointer,   float*)
PARAMETERFUNC(StringPointer,   int*)
PARAMETERFUNC(EntityPointer,   int*)
PARAMETERFUNC(FunctionPointer, int*)
PARAMETERFUNC(FieldPointer,    int*)

//-----------------------------------------------------------------------------
// Reflection
//-----------------------------------------------------------------------------

int Kzqcvm::NumEntityFields()
{
	return mHeader->fielddefs_num;
}

string Kzqcvm::GetFieldName(int i)
{
	if (i <= 0 || i >= mHeader->fielddefs_num)
		return "";
	return &mStringData[mFieldDefs[i].nameOffset];
}

QcvmDefinitionType Kzqcvm::GetFieldType(int i)
{
	if (i <= 0 || i >= mHeader->fielddefs_num)
		return NOTYPE;
	return QcvmDefinitionType(mFieldDefs[i].type & 0xff);
}

Field Kzqcvm::GetField(int i)
{
	if (i <= 0 || i >= mHeader->fielddefs_num)
		return Field(this, 0);
	return Field(this, mFieldDefs[i].offset);
}


int Kzqcvm::NumFunctions()
{
	return mHeader->functions_num;
}

string Kzqcvm::GetFunctionName(int i)
{
	if (i <= 0 || i >= mHeader->functions_num)
		return "";
	return &mStringData[mFunctions[i].nameOffset];
}

Function Kzqcvm::GetFunction(int i)
{
	if (i <= 0 || i >= mHeader->functions_num)
		return Function(this, 0);
	return Function(this, mFunctions[i].offsetFirstStatement);
}

//-----------------------------------------------------------------------------
} // namespace
//-----------------------------------------------------------------------------
