#include "continuous_task.h"


namespace Core
{

using Math::LinAlg::PinvSVD;


ContinuousTask::ContinuousTask()
    : Task()
{
}

ContinuousTask::~ContinuousTask()
{
}

Matrix ContinuousTask::K(const Vector &m, const Matrix &D) const
{
    assert(m.size() == m_dimX);
    assert(D.rows() == m_dimX);
    assert(D.cols() == m_dimX);
    return (D * G(m, D).transpose() + S(m, D)) * PinvSVD(R(m, D));
}

Matrix ContinuousTask::Psy(const Vector &m, const Matrix &D) const
{
    assert(m.size() == m_dimX);
    assert(D.rows() == m_dimX);
    assert(D.cols() == m_dimX);
    Matrix k = K(m, D);
    Matrix a = A(m, D);
    return a * D + D * a.transpose() + Q(m, D) - k * R(m, D) * k.transpose();
}


} // end Core
