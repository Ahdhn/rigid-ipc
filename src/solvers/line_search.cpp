// Functions for optimizing functions.
// Includes line search to find a step length to reduce a function.
#include "line_search.hpp"

#include <logger.hpp>
#include <profiler.hpp>

namespace ccd {
namespace opt {

    // Search along a search direction to find a scalar step_length in [0, 1]
    // such that f(x + step_length * dir) ≤ f(x).
    bool line_search(
        const Eigen::VectorXd& x,
        const Eigen::VectorXd& dir,
        const std::function<double(const Eigen::VectorXd&)>& f,
        double& step_length,
        const double min_step_length)
    {
        return line_search(
            x, dir, f, Eigen::VectorXd::Zero(dir.size()), step_length,
            min_step_length, 0.0);
    }

    // Search along a search direction to find a scalar step_length in [0, 1]
    // such that f(x + step_length * dir) ≤ f(x).
    bool line_search(
        const Eigen::VectorXd& x,
        const Eigen::VectorXd& dir,
        const std::function<double(const Eigen::VectorXd&)>& f,
        const Eigen::VectorXd& grad_fx,
        double& step_length,
        const double min_step_length,
        const double armijo_rule_coeff)
    {
        return constrained_line_search(
            x, dir, f, grad_fx, [](const Eigen::VectorXd&) { return true; },
            step_length, min_step_length, armijo_rule_coeff);
    }

    const static char* LS_BEGIN_LOG =
        "solver=constrainted_line_search action=BEGIN";
    const static char* LS_ARMIJO_LOG =
        "solver=constrainted_line_search iter={:d} action=armijo_rule";
    const static char* LS_MIN_LOG =
        "solver=constrainted_line_search iter={:d} action=minimization_rule";
    const static char* LS_BREAK_LOG =
        "solver=constrainted_line_search iter={:d} action=break_condition";
    const static char* LS_FAIL_LOG =
        "solver=constrainted_line_search action=END status=fail";
    // Search along a search direction to find a scalar step_length in [0, 1]
    // such that f(x + step_length * dir) ≤ f(x).
    // TODO: Filter the dof that violate the constraints. These are the indices
    // i where ϕ([g(x)]_i) = ∞.
    bool constrained_line_search(
        const Eigen::VectorXd& x,
        const Eigen::VectorXd& dir,
        const std::function<double(const Eigen::VectorXd&)>& f,
        const Eigen::VectorXd& grad_fx,
        const std::function<bool(const Eigen::VectorXd&)>& constraint,
        double& step_length,
        const double min_step_length,
        const double armijo_rule_coeff)
    {
        const double fx = f(x); // Function value we want to beat
        PROFILE_POINT("line_search");
        PROFILE_START();

        spdlog::trace(
            "{} step_length={:e} f(x0)={:e} x0={} dir={}", LS_BEGIN_LOG,
            step_length, fx, logger::fmt_eigen(x), logger::fmt_eigen(dir));

        int num_it = 1;
        std::function<bool()> minimization_rule;

        if (armijo_rule_coeff != 0.0) {
            const double wolfe1 = armijo_rule_coeff * dir.transpose() * grad_fx;
            minimization_rule = [&]() {
                Eigen::VectorXd xi = x + step_length * dir;
                auto f_xi = f(xi);
                auto f_wolfe = fx + step_length * wolfe1;
                spdlog::trace(
                    "{} step_length={:e} f(xi)={:e} f_wolfe={:e} xi={}",
                    fmt::format(LS_ARMIJO_LOG, num_it), step_length, f_xi,
                    f_wolfe, logger::fmt_eigen(xi));
                return f_xi <= f_wolfe;
            };
        } else {
            minimization_rule = [&]() {
                Eigen::VectorXd xi = x + step_length * dir;
                auto fxi = f(xi);
                spdlog::trace(
                    "{} step_length={:e} f(xi)={:e} f(x0)={:e} xi={}",
                    fmt::format(LS_MIN_LOG, num_it), step_length, fxi, fx,
                    logger::fmt_eigen(xi));
                return fxi < fx;
            };
        }

        double step_norm = (step_length * dir).norm();

        NAMED_PROFILE_POINT("line_search__minimization_rule", MINIMIZATION_RULE)
        NAMED_PROFILE_POINT("line_search__constraint", CONSTRAINT)

        bool success = false;
        while (step_norm >= min_step_length) {
            PROFILE_START(MINIMIZATION_RULE)
            bool min_rule = minimization_rule();
            PROFILE_MESSAGE(
                MINIMIZATION_RULE, fmt::format("min_rule,{}", min_rule))
            PROFILE_END(MINIMIZATION_RULE)

            PROFILE_START(CONSTRAINT)
            bool cstr = constraint(x + step_length * dir);
            PROFILE_MESSAGE(CONSTRAINT, fmt::format("cstr,{}", cstr))
            PROFILE_END(CONSTRAINT)

            spdlog::trace(
                "{} min_rule={} constraint={} step_norm={:e} step_length={:e}",
                fmt::format(LS_BREAK_LOG, num_it), min_rule, cstr, step_norm,
                step_length);

            if (min_rule && cstr) {
                success = true;
                break; // while loop
            }
            step_length /= 2.0;
            step_norm = (step_length * dir).norm();
            num_it += 1;
        }
        if (!success) {
            spdlog::debug(
                "{} step_norm={:e} step_length={:e} min_step_length={:e}",
                LS_FAIL_LOG, step_norm, step_length, min_step_length);
        }
        PROFILE_MESSAGE(
            ,
            fmt::format(
                "success,{},it,{},dir,{:10e}", success, num_it, dir.norm()))
        PROFILE_END();
        return success;
    }

    // Log samples along the search direction.
    void sample_search_direction(
        const Eigen::VectorXd& x,
        const Eigen::VectorXd& dir,
        const std::function<double(const Eigen::VectorXd&, Eigen::VectorXd&)>&
            f_and_gradf,
        double max_step)
    {
        Eigen::VectorXd grad_fx;

        Eigen::VectorXd sampling;
        int num_samples = 25;
        bool use_geometric_sampling = true;
        if (use_geometric_sampling) {
            sampling.resize(2 * num_samples + 1);
            double max_pow = log10(max_step);
            sampling.tail(num_samples) =
                pow(10, Eigen::ArrayXd::LinSpaced(num_samples, -16, max_pow));
            sampling(num_samples) = 0;
            sampling.head(num_samples) = -sampling.tail(num_samples).reverse();
        } else {
            sampling = Eigen::VectorXd::LinSpaced(
                2 * num_samples + 1, -max_step, max_step);
        }

        double fx0 = f_and_gradf(x, grad_fx);
        for (int i = 0; i < sampling.size(); i++) {
            double step_length = sampling(i);
            double fx = f_and_gradf(x + step_length * dir, grad_fx);
            spdlog::debug(
                "method=line_search step_length={:+.1e} obj={:018.16g} "
                "(obj_i-obj_0)={:+.16g} norm_grad={:g}",
                step_length, fx, fx - fx0, grad_fx.norm());
        }
    }

} // namespace opt
} // namespace ccd
