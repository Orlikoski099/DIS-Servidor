#include "CGNR.hpp"

ConjugateGradienteNR::ConjugateGradienteNR(const MatrixXd &mat, const VectorXd &g) : H(mat), g(g)
{
}

ConjugateGradienteNR::~ConjugateGradienteNR()
{
}

std::pair<Eigen::VectorXd, unsigned int> ConjugateGradienteNR::solve()
{
  unsigned int i;
  Eigen::VectorXd f = Eigen::VectorXd::Zero(H.cols());
  Eigen::VectorXd r = g - H * f;
  Eigen::VectorXd z = H.transpose() * r;
  Eigen::VectorXd p = z;

  Eigen::VectorXd out = f;
  double best_error = std::numeric_limits<double>::max();
  double r_old_norm = r.norm();

  for (i = 0; i < 250; i++)
  {
    auto w = H * p;
    double z_norm = std::pow(z.norm(), 2);
    double alpha = z_norm / std::pow(w.norm(), 2);
    f = f + alpha * p;
    r = r - alpha * w;
    double error = std::abs(r.norm() - r_old_norm);
    if (error < best_error)
    {
      best_error = error;
      out = f;
    }
    if (error < Tolerance)
      break;
    z = H.transpose() * r;
    double beta = std::pow(z.norm(), 2) / z_norm;
    p = z + beta * p;
    r_old_norm = r.norm();
  }

  return {out, i + 1};
}