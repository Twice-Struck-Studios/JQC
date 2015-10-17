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
kzqcvm/structs.h
*/

//-----------------------------------------------------------------------------
#ifndef KZQCVM_STRUCTS_H
#define KZQCVM_STRUCTS_H
//-----------------------------------------------------------------------------

#include <stdint.h>

//-----------------------------------------------------------------------------
namespace kzqcvm {
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Header struct
//-----------------------------------------------------------------------------

struct QcvmHeader {
	int32_t version; // should be 6
	int32_t crc;

	int32_t statements_offset;
	int32_t statements_num;

	int32_t globaldefs_offset;
	int32_t globaldefs_num;

	int32_t fielddefs_offset;
	int32_t fielddefs_num;

	int32_t functions_offset;
	int32_t functions_num;

	int32_t stringdata_offset;
	int32_t stringdata_size;

	int32_t globaldata_offset;
	int32_t globaldata_num;

	int32_t entity_size; // number of 32 bit words
} __attribute__((__packed__));

//-----------------------------------------------------------------------------
// Statement struct
//-----------------------------------------------------------------------------

struct QcvmStatement {
	int16_t instruction;
	int16_t parameter[3];
} __attribute__((__packed__));

//-----------------------------------------------------------------------------
// Definition struct
//-----------------------------------------------------------------------------

enum QcvmDefinitionType {
	NOTYPE   = 0,
	STRING   = 1,
	FLOAT    = 2,
	VECTOR   = 3,
	ENTITY   = 4,
	FIELD    = 5,
	FUNCTION = 6
};

struct QcvmDefinition {
	int16_t type;
	int16_t offset; // into globaldata for a globaldef, or from an entity for fielddefs
	int32_t nameOffset;
} __attribute__((__packed__));

//-----------------------------------------------------------------------------
// Function struct
//-----------------------------------------------------------------------------

struct QcvmFunction {
	int32_t offsetFirstStatement;
	int32_t offsetLocalsInGlobals;
	int32_t numLocals; // all parameters + locals
	int32_t profiling; // used at runtime
	int32_t nameOffset;
	int32_t fileNameOffset;
	int32_t numParameters;
	unsigned char parameterSizes[8]; // 1 or 3
} __attribute__((__packed__));

//-----------------------------------------------------------------------------
} // namespace
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
