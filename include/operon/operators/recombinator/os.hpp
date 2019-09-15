#ifndef OS_RECOMBINATOR_HPP
#define OS_RECOMBINATOR_HPP

#include "core/operator.hpp"

namespace Operon
{
    template<typename TEvaluator, typename TSelector, typename TCrossover, typename TMutator>
    class OffspringSelectionRecombinator : public RecombinatorBase<TEvaluator, TSelector, TCrossover, TMutator>
    {
        public:
            explicit OffspringSelectionRecombinator(TEvaluator& eval, TSelector& sel, TCrossover& cx, TMutator& mut) : RecombinatorBase<TEvaluator, TSelector, TCrossover, TMutator>(eval, sel, cx, mut) { }

            using T = typename TSelector::SelectableType;
            std::optional<T> operator()(operon::rand_t& random, double pCrossover, double pMutation) const override 
            {
                std::uniform_real_distribution<double> uniformReal;
                bool doCrossover = uniformReal(random) < pCrossover;
                bool doMutation  = uniformReal(random) < pMutation;

                if (!(doCrossover || doMutation)) return std::nullopt;

                constexpr bool Max       = TSelector::Maximization;
                constexpr gsl::index Idx = TSelector::SelectableIndex;

                auto population = this->Selector().Population();

                auto first = this->selector(random);
                auto fit   = population[first].Fitness[Idx];

                typename TSelector::SelectableType child;

                if (doCrossover)
                {
                    auto second = this->selector(random);
                    child.Genotype = this->crossover(random, population[first].Genotype, population[second].Genotype);

                    if constexpr(TSelector::Maximization) { fit = std::max(fit, population[second].Fitness[Idx]); }
                    else                                  { fit = std::min(fit, population[second].Fitness[Idx]); } 
                }

                if (doMutation)
                {
                    child.Genotype = doCrossover 
                        ? this->mutator(random, std::move(child.Genotype))
                        : this->mutator(random, population[first].Genotype);
                }

                auto f = this->evaluator(random, child);
                child.Fitness[Idx] = std::isfinite(f) ? f : (Max ? std::numeric_limits<double>::min() : std::numeric_limits<double>::max());

                if ((Max && child.Fitness[Idx] > fit) || (!Max && child.Fitness[Idx] < fit))
                {
                    return std::make_optional(child);
                }
                return std::nullopt;
            }

            void MaxSelectionPressure(size_t value) { maxSelectionPressure = value; }
            size_t MaxSelectionPressure() const { return maxSelectionPressure; }

            void Prepare(const gsl::span<const T> pop) const override
            {
                this->Selector().Prepare(pop);
                lastEvaluations = this->evaluator.get().FitnessEvaluations();
            }

            double SelectionPressure() const 
            { 
                if (this->Selector().Population().empty())
                {
                    return 0;
                }
                return (this->evaluator.get().FitnessEvaluations() - lastEvaluations) / static_cast<double>(this->Selector().Population().size()); 
            }

            bool Terminate() const override 
            {
                return RecombinatorBase<TEvaluator, TSelector, TCrossover, TMutator>::Terminate() || 
                    SelectionPressure() > maxSelectionPressure;
            };

        private:
            mutable size_t lastEvaluations;
            size_t maxSelectionPressure;
    };
} // namespace Operon

#endif
