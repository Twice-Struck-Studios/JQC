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
kzqcvm/manager.cpp
*/

#include "entitymanager.h"

#include <cstring>
#include <stdint.h>
#include <limits.h>
#include <assert.h>

//-----------------------------------------------------------------------------
namespace kzqcvm {
//-----------------------------------------------------------------------------

const int32_t ENTITIES_PER_PAGE = 0xff;
const int32_t PAGENUMBER_MASK   = 0xffffff00;
const int32_t PAGENUMBER_SHIFT  = 8;
const int32_t ONPAGE_MASK       = 0x000000ff;

// time is represented as a 64 bit int spread over two floats
const int HEADER_SIZE = 2;
#define ENTITY_TIME(ent) (*(int64_t*)(ent))
const int64_t ENTITY_INUSE_VALUE = LLONG_MAX;

//-----------------------------------------------------------------------------
// Structors
//-----------------------------------------------------------------------------

EntityManager::EntityManager()
{
	mInit = false;
}

EntityManager::~EntityManager()
{
	for (int i=0; i<(int)mEntityPages.size(); ++i)
	{
		delete[] mEntityPages[i];
	}
}

//-----------------------------------------------------------------------------
// Init - must init
//-----------------------------------------------------------------------------

void EntityManager::Init(int32_t entitySize, float entityReuseTime)
{
	mInit = true;

	mEntitySize      = entitySize + HEADER_SIZE;
	mPageSize        = ENTITIES_PER_PAGE * mEntitySize;
	mEntityReuseTime = entityReuseTime;
	CreateEntityPage();
}

//-----------------------------------------------------------------------------
// Create/Delete
//-----------------------------------------------------------------------------

void EntityManager::CreateEntityPage()
{
	assert(mEntityPages.size() < 0xffffff);
	float *data = new float[mPageSize];
	memset(data, 0, mPageSize * sizeof(float));
	mEntityPages.push_back(data);
}

int32_t EntityManager::CreateEntity(int64_t time)
{
	assert(mInit);
	for (int i=0; i<(int)mEntityPages.size(); ++i)
	{
		for (int j=0; j<mPageSize; j += mEntitySize)
		{
			if (ENTITY_TIME(&mEntityPages[i][j]) <= time)
			{
				ENTITY_TIME(&mEntityPages[i][j]) = ENTITY_INUSE_VALUE;
				for (int k=HEADER_SIZE; k<mEntitySize; ++k)
				{
					mEntityPages[i][j+k] = 0.0f;
				}
				return (i << PAGENUMBER_SHIFT) + (j / mEntitySize);
			}
		}
	}
	CreateEntityPage();
	ENTITY_TIME(&mEntityPages[mEntityPages.size()-1][0]) = ENTITY_INUSE_VALUE;
	return (mEntityPages.size() - 1) << PAGENUMBER_SHIFT;
}

void EntityManager::DeleteEntity(int32_t entityNum, int64_t time)
{
	int pageNumber = (entityNum & PAGENUMBER_MASK) >> PAGENUMBER_SHIFT;
	int index      = entityNum & ONPAGE_MASK;
	assert(pageNumber >= 0 && pageNumber < (int)mEntityPages.size());
	memset(&mEntityPages[pageNumber][index*mEntitySize], 0, mEntitySize*sizeof(float));
	ENTITY_TIME(&mEntityPages[pageNumber][index*mEntitySize]) = time + mEntityReuseTime;
}

//-----------------------------------------------------------------------------
// Address
//-----------------------------------------------------------------------------

// Notes...
//
// entityNum     = the sequential number of an entity
// address       = an address of any offset of an entity (except the base index)
// fieldOffset   = the offset of a field relative to the start of an entity
// entityAddress = the base address of an entity

// pageNumber        = (entityNum & PAGENUMBER_MASK) >> PAGENUMBER_SHIFT
// entityNumOnPage   = entityNum & ONPAGE_MASK
// entityIndexOnPage = entityNumOnPage*mEntitySize;
// fieldIndexOnPage  = entityNumOnPage*mEntitySize) + fieldOffset
#define PAGE_NUMBER(en) (((en) & PAGENUMBER_MASK) >> PAGENUMBER_SHIFT)
#define ENT_NUM_ON_PAGE(en) ((en) & ONPAGE_MASK)
#define ENT_INDEX_ON_PAGE(en) (((en) & ONPAGE_MASK) * mEntitySize)
#define FIELD_INDEX_ON_PAGE(en,fl) ((((en) & ONPAGE_MASK) * mEntitySize) + fl)

// entityNum        =  address / mEntitySize
// fieldOffset      =  address % mEntitySize
// entityAddress    =  address - fieldOffset

// entityAddress    =  entityNumber  * mEntitySize
// address          =  entityAddress + fieldOffset

// number of entities to iterate = mEntityPages.size() * ENTITIES_PER_PAGE

int32_t EntityManager::GetAddress(int32_t entityNum, int32_t fieldOffset)
{
	fieldOffset += HEADER_SIZE;
	// bounds check page
	int32_t pageNumber = PAGE_NUMBER(entityNum);
	if (pageNumber < 0 || pageNumber >= (int32_t)mEntityPages.size())
		return 0;
	// free check entity
	int32_t entityIndex = ENT_INDEX_ON_PAGE(entityNum);
	if (ENTITY_TIME(&mEntityPages[pageNumber][entityIndex]) != ENTITY_INUSE_VALUE)
		return 0;
	return (entityNum * mEntitySize) + fieldOffset;
}

float *EntityManager::GetPointer(int32_t entityNum, int32_t fieldOffset)
{
	fieldOffset += HEADER_SIZE;
	if (fieldOffset < HEADER_SIZE || fieldOffset >= mEntitySize)
		return 0;
	// bounds check page
	int32_t pageNumber  = PAGE_NUMBER(entityNum);
	if (pageNumber < 0 || pageNumber >= (int32_t)mEntityPages.size())
		return 0;
	// free check entity
	int32_t entityIndex = ENT_INDEX_ON_PAGE(entityNum);
	if (ENTITY_TIME(&mEntityPages[pageNumber][entityIndex]) != ENTITY_INUSE_VALUE)
		return 0;
	return &mEntityPages[pageNumber][entityIndex + fieldOffset];
}

//-----------------------------------------------------------------------------
// Read
//-----------------------------------------------------------------------------

bool EntityManager::ReadFloat(int32_t entityNum, int32_t fieldOffset, float *f)
{
	fieldOffset += HEADER_SIZE;
	if (fieldOffset < HEADER_SIZE || fieldOffset >= mEntitySize)
		return false;
	// bounds check page
	int32_t pageNumber = PAGE_NUMBER(entityNum);
	if (pageNumber < 0 || pageNumber >= (int32_t)mEntityPages.size())
		return false;
	// free check entity
	int32_t entityIndex = ENT_INDEX_ON_PAGE(entityNum);
	if (ENTITY_TIME(&mEntityPages[pageNumber][entityIndex]) != ENTITY_INUSE_VALUE)
		return false;
	*f = mEntityPages[pageNumber][entityIndex+fieldOffset];
	return true;
}

bool EntityManager::ReadVector(int32_t entityNum, int32_t fieldOffset, float *v)
{
	fieldOffset += HEADER_SIZE;
	if (fieldOffset < HEADER_SIZE || fieldOffset >= mEntitySize)
		return false;
	// bounds check page
	int32_t pageNumber = PAGE_NUMBER(entityNum);
	if (pageNumber < 0 || pageNumber >= (int32_t)mEntityPages.size())
		return false;
	// free check entity
	int32_t entityIndex = ENT_INDEX_ON_PAGE(entityNum);
	if (ENTITY_TIME(&mEntityPages[pageNumber][entityIndex]) != ENTITY_INUSE_VALUE)
		return false;
	v[0] = mEntityPages[pageNumber][entityIndex+fieldOffset  ];
	v[1] = mEntityPages[pageNumber][entityIndex+fieldOffset+1];
	v[2] = mEntityPages[pageNumber][entityIndex+fieldOffset+2];
	return true;
}

bool EntityManager::ReadInt(int32_t entityNum, int32_t fieldOffset, int *i)
{
	fieldOffset += HEADER_SIZE;
	if (fieldOffset < HEADER_SIZE || fieldOffset >= mEntitySize)
		return false;
	// bounds check page
	int32_t pageNumber = PAGE_NUMBER(entityNum);
	if (pageNumber < 0 || pageNumber >= (int32_t)mEntityPages.size())
		return false;
	// free check entity
	int32_t entityIndex = ENT_INDEX_ON_PAGE(entityNum);
	if (ENTITY_TIME(&mEntityPages[pageNumber][entityIndex]) != ENTITY_INUSE_VALUE)
		return false;
	*i = (int)mEntityPages[pageNumber][entityIndex+fieldOffset];
	return true;
}

//-----------------------------------------------------------------------------
// Write
//-----------------------------------------------------------------------------

bool EntityManager::WriteFloat (int32_t address, float f)
{
	int32_t entityNumber = address / mEntitySize;
	int32_t fieldOffset  = address % mEntitySize;
	if (fieldOffset < HEADER_SIZE || fieldOffset >= mEntitySize)
		return false;
	// bounds check page
	int32_t pageNumber = PAGE_NUMBER(entityNumber);
	if (pageNumber < 0 || pageNumber >= (int32_t)mEntityPages.size())
		return false;
	int32_t entityIndex = ENT_INDEX_ON_PAGE(entityNumber);
	if (ENTITY_TIME(&mEntityPages[pageNumber][entityIndex]) != ENTITY_INUSE_VALUE)
		return false;
	mEntityPages[pageNumber][entityIndex+fieldOffset] = f;
	return true;
}

bool EntityManager::WriteVector(int32_t address, const float *v)
{
	int32_t entityNumber = address / mEntitySize;
	int32_t fieldOffset  = address % mEntitySize;
	if (fieldOffset < HEADER_SIZE || fieldOffset >= mEntitySize)
		return false;
	// bounds check page
	int32_t pageNumber = PAGE_NUMBER(entityNumber);
	if (pageNumber < 0 || pageNumber >= (int32_t)mEntityPages.size())
		return false;
	int32_t entityIndex = ENT_INDEX_ON_PAGE(entityNumber);
	if (ENTITY_TIME(&mEntityPages[pageNumber][entityIndex]) != ENTITY_INUSE_VALUE)
		return false;
	mEntityPages[pageNumber][entityIndex+fieldOffset  ] = v[0];
	mEntityPages[pageNumber][entityIndex+fieldOffset+1] = v[1];
	mEntityPages[pageNumber][entityIndex+fieldOffset+2] = v[2];
	return true;
}

bool EntityManager::WriteInt(int32_t address, int i)
{
	int32_t entityNumber = address / mEntitySize;
	int32_t fieldOffset  = address % mEntitySize;
	if (fieldOffset < HEADER_SIZE || fieldOffset >= mEntitySize)
		return false;
	// bounds check page
	int32_t pageNumber = PAGE_NUMBER(entityNumber);
	if (pageNumber < 0 || pageNumber >= (int32_t)mEntityPages.size())
		return false;
	int32_t entityIndex = ENT_INDEX_ON_PAGE(entityNumber);
	if (ENTITY_TIME(&mEntityPages[pageNumber][entityIndex]) != ENTITY_INUSE_VALUE)
		return false;
	((int32_t*)&mEntityPages[pageNumber])[entityIndex+fieldOffset] = i;
	return true;
}

//-----------------------------------------------------------------------------
// Iterate
//-----------------------------------------------------------------------------

int32_t EntityManager::GetFirstEntity()
{
	for (int i=0; i<(int)mEntityPages.size(); ++i)
	{
		for (int j=0; j<ENTITIES_PER_PAGE; ++j)
		{
			if (mEntityPages[i][j*mEntitySize] == ENTITY_INUSE_VALUE)
			{
				return i*ENTITIES_PER_PAGE + j;
			}
		}
	}
	return -1;
}

int32_t EntityManager::GetNextEntity(int32_t entityNum)
{
	// If entityNum is invalid, it will still work, but if there are no
	// entities we will get the number we started with.
	// Also it's a weird kind of loop.

	int32_t pageNumber = PAGE_NUMBER(entityNum);
	assert(pageNumber >= 0 && pageNumber < (int32_t)mEntityPages.size());

	int num = entityNum + 1;
	while (num != entityNum)
	{
		if (num > (int)mEntityPages.size()*ENTITIES_PER_PAGE)
		{
			num = 0;
			if (num == entityNum)
				break;
		}

		int pageNumber  = PAGE_NUMBER(num);
		int entityIndex = ENT_INDEX_ON_PAGE(num);

		if (ENTITY_TIME(&mEntityPages[pageNumber][entityIndex]) != ENTITY_INUSE_VALUE)
			return num;

		++num;
	}
	return num;
}

//-----------------------------------------------------------------------------
} // namespace
//-----------------------------------------------------------------------------
