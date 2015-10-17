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
kzqcvm/load.cpp
*/

#include "kzqcvm.h"
#include "instructions.h"

#include <string.h>
#include <iostream>
#include <fstream>

//-----------------------------------------------------------------------------
namespace kzqcvm {
	using std::cout;
	using std::endl;
	using std::ifstream;
	using std::ios;
	using std::ios_base;
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Load
//-----------------------------------------------------------------------------

void Kzqcvm::Load()
{
	// open
	ifstream progsFile;
	progsFile.open(mFilename.c_str(), ios::binary | ios::in);
	if (!progsFile.is_open() || !progsFile.good() || progsFile.eof())
	{
		cout << "Could not open Progs " << mFilename << endl;
		return;
	}

	// get file size
	progsFile.seekg(0, ios_base::end);
	ifstream::pos_type end = progsFile.tellg();
	progsFile.seekg(0, ios_base::beg);
	ifstream::pos_type beg = progsFile.tellg();
	int fileSize = end - beg;

	// allocate and read
	mQcvmSize = fileSize;
	mQcvmData = new char[mQcvmSize];
	progsFile.read(mQcvmData, mQcvmSize);
	progsFile.close();

	// validate the header and read the arrays
	if (mQcvmSize < (int32_t)sizeof(QcvmHeader))
	{
		cout << "Progs " << mFilename << " is too small (smaller than header)" << endl;
		Unload();
		return;
	}
	mHeader = (QcvmHeader*)&mQcvmData[0];
	if (mHeader->version != 6)
	{
		cout << "Progs " << mFilename << " is an invalid version: " << mHeader->version << " should be 6" << endl;
		Unload();
		return;
	}

	// set up the other structs, bounds checking that they fit within the data
#define SETUP_PROGS_LUMP(ofs, num, type, pointer) \
	if ((ofs) + (num)*(int32_t)sizeof(type) > mQcvmSize || (ofs) < 0) \
	{ \
		cout << "Statements lump exceeds data size in " << mFilename << endl; \
		Unload(); \
		return; \
	} \
	pointer = (type*)&mQcvmData[(ofs)];
	SETUP_PROGS_LUMP(mHeader->statements_offset, mHeader->statements_num,  QcvmStatement,  mStatements)
	SETUP_PROGS_LUMP(mHeader->globaldefs_offset, mHeader->globaldefs_num,  QcvmDefinition, mGlobalDefs)
	SETUP_PROGS_LUMP(mHeader->fielddefs_offset,  mHeader->fielddefs_num,   QcvmDefinition, mFieldDefs )
	SETUP_PROGS_LUMP(mHeader->functions_offset,  mHeader->functions_num,   QcvmFunction,   mFunctions )
	SETUP_PROGS_LUMP(mHeader->stringdata_offset, mHeader->stringdata_size, char,           mStringData)
	SETUP_PROGS_LUMP(mHeader->globaldata_offset, mHeader->globaldata_num,  float,          mGlobalData)
#undef SETUP_PROGS_LUMP

	// Bounds checking - globaldefs
	for (int i=0; i<mHeader->globaldefs_num; ++i)
	{
		if (mGlobalDefs[i].nameOffset < 0 || mGlobalDefs[i].nameOffset >= mHeader->stringdata_size)
		{
			cout << "GlobalDef " << i << " name offset out of bounds in " << mFilename << endl;
			Unload();
			return;
		}
		if (mGlobalDefs[i].offset < 0 || mGlobalDefs[i].offset >= mHeader->globaldata_num)
		{
			cout << "GlobalDef " << i << " offset out of bounds in " << mFilename << endl;
			Unload();
			return;
		}
	}
	// Bounds checking - fielddefs
	for (int i=0; i<mHeader->fielddefs_num; ++i)
	{
		if (mFieldDefs[i].nameOffset < 0 || mFieldDefs[i].nameOffset >= mHeader->stringdata_size)
		{
			cout << "FieldDef " << i << " name offset out of bounds in " << mFilename << endl;
			Unload();
			return;
		}
		if (mFieldDefs[i].offset < 0 || mFieldDefs[i].offset >= mHeader->entity_size)
		{
			cout << "FieldDef " << i << " offset out of bounds in " << mFilename << endl;
			Unload();
			return;
		}
	}
	// Bounds checking - functions
	for (int i=0; i<mHeader->functions_num; ++i)
	{
		if (mFunctions[i].offsetFirstStatement >= mHeader->statements_num)
		{
			cout << "Function " << i << " first statement out of bounds in " << mFilename << endl;
			Unload();
			return;
		}
		if (mFunctions[i].offsetLocalsInGlobals < 0 ||
			mFunctions[i].offsetLocalsInGlobals + mFunctions[i].numLocals >= mHeader->globaldata_num)
		{
			cout << "Function " << i << " local parameters out of bounds in " << mFilename << endl;
			Unload();
			return;
		}
		if (mFunctions[i].nameOffset < 0 || mFunctions[i].nameOffset >= mHeader->stringdata_size)
		{
			cout << "Function " << i << " name offset out of bounds in " << mFilename << endl;
			Unload();
			return;
		}
		if (mFunctions[i].fileNameOffset < 0 || mFunctions[i].fileNameOffset >= mHeader->stringdata_size)
		{
			cout << "Function " << i << " filename offset out of bounds in " << mFilename << endl;
			Unload();
			return;
		}
	}
	// Bounds checking - statements
	for (int i=0; i<mHeader->statements_num; ++i)
	{
		if (mStatements[i].instruction < Instructions::MIN || mStatements[i].instruction > Instructions::MAX)
		{
			cout << "Instruction " << i << " is invalid in " << mFilename << endl;
			Unload();
			return;
		}

#define BOUNDS_CHECK_GLOBAL(n) \
			if (mStatements[i].parameter[(n)] < 0 || mStatements[i].parameter[(n)] >= mHeader->globaldata_num)\
			{\
				cout << "Instruction " << i << " parameter " << (n) << " out of bounds (globals) in " << mFilename << endl;\
				Unload();\
				return;\
			}
		int offset;
		switch (mStatements[i].instruction)
		{
		case Instructions::DONE:
			break;
		case Instructions::LOAD_F:
		case Instructions::LOAD_V:
		case Instructions::LOAD_S:
		case Instructions::LOAD_ENT:
		case Instructions::LOAD_FLD:
		case Instructions::LOAD_FNC:
		case Instructions::STORE_F:
		case Instructions::STORE_V:
		case Instructions::STORE_S:
		case Instructions::STORE_ENT:
		case Instructions::STORE_FLD:
		case Instructions::STORE_FNC:
		case Instructions::STOREP_F:
		case Instructions::STOREP_V:
		case Instructions::STOREP_S:
		case Instructions::STOREP_ENT:
		case Instructions::STOREP_FLD:
		case Instructions::STOREP_FNC:
			BOUNDS_CHECK_GLOBAL(0)
			BOUNDS_CHECK_GLOBAL(1)
			break;
		case Instructions::NOT_F:
		case Instructions::NOT_V:
		case Instructions::NOT_S:
		case Instructions::NOT_ENT:
		case Instructions::NOT_FNC:
		case Instructions::STATE:
			BOUNDS_CHECK_GLOBAL(0)
			BOUNDS_CHECK_GLOBAL(2)
			break;
		case Instructions::IF:
		case Instructions::IFNOT:
			BOUNDS_CHECK_GLOBAL(0)
			offset = i + mStatements[i].parameter[1];
			if (offset < 0 || offset >= mHeader->statements_num)
			{
				cout << "Instruction " << i << " parameter 1 out of bounds (statements) in " << mFilename << endl;
				Unload();
				return;
			}
			break;
		case Instructions::CALL0:
		case Instructions::CALL1:
		case Instructions::CALL2:
		case Instructions::CALL3:
		case Instructions::CALL4:
		case Instructions::CALL5:
		case Instructions::CALL6:
		case Instructions::CALL7:
		case Instructions::CALL8:
		case Instructions::RETURN:
			BOUNDS_CHECK_GLOBAL(0)
			break;
		case Instructions::GOTO:
			offset = i + mStatements[i].parameter[0];
			if (offset < 0 || offset >= mHeader->statements_num)
			{
				cout << "Instruction " << i << " parameter 0 out of bounds (statements) in " << mFilename << endl;
				Unload();
				return;
			}
			break;
		default:
			BOUNDS_CHECK_GLOBAL(0)
			BOUNDS_CHECK_GLOBAL(1)
			BOUNDS_CHECK_GLOBAL(2)
			break;
		}
	}

	// make sure the string data is null terminated
	mStringData[mHeader->stringdata_size - 1] = '\0';

	// make sure the last instruction is 'DONE'
	mStatements[mHeader->statements_num - 1].instruction = Instructions::DONE;

	// write our global def metadata
	mGlobalDefData = new char[mHeader->globaldefs_num];
	// first build a temporary list of local variables
	char isLocal[mHeader->globaldata_num];
	for (int i=0; i<mHeader->globaldata_num; ++i)
	{
		isLocal[i] = 0;
	}
	for (int i=0; i<mHeader->functions_num; ++i)
	{
		for (int j=0; j<mFunctions[i].numLocals; ++j)
		{
			isLocal[mFunctions[i].offsetLocalsInGlobals+j] = true;
		}
	}
	// then parse all the globals
	bool end_sys = false;
	for (int i=0; i<mHeader->globaldata_num; ++i)
	{
		if (!end_sys)
		{
			mGlobalDefData[i] = 0;
			if (!end_sys)
				mGlobalDefData[i] |= GLOBAL_DEF_SYSTEM;
			if (!strcmp(&mStringData[mGlobalDefs[i].nameOffset], "end_sys_globals"))
				mGlobalDefData[i] |= GLOBAL_DEF_SPECIAL;
			if (!strcmp(&mStringData[mGlobalDefs[i].nameOffset], "end_sys_fields"))
			{
				mGlobalDefData[i] |= GLOBAL_DEF_SPECIAL;
				end_sys = true;
			}
			if (isLocal[mGlobalDefs[i].offset])
			{
				mGlobalDefData[i] |= GLOBAL_DEF_LOCAL;
			}
		}
	}

	// make a reverse lookup table for field types per offset
	mFieldOffsetTypes = new QcvmDefinitionType[mHeader->entity_size];
	for (int i=0; i<mHeader->entity_size; ++i)
	{
		mFieldOffsetTypes[i] = NOTYPE;
	}
	for (int i=0; i<mHeader->fielddefs_num; ++i)
	{
		QcvmDefinitionType type = QcvmDefinitionType(mFieldDefs[i].type);
		int16_t offset = mFieldDefs[i].offset;
		if (mFieldOffsetTypes[offset] != VECTOR)
			mFieldOffsetTypes[offset] = type;
	}

	// init the managers
	mEntityManager.Init(mHeader->entity_size, ENTITY_REUSE_DELAY);
	mStringManager.Init(mStringData, mHeader->stringdata_size);

	// and we're done
	cout << "Successfully loaded progs " << mFilename << endl;
}

//-----------------------------------------------------------------------------
} // namespace
//-----------------------------------------------------------------------------
