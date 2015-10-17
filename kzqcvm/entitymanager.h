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
kzqcvm/entitymanager.h
*/

//-----------------------------------------------------------------------------
#ifndef KZQCVM_ENTITYMANAGER_H
#define KZQCVM_ENTITYMANAGER_H
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <vector>

//-----------------------------------------------------------------------------
namespace kzqcvm {
	using std::vector;
//-----------------------------------------------------------------------------

class EntityManager {
public:
	EntityManager();
	~EntityManager();

	void Init(int32_t entitySize, float entityReuseTime);

	// Returns an address usable by the Write* methods below.
	// Returns 0 if the entity or field were out of bounds or if entityNum
	// specifies an unused entity.
	int32_t  GetAddress(int32_t entityNum, int32_t fieldOffset);
	// Returns a pointer to the actual data.
	// Returns NULL if the entity or field were out of bounds or if entityNum
	// specifies an unused entity.
	float   *GetPointer(int32_t entityNum, int32_t fieldOffset);

	// Last parameter is written to the specified address. Address is simply
	// the same number as given by GetAddress.
	// Returns true if the entity was in use,
	// false if it was out of bounds or freed.
	bool WriteFloat (int32_t address, float f);
	bool WriteVector(int32_t address, const float *v);
	bool WriteInt   (int32_t address, int i);

	// Result is written to the last parameter.
	// Returns true if the entity was in use,
	// false if it was out of bounds or freed.
	bool ReadFloat (int32_t entityNum, int32_t fieldOffset, float *f);
	bool ReadVector(int32_t entityNum, int32_t fieldOffset, float *v);
	bool ReadInt   (int32_t entityNum, int32_t fieldOffset, int *i);

	int32_t CreateEntity(int64_t time);
	void    DeleteEntity(int32_t entityNum, int64_t time);

	int32_t GetFirstEntity();
	int32_t GetNextEntity(int32_t entityNum);

private:
	void CreateEntityPage();

	bool  mInit;

	int   mEntitySize;
	int   mPageSize;
	float mEntityReuseTime;

	// In this implementation, we divide entities up into pages.
	// Used entities have a value of FLT_MAX while unused entities use the
	// value to check if they are available yet.
	// On deletion we set to time + entityReuseTime
	// On creation we requre that it <= time
	vector<float*> mEntityPages;
};

//-----------------------------------------------------------------------------
} // namespace
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
