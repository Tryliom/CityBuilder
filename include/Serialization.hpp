#pragma once
#include <stdint.h>

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

Serializer SerializerOpenFile(bool isWriting);

void SerializerFileClose(Serializer* serializer);
