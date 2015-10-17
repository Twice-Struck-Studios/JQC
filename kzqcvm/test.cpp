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
kzqcvm/test.cpp
*/

#include "test.h"

#include <iostream>

#include "kzqcvm.h"
#include "data.h"

//-----------------------------------------------------------------------------
namespace kzqcvm {
	using std::cout;
	using std::endl;
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Testing - builtins
//-----------------------------------------------------------------------------

bool vm_ThrowError(Kzqcvm *qcvm, int builtinNum)
{
	cout << "Throwing error on purpose" << endl;
	return false;
}

bool vm_Spawn(Kzqcvm *qcvm, int builtinNum)
{
	EntityPointer ret = qcvm->GetReturnEntityPointer();
	ret.Set(qcvm->CreateEntity(2.0f));
	return true;
}

bool vm_Remove(Kzqcvm *qcvm, int builtinNum)
{
	EntityPointer parm1 = qcvm->GetParameterEntityPointer(0);
	qcvm->DeleteEntity(parm1.Get(), 0.0f);
	return true;
}

bool vm_Zone(Kzqcvm *qcvm, int builtinNum)
{
	StringPointer ret   = qcvm->GetReturnStringPointer();
	StringPointer parm1 = qcvm->GetParameterStringPointer(0);
	ret.Set(qcvm->Alloc(parm1.Get()));
	return true;
}

bool vm_Unzone(Kzqcvm *qcvm, int builtinNum)
{
	StringPointer parm1 = qcvm->GetParameterStringPointer(0);
	qcvm->Free(parm1.Get());
	return true;
}

bool vm_FirstEntity(Kzqcvm *qcvm, int builtinNum)
{
	EntityPointer ret = qcvm->GetReturnEntityPointer();
	ret.Set(qcvm->GetFirstEntity());
	return true;
}

bool vm_NextEntity(Kzqcvm *qcvm, int builtinNum)
{
	EntityPointer ret   = qcvm->GetReturnEntityPointer();
	EntityPointer parm1 = qcvm->GetParameterEntityPointer(0);
	ret.Set(qcvm->NextEntity(parm1.Get()));
	return true;
}

//-----------------------------------------------------------------------------
// Testing - run tests
//-----------------------------------------------------------------------------

bool Test()
{
	// load the test vm
	Kzqcvm testProgs("progs/test.dat");
	if (!testProgs.IsLoaded())
	{
		cout << "the vm failed to load" << endl;
		return false;
	}

	testProgs.Dump();

	// attach builtins
	testProgs.AddBuiltin(vm_ThrowError, 1);
	testProgs.AddBuiltin(vm_Spawn,      2);
	testProgs.AddBuiltin(vm_Remove,     3);
	testProgs.AddBuiltin(vm_Zone,       4);
	testProgs.AddBuiltin(vm_Unzone,     5);

	// get a non existent function
	Function noFunc = testProgs.GetFunction("Does_Not_Exist");
	if (noFunc)
	{
		cout << "found Function 'Does_Not_Exist'" << endl;
		return false;
	}

	// get the function 'main'
	Function mainFunc = testProgs.GetFunction("main");
	if (!mainFunc)
	{
		cout << "could not get Function 'main'" << endl;
		return false;
	}

	// run the function
	if(!mainFunc.Run())
	{
		cout << "main function threw an error" << endl;
		return false;
	}

	// check that the value ran
	FloatPointer ret = testProgs.GetReturnFloatPointer();
	if (ret.Get() != 1.0f)
	{
		cout << "main function returned false" << endl;
		return false;
	}

	// test main_error for throwing errors
	Function mainError = testProgs.GetFunction("main_error_throw");
	if (!mainError)
	{
		cout << "could not get Function 'main_error_throw'" << endl;
		return false;
	}
	if (mainError.Run())
	{
		cout << "main_error_throw did not throw an error" << endl;
		return false;
	}
	cout << testProgs.GetErrorMessages() << endl;
	testProgs.ClearErrors();

	// test main_error for throwing errors
	Function mainError2 = testProgs.GetFunction("main_error_invalidbuiltin");
	if (!mainError2)
	{
		cout << "could not get Function 'main_error_invalidbuiltin'" << endl;
		return false;
	}
	if (mainError2.Run())
	{
		cout << "main_error_invalidbuiltin did not throw an error" << endl;
		return false;
	}
	cout << testProgs.GetErrorMessages() << endl;
	testProgs.ClearErrors();

	return true;
}

//-----------------------------------------------------------------------------
// Testing - main
//-----------------------------------------------------------------------------

bool DoTests()
{
	if (Test())
	{
		cout << "Tests successful!" << endl;
		return true;
	}
	else
	{
		cout << "Tests failed" << endl;
		return false;
	}
}

//-----------------------------------------------------------------------------
} // namespace
//-----------------------------------------------------------------------------
