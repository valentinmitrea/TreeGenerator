#include "tools/Random.h"

#include <cstdlib>


//functie ce genereaza un numar intreg aleator intre x si y
int Random::randomInt(int x, int y)
{
	return x + rand() % (y - x + 1);
}


//functie ce genereaza un numar real aleator intre x si y
float Random::randomFloat(float x, float y)
{
	return x + rand() / (RAND_MAX / (y - x));
}
