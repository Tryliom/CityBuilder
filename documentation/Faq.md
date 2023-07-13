[Return](../README.md)

## How to compile the shader

Run `buildShader.bat`

## How to use the random number generator with seed

1. Define the seed using `Random::setSeed(int seed)`
2. Make `Random` use that seed by calling `Random::useSeed()`
3. Use `Random` as usual
4. Make sure to stop use the seed after you are done using it by calling `Random::stopUsingSeed()`

Example:
```c++
Random::SetSeed(42);
Random::UseSeed();

for (Tile& tile : road.Tiles)
{
    tile.Texture = Texture((Road) Random::Range(0, (int) Road::Flower3));
}

Random::StopUseSeed();
```