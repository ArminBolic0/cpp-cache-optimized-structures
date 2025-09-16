#include <vector>
#include <chrono>
#include <iostream>
#include <random>
#include <algorithm>

using Clock = std::chrono::high_resolution_clock;

struct ParticleAos
{
    float x, y, z;
    double mass; // we use double to make memory non-contiguous for better benchmark, because of 4 bytes of padding we will have after z
};


struct ParticlesSoA
{
    std::vector<float> x, y, z, mass;
    ParticlesSoA(size_t n)
    {
        x.resize(n), y.resize(n), z.resize(n), mass.resize(n);
    }
};

double benchmarkAoS(size_t n, int repeats = 5)/*
                                                benchmark:
                                                - n: number of particles
                                                - repeats: how many times to repeat the loop (best time is taken)
                                                Returns: best execution time in seconds
                                              */
{
    std::vector<ParticleAos> particles; particles.reserve(n); // we reserve to avoid memory reallocation
    std::mt19937_64 rng(123); // we hardcode seed so we have same random values every test to ensure reproducibility
    std::uniform_real_distribution<float> dist(-1000.f, 1000.f); //we define random distribution in range -1000 1000
    for (size_t i = 0; i < n; i++) particles.push_back({ dist(rng), dist(rng), dist(rng), dist(rng) });
    double bestTime = 1e300;
    for (size_t i = 0; i < repeats; i++)
    {
        auto startTime = Clock::now();
        double sum = 0;
        for (size_t k = 0; k < n; k++)
        {
            if (particles[k].x > 0.0f) sum += particles[k].mass;
        }
        auto endTime = Clock::now();
        std::chrono::duration<double> duration = endTime - startTime;
        bestTime = std::min(bestTime, duration.count());
        if (sum == 0) std::cout << ""; // with this line we prevent optimizing away, we basically trick compiler to think sum is used so it doesnt optimize the loop away entirely aka Dummy read
    }

    return bestTime;
}

double benchmarkSoA(size_t n, int repeats = 5)
{
    ParticlesSoA particles(n);
    std::mt19937_64 rng(123);
    std::uniform_real_distribution<float> dist(-1000.f, 1000.f);
    for (size_t i = 0; i < n; i++) { // in case of SoA, we dont store whole objects in arrays, rather we store each field in its own array, good to use when we have field-centric operations 
        particles.x[i] = dist(rng);
        particles.y[i] = dist(rng);
        particles.z[i] = dist(rng);
        particles.mass[i] = dist(rng);
    }
    double bestTime = 1e300;
    for (size_t i = 0; i < repeats; i++)
    {
        auto startTime = Clock::now();
        double sum = 0;
        for (size_t k = 0; k < particles.x.size(); k++)
        {
            if (particles.x[k] > 0.0f) sum += particles.mass[k];
        }
        auto endTime = Clock::now();
        std::chrono::duration<double> duration = endTime - startTime;
        bestTime = std::min(bestTime, duration.count());
        if (sum == 0) std::cout << "";
    }
    return bestTime;
}

int main()
{
    const size_t N = 5'000'000; // use this to modify how many particles you want created
    double benchmarkAoSTime = benchmarkAoS(N);
    double benchmarkSoATime = benchmarkSoA(N);
    std::cout << "AoS time: " << benchmarkAoSTime << " s\n";// you can also add second argument to modify how many repeats you want
    std::cout << "SoA time: " << benchmarkSoATime << " s\n";
    std::cout << "Time difference: " << benchmarkAoSTime - benchmarkSoATime << std::endl;
    std::cout << "This shows us that using SoA if we have field-centric operations can improve time duration by a lot." << std::endl;
    return 0;
}


