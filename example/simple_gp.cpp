#include "operon/algorithms/config.hpp"
#include "operon/algorithms/nsga2.hpp"
#include "operon/core/dataset.hpp"
#include "operon/core/individual.hpp"
#include "operon/core/problem.hpp"
#include "operon/core/pset.hpp"
#include "operon/core/range.hpp"
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
#include "spdlog/spdlog.h"

using Operon::Dataset;
using Operon::Individual;
using Operon::PrimitiveSet;
using Operon::Problem;
using Operon::Range;

using Operon::BalancedTreeCreator;
using Operon::BasicOffspringGenerator;
using Operon::ChangeFunctionMutation;
using Operon::ChangeVariableMutation;
using Operon::CrowdedComparison;
using Operon::Evaluator;
using Operon::GeneticAlgorithmConfig;
using Operon::Interpreter;
using Operon::KeepBestReinserter;
using Operon::LengthEvaluator;
using Operon::MultiEvaluator;
using Operon::MultiMutation;
using Operon::NSGA2;
using Operon::OnePointMutation;
using Operon::RankIntersectSorter;
using Operon::SubtreeCrossover;
using Operon::TournamentSelector;
using Operon::UniformCoefficientInitializer;
using Operon::UniformTreeInitializer;

int main(int argc, char** argv) {
  spdlog::info("ok!");

  Dataset data("/home/breakds/tmp/gp_data.csv", true);

  spdlog::info("Dataset is now loaded with {} columns", data.Cols());

  for (const std::string& name : data.VariableNames()) {
    spdlog::info("Column '{}'", name);
  }

  Range train_range{0, data.Rows() / 2};
  Range valid_range{data.Rows() / 2, data.Rows()};
  Problem problem(
      data, data.Variables(), data.GetVariable("y").value(), train_range, valid_range);
  problem.GetPrimitiveSet().SetConfig(PrimitiveSet::Arithmetic);

  // Setup mutation and crossover
  double internal_node_bias = 0.9;
  size_t max_tree_depth     = 10;
  size_t max_tree_length    = 50;
  SubtreeCrossover crossover{internal_node_bias, max_tree_depth, max_tree_length};
  MultiMutation mutation;
  // OnePointMutation one_point;
  ChangeVariableMutation change_var{problem.InputVariables()};
  ChangeFunctionMutation change_func{problem.GetPrimitiveSet()};
  // mutation.Add(one_point, 1.0);
  mutation.Add(change_var, 1.0);
  mutation.Add(change_func, 1.0);

  CrowdedComparison comp;

  // set up the selector
  TournamentSelector selector(comp);
  selector.SetTournamentSize(5);

  KeepBestReinserter reinserter(comp);

  GeneticAlgorithmConfig config{
      .Generations          = 100,
      .Evaluations          = 1000000,
      .Iterations           = 0,
      .PopulationSize       = 1000,
      .PoolSize             = 1000,
      .Seed                 = 42,
      .TimeLimit            = ~size_t{0},
      .CrossoverProbability = 1.0,
      .MutationProbability  = 0.25,
      .Epsilon              = 1e-8,
  };

  Interpreter interpreter;
  Evaluator base_evaluator(problem, interpreter);
  LengthEvaluator length_evaluator(problem);
  MultiEvaluator evaluator(problem);
  evaluator.Add(base_evaluator);
  evaluator.Add(length_evaluator);

  UniformCoefficientInitializer coeff_init;

  BalancedTreeCreator creator(problem.GetPrimitiveSet(), problem.InputVariables(), 0.0);
  BasicOffspringGenerator generator(evaluator, crossover, mutation, selector, selector);

  const size_t maxDepth  = 1000;
  const size_t maxLength = 50;
  auto [amin, amax]      = problem.GetPrimitiveSet().FunctionArityLimits();
  UniformTreeInitializer initializer(creator);
  initializer.ParameterizeDistribution(amin + 1, maxLength);
  initializer.SetMinDepth(1);
  initializer.SetMaxDepth(maxDepth);
  initializer.ParameterizeDistribution(0UL, 1UL);

  RankIntersectSorter sorter;

  NSGA2 gp{problem, config, initializer, coeff_init, generator, reinserter, sorter};

  auto report = [&]() { fmt::print("generation {}\n", gp.Generation()); };

  Operon::RandomGenerator random(1234);
  gp.Run(random, report, 10);

  return 0;
}
