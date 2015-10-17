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
kzqcvm/stringmanager.cpp
*/

#include "stringmanager.h"

#include <assert.h>
#include <string.h>
#include <iostream>

//-----------------------------------------------------------------------------
namespace kzqcvm {
	using std::cout;
	using std::endl;
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Structors
//-----------------------------------------------------------------------------

StringManager::StringManager()
{
	mInit = false;
}

StringManager::~StringManager()
{
	for (int i=0; i<(int)mTempStrings.size(); ++i)
	{
		delete[] mTempStrings[i];
	}
	for (int i=0; i<(int)mZoneStrings.size(); ++i)
	{
		delete[] mZoneStrings[i];
	}
}

//-----------------------------------------------------------------------------
// Init - must init
//-----------------------------------------------------------------------------

void StringManager::Init(char *stringConstants, int32_t stringConstantsSize)
{
	mInit = true;

	mConstants     = stringConstants;
	mConstantsSize = stringConstantsSize;

	mTempStrings.reserve(512);
	mZoneStrings.reserve(512);
}

//-----------------------------------------------------------------------------
// Get String
//-----------------------------------------------------------------------------

char *StringManager::GetString(int32_t stringnum)
{
	assert(mInit);
	if (stringnum >= 0)
	{
		if (stringnum < mConstantsSize)
		{
			// constants
			return &mConstants[stringnum];
		}
		// zone
		stringnum -= mConstantsSize;
		if (stringnum < (int)mZoneStrings.size())
			return mZoneStrings[stringnum];
	}
	else
	{
		// temp
		stringnum = (stringnum * -1) - 1;
		if (stringnum < (int)mTempStrings.size())
			return mTempStrings[stringnum];
	}
	return 0;
}

//-----------------------------------------------------------------------------
// Zone
//-----------------------------------------------------------------------------

int32_t StringManager::Zone(string str)
{
	char *newstr = new char[str.length()+1];
	strcpy(newstr, str.c_str());

	for (int i=0; i<(int)mZoneStrings.size(); ++i)
	{
		if (mZoneStrings[i] == 0)
		{
			mZoneStrings[i] = newstr;
			return i + mConstantsSize;
		}
	}

	int32_t index = mZoneStrings.size();
	mZoneStrings.push_back(newstr);
	return index + mConstantsSize;
}

bool StringManager::Unzone(int32_t stringnum)
{
	if (stringnum < mConstantsSize)
		return false;

	stringnum -= mConstantsSize;
	if (stringnum >= (int)mZoneStrings.size())
		return false;

	if (mZoneStrings[stringnum] == 0)
		return false;

	delete[] mZoneStrings[stringnum];
	mZoneStrings[stringnum] = 0;

	return true;
}

//-----------------------------------------------------------------------------
// TempStrings
//-----------------------------------------------------------------------------

int32_t StringManager::TempString(string str)
{
	char *newstr = new char[str.length()+1];
	strcpy(newstr, str.c_str());

	int32_t index = mTempStrings.size();
	mTempStrings.push_back(newstr);
	return -(index+1);
}

void StringManager::ClearTempStrings()
{
	for (int i=0; i<(int)mTempStrings.size(); ++i)
	{
		delete[] mTempStrings[i];
	}
	mTempStrings.clear();
}

//-----------------------------------------------------------------------------
} // namespace
//-----------------------------------------------------------------------------
