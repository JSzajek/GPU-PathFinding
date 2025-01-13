#include "RandomUtils.h"

#include <assert.h>
#include <stack>
#include <vector>

uint32_t RandUtils::mSeed = 1;

template<typename T>
T RandUtils::RandomRange(T min, T max)
{
	assert(false);
	return T();
}

template<>
size_t RandUtils::RandomRange<size_t>(size_t min, size_t max)
{
	return (std::rand() % (max - min + 1)) + min;
}

template<>
uint32_t RandUtils::RandomRange<uint32_t>(uint32_t min, uint32_t max)
{
	return (std::rand() % (max - min + 1)) + min;
}

template<>
uint16_t RandUtils::RandomRange<uint16_t>(uint16_t min, uint16_t max)
{
	return (std::rand() % (max - min + 1)) + min;
}

template<>
uint8_t RandUtils::RandomRange<uint8_t>(uint8_t min, uint8_t max)
{
	return (std::rand() % (max - min + 1)) + min;
}

template<>
float RandUtils::RandomRange<float>(float min, float max)
{
	float randFloat = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
	return min + randFloat * (max - min);
}

float RandUtils::Rand()
{
	return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
}

float RandUtils::Rand(float x)
{
	return Fract(std::sin(x) * 43758.5453f);
}

float RandUtils::Fract(float n)
{
	return n - static_cast<long>(n);
}