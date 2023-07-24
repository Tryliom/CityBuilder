#include "Tile.h"


void Serialize(Serializer* ser, Tile* tile)
{
	Serialize(ser, (int*)&tile->Type);
	Serialize(ser, &tile->Progress);
	Serialize(ser, &tile->IsBuilt);
	Serialize(ser, &tile->NeedToBeDestroyed);
	Serialize(ser, &tile->TreeGrowth);
	Serialize(ser, &tile->TreeSpawnTimer);

	for (auto& item : *tile->Inventory)
	{
		Serialize(ser, &item.second);//Items(i)

		/* //Debug Inventory MayorHouse and Logisitics Center
		if (tile->Type == TileType::MayorHouse)
		{
			printf("Element %i : %i \n ", item.first, item.second);
		}

		if (tile->Type == TileType::LogisticsCenter)
		{
			printf("Element %i : %i \n ", item.first, item.second);
		}
		*/
	}
}