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
kzqcvm/stringmanager.h
*/

//-----------------------------------------------------------------------------
#ifndef KZQCVM_STRINGMANAGER_H
#define KZQCVM_STRINGMANAGER_H
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <string>
#include <vector>

//-----------------------------------------------------------------------------
namespace kzqcvm {
	using std::string;
	using std::vector;
//-----------------------------------------------------------------------------

class StringManager {
public:
	StringManager();
	~StringManager();

	void Init(char *stringConstants, int32_t stringConstantsSize);

	// returns 0 if the stringnum is invalid, but blank if it's null
	char    *GetString(int32_t stringnum);

	int32_t  Zone(string str);
	bool     Unzone(int32_t stringnum);

	int32_t  TempString(string str);
	void     ClearTempStrings();

private:
	bool     mInit;

	char    *mConstants;
	int32_t  mConstantsSize;

	vector<char*> mTempStrings;
	vector<char*> mZoneStrings;
};

//-----------------------------------------------------------------------------
} // namespace
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
