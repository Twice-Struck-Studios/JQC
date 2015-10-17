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
kzqcvm/kzqcvm.cpp
*/

#include "kzqcvm.h"
#include "instructions.h"

#include <iostream>

//-----------------------------------------------------------------------------
namespace kzqcvm {
	using std::cout;
	using std::endl;
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Structors
//-----------------------------------------------------------------------------

Kzqcvm::Kzqcvm(string filename)
{
	mFilename   = filename;

	mQcvmSize   = 0;
	mQcvmData   = NULL;
	mHeader     = NULL;
	mStatements = NULL;
	mGlobalDefs = NULL;
	mFieldDefs  = NULL;
	mFunctions  = NULL;
	mStringData = NULL;
	mGlobalData = NULL;

	mGlobalDefData = NULL;
	mFieldOffsetTypes = NULL;

	mError      = ERR_NONE;

	dataObject  = NULL;

	Load();
}

Kzqcvm::~Kzqcvm()
{
	Unload();
}

//-----------------------------------------------------------------------------
// Unload
//-----------------------------------------------------------------------------

void Kzqcvm::Unload()
{
	delete[] mQcvmData;
	mQcvmSize   = 0;
	mQcvmData   = NULL;
	mHeader     = NULL;
	mStatements = NULL;
	mGlobalDefs = NULL;
	mFieldDefs  = NULL;
	mFunctions  = NULL;
	mStringData = NULL;
	mGlobalData = NULL;

	delete[] mGlobalDefData;
	delete[] mFieldOffsetTypes;
}

//-----------------------------------------------------------------------------
// Name for global offset
//-----------------------------------------------------------------------------

string Kzqcvm::NameForGlobalOffset(int16_t ofs)
{
	for (int i=0; i<mHeader->globaldefs_num; ++i)
	{
		if (mGlobalDefs[i].offset == ofs)
		{
			return string(&mStringData[mGlobalDefs[i].nameOffset]);
		}
	}
	return "?";
}

string Kzqcvm::NameForGlobalOffset(int16_t ofs, QcvmDefinitionType type)
{
	for (int i=0; i<mHeader->globaldefs_num; ++i)
	{
		if (((mGlobalDefs[i].type & GLOBALDEF_TYPE_MASK) == type) && (mGlobalDefs[i].offset == ofs))
		{
			return string(&mStringData[mGlobalDefs[i].nameOffset]);
		}
	}
	return "?";
}

string Kzqcvm::NameForFieldOffset(int16_t ofs)
{
	for (int i=0; i<mHeader->fielddefs_num; ++i)
	{
		if (mFieldDefs[i].offset == ofs)
		{
			return string(&mStringData[mFieldDefs[i].nameOffset]);
		}
	}
	return "?";
}

string Kzqcvm::NameForFieldOffset(int16_t ofs, QcvmDefinitionType type)
{
	for (int i=0; i<mHeader->fielddefs_num; ++i)
	{
		if (((mFieldDefs[i].type & GLOBALDEF_TYPE_MASK) == type) && (mFieldDefs[i].offset == ofs))
		{
			return string(&mStringData[mFieldDefs[i].nameOffset]);
		}
	}
	return "?";
}

//-----------------------------------------------------------------------------
// Dump the data structures
//-----------------------------------------------------------------------------

void Kzqcvm::Dump()
{
	// globaldefs
	cout << "Globals:" << endl;
	cout << "Number,Name,Type,Offset,DefaultValue" << endl;
	for (int i=0; i<mHeader->globaldefs_num; ++i)
	{
		cout << i << ",";
		cout << &mStringData[mGlobalDefs[i].nameOffset] << ",";
		cout << (mGlobalDefs[i].type & GLOBALDEF_TYPE_MASK) << ",";
		cout << mGlobalDefs[i].offset << ",";
		if (mGlobalDefs[i].type == FLOAT)
		{
			cout << mGlobalData[mGlobalDefs[i].offset] << endl;
		}
		else if (mGlobalDefs[i].type == VECTOR)
		{
			cout << "'" << mGlobalData[mGlobalDefs[i].offset  ]
			     << " " << mGlobalData[mGlobalDefs[i].offset+1]
			     << " " << mGlobalData[mGlobalDefs[i].offset+2]
			     << "'" << endl;
		}
		else
		{
			cout << (*(int*)&mGlobalData[mGlobalDefs[i].offset]) << endl;
		}
	}
	cout << endl;

	// fielddefs
	cout << "Fields:" << endl;
	cout << "Number,Name,Type,Offset" << endl;
	for (int i=0; i<mHeader->fielddefs_num; ++i)
	{
		cout << i << ",";
		cout << &mStringData[mFieldDefs[i].nameOffset] << ",";
		cout << (mFieldDefs[i].type) << ",";
		cout << mFieldDefs[i].offset << endl;
	}
	cout << endl;

	// functions
	cout << "Functions:" << endl;
	cout << "Number,Name,OffsetLocalsInGlobals,OffsetFirstStatement" << endl;
	for (int i=0; i<mHeader->functions_num; ++i)
	{
		cout << i << ",";
		cout << &mStringData[mFunctions[i].nameOffset] << ",";
		cout << mFunctions[i].offsetLocalsInGlobals << ",";
		cout << mFunctions[i].offsetFirstStatement << endl;
	}
	cout << endl;

	// instructions
	cout << "Instructions:" << endl;
	cout << "Number,Instruction,ParmA,ParmB,ParmC" << endl;
	for (int i=0; i<mHeader->statements_num; ++i)
	{
		cout << i << ",";
		cout << GetInstructionName(mStatements[i].instruction) << ",";
		cout << mStatements[i].parameter[0] << ",";
		cout << mStatements[i].parameter[1] << ",";
		cout << mStatements[i].parameter[2] << endl;
	}
	cout << endl;

	// global data
	cout << "Global data:" << endl;
	cout << "Offset,AsFloat,AsInt" << endl;
	for (int i=0; i<mHeader->globaldata_num; ++i)
	{
		cout << i << ",";
		cout << mGlobalData[i] << ",";
		cout << *(int*)&mGlobalData[i] << endl;
	}
	cout << endl;

	// string data
	cout << "String data:" << endl;
	for (int i=0; i<mHeader->stringdata_size; ++i)
	{
		char c = mStringData[i];
		if (c < 0x20)
			cout << "?";
		else
			cout << c;
		// wrap
		if ((i & 0x3f) == 0x3f)
			cout << endl;
	}
	cout << endl;
	cout << endl;
}

//-----------------------------------------------------------------------------
} // namespace
//-----------------------------------------------------------------------------
