#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "Serialization.hpp"

//Enum For the Versions
enum : int32_t
{
	SV_INITIAL = 1,
	SV_InitV2,
	SV_RemoveP3P4Score,
	// Don't remove this
	SV_LatestPlusOne
};

#define SV_LatestVersion (SV_LatestPlusOne - 1)

void Serialize(Serializer* serializer, void* dataPtr, size_t dataSize)
{
	if (serializer->IsWriting)
	{
		fwrite(dataPtr, dataSize, 1, serializer->FilePtr);
	}
	else
	{
		fread(dataPtr, dataSize, 1, serializer->FilePtr);
	}
}


void Serialize(Serializer* serializer, int32_t* dataPtr) {
	Serialize(serializer, dataPtr, sizeof(*dataPtr));
}

void Serialize(Serializer* serializer, float* dataPtr) {
	Serialize(serializer, dataPtr, sizeof(*dataPtr));
}

Serializer SerializerOpenFile(bool isWriting)
{
	Serializer serializer;
	serializer.IsWriting = isWriting;
	if(serializer.IsWriting)
	{
		serializer.FilePtr = fopen("myfile.bin", "wb");
	}else
	{
		serializer.FilePtr = fopen("myfile.bin", "rb");
	}
	return serializer;
}



void SerializerFileClose(Serializer* serializer)
{
	fclose(serializer->FilePtr);
}
