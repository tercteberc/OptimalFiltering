#ifndef DISCRETE_FOS_H
#define DISCRETE_FOS_H

#include "src/core/discrete_filter.h"


namespace Filters
{

namespace Discrete
{


//! \brief Класс, реализующий дискретный фильтр оптимальной структуры.

class FOS : public Core::DiscreteFilter
{

public:
    //! \brief Конструктор.
    FOS(Core::PtrFilterParameters params, Core::PtrTask task);


protected:
    /*!
     \brief Выполняет алгоритм.

     Вектор \f$Z_t\f$ оценки вычисляется следующим образом:
     \f[\Lambda_k = \tau_{k-1}(U_{k-1}, T_{k-1}),\f]
     \f[\Psi_k = \Theta_{k-1}(U_{k-1}, T_{k-1}),\f]
     \f[Z_k = \Lambda_k +
        \Psi_k \cdot G_k^T(\Lambda_k, \Psi_k) \cdot F_k^{-1}(\Lambda_k, \Psi_k) \cdot [Y_k - h_k(\Lambda_k, \Psi_k)].\f]
     \f[U_k = \Gamma_k^y \cdot Y_k + \Gamma_k^z \cdot Z_k + \chi_k.\f]

     Параметры \f$\Gamma_k^y,\ \Gamma_k^z,\ T_k,\ \chi_k\f$ вычисляются
     в computeParams(size_t, Array<Math::Vector> &, Math::Matrix &).

     \see Core::DiscreteFilter
     \see Core::DiscreteTask
    */
    void algorithm() override;


private:
    /*!
      \brief Вычисляет параметры и заполняет массив \f$U_k\f$.
      \param[in] k - индекс, соответствующий времени \f$t = k \cdot \delta t\f$.
      \param[out] u - масcив под выборку \f$U_k\f$.
      \param[out] T - матрица \f$T_k\f$.

     \f[D_k^\delta = cov([Y_k^T\ \ Z_k^T]^T),\f]
     \f[[\Gamma_k^y\ \ \Gamma_k^z] = [D_{kk}^{xy}\ \ D_{kk}^{xz}] \cdot (D_k^\delta)^{-1},\f]
     \f[\chi_k = m_k^x - \Gamma_k^y \cdot m_k^y - \Gamma_k^z \cdot m_k^z,\f]
     \f[T_k = D_k^x - \Gamma_k^y \cdot (D_{kk}^{xy})^T - \Gamma_k^z \cdot (D_{kk}^{xz})^T.\f]
     */
    void computeParams(size_t k, Array<Math::Vector> &u, Math::Matrix &T);
};


} // end Discrete

} // end Filters


#endif // DISCRETE_FOS_H
