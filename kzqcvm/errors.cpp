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
kzqcvm/errors.cpp
*/

#include "errors.h"

#include <cstdio>
#include <iostream>
#include <string>

#include "kzqcvm.h"
#include "instructions.h"

//-----------------------------------------------------------------------------
namespace kzqcvm {
	using std::endl;
	using std::string;
//-----------------------------------------------------------------------------

void Kzqcvm::StartError(QcvmError errorType, string errorName)
{
	mError = errorType;
	mErrorLog << "================" << endl;
	mErrorLog << errorName << endl;
	mErrorLog << "================" << endl;
}

void Kzqcvm::BuiltinError(string message)
{
	StartError(ERR_BUILTIN_ERROR, message);
}

void Kzqcvm::AddErrorLine(string message)
{
	mErrorLog << message << endl;
}

void Kzqcvm::TraceFunction(QcvmFunction *func, QcvmStatement *programCounter)
{
	mErrorLog << "  in " << &mStringData[func->nameOffset] << endl;

	QcvmStatement *statement = programCounter;
	int statementNum = statement - mStatements;
	for (int i=0; i<6 && statement >= mStatements; ++i, --statement, --statementNum)
	{
		if (statement->instruction == Instructions::DONE)
			break;

		InstructionInfo *info = GetInstructionInfo(statement->instruction);

		mErrorLog << statementNum << ": " << info->name << " ";
		for (int j=0; j<3; ++j)
		{
			mErrorLog << statement->parameter[j];
			switch (info->parm[j])
			{
			case IT_NONE:
				break;
			case IT_DIRECT:
				mErrorLog << "(*direct*)";
				break;
			case IT_FLOAT:
				mErrorLog << "(" << NameForGlobalOffset(statement->parameter[j], FLOAT) << ")";
				mErrorLog << "=" << mGlobalData[statement->parameter[j]];
				break;
			case IT_VECTOR:
				mErrorLog << "(" << NameForGlobalOffset(statement->parameter[j], VECTOR) << ")";
				mErrorLog << "='" << mGlobalData[statement->parameter[j]];
				mErrorLog << " " << mGlobalData[statement->parameter[j]+1];
				mErrorLog << " " << mGlobalData[statement->parameter[j]+2] << "'";
				break;
			default:
				mErrorLog << "(" << NameForGlobalOffset(statement->parameter[j]) << ")";
				mErrorLog << "=" << *(int*)&mGlobalData[statement->parameter[j]];
				break;
			}
			mErrorLog << " ";
		}
		mErrorLog << endl;
	}
}

QcvmError Kzqcvm::GetLastError()
{
	return mError;
}

string Kzqcvm::GetErrorMessages()
{
	return mErrorLog.str();
}

void Kzqcvm::ClearErrors()
{
	mError = ERR_NONE;
	mErrorLog.str(string());
}

//-----------------------------------------------------------------------------
} // namespace
//-----------------------------------------------------------------------------
