#include "Tile.h"


void Serialize(Serializer* ser, Tile* tile)
{
	Serialize(ser, (int*)&tile->Type);
}