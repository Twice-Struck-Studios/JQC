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
kzqcvm/errors.h
*/

//-----------------------------------------------------------------------------
#ifndef KZQCVM_ERRORS_H
#define KZQCVM_ERRORS_H
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
namespace kzqcvm {
//-----------------------------------------------------------------------------

enum QcvmError {
	ERR_NONE,
	ERR_RUNAWAY_LOOP,
	ERR_FUNCTION_NOT_FOUND,
	ERR_BUILTIN_NOT_FOUND,
	ERR_BUILTIN_ERROR,
	ERR_INVALID_READ,
	ERR_INVALID_WRITE,
	ERR_INVALID_INSTRUCTION,
	ERR_NOT_IMPLEMENTED
};

//-----------------------------------------------------------------------------
} // namespace
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
