#pragma once
#include "defs.h"


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


    }
}