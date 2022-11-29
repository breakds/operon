// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright 2019-2022 Heal Research

#include <taskflow/taskflow.hpp>
#include "operon/interpreter/interpreter.hpp"

namespace Operon {
    auto EvaluateTrees(std::vector<Operon::Tree> const& trees, Operon::Dataset const& dataset, Operon::Range range, size_t nthread) -> std::vector<std::vector<Operon::Scalar>> {
        if (nthread == 0) { nthread = std::thread::hardware_concurrency(); }
        tf::Executor executor(nthread);
        tf::Taskflow taskflow;
        std::vector<std::vector<Operon::Scalar>> result(trees.size());
        Operon::Interpreter interpreter;

        taskflow.for_each_index(size_t{0}, size_t{trees.size()}, size_t{1}, [&](size_t i) {
            result[i].resize(range.Size());
            interpreter.Evaluate<Operon::Scalar>(trees[i], dataset, range, result[i]);
        });
        executor.run(taskflow);
        executor.wait_for_all();
        return result;
    }

    auto EvaluateTrees(std::vector<Operon::Tree> const& trees, Operon::Dataset const& dataset, Operon::Range range, Operon::Span<Operon::Scalar> result, size_t nthread) -> void {
        if (nthread == 0) { nthread = std::thread::hardware_concurrency(); }
        tf::Executor executor(nthread);
        tf::Taskflow taskflow;
        Operon::Interpreter interpreter;

        taskflow.for_each_index(size_t{0}, size_t{trees.size()}, size_t{1}, [&](size_t i) {
            auto res = result.subspan(i * range.Size(), range.Size());
            interpreter.Evaluate<Operon::Scalar>(trees[i], dataset, range, res);
        });
        executor.run(taskflow);
        executor.wait_for_all();
    }
} // namespace Operon
