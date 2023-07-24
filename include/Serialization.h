#pragma once
#include <stdint.h>

#include "Maths.h"

//Enum For the Versions
enum SerilizerVersion : int
{
	SV_INITIAL = 1,
	SV_LatestVersion,
	// Don't remove this
	SV_LatestPlusOne
};

//Serializer struct 
struct Serializer
{
	int DataVersion;
	FILE* FilePtr;
	bool IsWriting = true;
};


void Serialize(Serializer* serializer, void* dataPtr, size_t dataSize);

void Serialize(Serializer* serializer, int32_t* dataPtr);

void Serialize(Serializer* serializer, float* dataPtr);

void Serialize(Serializer* serializer, bool* dataPtr);

Serializer SerializerOpenFile(bool isWriting, const char* file);

void SerializeIncludingVersion(Serializer* serializer);

void SerializerFileClose(Serializer* serializer);
void Serialize(Serializer* ser, Vector2F* vector2f);
