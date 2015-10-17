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
kzqcvm/functions.cpp
*/

#include "kzqcvm.h"
#include "data.h"

#include <string>
#include <map>
#include <iostream>

//-----------------------------------------------------------------------------
namespace kzqcvm {
	using std::string;
//-----------------------------------------------------------------------------

Function Kzqcvm::GetFunction(string name)
{
	for (int i=0; i<mHeader->functions_num; ++i)
	{
		if (name.compare(&mStringData[mFunctions[i].nameOffset]) == 0)
		{
			return Function(this, i);
		}
	}
	return Function(this, 0);
}

bool Kzqcvm::RunFunction(Function &func)
{
	int count = 0;
	return RunFunction(func.number, &count);
}

//-----------------------------------------------------------------------------
} // namespace
//-----------------------------------------------------------------------------
