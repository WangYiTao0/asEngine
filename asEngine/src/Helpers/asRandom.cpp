#include "aspch.h"
#include "asRandom.h"
#include <random>

namespace as
{
	namespace asRandom
	{
		std::random_device           rand_dev;
		std::mt19937                 generator(rand_dev());

		int asRandom::getRandom(int minValue, int maxValue)
		{
			std::uniform_int_distribution<int>  distr(minValue, maxValue);
			return distr(generator);
		}
		int asRandom::getRandom(int maxValue)
		{
			return getRandom(0, maxValue);
		}
	}
}
