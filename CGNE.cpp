#include "CGNE.hpp"

ConjugateGradientNE::ConjugateGradientNE(const MatrixXd &H, const VectorXd &g) : H(H), g(g)
{
}
std::pair<VectorXd, int> ConjugateGradientNE::solve()
{
  int i;
  Eigen::VectorXd f = Eigen::VectorXd::Zero(H.cols());
  Eigen::VectorXd r = g - (H * f);
  Eigen::VectorXd p = H.transpose() * r;
  Eigen::VectorXd out = f;
  double best_error = std::numeric_limits<double>::max();
  double r_old_norm = r.norm();

  for (i = 0; i < f.size(); i++)
  {
    double alpha_num = r.transpose() * r;
    double alpha_den = p.transpose() * p;
    double alpha = alpha_num / alpha_den;
    f = f + (alpha * p);
    r = r - (alpha * (H * p));
    double error = std::abs(r.norm() - r_old_norm);
    if ((error < best_error))
    {
      best_error = error;
      out = f;
    }
    if (error < Tolerance)
      break;
    double beta_num = r.transpose() * r;
    const double &beta_den = alpha_num;
    double beta = beta_num / beta_den;
    p = (H.transpose() * r) + (beta * p);
    r_old_norm = r.norm();
  }

  return {out, i + 1};
}