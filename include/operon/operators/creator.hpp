// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright 2019-2023 Heal Research

#ifndef OPERON_CREATOR_HPP
#define OPERON_CREATOR_HPP

#include "operon/core/operator.hpp"
#include "operon/operon_export.hpp"
#include "operon/core/variable.hpp"

namespace Operon {

class Tree;
class PrimitiveSet;

// the creator builds a new tree using the existing pset and allowed inputs
struct OPERON_EXPORT CreatorBase : public OperatorBase<Tree, size_t, size_t, size_t> {
    CreatorBase(PrimitiveSet const& pset, Operon::Span<Operon::Hash const> variables)
        : pset_(pset)
        , variables_(variables.begin(), variables.end())
    {
    }

    [[nodiscard]] auto GetPrimitiveSet() const -> PrimitiveSet const& { return pset_.get(); }
    void SetPrimitiveSet(PrimitiveSet const& pset) { pset_ = pset; }

    [[nodiscard]] auto GetVariables() const -> Operon::Span<Operon::Hash const> { return variables_; }
    auto SetVariables(Operon::Span<Operon::Hash const> variables) { variables_ = std::vector<Operon::Hash>(variables.begin(), variables.end()); }

private:
    std::reference_wrapper<PrimitiveSet const> pset_;
    std::vector<Operon::Hash> variables_;
};

// this tree creator expands bread-wise using a "horizon" of open expansion slots
// at the end the breadth sequence of nodes is converted to a postfix sequence
// if the depth is not limiting, the target length is guaranteed to be reached
class OPERON_EXPORT BalancedTreeCreator final : public CreatorBase {
public:
    BalancedTreeCreator(PrimitiveSet const& pset, Operon::Span<Operon::Hash const> variables, double bias = 0.0)
        : CreatorBase(pset, variables)
        , irregularityBias_(bias)
    {
    }

    auto operator()(Operon::RandomGenerator& random, size_t targetLen, size_t minDepth, size_t maxDepth) const -> Tree override;

    void SetBias(double bias) { irregularityBias_ = bias; }
    [[nodiscard]] auto GetBias() const -> double { return irregularityBias_; }

private:
    double irregularityBias_;
};

class OPERON_EXPORT GrowTreeCreator final : public CreatorBase {
    public:
        GrowTreeCreator(PrimitiveSet const& pset, Operon::Span<Operon::Hash const> variables)
            : CreatorBase(pset, variables) 
        { }

    auto operator()(Operon::RandomGenerator& random, size_t targetLen, size_t minDepth, size_t maxDepth) const -> Tree override;
};

class OPERON_EXPORT ProbabilisticTreeCreator final : public CreatorBase {
public:
    ProbabilisticTreeCreator(PrimitiveSet const& pset, Operon::Span<Operon::Hash const> variables, double bias = 0.0)
        : CreatorBase(pset, variables)
        , irregularityBias_(bias)
    {
    }
    
    auto operator()(Operon::RandomGenerator& random, size_t targetLen, size_t minDepth, size_t maxDepth) const -> Tree override;

private:
    double irregularityBias_;
};

} // namespace Operon

#endif
