#include <catch2/catch.hpp>

#include <Eigen/Eigenvalues>
#include <solvers/newton_solver.hpp>
#include <utils/eigen_ext.hpp>

using namespace ccd;
using namespace opt;

TEST_CASE("Simple tests of Newton's Method", "[opt][newtons_method]")
{
    int num_vars = GENERATE(1, 10, 100);

    // Setup problem
    // -----------------------------------------------------------------
    class AdHocProblem : public virtual OptimizationProblem {
    public:
        AdHocProblem(int num_vars)
        {
            num_vars_ = num_vars;
            x0.resize(num_vars);
            x0.setRandom();
            is_dof_fixed_ = Eigen::VectorXb::Zero(num_vars);
        }

        void compute_objective(
            const Eigen::VectorXd& x,
            double& fx,
            Eigen::VectorXd& grad_fx,
            Eigen::SparseMatrix<double>& hess_fx,
            bool compute_grad = true,
            bool compute_hess = true) override
        {
            fx = x.squaredNorm() / 2.0;
            if (compute_grad) {
                grad_fx = x;
            }
            if (compute_hess) {
                hess_fx =
                    Eigen::MatrixXd::Identity(x.rows(), x.rows()).sparseView();
            }
        }

        bool has_collisions(
            const Eigen::VectorXd&, const Eigen::VectorXd&) const override
        {
            return false;
        }

        const Eigen::VectorXd& starting_point() override { return x0; }
        int num_vars() const override { return num_vars_; }
        const Eigen::VectorXb& is_dof_fixed() override { return is_dof_fixed_; }

        double compute_min_distance(const Eigen::VectorXd& x) const override
        {
            return -1;
        };

        int num_vars_;
        Eigen::VectorXb is_dof_fixed_;
        Eigen::VectorXd x0;
    };

    AdHocProblem problem(num_vars);

    NewtonSolver solver;
    solver.set_problem(problem);
    solver.init_solve();
    OptimizationResults results = solver.solve(problem.starting_point());
    REQUIRE(results.success);
    CHECK(results.x.squaredNorm() == Approx(0).margin(1e-6));
    CHECK(results.minf == Approx(0).margin(1e-6));
}

TEST_CASE("Test Newton direction solve", "[opt][newtons_method][newton_dir]")
{
    int num_vars = 1000;
    Eigen::VectorXd x(num_vars);
    x.setRandom();
    // f = x^2
    Eigen::VectorXd gradient = 2 * x;
    Eigen::SparseMatrix<double> hessian =
        Eigen::SparseDiagonal<double>(2 * Eigen::VectorXd::Ones(num_vars));
    Eigen::VectorXd delta_x;
    ccd::opt::NewtonSolver solver;
    solver.compute_direction(gradient, hessian, delta_x);
    CHECK((x + delta_x).squaredNorm() == Approx(0.0));
}

TEST_CASE("Test making a matrix SPD", "[opt][make_spd]")
{
    Eigen::SparseMatrix<double> A =
        Eigen::MatrixXd::Random(100, 100).sparseView();
    double mu = ccd::opt::make_matrix_positive_definite(A);
    CAPTURE(mu);
    auto eig_vals = Eigen::MatrixXd(A).eigenvalues();
    for (int i = 0; i < eig_vals.size(); i++) {
        CHECK(eig_vals(i).real() >= Approx(0.0).margin(1e-12));
    }
}
