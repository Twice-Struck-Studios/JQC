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
kzqcvm/run.cpp
*/

#include "kzqcvm.h"
#include "instructions.h"

#include <string.h>

#include <iostream>

//-----------------------------------------------------------------------------
namespace kzqcvm {
	using std::cout;
	using std::endl;
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Run Function
//-----------------------------------------------------------------------------

//#define FUNCTION_DEBUG

#define PARM_A (currentStatement->parameter[0])
#define PARM_B (currentStatement->parameter[1])
#define PARM_C (currentStatement->parameter[2])
#define COPY_VEC(a,b) (b)[0] = (a)[0]; (b)[1] = (a)[1]; (b)[2] = (a)[2];

#define GET_STRING(a) (mStringManager.GetString(a))

// return false if execution was halted, else true
bool Kzqcvm::RunFunction(int functionNum, int *instructionCount)
{
	// bounds check the index
	if (functionNum <= 0 || functionNum >= mHeader->functions_num)
	{
		StartError(ERR_FUNCTION_NOT_FOUND, "Invalid function index");
		mErrorLog << "Invalid function index " << functionNum << endl;
		return false;
	}
	QcvmFunction *function = &mFunctions[functionNum];
#ifdef FUNCTION_DEBUG
	cout << "Entering function " << &mStringData[function->nameOffset] << endl;
#endif

	if (function->offsetFirstStatement < 0)
	{
		bool result = RunBuiltin(-(function->offsetFirstStatement));
		if (!result)
		{
			mErrorLog << "in builtin #" << -function->offsetFirstStatement
				<< ": " << &mStringData[function->nameOffset] << endl;
		}
		return result;
	}

	// backup the existing local values in global data
	float stackData[function->numLocals];
	for (int i=0; i<function->numLocals; ++i)
	{
		stackData[i] = mGlobalData[function->offsetLocalsInGlobals+i];
	}

	// copy the parameters over the local values in global data
	for (int i=0, ofs=0; i<function->numParameters; ++i)
	{
		for (int j=0; j<function->parameterSizes[i]; ++j, ++ofs)
		{
			mGlobalData[function->offsetLocalsInGlobals+ofs] = mGlobalData[OFS_PARM0 + (i*3) + j];
		}
	}

	// some useful stuff
	int32_t *intGlobalData = (int32_t*)mGlobalData;

	// iterate instructions till we get a return (or a crash)
	QcvmStatement *currentStatement = &mStatements[function->offsetFirstStatement];

	int stopcode;
	const int STOP_SUCCESS               =  1;
	const int STOP_ERROR_HANDLED_ALREADY = -1;
	const int STOP_ERROR_ENTITY_READ     = -2;
	const int STOP_ERROR_ENTITY_WRITE    = -3;
	const int STOP_ERROR_RUNAWAY_LOOP    = -4;
	for (stopcode=0; stopcode==0; ++(*instructionCount), ++currentStatement)
	{
		// TODO: customize runaway loop length
		// stop after 2097151 instructions
		// lowest bits are masked out and if any higher are set we exit
		if (*instructionCount & 0xffe00000)
		{
			stopcode = STOP_ERROR_RUNAWAY_LOOP;
			goto end_of_instructions;
		}

		switch (currentStatement->instruction)
		{
		//---------------------------------------------------------------------
		// return
		case Instructions::DONE:
		case Instructions::RETURN:
			COPY_VEC(&mGlobalData[PARM_A], &mGlobalData[OFS_RETURN])
			stopcode = STOP_SUCCESS;
			break;
		//---------------------------------------------------------------------
		// arithmetic
		case Instructions::MUL_F:
			mGlobalData[PARM_C] = mGlobalData[PARM_A] * mGlobalData[PARM_B];
			break;
		case Instructions::MUL_V:
			mGlobalData[PARM_C] =
				mGlobalData[PARM_A  ] * mGlobalData[PARM_B  ] +
				mGlobalData[PARM_A+1] * mGlobalData[PARM_B+1] +
				mGlobalData[PARM_A+2] * mGlobalData[PARM_B+2];
			break;
		case Instructions::MUL_FV:
			mGlobalData[PARM_C  ] = mGlobalData[PARM_A] * mGlobalData[PARM_B  ];
			mGlobalData[PARM_C+1] = mGlobalData[PARM_A] * mGlobalData[PARM_B+1];
			mGlobalData[PARM_C+2] = mGlobalData[PARM_A] * mGlobalData[PARM_B+2];
			break;
		case Instructions::MUL_VF:
			mGlobalData[PARM_C  ] = mGlobalData[PARM_A  ] * mGlobalData[PARM_B];
			mGlobalData[PARM_C+1] = mGlobalData[PARM_A+1] * mGlobalData[PARM_B];
			mGlobalData[PARM_C+2] = mGlobalData[PARM_A+2] * mGlobalData[PARM_B];
			break;
		case Instructions::DIV_F:
			mGlobalData[PARM_C] = mGlobalData[PARM_A] / mGlobalData[PARM_B];
			break;
		case Instructions::ADD_F:
			mGlobalData[PARM_C] = mGlobalData[PARM_A] + mGlobalData[PARM_B];
			break;
		case Instructions::ADD_V:
			mGlobalData[PARM_C  ] = mGlobalData[PARM_A  ] + mGlobalData[PARM_B  ];
			mGlobalData[PARM_C+1] = mGlobalData[PARM_A+1] + mGlobalData[PARM_B+1];
			mGlobalData[PARM_C+2] = mGlobalData[PARM_A+2] + mGlobalData[PARM_B+2];
			break;
		case Instructions::SUB_F:
			mGlobalData[PARM_C] = mGlobalData[PARM_A] - mGlobalData[PARM_B];
			break;
		case Instructions::SUB_V:
			mGlobalData[PARM_C  ] = mGlobalData[PARM_A  ] - mGlobalData[PARM_B  ];
			mGlobalData[PARM_C+1] = mGlobalData[PARM_A+1] - mGlobalData[PARM_B+1];
			mGlobalData[PARM_C+2] = mGlobalData[PARM_A+2] - mGlobalData[PARM_B+2];
			break;
		//---------------------------------------------------------------------
		// logical equality
		case Instructions::EQ_F:
			mGlobalData[PARM_C] = (mGlobalData[PARM_A] == mGlobalData[PARM_B]);
			break;
		case Instructions::EQ_V:
			mGlobalData[PARM_C] =
				(mGlobalData[PARM_A  ] == mGlobalData[PARM_B  ] &&
				 mGlobalData[PARM_A+1] == mGlobalData[PARM_B+1] &&
				 mGlobalData[PARM_A+2] == mGlobalData[PARM_B+2]);
			break;
		case Instructions::EQ_S:
			mGlobalData[PARM_C] = (strcmp(GET_STRING(intGlobalData[PARM_A]), GET_STRING(intGlobalData[PARM_B])) == 0);
			break;
		case Instructions::EQ_E:
		case Instructions::EQ_FNC:
			mGlobalData[PARM_C] = (intGlobalData[PARM_A] == intGlobalData[PARM_B]);
			break;
		//---------------------------------------------------------------------
		// logical inequality
		case Instructions::NE_F:
			mGlobalData[PARM_C] = (mGlobalData[PARM_A] != mGlobalData[PARM_B]);
			break;
		case Instructions::NE_V:
			mGlobalData[PARM_C] =
				(mGlobalData[PARM_A  ] != mGlobalData[PARM_B  ] ||
				 mGlobalData[PARM_A+1] != mGlobalData[PARM_B+1] ||
				 mGlobalData[PARM_A+2] != mGlobalData[PARM_B+2]);
			break;
		case Instructions::NE_S:
			mGlobalData[PARM_C] = (strcmp(GET_STRING(intGlobalData[PARM_A]), GET_STRING(intGlobalData[PARM_B])) != 0);
			break;
		case Instructions::NE_E:
		case Instructions::NE_FNC:
			mGlobalData[PARM_C] = (intGlobalData[PARM_A] != intGlobalData[PARM_B]);
			break;
		//---------------------------------------------------------------------
		// comparison
		case Instructions::LE:
			mGlobalData[PARM_C] = (mGlobalData[PARM_A] <= mGlobalData[PARM_B]);
			break;
		case Instructions::GE:
			mGlobalData[PARM_C] = (mGlobalData[PARM_A] >= mGlobalData[PARM_B]);
			break;
		case Instructions::LT:
			mGlobalData[PARM_C] = (mGlobalData[PARM_A] < mGlobalData[PARM_B]);
			break;
		case Instructions::GT:
			mGlobalData[PARM_C] = (mGlobalData[PARM_A] > mGlobalData[PARM_B]);
			break;
		//---------------------------------------------------------------------
		// load from entity
		case Instructions::LOAD_F:
			if (!mEntityManager.ReadFloat(intGlobalData[PARM_A], intGlobalData[PARM_B], &mGlobalData[PARM_C]))
				stopcode = STOP_ERROR_ENTITY_READ;
			break;
		case Instructions::LOAD_V:
			if (!mEntityManager.ReadVector(intGlobalData[PARM_A], intGlobalData[PARM_B], &mGlobalData[PARM_C]))
				stopcode = STOP_ERROR_ENTITY_READ;
			break;
		case Instructions::LOAD_S:
		case Instructions::LOAD_ENT:
		case Instructions::LOAD_FLD:
		case Instructions::LOAD_FNC:
			if (!mEntityManager.ReadInt(intGlobalData[PARM_A], intGlobalData[PARM_B], &intGlobalData[PARM_C]))
				stopcode = STOP_ERROR_ENTITY_READ;
			break;
		//---------------------------------------------------------------------
		// address entity
		case Instructions::ADDRESS:
			intGlobalData[PARM_C] = mEntityManager.GetAddress(intGlobalData[PARM_A], intGlobalData[PARM_B]);
			break;
		//---------------------------------------------------------------------
		// store (copy)
		case Instructions::STORE_F:
		case Instructions::STORE_S:
		case Instructions::STORE_ENT:
		case Instructions::STORE_FLD:
		case Instructions::STORE_FNC:
			mGlobalData[PARM_B] = mGlobalData[PARM_A];
			break;
		case Instructions::STORE_V:
			COPY_VEC(&mGlobalData[PARM_A], &mGlobalData[PARM_B]);
			break;
		//---------------------------------------------------------------------
		// store (addressed)
		case Instructions::STOREP_F:
			if (!mEntityManager.WriteFloat(intGlobalData[PARM_B], mGlobalData[PARM_A]))
				stopcode = STOP_ERROR_ENTITY_WRITE;
			break;
		case Instructions::STOREP_V:
			if (!mEntityManager.WriteVector(intGlobalData[PARM_B], &mGlobalData[PARM_A]))
				stopcode = STOP_ERROR_ENTITY_WRITE;
			break;
		case Instructions::STOREP_S:
		case Instructions::STOREP_ENT:
		case Instructions::STOREP_FLD:
		case Instructions::STOREP_FNC:
			if (!mEntityManager.WriteInt(intGlobalData[PARM_B], intGlobalData[PARM_A]))
				stopcode = STOP_ERROR_ENTITY_WRITE;
			break;
		//---------------------------------------------------------------------
		// logical not
		case Instructions::NOT_F:
			mGlobalData[PARM_C] = !mGlobalData[PARM_A];
			break;
		case Instructions::NOT_V:
			mGlobalData[PARM_C] = !mGlobalData[PARM_A  ] && !mGlobalData[PARM_A+1] && !mGlobalData[PARM_A+2];
			break;
		case Instructions::NOT_S:
		case Instructions::NOT_ENT:
		case Instructions::NOT_FNC:
			mGlobalData[PARM_C] = !intGlobalData[PARM_A];
			break;
		//---------------------------------------------------------------------
		// if, ifnot (jump)
		case Instructions::IF:
			if (mGlobalData[PARM_A]) currentStatement += PARM_B;
			continue;
		case Instructions::IFNOT:
			if (!mGlobalData[PARM_A]) currentStatement += PARM_B;
			continue;
		//---------------------------------------------------------------------
		// function calls
		case Instructions::CALL0:
		case Instructions::CALL1:
		case Instructions::CALL2:
		case Instructions::CALL3:
		case Instructions::CALL4:
		case Instructions::CALL5:
		case Instructions::CALL6:
		case Instructions::CALL7:
		case Instructions::CALL8:
			mNumCallParameters = (int)currentStatement->instruction - (int)Instructions::CALL0;
			if (!RunFunction(intGlobalData[PARM_A], instructionCount))
				stopcode = STOP_ERROR_HANDLED_ALREADY;
			break;
		//---------------------------------------------------------------------
		// state
		case Instructions::STATE:
			StartError(ERR_NOT_IMPLEMENTED, "Progs Instruction STATE not implemented");
			stopcode = STOP_ERROR_HANDLED_ALREADY;
			break;
		//---------------------------------------------------------------------
		// goto (jump)
		case Instructions::GOTO:
			currentStatement += PARM_A;
			continue;
		//---------------------------------------------------------------------
		// logical and/or
		case Instructions::AND:
			mGlobalData[PARM_C] = mGlobalData[PARM_A] && mGlobalData[PARM_B];
			break;
		case Instructions::OR:
			mGlobalData[PARM_C] = mGlobalData[PARM_A] || mGlobalData[PARM_B];
			break;
		//---------------------------------------------------------------------
		// bitwise and/or
		case Instructions::BITAND:
			mGlobalData[PARM_C] = (float)((int)mGlobalData[PARM_A] & (int)mGlobalData[PARM_B]);
			break;
		case Instructions::BITOR:
			mGlobalData[PARM_C] = (float)((int)mGlobalData[PARM_A] | (int)mGlobalData[PARM_B]);
			break;
		//---------------------------------------------------------------------
		default:
			StartError(ERR_INVALID_INSTRUCTION, "Invalid instruction");
			stopcode = STOP_ERROR_HANDLED_ALREADY;
			break;
		}
		end_of_instructions:;
	}

	switch (stopcode)
	{
	case STOP_ERROR_ENTITY_READ:
		StartError(ERR_INVALID_READ, "Attempted read from invalid entity");
		break;
	case STOP_ERROR_ENTITY_WRITE:
		StartError(ERR_INVALID_WRITE, "Attempted write to invalid entity");
		break;
	case STOP_ERROR_RUNAWAY_LOOP:
		StartError(ERR_RUNAWAY_LOOP, "Maximum instruction limit reached");
		break;
	default:
		break;
	}

	if (stopcode < 0)
	{
		TraceFunction(function, --currentStatement);
	}

	// Then copy the stuff from our stack back into globals
	for (int i=0; i<function->numLocals; ++i)
	{
		mGlobalData[function->offsetLocalsInGlobals+i] = stackData[i];
	}

#ifdef FUNCTION_DEBUG
	cout << "Leaving function " << &mStringData[function->nameOffset] << endl;
#endif

	if (stopcode < 0)
		return false;
	return true;
}

//-----------------------------------------------------------------------------
} // namespace
//-----------------------------------------------------------------------------
