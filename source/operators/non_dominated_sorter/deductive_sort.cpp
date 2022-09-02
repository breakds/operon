// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright 2019-2022 Heal Research

#include "operon/operators/non_dominated_sorter.hpp"
#include "operon/core/individual.hpp"

namespace Operon {

    auto DeductiveSorter::Sort(Operon::Span<Operon::Individual const> pop, Operon::Scalar eps) const -> NondominatedSorterBase::Result
    {
        size_t n = 0; // total number of sorted solutions
        std::vector<std::vector<size_t>> fronts;
        std::vector<bool> dominated(pop.size(), false);
        std::vector<bool> sorted(pop.size(), false);
        auto dominatedOrSorted = [&](size_t i) { return sorted[i] || dominated[i]; };

        while (n < pop.size()) {
            std::vector<size_t> front;

            for (size_t i = 0; i < pop.size(); ++i) {
                if (!dominatedOrSorted(i)) {
                    for (size_t j = i + 1; j < pop.size(); ++j) {
                        if (!dominatedOrSorted(j)) {
                            auto const& lhs = pop[i];
                            auto const& rhs = pop[j];
                            auto res = ParetoDominance{}(lhs.Fitness, rhs.Fitness, eps);
                            dominated[i] = (res == Dominance::Right);
                            dominated[j] = (res == Dominance::Left);

                            if (dominated[i]) {
                                break;
                            }
                        }
                    }

                    if (!dominated[i]) {
                        front.push_back(i);
                        sorted[i] = true;
                    }
                }
            }
            std::fill(dominated.begin(), dominated.end(), 0UL);
            n += front.size();
            fronts.push_back(front);
        }
        return fronts;
    }
} // namespace Operon
