#pragma once
#include "defs.h"
#include <random>
#include <cmath>
#include <thread>
#include <functional>


namespace LxGeo
{
    namespace GeometryFactoryShared
    {


        // Particle class
        template<typename Func>
        class Particle {
        public:
            double position;
            double velocity;
            double bestPosition;
            double bestFitness;
            Func& f;

            Particle(Func& _f, double minRange, double maxRange, std::mt19937& rng)
                : position(getRandomInRange(minRange, maxRange, rng)),
                velocity(0.0),
                bestPosition(position),
                bestFitness(_f(position)),
                f(_f) {}

            void updateVelocity(double globalBestPosition, double inertiaWeight, double cognitiveWeight, double socialWeight, std::mt19937& rng) {
                std::uniform_real_distribution<double> dist(0.0, 1.0);
                double r1 = dist(rng);
                double r2 = dist(rng);
                velocity = inertiaWeight * velocity + cognitiveWeight * r1 * (bestPosition - position) + socialWeight * r2 * (globalBestPosition - position);
            }

            void updatePosition() {
                position += velocity;
                double fitness = f(position);
                if (fitness < bestFitness) {
                    bestFitness = fitness;
                    bestPosition = position;
                }
            }

        private:
            double getRandomInRange(double min, double max, std::mt19937& rng) {
                std::uniform_real_distribution<double> dist(min, max);
                return dist(rng);
            }
        };

        // Particle Swarm Optimization algorithm
        template<typename Func>
        double particleSwarmOptimization(Func f, double minRange, double maxRange, int numParticles, int maxIterations) {
            const double inertiaWeight = 0.8;
            const double cognitiveWeight = 1.2;
            const double socialWeight = 1.2;

            std::random_device rd;
            std::mt19937 rng(rd());

            std::vector<Particle<Func>> particles;
            double globalBestPosition;
            double globalBestFitness = std::numeric_limits<double>::max();

            // Initialize particles
            for (int i = 0; i < numParticles; ++i) {
                particles.emplace_back(f, minRange, maxRange, rng);
                if (particles[i].bestFitness < globalBestFitness) {
                    globalBestFitness = particles[i].bestFitness;
                    globalBestPosition = particles[i].bestPosition;
                }
            }

            // PSO iterations
            for (int iteration = 0; iteration < maxIterations; ++iteration) {
                for (auto& particle : particles) {
                    particle.updateVelocity(globalBestPosition, inertiaWeight, cognitiveWeight, socialWeight, rng);
                    particle.updatePosition();
                    if (particle.bestFitness < globalBestFitness) {
                        globalBestFitness = particle.bestFitness;
                        globalBestPosition = particle.bestPosition;
                    }
                }
            }

            return globalBestPosition;
        }

        namespace ND {

            template<typename Func>
            class Particle {
            public:
                std::vector<double> position;
                std::vector<double> velocity;
                std::vector<double> bestPosition;
                double bestFitness;
                Func& f;

                Particle(Func& _f, int numDimensions, const std::vector<double>& _minRange, const std::vector<double>& _maxRange, std::mt19937& rng)
                    : position(numDimensions),
                    velocity(numDimensions),
                    bestPosition(numDimensions),
                    bestFitness(std::numeric_limits<double>::max()), minRange(_minRange), maxRange(_maxRange),f(_f) {

                    std::uniform_real_distribution<double> dist(0.0, 1.0);

                    for (int i = 0; i < numDimensions; ++i) {
                        position[i] = dist(rng) * (maxRange[i] - minRange[i]) + minRange[i];
                        velocity[i] = dist(rng) * (maxRange[i] - minRange[i]);
                        bestPosition[i] = position[i];
                    }

                    bestFitness = f(position);
                }

                void updateVelocity(const std::vector<double>& globalBestPosition, double inertiaWeight, double cognitiveWeight, double socialWeight, std::mt19937& rng) {
                    std::uniform_real_distribution<double> dist(0.0, 1.0);

                    for (size_t i = 0; i < velocity.size(); ++i) {
                        double r1 = dist(rng);
                        double r2 = dist(rng);
                        velocity[i] = inertiaWeight * velocity[i] + cognitiveWeight * r1 * (bestPosition[i] - position[i]) + socialWeight * r2 * (globalBestPosition[i] - position[i]);

                        // Apply bounds
                        if (velocity[i] < minRange[i]) {
                            velocity[i] = abs(r1)*minRange[i];
                        }
                        else if (velocity[i] > maxRange[i]) {
                            velocity[i] = abs(r2)*maxRange[i];
                        }
                    }
                }

                void updatePosition() {
                    for (size_t i = 0; i < position.size(); ++i) {
                        position[i] += velocity[i];

                        // Apply bounds
                        if (position[i] < minRange[i]) {
                            position[i] = minRange[i]+(minRange[i]- position[i]);
                            velocity[i] /= 2;
                        }
                        else if (position[i] > maxRange[i]) {
                            position[i] = maxRange[i]-(position[i]- maxRange[i]);
                            velocity[i] /= 2;
                        }
                    }
                    double fitness = f(position);
                    //std::cout << int(this) << " : pos: " << position[0] << " " << position[1] << " fitness: " << fitness << std::endl;
                    if (fitness < bestFitness) {
                        bestFitness = fitness;
                        bestPosition = position;
                    }
                }

            private:
                std::vector<double> minRange;
                std::vector<double> maxRange;
            };

            template<typename Func>
            std::vector<double> psoOptimization(Func f, int numParticles, int numDimensions, const std::vector<double>& minRange, const std::vector<double>& maxRange, int maxIterations) {
                std::random_device rd;
                std::mt19937 rng(rd());

                std::vector<Particle<Func>> particles;
                std::vector<double> globalBestPosition(numDimensions);
                double globalBestFitness = std::numeric_limits<double>::max();

                // Initialize particles
                for (int i = 0; i < numParticles; ++i) {
                    particles.emplace_back(f, numDimensions, minRange, maxRange, rng);

                    // Update global best
                    if (particles[i].bestFitness < globalBestFitness) {
                        globalBestFitness = particles[i].bestFitness;
                        globalBestPosition = particles[i].bestPosition;
                    }
                }

                // Particle Swarm Optimization loop
                for (int iter = 0; iter < maxIterations; ++iter) {
                    double inertiaWeight = 0.9;  // Inertia weight
                    double cognitiveWeight = 2.0;  // Cognitive weight
                    double socialWeight = 2.0;  // Social weight

                    // Update particles
                    if (false) {
                        std::vector<std::thread> threads;
                        for (int i = 0; i < numParticles; ++i) {
                            threads.emplace_back([&particles, &globalBestPosition, &inertiaWeight, &cognitiveWeight, &socialWeight, i, &rng]() {
                                particles[i].updateVelocity(globalBestPosition, inertiaWeight, cognitiveWeight, socialWeight, rng);
                                particles[i].updatePosition();
                                });
                        }

                        // Wait for all threads to finish
                        for (auto& thread : threads) {
                            thread.join();
                        }
                    }
                    else {
                        for (int i = 0; i < numParticles; ++i) {
                            particles[i].updateVelocity(globalBestPosition, inertiaWeight, cognitiveWeight, socialWeight, rng);
                            particles[i].updatePosition();
                        }
                    }

                    // Update global best
                    for (int i = 0; i < numParticles; ++i) {
                        if (particles[i].bestFitness < globalBestFitness) {
                            globalBestFitness = particles[i].bestFitness;
                            globalBestPosition = particles[i].bestPosition;
                        }
                    }
                }

                return globalBestPosition;
            }

        }


    }
}