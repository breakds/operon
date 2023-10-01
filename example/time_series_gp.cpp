#include <chrono>
#include <memory>

#include "fmt/core.h"
#include "operon/algorithms/nsga2.hpp"
#include "operon/core/problem.hpp"
#include "operon/core/version.hpp"
#include "operon/formatter/formatter.hpp"
#include "operon/interpreter/interpreter.hpp"
#include "operon/operators/creator.hpp"
#include "operon/operators/crossover.hpp"
#include "operon/operators/evaluator.hpp"
#include "operon/operators/generator.hpp"
#include "operon/operators/initializer.hpp"
#include "operon/operators/mutation.hpp"
#include "operon/operators/non_dominated_sorter.hpp"
#include "operon/operators/reinserter.hpp"
#include "operon/operators/selector.hpp"

int main()
{
    // Step 0 - Initialize the configuration.
    Operon::GeneticAlgorithmConfig config {
        .Generations = 100,
        .Evaluations = 1000000,
        .Iterations = 0,
        .PopulationSize = 100,
        .PoolSize = 100,
        .Seed = std::random_device {}(),
        .TimeLimit = 3600,
        .CrossoverProbability = 0.5,
        .MutationProbability = 0.5,
        .Epsilon = 1e-4,
    };

    fmt::print("Config.Generations: {}\n", config.Generations);

    // +-------------------------------------------------------------+
    // | Step 1 - Set up the problem, i.e., the "what"               |
    // +-------------------------------------------------------------+

    // --> Step 1.1 Dataset
    Operon::Dataset dataset { "/home/breakds/tmp/gp_data.csv", true /* header */ };
    Operon::Range train_range { 0, dataset.Rows<size_t>() / 3 };
    Operon::Range test_range { train_range.End(), 2 * dataset.Rows<size_t>() / 3 };
    Operon::Range valid_range { test_range.End(), dataset.Rows<size_t>() };

    fmt::print("Training Size: {}\n", train_range.Size());
    fmt::print("Testing size: {}\n", test_range.Size());
    fmt::print("Validation size: {}\n", valid_range.Size());

    // --> Step 1.final - Create the problem
    Operon::Problem problem {
        dataset, /* ownership transferred */
        train_range,
        test_range,
        valid_range,
    };

    // --> Step 1.extra - Update the primitive (operator) set, a.k.a. pset.

    // TODO(breakds): Implement this

    // +-------------------------------------------------------------+
    // | Step 2 - Setup the solver (NSGA2 instance), i.e. the "how"  |
    // +-------------------------------------------------------------+

    // --> Step 2.1 - TreeCreator. It is a basic component that is called to
    //                assemble a tree based on the existing pset and inputs.
    //                TreeCreator inherits from OperatorBase.

    auto creator = std::make_unique<Operon::BalancedTreeCreator>(
        problem.GetPrimitiveSet(),
        problem.GetInputs(),
        0.0 /* irregularity bias */);

    // --> Step 2.2 - TreeInitializer. This uses the creator under the hood and
    //                initialize the trees. TreeInitializer inherits from
    //                OperatorBase.

    Operon::UniformTreeInitializer tree_init(*creator);
    tree_init.ParameterizeDistribution(size_t(2), size_t(15) /* max length */);
    tree_init.SetMinDepth(1);
    tree_init.SetMaxDepth(1000);

    // --> Step 2.3 - CoefficientInitializer. My current understanding is that
    //                this is used to initialize the parameters that need to be
    //                "trained". For pure symbolic training there should be no
    //                parameters to train (as far as I understand ...)

    Operon::CoefficientInitializer<std::uniform_int_distribution<int>> coeff_init {};
    coeff_init.ParameterizeDistribution(-5, 5);

    // Create a genetic algorithm solving engine using the NSGA 2 algorithm.
    // Operon::NSGA2 gp {};
    return 0;
}
