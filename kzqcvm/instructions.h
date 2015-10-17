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
kzqcvm/instructions.h
*/

//-----------------------------------------------------------------------------
#ifndef KZQCVM_INSTRUCTIONS_H
#define KZQCVM_INSTRUCTIONS_H
//-----------------------------------------------------------------------------

#include <stdint.h>

//-----------------------------------------------------------------------------
namespace kzqcvm {
//-----------------------------------------------------------------------------

struct Instructions {
	static const int16_t DONE       = 0x0000;

	static const int16_t MUL_F      = 0x0001;
	static const int16_t MUL_V      = 0x0002;
	static const int16_t MUL_FV     = 0x0003;
	static const int16_t MUL_VF     = 0x0004;
	static const int16_t DIV_F      = 0x0005;
	static const int16_t ADD_F      = 0x0006;
	static const int16_t ADD_V      = 0x0007;
	static const int16_t SUB_F      = 0x0008;
	static const int16_t SUB_V      = 0x0009;

	static const int16_t EQ_F       = 0x000A;
	static const int16_t EQ_V       = 0x000B;
	static const int16_t EQ_S       = 0x000C;
	static const int16_t EQ_E       = 0x000D;
	static const int16_t EQ_FNC     = 0x000E;

	static const int16_t NE_F       = 0x000F;
	static const int16_t NE_V       = 0x0010;
	static const int16_t NE_S       = 0x0011;
	static const int16_t NE_E       = 0x0012;
	static const int16_t NE_FNC     = 0x0013;

	static const int16_t LE         = 0x0014;
	static const int16_t GE         = 0x0015;
	static const int16_t LT         = 0x0016;
	static const int16_t GT         = 0x0017;

	static const int16_t LOAD_F     = 0x0018;
	static const int16_t LOAD_V     = 0x0019;
	static const int16_t LOAD_S     = 0x001A;
	static const int16_t LOAD_ENT   = 0x001B;
	static const int16_t LOAD_FLD   = 0x001C;
	static const int16_t LOAD_FNC   = 0x001D;

	static const int16_t ADDRESS    = 0x001E;

	static const int16_t STORE_F    = 0x001F;
	static const int16_t STORE_V    = 0x0020;
	static const int16_t STORE_S    = 0x0021;
	static const int16_t STORE_ENT  = 0x0022;
	static const int16_t STORE_FLD  = 0x0023;
	static const int16_t STORE_FNC  = 0x0024;

	static const int16_t STOREP_F   = 0x0025;
	static const int16_t STOREP_V   = 0x0026;
	static const int16_t STOREP_S   = 0x0027;
	static const int16_t STOREP_ENT = 0x0028;
	static const int16_t STOREP_FLD = 0x0029;
	static const int16_t STOREP_FNC = 0x002A;

	static const int16_t RETURN     = 0x002B;

	static const int16_t NOT_F      = 0x002C;
	static const int16_t NOT_V      = 0x002D;
	static const int16_t NOT_S      = 0x002E;
	static const int16_t NOT_ENT    = 0x002F;
	static const int16_t NOT_FNC    = 0x0030;

	static const int16_t IF         = 0x0031;
	static const int16_t IFNOT      = 0x0032;

	static const int16_t CALL0      = 0x0033;
	static const int16_t CALL1      = 0x0034;
	static const int16_t CALL2      = 0x0035;
	static const int16_t CALL3      = 0x0036;
	static const int16_t CALL4      = 0x0037;
	static const int16_t CALL5      = 0x0038;
	static const int16_t CALL6      = 0x0039;
	static const int16_t CALL7      = 0x003A;
	static const int16_t CALL8      = 0x003B;

	static const int16_t STATE      = 0x003C;

	static const int16_t GOTO       = 0x003D;

	static const int16_t AND        = 0x003E;
	static const int16_t OR         = 0x003F;

	static const int16_t BITAND     = 0x0040;
	static const int16_t BITOR      = 0x0041;

	// these aren't instructions, but shorthand for the range
	static const int16_t MIN        = 0x0000;
	static const int16_t MAX        = 0x0041;
};

enum InstructionParameterType {
	IT_NONE,
	IT_DIRECT,
	IT_FLOAT,
	IT_VECTOR,
	IT_STRING,
	IT_ENTITY,
	IT_FIELD,
	IT_FUNCTION,
	IT_ADDRESS
};

struct InstructionInfo {
	int16_t     value;
	const char *name;
	InstructionParameterType parm[3];
};

extern InstructionInfo instruction_info[];

InstructionInfo *GetInstructionInfo(int16_t instruction);
const char *GetInstructionName(int16_t instruction);

//-----------------------------------------------------------------------------
} // namespace
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
