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
kzqcvm/builtins.cpp
*/

#include "kzqcvm.h"

#include <string.h>
#include <map>
#include <iostream>

//-----------------------------------------------------------------------------
namespace kzqcvm {
	using std::map;
	using std::pair;
	using std::cout;
	using std::endl;
//-----------------------------------------------------------------------------

void Kzqcvm::AddBuiltin(BuiltinCallback builtin, int number)
{
	RemoveBuiltin(number);
	mBuiltins.insert(pair<int,BuiltinCallback>(-number, builtin));
}

void Kzqcvm::RemoveBuiltin(int number)
{
	map<int,BuiltinCallback>::iterator it = mBuiltins.find(-number);
	if (it != mBuiltins.end())
	{
		mBuiltins.erase(it);
	}
}

int Kzqcvm::FindBuiltinNumber(string name)
{
	for (int i=0; i<mHeader->functions_num; ++i)
	{
		if (mFunctions[i].offsetFirstStatement < 0)
		{
			if (name.compare(&mStringData[mFunctions[i].nameOffset]) == 0)
			{
				return -mFunctions[i].offsetFirstStatement;
			}
		}
	}
	return 0;
}

bool Kzqcvm::RunBuiltin(int builtinNum)
{
	map<int,BuiltinCallback>::iterator it = mBuiltins.find(-builtinNum);
	if (it == mBuiltins.end())
	{
		it = mBuiltins.find(0);
		if (it == mBuiltins.end())
		{
			StartError(ERR_BUILTIN_NOT_FOUND, "Builtin not found");
			return false;
		}
	}
	return (*it).second(this, -builtinNum);
}

//-----------------------------------------------------------------------------
} // namespace
//-----------------------------------------------------------------------------
