#pragma once

#include <cstdlib>
#include <random>

// Noise Function Reference: https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83

class RandUtils
{
public:
	static void SeedRandom(uint32_t seed) { std::srand(seed); }

	static uint32_t GetSeed() { return mSeed; }

	template<typename T>
	static T RandomRange(T min, T max);

	static float Rand();
	static float Rand(float x);

	static float Fract(float n);
private:
	static uint32_t mSeed;
};