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
kzqcvm/instructions.cpp
*/

#include "instructions.h"

#include <stdint.h>

//-----------------------------------------------------------------------------
namespace kzqcvm {
//-----------------------------------------------------------------------------

InstructionInfo instruction_info[] = {
	{ Instructions::DONE,       "DONE",       { IT_VECTOR,   IT_NONE,     IT_NONE     } },

	{ Instructions::MUL_F,      "MUL_F",      { IT_FLOAT,    IT_FLOAT,    IT_FLOAT    } },
	{ Instructions::MUL_V,      "MUL_V",      { IT_VECTOR,   IT_VECTOR,   IT_FLOAT    } },
	{ Instructions::MUL_FV,     "MUL_FV",     { IT_FLOAT,    IT_VECTOR,   IT_FLOAT    } },
	{ Instructions::MUL_VF,     "MUL_VF",     { IT_VECTOR,   IT_FLOAT,    IT_FLOAT    } },
	{ Instructions::DIV_F,      "DIF_F",      { IT_FLOAT,    IT_FLOAT,    IT_FLOAT    } },
	{ Instructions::ADD_F,      "ADD_F",      { IT_FLOAT,    IT_FLOAT,    IT_FLOAT    } },
	{ Instructions::ADD_V,      "ADD_V",      { IT_VECTOR,   IT_VECTOR,   IT_VECTOR   } },
	{ Instructions::SUB_F,      "SUB_F",      { IT_FLOAT,    IT_FLOAT,    IT_FLOAT    } },
	{ Instructions::SUB_V,      "SUB_V",      { IT_VECTOR,   IT_VECTOR,   IT_VECTOR   } },

	{ Instructions::EQ_F,       "EQ_F",       { IT_FLOAT,    IT_FLOAT,    IT_FLOAT    } },
	{ Instructions::EQ_V,       "EQ_V",       { IT_VECTOR,   IT_VECTOR,   IT_FLOAT    } },
	{ Instructions::EQ_S,       "EQ_S",       { IT_STRING,   IT_STRING,   IT_FLOAT    } },
	{ Instructions::EQ_E,       "EQ_E",       { IT_ENTITY,   IT_ENTITY,   IT_FLOAT    } },
	{ Instructions::EQ_FNC,     "EQ_FNC",     { IT_FUNCTION, IT_FUNCTION, IT_FLOAT    } },

	{ Instructions::NE_F,       "NE_F",       { IT_FLOAT,    IT_FLOAT,    IT_FLOAT    } },
	{ Instructions::NE_V,       "NE_V",       { IT_VECTOR,   IT_VECTOR,   IT_FLOAT    } },
	{ Instructions::NE_S,       "NE_S",       { IT_STRING,   IT_STRING,   IT_FLOAT    } },
	{ Instructions::NE_E,       "NE_E",       { IT_ENTITY,   IT_ENTITY,   IT_FLOAT    } },
	{ Instructions::NE_FNC,     "NE_FNC",     { IT_FUNCTION, IT_FUNCTION, IT_FLOAT    } },

	{ Instructions::LE,         "LE",         { IT_FLOAT,    IT_FLOAT,    IT_FLOAT    } },
	{ Instructions::GE,         "GE",         { IT_FLOAT,    IT_FLOAT,    IT_FLOAT    } },
	{ Instructions::LT,         "LT",         { IT_FLOAT,    IT_FLOAT,    IT_FLOAT    } },
	{ Instructions::GT,         "GT",         { IT_FLOAT,    IT_FLOAT,    IT_FLOAT    } },

	{ Instructions::LOAD_F,     "LOAD_F",     { IT_ENTITY,   IT_FIELD,    IT_FLOAT    } },
	{ Instructions::LOAD_V,     "LOAD_V",     { IT_ENTITY,   IT_FIELD,    IT_VECTOR   } },
	{ Instructions::LOAD_S,     "LOAD_S",     { IT_ENTITY,   IT_FIELD,    IT_STRING   } },
	{ Instructions::LOAD_ENT,   "LOAD_ENT",   { IT_ENTITY,   IT_FIELD,    IT_ENTITY   } },
	{ Instructions::LOAD_F,     "LOAD_FLD",   { IT_ENTITY,   IT_FIELD,    IT_FIELD    } },
	{ Instructions::LOAD_F,     "LOAD_FNC",   { IT_ENTITY,   IT_FIELD,    IT_FUNCTION } },

	{ Instructions::ADDRESS,    "ADDRESS",    { IT_ENTITY,   IT_FIELD,    IT_ADDRESS  } },

	{ Instructions::STORE_F,    "STORE_F",    { IT_FLOAT,    IT_FLOAT,    IT_NONE     } },
	{ Instructions::STORE_V,    "STORE_V",    { IT_VECTOR,   IT_VECTOR,   IT_NONE     } },
	{ Instructions::STORE_S,    "STORE_S",    { IT_STRING,   IT_STRING,   IT_NONE     } },
	{ Instructions::STORE_ENT,  "STORE_ENT",  { IT_ENTITY,   IT_ENTITY,   IT_NONE     } },
	{ Instructions::STORE_FLD,  "STORE_FLD",  { IT_FIELD,    IT_FIELD,    IT_NONE     } },
	{ Instructions::STORE_FNC,  "STORE_FNC",  { IT_FUNCTION, IT_FUNCTION, IT_NONE     } },

	{ Instructions::STOREP_F,   "STOREP_F",   { IT_FLOAT,    IT_ADDRESS,  IT_NONE     } },
	{ Instructions::STOREP_V,   "STOREP_V",   { IT_VECTOR,   IT_ADDRESS,  IT_NONE     } },
	{ Instructions::STOREP_S,   "STOREP_S",   { IT_STRING,   IT_ADDRESS,  IT_NONE     } },
	{ Instructions::STOREP_ENT, "STOREP_ENT", { IT_ENTITY,   IT_ADDRESS,  IT_NONE     } },
	{ Instructions::STOREP_FLD, "STOREP_FLD", { IT_FIELD,    IT_ADDRESS,  IT_NONE     } },
	{ Instructions::STOREP_FNC, "STOREP_FNC", { IT_FUNCTION, IT_ADDRESS,  IT_NONE     } },

	{ Instructions::RETURN,     "RETURN",     { IT_VECTOR,   IT_NONE,     IT_NONE     } },

	{ Instructions::NOT_F,      "NOT_F",      { IT_FLOAT,    IT_NONE,     IT_FLOAT    } },
	{ Instructions::NOT_V,      "NOT_V",      { IT_VECTOR,   IT_NONE,     IT_FLOAT    } },
	{ Instructions::NOT_S,      "NOT_S",      { IT_STRING,   IT_NONE,     IT_FLOAT    } },
	{ Instructions::NOT_ENT,    "NOT_ENT",    { IT_ENTITY,   IT_NONE,     IT_FLOAT    } },
	{ Instructions::NOT_FNC,    "NOT_FNC",    { IT_FUNCTION, IT_NONE,     IT_FLOAT    } },

	{ Instructions::IF,         "IF",         { IT_FLOAT,    IT_DIRECT,   IT_NONE     } },
	{ Instructions::IFNOT,      "IFNOT",      { IT_FLOAT,    IT_DIRECT,   IT_NONE     } },

	{ Instructions::CALL0,      "CALL0",      { IT_FUNCTION, IT_NONE,     IT_NONE     } },
	{ Instructions::CALL1,      "CALL1",      { IT_FUNCTION, IT_NONE,     IT_NONE     } },
	{ Instructions::CALL2,      "CALL2",      { IT_FUNCTION, IT_NONE,     IT_NONE     } },
	{ Instructions::CALL3,      "CALL3",      { IT_FUNCTION, IT_NONE,     IT_NONE     } },
	{ Instructions::CALL4,      "CALL4",      { IT_FUNCTION, IT_NONE,     IT_NONE     } },
	{ Instructions::CALL5,      "CALL5",      { IT_FUNCTION, IT_NONE,     IT_NONE     } },
	{ Instructions::CALL6,      "CALL6",      { IT_FUNCTION, IT_NONE,     IT_NONE     } },
	{ Instructions::CALL7,      "CALL7",      { IT_FUNCTION, IT_NONE,     IT_NONE     } },
	{ Instructions::CALL8,      "CALL8",      { IT_FUNCTION, IT_NONE,     IT_NONE     } },

	{ Instructions::STATE,      "STATE",      { IT_FLOAT,    IT_FUNCTION, IT_NONE     } },

	{ Instructions::GOTO,       "GOTO",       { IT_DIRECT,   IT_NONE,     IT_NONE     } },

	{ Instructions::AND,        "AND",        { IT_FLOAT,    IT_FLOAT,    IT_FLOAT    } },
	{ Instructions::OR,         "OR",         { IT_FLOAT,    IT_FLOAT,    IT_FLOAT    } },

	{ Instructions::BITAND,     "BITAND",     { IT_FLOAT,    IT_FLOAT,    IT_FLOAT    } },
	{ Instructions::BITOR,      "BITOR",      { IT_FLOAT,    IT_FLOAT,    IT_FLOAT    } },

	{ 0x000,                    "UNKNOWN",    { IT_NONE,     IT_NONE,     IT_NONE     } }
};

const int num_instruction_info = sizeof(instruction_info) / sizeof(InstructionInfo);

InstructionInfo *GetInstructionInfo(int16_t instruction)
{
	for (int i=0; i<num_instruction_info; ++i)
	{
		if (instruction_info[i].value == instruction)
		{
			return &instruction_info[i];
		}
	}
	return &instruction_info[num_instruction_info-1];
}

const char *GetInstructionName(int16_t instruction)
{
	return GetInstructionInfo(instruction)->name;
}

//-----------------------------------------------------------------------------
} // namespace
//-----------------------------------------------------------------------------
