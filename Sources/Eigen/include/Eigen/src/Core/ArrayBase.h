// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2009 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef EIGEN_ARRAYBASE_H
#define EIGEN_ARRAYBASE_H

// IWYU pragma: private
#include "./InternalHeaderCheck.h"

namespace Eigen { 

template<typename ExpressionType> class MatrixWrapper;

/** \class ArrayBase
  * \ingroup Core_Module
  *
  * \brief Base class for all 1D and 2D array, and related expressions
  *
  * An array is similar to a dense vector or matrix. While matrices are mathematical
  * objects with well defined linear algebra operators, an array is just a collection
  * of scalar values arranged in a one or two dimensional fashion. As the main consequence,
  * all operations applied to an array are performed coefficient wise. Furthermore,
  * arrays support scalar math functions of the c++ standard library (e.g., std::sin(x)), and convenient
  * constructors allowing to easily write generic code working for both scalar values
  * and arrays.
  *
  * This class is the base that is inherited by all array expression types.
  *
  * \tparam Derived is the derived type, e.g., an array or an expression type.
  *
  * This class can be extended with the help of the plugin mechanism described on the page
  * \ref TopicCustomizing_Plugins by defining the preprocessor symbol \c EIGEN_ARRAYBASE_PLUGIN.
  *
  * \sa class MatrixBase, \ref TopicClassHierarchy
  */
template<typename Derived> class ArrayBase
  : public DenseBase<Derived>
{
  public:
#ifndef EIGEN_PARSED_BY_DOXYGEN
    /** The base class for a given storage type. */
    typedef ArrayBase StorageBaseType;

    typedef ArrayBase Eigen_BaseClassForSpecializationOfGlobalMathFuncImpl;

    typedef typename internal::traits<Derived>::StorageKind StorageKind;
    typedef typename internal::traits<Derived>::Scalar Scalar;
    typedef typename internal::packet_traits<Scalar>::type PacketScalar;
    typedef typename NumTraits<Scalar>::Real RealScalar;

    typedef DenseBase<Derived> Base;
    using Base::RowsAtCompileTime;
    using Base::ColsAtCompileTime;
    using Base::SizeAtCompileTime;
    using Base::MaxRowsAtCompileTime;
    using Base::MaxColsAtCompileTime;
    using Base::MaxSizeAtCompileTime;
    using Base::IsVectorAtCompileTime;
    using Base::Flags;
    
    using Base::derived;
    using Base::const_cast_derived;
    using Base::rows;
    using Base::cols;
    using Base::size;
    using Base::coeff;
    using Base::coeffRef;
    using Base::lazyAssign;
    using Base::operator-;
    using Base::operator=;
    using Base::operator+=;
    using Base::operator-=;
    using Base::operator*=;
    using Base::operator/=;

    typedef typename Base::CoeffReturnType CoeffReturnType;

#endif // not EIGEN_PARSED_BY_DOXYGEN

#ifndef EIGEN_PARSED_BY_DOXYGEN
    typedef typename Base::PlainObject PlainObject;

    /** \internal Represents a matrix with all coefficients equal to one another*/
    typedef CwiseNullaryOp<internal::scalar_constant_op<Scalar>,PlainObject> ConstantReturnType;
#endif // not EIGEN_PARSED_BY_DOXYGEN

#define EIGEN_CURRENT_STORAGE_BASE_CLASS Eigen::ArrayBase
#define EIGEN_DOC_UNARY_ADDONS(X,Y)

// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008-2009 Gael Guennebaud <gael.guennebaud@inria.fr>
// Copyright (C) 2006-2008 Benoit Jacob <jacob.benoit.1@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

// This file is included into the body of the base classes supporting matrix specific coefficient-wise functions.
// This include MatrixBase and SparseMatrixBase.


typedef CwiseUnaryOp<internal::scalar_abs_op<Scalar>, const Derived> CwiseAbsReturnType;
typedef CwiseUnaryOp<internal::scalar_abs2_op<Scalar>, const Derived> CwiseAbs2ReturnType;
typedef CwiseUnaryOp<internal::scalar_arg_op<Scalar>, const Derived> CwiseArgReturnType;
typedef CwiseUnaryOp<internal::scalar_carg_op<Scalar>, const Derived> CwiseCArgReturnType;
typedef CwiseUnaryOp<internal::scalar_sqrt_op<Scalar>, const Derived> CwiseSqrtReturnType;
typedef CwiseUnaryOp<internal::scalar_sign_op<Scalar>, const Derived> CwiseSignReturnType;
typedef CwiseUnaryOp<internal::scalar_inverse_op<Scalar>, const Derived> CwiseInverseReturnType;

/// \returns an expression of the coefficient-wise absolute value of \c *this
///
/// Example: \include MatrixBase_cwiseAbs.cpp
/// Output: \verbinclude MatrixBase_cwiseAbs.out
///
EIGEN_DOC_UNARY_ADDONS(cwiseAbs,absolute value)
///
/// \sa cwiseAbs2()
///
EIGEN_DEVICE_FUNC
EIGEN_STRONG_INLINE const CwiseAbsReturnType
cwiseAbs() const { return CwiseAbsReturnType(derived()); }

/// \returns an expression of the coefficient-wise squared absolute value of \c *this
///
/// Example: \include MatrixBase_cwiseAbs2.cpp
/// Output: \verbinclude MatrixBase_cwiseAbs2.out
///
EIGEN_DOC_UNARY_ADDONS(cwiseAbs2,squared absolute value)
///
/// \sa cwiseAbs()
///
EIGEN_DEVICE_FUNC
EIGEN_STRONG_INLINE const CwiseAbs2ReturnType
cwiseAbs2() const { return CwiseAbs2ReturnType(derived()); }

/// \returns an expression of the coefficient-wise square root of *this.
///
/// Example: \include MatrixBase_cwiseSqrt.cpp
/// Output: \verbinclude MatrixBase_cwiseSqrt.out
///
EIGEN_DOC_UNARY_ADDONS(cwiseSqrt,square-root)
///
/// \sa cwisePow(), cwiseSquare()
///
EIGEN_DEVICE_FUNC
inline const CwiseSqrtReturnType
cwiseSqrt() const { return CwiseSqrtReturnType(derived()); }

/// \returns an expression of the coefficient-wise signum of *this.
///
/// Example: \include MatrixBase_cwiseSign.cpp
/// Output: \verbinclude MatrixBase_cwiseSign.out
///
EIGEN_DOC_UNARY_ADDONS(cwiseSign,sign function)
///
EIGEN_DEVICE_FUNC
inline const CwiseSignReturnType
cwiseSign() const { return CwiseSignReturnType(derived()); }


/// \returns an expression of the coefficient-wise inverse of *this.
///
/// Example: \include MatrixBase_cwiseInverse.cpp
/// Output: \verbinclude MatrixBase_cwiseInverse.out
///
EIGEN_DOC_UNARY_ADDONS(cwiseInverse,inverse)
///
/// \sa cwiseProduct()
///
EIGEN_DEVICE_FUNC
inline const CwiseInverseReturnType
cwiseInverse() const { return CwiseInverseReturnType(derived()); }

/// \returns an expression of the coefficient-wise phase angle of \c *this
///
/// Example: \include MatrixBase_cwiseArg.cpp
/// Output: \verbinclude MatrixBase_cwiseArg.out
///
EIGEN_DOC_UNARY_ADDONS(cwiseArg,arg)

EIGEN_DEVICE_FUNC
inline const CwiseArgReturnType
cwiseArg() const { return CwiseArgReturnType(derived()); }

EIGEN_DEVICE_FUNC
EIGEN_STRONG_INLINE const CwiseCArgReturnType
cwiseCArg() const { return CwiseCArgReturnType(derived()); }

template <typename ScalarExponent>
using CwisePowReturnType =
    std::enable_if_t<internal::is_arithmetic<typename NumTraits<ScalarExponent>::Real>::value,
                     CwiseUnaryOp<internal::scalar_unary_pow_op<Scalar, ScalarExponent>, const Derived>>;

template <typename ScalarExponent>
EIGEN_DEVICE_FUNC inline const CwisePowReturnType<ScalarExponent> cwisePow(const ScalarExponent& exponent) const {
  return CwisePowReturnType<ScalarExponent>(derived(), internal::scalar_unary_pow_op<Scalar, ScalarExponent>(exponent));
}




typedef CwiseUnaryOp<internal::scalar_abs_op<Scalar>, const Derived> AbsReturnType;
typedef CwiseUnaryOp<internal::scalar_arg_op<Scalar>, const Derived> ArgReturnType;
typedef CwiseUnaryOp<internal::scalar_carg_op<Scalar>, const Derived> CArgReturnType;
typedef CwiseUnaryOp<internal::scalar_abs2_op<Scalar>, const Derived> Abs2ReturnType;
typedef CwiseUnaryOp<internal::scalar_sqrt_op<Scalar>, const Derived> SqrtReturnType;
typedef CwiseUnaryOp<internal::scalar_rsqrt_op<Scalar>, const Derived> RsqrtReturnType;
typedef CwiseUnaryOp<internal::scalar_sign_op<Scalar>, const Derived> SignReturnType;
typedef CwiseUnaryOp<internal::scalar_inverse_op<Scalar>, const Derived> InverseReturnType;
typedef CwiseUnaryOp<internal::scalar_boolean_not_op<Scalar>, const Derived> BooleanNotReturnType;
typedef CwiseUnaryOp<internal::scalar_bitwise_not_op<Scalar>, const Derived> BitwiseNotReturnType;

typedef CwiseUnaryOp<internal::scalar_exp_op<Scalar>, const Derived> ExpReturnType;
typedef CwiseUnaryOp<internal::scalar_expm1_op<Scalar>, const Derived> Expm1ReturnType;
typedef CwiseUnaryOp<internal::scalar_log_op<Scalar>, const Derived> LogReturnType;
typedef CwiseUnaryOp<internal::scalar_log1p_op<Scalar>, const Derived> Log1pReturnType;
typedef CwiseUnaryOp<internal::scalar_log10_op<Scalar>, const Derived> Log10ReturnType;
typedef CwiseUnaryOp<internal::scalar_log2_op<Scalar>, const Derived> Log2ReturnType;
typedef CwiseUnaryOp<internal::scalar_cos_op<Scalar>, const Derived> CosReturnType;
typedef CwiseUnaryOp<internal::scalar_sin_op<Scalar>, const Derived> SinReturnType;
typedef CwiseUnaryOp<internal::scalar_tan_op<Scalar>, const Derived> TanReturnType;
typedef CwiseUnaryOp<internal::scalar_acos_op<Scalar>, const Derived> AcosReturnType;
typedef CwiseUnaryOp<internal::scalar_asin_op<Scalar>, const Derived> AsinReturnType;
typedef CwiseUnaryOp<internal::scalar_atan_op<Scalar>, const Derived> AtanReturnType;
typedef CwiseUnaryOp<internal::scalar_tanh_op<Scalar>, const Derived> TanhReturnType;
typedef CwiseUnaryOp<internal::scalar_logistic_op<Scalar>, const Derived> LogisticReturnType;
typedef CwiseUnaryOp<internal::scalar_sinh_op<Scalar>, const Derived> SinhReturnType;
typedef CwiseUnaryOp<internal::scalar_atanh_op<Scalar>, const Derived> AtanhReturnType;
typedef CwiseUnaryOp<internal::scalar_asinh_op<Scalar>, const Derived> AsinhReturnType;
typedef CwiseUnaryOp<internal::scalar_acosh_op<Scalar>, const Derived> AcoshReturnType;
typedef CwiseUnaryOp<internal::scalar_cosh_op<Scalar>, const Derived> CoshReturnType;
typedef CwiseUnaryOp<internal::scalar_square_op<Scalar>, const Derived> SquareReturnType;
typedef CwiseUnaryOp<internal::scalar_cube_op<Scalar>, const Derived> CubeReturnType;
typedef CwiseUnaryOp<internal::scalar_round_op<Scalar>, const Derived> RoundReturnType;
typedef CwiseUnaryOp<internal::scalar_rint_op<Scalar>, const Derived> RintReturnType;
typedef CwiseUnaryOp<internal::scalar_floor_op<Scalar>, const Derived> FloorReturnType;
typedef CwiseUnaryOp<internal::scalar_ceil_op<Scalar>, const Derived> CeilReturnType;
typedef CwiseUnaryOp<internal::scalar_isnan_op<Scalar>, const Derived> IsNaNReturnType;
typedef CwiseUnaryOp<internal::scalar_isinf_op<Scalar>, const Derived> IsInfReturnType;
typedef CwiseUnaryOp<internal::scalar_isfinite_op<Scalar>, const Derived> IsFiniteReturnType;

/** \returns an expression of the coefficient-wise absolute value of \c *this
  *
  * Example: \include Cwise_abs.cpp
  * Output: \verbinclude Cwise_abs.out
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_abs">Math functions</a>, abs2()
  */
EIGEN_DEVICE_FUNC
EIGEN_STRONG_INLINE const AbsReturnType
abs() const
{
  return AbsReturnType(derived());
}

/** \returns an expression of the coefficient-wise phase angle of \c *this
  *
  * Example: \include Cwise_arg.cpp
  * Output: \verbinclude Cwise_arg.out
  *
  * \sa abs()
  */
EIGEN_DEVICE_FUNC
EIGEN_STRONG_INLINE const ArgReturnType
arg() const
{
  return ArgReturnType(derived());
}

EIGEN_DEVICE_FUNC
EIGEN_STRONG_INLINE const CArgReturnType
carg() const { return CArgReturnType(derived()); }

/** \returns an expression of the coefficient-wise squared absolute value of \c *this
  *
  * Example: \include Cwise_abs2.cpp
  * Output: \verbinclude Cwise_abs2.out
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_abs2">Math functions</a>, abs(), square()
  */
EIGEN_DEVICE_FUNC
EIGEN_STRONG_INLINE const Abs2ReturnType
abs2() const
{
  return Abs2ReturnType(derived());
}

/** \returns an expression of the coefficient-wise exponential of *this.
  *
  * This function computes the coefficient-wise exponential. The function MatrixBase::exp() in the
  * unsupported module MatrixFunctions computes the matrix exponential.
  *
  * Example: \include Cwise_exp.cpp
  * Output: \verbinclude Cwise_exp.out
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_exp">Math functions</a>, pow(), log(), sin(), cos()
  */
EIGEN_DEVICE_FUNC
inline const ExpReturnType
exp() const
{
  return ExpReturnType(derived());
}

/** \returns an expression of the coefficient-wise exponential of *this minus 1.
  *
  * In exact arithmetic, \c x.expm1() is equivalent to \c x.exp() - 1,
  * however, with finite precision, this function is much more accurate when \c x is close to zero.
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_expm1">Math functions</a>, exp()
  */
EIGEN_DEVICE_FUNC
inline const Expm1ReturnType
expm1() const
{
  return Expm1ReturnType(derived());
}

/** \returns an expression of the coefficient-wise logarithm of *this.
  *
  * This function computes the coefficient-wise logarithm. The function MatrixBase::log() in the
  * unsupported module MatrixFunctions computes the matrix logarithm.
  *
  * Example: \include Cwise_log.cpp
  * Output: \verbinclude Cwise_log.out
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_log">Math functions</a>, log()
  */
EIGEN_DEVICE_FUNC
inline const LogReturnType
log() const
{
  return LogReturnType(derived());
}

/** \returns an expression of the coefficient-wise logarithm of 1 plus \c *this.
  *
  * In exact arithmetic, \c x.log() is equivalent to \c (x+1).log(),
  * however, with finite precision, this function is much more accurate when \c x is close to zero.
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_log1p">Math functions</a>, log()
  */
EIGEN_DEVICE_FUNC
inline const Log1pReturnType
log1p() const
{
  return Log1pReturnType(derived());
}

/** \returns an expression of the coefficient-wise base-10 logarithm of *this.
  *
  * This function computes the coefficient-wise base-10 logarithm.
  *
  * Example: \include Cwise_log10.cpp
  * Output: \verbinclude Cwise_log10.out
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_log10">Math functions</a>, log()
  */
EIGEN_DEVICE_FUNC
inline const Log10ReturnType
log10() const
{
  return Log10ReturnType(derived());
}

/** \returns an expression of the coefficient-wise base-2 logarithm of *this.
  *
  * This function computes the coefficient-wise base-2 logarithm.
  *
  */
EIGEN_DEVICE_FUNC
inline const Log2ReturnType
log2() const
{
  return Log2ReturnType(derived());
}

/** \returns an expression of the coefficient-wise square root of *this.
  *
  * This function computes the coefficient-wise square root. The function MatrixBase::sqrt() in the
  * unsupported module MatrixFunctions computes the matrix square root.
  *
  * Example: \include Cwise_sqrt.cpp
  * Output: \verbinclude Cwise_sqrt.out
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_sqrt">Math functions</a>, pow(), square()
  */
EIGEN_DEVICE_FUNC
inline const SqrtReturnType
sqrt() const
{
  return SqrtReturnType(derived());
}

/** \returns an expression of the coefficient-wise inverse square root of *this.
  *
  * This function computes the coefficient-wise inverse square root.
  *
  * Example: \include Cwise_sqrt.cpp
  * Output: \verbinclude Cwise_sqrt.out
  *
  * \sa pow(), square()
  */
EIGEN_DEVICE_FUNC
inline const RsqrtReturnType
rsqrt() const
{
  return RsqrtReturnType(derived());
}

/** \returns an expression of the coefficient-wise signum of *this.
  *
  * This function computes the coefficient-wise signum.
  *
  * Example: \include Cwise_sign.cpp
  * Output: \verbinclude Cwise_sign.out
  *
  * \sa pow(), square()
  */
EIGEN_DEVICE_FUNC
inline const SignReturnType
sign() const
{
  return SignReturnType(derived());
}


/** \returns an expression of the coefficient-wise cosine of *this.
  *
  * This function computes the coefficient-wise cosine. The function MatrixBase::cos() in the
  * unsupported module MatrixFunctions computes the matrix cosine.
  *
  * Example: \include Cwise_cos.cpp
  * Output: \verbinclude Cwise_cos.out
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_cos">Math functions</a>, sin(), acos()
  */
EIGEN_DEVICE_FUNC
inline const CosReturnType
cos() const
{
  return CosReturnType(derived());
}


/** \returns an expression of the coefficient-wise sine of *this.
  *
  * This function computes the coefficient-wise sine. The function MatrixBase::sin() in the
  * unsupported module MatrixFunctions computes the matrix sine.
  *
  * Example: \include Cwise_sin.cpp
  * Output: \verbinclude Cwise_sin.out
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_sin">Math functions</a>, cos(), asin()
  */
EIGEN_DEVICE_FUNC
inline const SinReturnType
sin() const
{
  return SinReturnType(derived());
}

/** \returns an expression of the coefficient-wise tan of *this.
  *
  * Example: \include Cwise_tan.cpp
  * Output: \verbinclude Cwise_tan.out
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_tan">Math functions</a>, cos(), sin()
  */
EIGEN_DEVICE_FUNC
inline const TanReturnType
tan() const
{
  return TanReturnType(derived());
}

/** \returns an expression of the coefficient-wise arc tan of *this.
  *
  * Example: \include Cwise_atan.cpp
  * Output: \verbinclude Cwise_atan.out
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_atan">Math functions</a>, tan(), asin(), acos()
  */
EIGEN_DEVICE_FUNC
inline const AtanReturnType
atan() const
{
  return AtanReturnType(derived());
}

/** \returns an expression of the coefficient-wise arc cosine of *this.
  *
  * Example: \include Cwise_acos.cpp
  * Output: \verbinclude Cwise_acos.out
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_acos">Math functions</a>, cos(), asin()
  */
EIGEN_DEVICE_FUNC
inline const AcosReturnType
acos() const
{
  return AcosReturnType(derived());
}

/** \returns an expression of the coefficient-wise arc sine of *this.
  *
  * Example: \include Cwise_asin.cpp
  * Output: \verbinclude Cwise_asin.out
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_asin">Math functions</a>, sin(), acos()
  */
EIGEN_DEVICE_FUNC
inline const AsinReturnType
asin() const
{
  return AsinReturnType(derived());
}

/** \returns an expression of the coefficient-wise hyperbolic tan of *this.
  *
  * Example: \include Cwise_tanh.cpp
  * Output: \verbinclude Cwise_tanh.out
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_tanh">Math functions</a>, tan(), sinh(), cosh()
  */
EIGEN_DEVICE_FUNC
inline const TanhReturnType
tanh() const
{
  return TanhReturnType(derived());
}

/** \returns an expression of the coefficient-wise hyperbolic sin of *this.
  *
  * Example: \include Cwise_sinh.cpp
  * Output: \verbinclude Cwise_sinh.out
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_sinh">Math functions</a>, sin(), tanh(), cosh()
  */
EIGEN_DEVICE_FUNC
inline const SinhReturnType
sinh() const
{
  return SinhReturnType(derived());
}

/** \returns an expression of the coefficient-wise hyperbolic cos of *this.
  *
  * Example: \include Cwise_cosh.cpp
  * Output: \verbinclude Cwise_cosh.out
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_cosh">Math functions</a>, tanh(), sinh(), cosh()
  */
EIGEN_DEVICE_FUNC
inline const CoshReturnType
cosh() const
{
  return CoshReturnType(derived());
}

/** \returns an expression of the coefficient-wise inverse hyperbolic tan of *this.
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_atanh">Math functions</a>, atanh(), asinh(), acosh()
  */
EIGEN_DEVICE_FUNC
inline const AtanhReturnType
atanh() const
{
  return AtanhReturnType(derived());
}

/** \returns an expression of the coefficient-wise inverse hyperbolic sin of *this.
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_asinh">Math functions</a>, atanh(), asinh(), acosh()
  */
EIGEN_DEVICE_FUNC
inline const AsinhReturnType
asinh() const
{
  return AsinhReturnType(derived());
}

/** \returns an expression of the coefficient-wise inverse hyperbolic cos of *this.
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_acosh">Math functions</a>, atanh(), asinh(), acosh()
  */
EIGEN_DEVICE_FUNC
inline const AcoshReturnType
acosh() const
{
  return AcoshReturnType(derived());
}

/** \returns an expression of the coefficient-wise logistic of *this.
  */
EIGEN_DEVICE_FUNC
inline const LogisticReturnType
logistic() const
{
  return LogisticReturnType(derived());
}

/** \returns an expression of the coefficient-wise inverse of *this.
  *
  * Example: \include Cwise_inverse.cpp
  * Output: \verbinclude Cwise_inverse.out
  *
  * \sa operator/(), operator*()
  */
EIGEN_DEVICE_FUNC
inline const InverseReturnType
inverse() const
{
  return InverseReturnType(derived());
}

/** \returns an expression of the coefficient-wise square of *this.
  *
  * Example: \include Cwise_square.cpp
  * Output: \verbinclude Cwise_square.out
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_squareE">Math functions</a>, abs2(), cube(), pow()
  */
EIGEN_DEVICE_FUNC
inline const SquareReturnType
square() const
{
  return SquareReturnType(derived());
}

/** \returns an expression of the coefficient-wise cube of *this.
  *
  * Example: \include Cwise_cube.cpp
  * Output: \verbinclude Cwise_cube.out
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_cube">Math functions</a>, square(), pow()
  */
EIGEN_DEVICE_FUNC
inline const CubeReturnType
cube() const
{
  return CubeReturnType(derived());
}

/** \returns an expression of the coefficient-wise rint of *this.
  *
  * Example: \include Cwise_rint.cpp
  * Output: \verbinclude Cwise_rint.out
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_rint">Math functions</a>, ceil(), floor()
  */
EIGEN_DEVICE_FUNC
inline const RintReturnType
rint() const
{
  return RintReturnType(derived());
}

/** \returns an expression of the coefficient-wise round of *this.
  *
  * Example: \include Cwise_round.cpp
  * Output: \verbinclude Cwise_round.out
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_round">Math functions</a>, ceil(), floor()
  */
EIGEN_DEVICE_FUNC
inline const RoundReturnType
round() const
{
  return RoundReturnType(derived());
}

/** \returns an expression of the coefficient-wise floor of *this.
  *
  * Example: \include Cwise_floor.cpp
  * Output: \verbinclude Cwise_floor.out
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_floor">Math functions</a>, ceil(), round()
  */
EIGEN_DEVICE_FUNC
inline const FloorReturnType
floor() const
{
  return FloorReturnType(derived());
}

/** \returns an expression of the coefficient-wise ceil of *this.
  *
  * Example: \include Cwise_ceil.cpp
  * Output: \verbinclude Cwise_ceil.out
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_ceil">Math functions</a>, floor(), round()
  */
EIGEN_DEVICE_FUNC
inline const CeilReturnType
ceil() const
{
  return CeilReturnType(derived());
}

template<int N> struct ShiftRightXpr {
  typedef CwiseUnaryOp<internal::scalar_shift_right_op<Scalar, N>, const Derived> Type;
};

/** \returns an expression of \c *this with the \a Scalar type arithmetically
  * shifted right by \a N bit positions.
  *
  * The template parameter \a N specifies the number of bit positions to shift.
  * 
  * \sa shiftLeft()
  */
template<int N>
EIGEN_DEVICE_FUNC
typename ShiftRightXpr<N>::Type
shiftRight() const
{
  return typename ShiftRightXpr<N>::Type(derived());
}


template<int N> struct ShiftLeftXpr {
  typedef CwiseUnaryOp<internal::scalar_shift_left_op<Scalar, N>, const Derived> Type;
};

/** \returns an expression of \c *this with the \a Scalar type logically
  * shifted left by \a N bit positions.
  *
  * The template parameter \a N specifies the number of bit positions to shift.
  *
  * \sa shiftRight()
  */
template<int N>
EIGEN_DEVICE_FUNC
typename ShiftLeftXpr<N>::Type
shiftLeft() const
{
  return typename ShiftLeftXpr<N>::Type(derived());
}

/** \returns an expression of the coefficient-wise isnan of *this.
  *
  * Example: \include Cwise_isNaN.cpp
  * Output: \verbinclude Cwise_isNaN.out
  *
  * \sa isfinite(), isinf()
  */
EIGEN_DEVICE_FUNC
inline const IsNaNReturnType
isNaN() const
{
  return IsNaNReturnType(derived());
}

/** \returns an expression of the coefficient-wise isinf of *this.
  *
  * Example: \include Cwise_isInf.cpp
  * Output: \verbinclude Cwise_isInf.out
  *
  * \sa isnan(), isfinite()
  */
EIGEN_DEVICE_FUNC
inline const IsInfReturnType
isInf() const
{
  return IsInfReturnType(derived());
}

/** \returns an expression of the coefficient-wise isfinite of *this.
  *
  * Example: \include Cwise_isFinite.cpp
  * Output: \verbinclude Cwise_isFinite.out
  *
  * \sa isnan(), isinf()
  */
EIGEN_DEVICE_FUNC
inline const IsFiniteReturnType
isFinite() const
{
  return IsFiniteReturnType(derived());
}

/** \returns an expression of the coefficient-wise ! operator of *this
  *
  * Example: \include Cwise_boolean_not.cpp
  * Output: \verbinclude Cwise_boolean_not.out
  *
  * \sa operator!=()
  */
EIGEN_DEVICE_FUNC
inline const BooleanNotReturnType
operator!() const
{
  return BooleanNotReturnType(derived());
}

/** \returns an expression of the bitwise ~ operator of *this
  */
EIGEN_DEVICE_FUNC
inline const BitwiseNotReturnType
operator~() const
{
  return BitwiseNotReturnType(derived());
}


// --- SpecialFunctions module ---

typedef CwiseUnaryOp<internal::scalar_lgamma_op<Scalar>, const Derived> LgammaReturnType;
typedef CwiseUnaryOp<internal::scalar_digamma_op<Scalar>, const Derived> DigammaReturnType;
typedef CwiseUnaryOp<internal::scalar_erf_op<Scalar>, const Derived> ErfReturnType;
typedef CwiseUnaryOp<internal::scalar_erfc_op<Scalar>, const Derived> ErfcReturnType;
typedef CwiseUnaryOp<internal::scalar_ndtri_op<Scalar>, const Derived> NdtriReturnType;

/** \cpp11 \returns an expression of the coefficient-wise ln(|gamma(*this)|).
  *
  * \specialfunctions_module
  *
  * \note This function supports only float and double scalar types in c++11 mode. To support other scalar types,
  * or float/double in non c++11 mode, the user has to provide implementations of lgamma(T) for any scalar
  * type T to be supported.
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_lgamma">Math functions</a>, digamma()
  */
EIGEN_DEVICE_FUNC
inline const LgammaReturnType
lgamma() const
{
  return LgammaReturnType(derived());
}

/** \returns an expression of the coefficient-wise digamma (psi, derivative of lgamma).
  *
  * \specialfunctions_module
  *
  * \note This function supports only float and double scalar types. To support other scalar types,
  * the user has to provide implementations of digamma(T) for any scalar
  * type T to be supported.
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_digamma">Math functions</a>, Eigen::digamma(), Eigen::polygamma(), lgamma()
  */
EIGEN_DEVICE_FUNC
inline const DigammaReturnType
digamma() const
{
  return DigammaReturnType(derived());
}

/** \cpp11 \returns an expression of the coefficient-wise Gauss error
  * function of *this.
  *
  * \specialfunctions_module
  *
  * \note This function supports only float and double scalar types in c++11 mode. To support other scalar types,
  * or float/double in non c++11 mode, the user has to provide implementations of erf(T) for any scalar
  * type T to be supported.
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_erf">Math functions</a>, erfc()
  */
EIGEN_DEVICE_FUNC
inline const ErfReturnType
erf() const
{
  return ErfReturnType(derived());
}

/** \cpp11 \returns an expression of the coefficient-wise Complementary error
  * function of *this.
  *
  * \specialfunctions_module
  *
  * \note This function supports only float and double scalar types in c++11 mode. To support other scalar types,
  * or float/double in non c++11 mode, the user has to provide implementations of erfc(T) for any scalar
  * type T to be supported.
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_erfc">Math functions</a>, erf()
  */
EIGEN_DEVICE_FUNC
inline const ErfcReturnType
erfc() const
{
  return ErfcReturnType(derived());
}

/** \returns an expression of the coefficient-wise inverse of the CDF of the Normal distribution function
  * function of *this.
  *
  * \specialfunctions_module
  * 
  * In other words, considering `x = ndtri(y)`, it returns the argument, x, for which the area under the
  * Gaussian probability density function (integrated from minus infinity to x) is equal to y.
  *
  * \note This function supports only float and double scalar types. To support other scalar types,
  * the user has to provide implementations of ndtri(T) for any scalar type T to be supported.
  *
  * \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_ndtri">Math functions</a>
  */
EIGEN_DEVICE_FUNC
inline const NdtriReturnType
ndtri() const
{
  return NdtriReturnType(derived());
}

template <typename ScalarExponent>
using UnaryPowReturnType =
    std::enable_if_t<internal::is_arithmetic<typename NumTraits<ScalarExponent>::Real>::value,
                     CwiseUnaryOp<internal::scalar_unary_pow_op<Scalar, ScalarExponent>, const Derived>>;

#ifndef EIGEN_PARSED_BY_DOXYGEN
template <typename ScalarExponent>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE const UnaryPowReturnType<ScalarExponent> pow(
    const ScalarExponent& exponent) const {
  return UnaryPowReturnType<ScalarExponent>(derived(), internal::scalar_unary_pow_op<Scalar, ScalarExponent>(exponent));
#else
/** \returns an expression of the coefficients of \c *this rasied to the constant power \a exponent
 *
 * \tparam T is the scalar type of \a exponent. It must be compatible with the scalar type of the given expression.
 *
 * This function computes the coefficient-wise power. The function MatrixBase::pow() in the
 * unsupported module MatrixFunctions computes the matrix power.
 *
 * Example: \include Cwise_pow.cpp
 * Output: \verbinclude Cwise_pow.out
 *
 * \sa ArrayBase::pow(ArrayBase), square(), cube(), exp(), log()
 */
template <typename ScalarExponent>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE const UnaryPowReturnType<ScalarExponent> pow(
    const ScalarExponent& exponent) const;
#endif
}


// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008-2016 Gael Guennebaud <gael.guennebaud@inria.fr>
// Copyright (C) 2006-2008 Benoit Jacob <jacob.benoit.1@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

// This file is a base class plugin containing common coefficient wise functions.

/** \returns an expression of the difference of \c *this and \a other
  *
  * \note If you want to subtract a given scalar from all coefficients, see Cwise::operator-().
  *
  * \sa class CwiseBinaryOp, operator-=()
  */
EIGEN_MAKE_CWISE_BINARY_OP(operator-,difference)

/** \returns an expression of the sum of \c *this and \a other
  *
  * \note If you want to add a given scalar to all coefficients, see Cwise::operator+().
  *
  * \sa class CwiseBinaryOp, operator+=()
  */
EIGEN_MAKE_CWISE_BINARY_OP(operator+,sum)

/** \returns an expression of a custom coefficient-wise operator \a func of *this and \a other
  *
  * The template parameter \a CustomBinaryOp is the type of the functor
  * of the custom operator (see class CwiseBinaryOp for an example)
  *
  * Here is an example illustrating the use of custom functors:
  * \include class_CwiseBinaryOp.cpp
  * Output: \verbinclude class_CwiseBinaryOp.out
  *
  * \sa class CwiseBinaryOp, operator+(), operator-(), cwiseProduct()
  */
template<typename CustomBinaryOp, typename OtherDerived>
EIGEN_DEVICE_FUNC
EIGEN_STRONG_INLINE const CwiseBinaryOp<CustomBinaryOp, const Derived, const OtherDerived>
binaryExpr(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived> &other, const CustomBinaryOp& func = CustomBinaryOp()) const
{
  return CwiseBinaryOp<CustomBinaryOp, const Derived, const OtherDerived>(derived(), other.derived(), func);
}


#ifndef EIGEN_PARSED_BY_DOXYGEN
EIGEN_MAKE_SCALAR_BINARY_OP(operator*,product)
#else
/** \returns an expression of \c *this scaled by the scalar factor \a scalar
  *
  * \tparam T is the scalar type of \a scalar. It must be compatible with the scalar type of the given expression.
  */
template<typename T>
const CwiseBinaryOp<internal::scalar_product_op<Scalar,T>,Derived,Constant<T> > operator*(const T& scalar) const;
/** \returns an expression of \a expr scaled by the scalar factor \a scalar
  *
  * \tparam T is the scalar type of \a scalar. It must be compatible with the scalar type of the given expression.
  */
template<typename T> friend
const CwiseBinaryOp<internal::scalar_product_op<T,Scalar>,Constant<T>,Derived> operator*(const T& scalar, const StorageBaseType& expr);
#endif



#ifndef EIGEN_PARSED_BY_DOXYGEN
EIGEN_MAKE_SCALAR_BINARY_OP_ONTHERIGHT(operator/,quotient)
#else
/** \returns an expression of \c *this divided by the scalar value \a scalar
  *
  * \tparam T is the scalar type of \a scalar. It must be compatible with the scalar type of the given expression.
  */
template<typename T>
const CwiseBinaryOp<internal::scalar_quotient_op<Scalar,T>,Derived,Constant<T> > operator/(const T& scalar) const;
#endif

/** \returns an expression of the coefficient-wise boolean \b and operator of \c *this and \a other
 *
 * Example: \include Cwise_boolean_and.cpp
 * Output: \verbinclude Cwise_boolean_and.out
 *
 * \sa operator||(), select()
 */
template <typename OtherDerived>
EIGEN_DEVICE_FUNC inline const CwiseBinaryOp<internal::scalar_boolean_and_op<Scalar>, const Derived, const OtherDerived>
operator&&(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived>& other) const {
  return CwiseBinaryOp<internal::scalar_boolean_and_op<Scalar>, const Derived, const OtherDerived>(derived(),
                                                                                                   other.derived());
}

/** \returns an expression of the coefficient-wise boolean \b or operator of \c *this and \a other
 *
 * Example: \include Cwise_boolean_or.cpp
 * Output: \verbinclude Cwise_boolean_or.out
 *
 * \sa operator&&(), select()
 */
template <typename OtherDerived>
EIGEN_DEVICE_FUNC inline const CwiseBinaryOp<internal::scalar_boolean_or_op<Scalar>, const Derived, const OtherDerived>
operator||(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived>& other) const {
  return CwiseBinaryOp<internal::scalar_boolean_or_op<Scalar>, const Derived, const OtherDerived>(derived(),
                                                                                                  other.derived());
}

/** \returns an expression of the bitwise \b and operator of \c *this and \a other
 *
 * \sa operator|(), operator^()
 */
template <typename OtherDerived>
EIGEN_DEVICE_FUNC inline const CwiseBinaryOp<internal::scalar_bitwise_and_op<Scalar>, const Derived, const OtherDerived>
operator&(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived>& other) const {
  return CwiseBinaryOp<internal::scalar_bitwise_and_op<Scalar>, const Derived, const OtherDerived>(derived(),
                                                                                                   other.derived());
}

/** \returns an expression of the bitwise boolean \b or operator of \c *this and \a other
 *
 * \sa operator&(), operator^()
 */
template <typename OtherDerived>
EIGEN_DEVICE_FUNC inline const CwiseBinaryOp<internal::scalar_bitwise_or_op<Scalar>, const Derived, const OtherDerived>
operator|(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived>& other) const {
  return CwiseBinaryOp<internal::scalar_bitwise_or_op<Scalar>, const Derived, const OtherDerived>(derived(),
                                                                                                  other.derived());
}

/** \returns an expression of the bitwise xor operator of *this and \a other
 * \sa operator&(), operator|()
 */
template <typename OtherDerived>
EIGEN_DEVICE_FUNC inline const CwiseBinaryOp<internal::scalar_bitwise_xor_op<Scalar>, const Derived, const OtherDerived>
operator^(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived>& other) const {
  return CwiseBinaryOp<internal::scalar_bitwise_xor_op<Scalar>, const Derived, const OtherDerived>(derived(),
                                                                                                   other.derived());
}


// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008-2009 Gael Guennebaud <gael.guennebaud@inria.fr>
// Copyright (C) 2006-2008 Benoit Jacob <jacob.benoit.1@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

// This file is a base class plugin containing matrix specifics coefficient wise functions.

/** \returns an expression of the Schur product (coefficient wise product) of *this and \a other
  *
  * Example: \include MatrixBase_cwiseProduct.cpp
  * Output: \verbinclude MatrixBase_cwiseProduct.out
  *
  * \sa class CwiseBinaryOp, cwiseAbs2
  */
template<typename OtherDerived>
EIGEN_DEVICE_FUNC
EIGEN_STRONG_INLINE const EIGEN_CWISE_BINARY_RETURN_TYPE(Derived,OtherDerived,product)
cwiseProduct(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived> &other) const
{
  return EIGEN_CWISE_BINARY_RETURN_TYPE(Derived,OtherDerived,product)(derived(), other.derived());
}

template<typename OtherDerived> using CwiseBinaryEqualReturnType = CwiseBinaryOp<internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_EQ>, const Derived, const OtherDerived>;
template<typename OtherDerived> using CwiseBinaryNotEqualReturnType = CwiseBinaryOp<internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_NEQ>, const Derived, const OtherDerived>;
template<typename OtherDerived> using CwiseBinaryLessReturnType = CwiseBinaryOp<internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_LT>, const Derived, const OtherDerived>;
template<typename OtherDerived> using CwiseBinaryGreaterReturnType = CwiseBinaryOp<internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_GT>, const Derived, const OtherDerived>;
template<typename OtherDerived> using CwiseBinaryLessOrEqualReturnType = CwiseBinaryOp<internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_LE>, const Derived, const OtherDerived>;
template<typename OtherDerived> using CwiseBinaryGreaterOrEqualReturnType = CwiseBinaryOp<internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_GE>, const Derived, const OtherDerived>;

/** \returns an expression of the coefficient-wise == operator of *this and \a other
  *
  * \warning this performs an exact comparison, which is generally a bad idea with floating-point types.
  * In order to check for equality between two vectors or matrices with floating-point coefficients, it is
  * generally a far better idea to use a fuzzy comparison as provided by isApprox() and
  * isMuchSmallerThan().
  *
  * Example: \include MatrixBase_cwiseEqual.cpp
  * Output: \verbinclude MatrixBase_cwiseEqual.out
  *
  * \sa cwiseNotEqual(), isApprox(), isMuchSmallerThan()
  */
template<typename OtherDerived>
EIGEN_DEVICE_FUNC
inline const CwiseBinaryEqualReturnType<OtherDerived>
cwiseEqual(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived> &other) const
{
  return CwiseBinaryEqualReturnType<OtherDerived>(derived(), other.derived());
}

/** \returns an expression of the coefficient-wise != operator of *this and \a other
  *
  * \warning this performs an exact comparison, which is generally a bad idea with floating-point types.
  * In order to check for equality between two vectors or matrices with floating-point coefficients, it is
  * generally a far better idea to use a fuzzy comparison as provided by isApprox() and
  * isMuchSmallerThan().
  *
  * Example: \include MatrixBase_cwiseNotEqual.cpp
  * Output: \verbinclude MatrixBase_cwiseNotEqual.out
  *
  * \sa cwiseEqual(), isApprox(), isMuchSmallerThan()
  */
template<typename OtherDerived>
EIGEN_DEVICE_FUNC
inline const CwiseBinaryNotEqualReturnType<OtherDerived>
cwiseNotEqual(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived> &other) const
{
  return CwiseBinaryNotEqualReturnType<OtherDerived>(derived(), other.derived());
}

/** \returns an expression of the coefficient-wise < operator of *this and \a other */
template<typename OtherDerived>
EIGEN_DEVICE_FUNC
inline const CwiseBinaryLessReturnType<OtherDerived>
cwiseLess(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived>& other) const
{
  return CwiseBinaryLessReturnType<OtherDerived>(derived(), other.derived());
}

/** \returns an expression of the coefficient-wise > operator of *this and \a other */
template<typename OtherDerived>
EIGEN_DEVICE_FUNC
inline const CwiseBinaryGreaterReturnType<OtherDerived>
cwiseGreater(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived>& other) const
{
  return CwiseBinaryGreaterReturnType<OtherDerived>(derived(), other.derived());
}

/** \returns an expression of the coefficient-wise <= operator of *this and \a other */
template<typename OtherDerived>
EIGEN_DEVICE_FUNC
inline const CwiseBinaryLessOrEqualReturnType<OtherDerived>
cwiseLessOrEqual(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived>& other) const
{
  return CwiseBinaryLessOrEqualReturnType<OtherDerived>(derived(), other.derived());
}

/** \returns an expression of the coefficient-wise >= operator of *this and \a other */
template<typename OtherDerived>
EIGEN_DEVICE_FUNC
inline const CwiseBinaryGreaterOrEqualReturnType<OtherDerived>
cwiseGreaterOrEqual(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived>& other) const
{
  return CwiseBinaryGreaterOrEqualReturnType<OtherDerived>(derived(), other.derived());
}

/** \returns an expression of the coefficient-wise min of *this and \a other
  *
  * Example: \include MatrixBase_cwiseMin.cpp
  * Output: \verbinclude MatrixBase_cwiseMin.out
  *
  * \sa class CwiseBinaryOp, max()
  */
template<int NaNPropagation=PropagateFast, typename OtherDerived>
EIGEN_DEVICE_FUNC
EIGEN_STRONG_INLINE const CwiseBinaryOp<internal::scalar_min_op<Scalar,Scalar,NaNPropagation>, const Derived, const OtherDerived>
cwiseMin(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived> &other) const
{
  return CwiseBinaryOp<internal::scalar_min_op<Scalar,Scalar,NaNPropagation>, const Derived, const OtherDerived>(derived(), other.derived());
}

/** \returns an expression of the coefficient-wise min of *this and scalar \a other
  *
  * \sa class CwiseBinaryOp, min()
  */
template<int NaNPropagation=PropagateFast>
EIGEN_DEVICE_FUNC
EIGEN_STRONG_INLINE const CwiseBinaryOp<internal::scalar_min_op<Scalar,Scalar,NaNPropagation>, const Derived, const ConstantReturnType>
cwiseMin(const Scalar &other) const
{
  return cwiseMin<NaNPropagation>(Derived::Constant(rows(), cols(), other));
}

/** \returns an expression of the coefficient-wise max of *this and \a other
  *
  * Example: \include MatrixBase_cwiseMax.cpp
  * Output: \verbinclude MatrixBase_cwiseMax.out
  *
  * \sa class CwiseBinaryOp, min()
  */
template<int NaNPropagation=PropagateFast, typename OtherDerived>
EIGEN_DEVICE_FUNC
EIGEN_STRONG_INLINE const CwiseBinaryOp<internal::scalar_max_op<Scalar,Scalar,NaNPropagation>, const Derived, const OtherDerived>
cwiseMax(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived> &other) const
{
  return CwiseBinaryOp<internal::scalar_max_op<Scalar,Scalar,NaNPropagation>, const Derived, const OtherDerived>(derived(), other.derived());
}

/** \returns an expression of the coefficient-wise max of *this and scalar \a other
  *
  * \sa class CwiseBinaryOp, min()
  */
template<int NaNPropagation=PropagateFast>
EIGEN_DEVICE_FUNC
EIGEN_STRONG_INLINE const CwiseBinaryOp<internal::scalar_max_op<Scalar,Scalar,NaNPropagation>, const Derived, const ConstantReturnType>
cwiseMax(const Scalar &other) const
{
  return cwiseMax<NaNPropagation>(Derived::Constant(rows(), cols(), other));
}


/** \returns an expression of the coefficient-wise quotient of *this and \a other
  *
  * Example: \include MatrixBase_cwiseQuotient.cpp
  * Output: \verbinclude MatrixBase_cwiseQuotient.out
  *
  * \sa class CwiseBinaryOp, cwiseProduct(), cwiseInverse()
  */
template<typename OtherDerived>
EIGEN_DEVICE_FUNC
EIGEN_STRONG_INLINE const CwiseBinaryOp<internal::scalar_quotient_op<Scalar>, const Derived, const OtherDerived>
cwiseQuotient(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived> &other) const
{
  return CwiseBinaryOp<internal::scalar_quotient_op<Scalar>, const Derived, const OtherDerived>(derived(), other.derived());
}

using CwiseScalarEqualReturnType = CwiseBinaryOp<internal::scalar_cmp_op<Scalar,Scalar,internal::cmp_EQ>, const Derived, const ConstantReturnType>;
using CwiseScalarNotEqualReturnType = CwiseBinaryOp<internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_NEQ>, const Derived, const ConstantReturnType>;
using CwiseScalarLessReturnType = CwiseBinaryOp<internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_LT>, const Derived, const ConstantReturnType>;
using CwiseScalarGreaterReturnType = CwiseBinaryOp<internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_GT>, const Derived, const ConstantReturnType>;
using CwiseScalarLessOrEqualReturnType = CwiseBinaryOp<internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_LE>, const Derived, const ConstantReturnType>;
using CwiseScalarGreaterOrEqualReturnType = CwiseBinaryOp<internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_GE>, const Derived, const ConstantReturnType>;

/** \returns an expression of the coefficient-wise == operator of \c *this and a scalar \a s
  *
  * \warning this performs an exact comparison, which is generally a bad idea with floating-point types.
  * In order to check for equality between two vectors or matrices with floating-point coefficients, it is
  * generally a far better idea to use a fuzzy comparison as provided by isApprox() and
  * isMuchSmallerThan().
  *
  * \sa cwiseEqual(const MatrixBase<OtherDerived> &) const
  */
EIGEN_DEVICE_FUNC
inline const CwiseScalarEqualReturnType
cwiseEqual(const Scalar& s) const
{
  return CwiseScalarEqualReturnType(derived(), Derived::Constant(rows(), cols(), s));
}


/** \returns an expression of the coefficient-wise == operator of \c *this and a scalar \a s
  *
  * \warning this performs an exact comparison, which is generally a bad idea with floating-point types.
  * In order to check for equality between two vectors or matrices with floating-point coefficients, it is
  * generally a far better idea to use a fuzzy comparison as provided by isApprox() and
  * isMuchSmallerThan().
  *
  * \sa cwiseEqual(const MatrixBase<OtherDerived> &) const
  */
EIGEN_DEVICE_FUNC
inline const CwiseScalarNotEqualReturnType
cwiseNotEqual(const Scalar& s) const
{
  return CwiseScalarNotEqualReturnType(derived(), Derived::Constant(rows(), cols(), s));
}

/** \returns an expression of the coefficient-wise < operator of \c *this and a scalar \a s */
EIGEN_DEVICE_FUNC
inline const CwiseScalarLessReturnType
cwiseLess(const Scalar& s) const
{
  return CwiseScalarLessReturnType(derived(), Derived::Constant(rows(), cols(), s));
}

/** \returns an expression of the coefficient-wise > operator of \c *this and a scalar \a s */
EIGEN_DEVICE_FUNC
inline const CwiseScalarGreaterReturnType
cwiseGreater(const Scalar& s) const
{
  return CwiseScalarGreaterReturnType(derived(), Derived::Constant(rows(), cols(), s));
}

/** \returns an expression of the coefficient-wise <= operator of \c *this and a scalar \a s */
EIGEN_DEVICE_FUNC
inline const CwiseScalarLessOrEqualReturnType
cwiseLessOrEqual(const Scalar& s) const
{
  return CwiseScalarLessOrEqualReturnType(derived(), Derived::Constant(rows(), cols(), s));
}

/** \returns an expression of the coefficient-wise >= operator of \c *this and a scalar \a s */
EIGEN_DEVICE_FUNC
inline const CwiseScalarGreaterOrEqualReturnType
cwiseGreaterOrEqual(const Scalar& s) const
{
  return CwiseScalarGreaterOrEqualReturnType(derived(), Derived::Constant(rows(), cols(), s));
}

template<typename OtherDerived> using CwiseBinaryTypedEqualReturnType = CwiseBinaryOp<internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_EQ, true>, const Derived, const OtherDerived>;
template<typename OtherDerived> using CwiseBinaryTypedNotEqualReturnType = CwiseBinaryOp<internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_NEQ, true>, const Derived, const OtherDerived>;
template<typename OtherDerived> using CwiseBinaryTypedLessReturnType = CwiseBinaryOp<internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_LT, true>, const Derived, const OtherDerived>;
template<typename OtherDerived> using CwiseBinaryTypedGreaterReturnType = CwiseBinaryOp<internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_GT, true>, const Derived, const OtherDerived>;
template<typename OtherDerived> using CwiseBinaryTypedLessOrEqualReturnType = CwiseBinaryOp<internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_LE, true>, const Derived, const OtherDerived>;
template<typename OtherDerived> using CwiseBinaryTypedGreaterOrEqualReturnType = CwiseBinaryOp<internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_GE, true>, const Derived, const OtherDerived>;

template <typename OtherDerived>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE const CwiseBinaryTypedEqualReturnType<OtherDerived>
cwiseTypedEqual(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived>& other) const { return CwiseBinaryTypedEqualReturnType<OtherDerived>(derived(), other.derived()); }

template <typename OtherDerived>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE const CwiseBinaryTypedNotEqualReturnType<OtherDerived>
cwiseTypedNotEqual(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived>& other) const { return CwiseBinaryTypedNotEqualReturnType<OtherDerived>(derived(), other.derived()); }

template <typename OtherDerived>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE const CwiseBinaryTypedLessReturnType<OtherDerived>
cwiseTypedLess(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived>& other) const { return CwiseBinaryTypedLessReturnType<OtherDerived>(derived(), other.derived()); }

template <typename OtherDerived>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE const CwiseBinaryTypedGreaterReturnType<OtherDerived>
cwiseTypedGreater(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived>& other) const { return CwiseBinaryTypedGreaterReturnType<OtherDerived>(derived(), other.derived()); }

template <typename OtherDerived>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE const CwiseBinaryTypedLessOrEqualReturnType<OtherDerived>
cwiseTypedLessOrEqual(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived>& other) const { return CwiseBinaryTypedLessOrEqualReturnType<OtherDerived>(derived(), other.derived()); }

template <typename OtherDerived>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE const CwiseBinaryTypedGreaterOrEqualReturnType<OtherDerived>
cwiseTypedGreaterOrEqual(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived>& other) const { return CwiseBinaryTypedGreaterOrEqualReturnType<OtherDerived>(derived(), other.derived()); }

using CwiseScalarTypedEqualReturnType = CwiseBinaryOp<internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_EQ, true>, const Derived, const ConstantReturnType>;
using CwiseScalarTypedNotEqualReturnType = CwiseBinaryOp<internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_NEQ, true>, const Derived, const ConstantReturnType>;
using CwiseScalarTypedLessReturnType = CwiseBinaryOp<internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_LT, true>, const Derived, const ConstantReturnType>;
using CwiseScalarTypedGreaterReturnType = CwiseBinaryOp<internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_GT, true>, const Derived, const ConstantReturnType>;
using CwiseScalarTypedLessOrEqualReturnType = CwiseBinaryOp<internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_LE, true>, const Derived, const ConstantReturnType>;
using CwiseScalarTypedGreaterOrEqualReturnType = CwiseBinaryOp<internal::scalar_cmp_op<Scalar, Scalar, internal::cmp_GE, true>, const Derived, const ConstantReturnType>;

EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE const CwiseScalarTypedEqualReturnType
cwiseTypedEqual(const Scalar& s) const { return CwiseScalarTypedEqualReturnType(derived(), ConstantReturnType(rows(), cols(), s)); }

EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE const CwiseScalarTypedNotEqualReturnType
cwiseTypedNotEqual(const Scalar& s) const { return CwiseScalarTypedNotEqualReturnType(derived(), ConstantReturnType(rows(), cols(), s)); }

EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE const CwiseScalarTypedLessReturnType
cwiseTypedLess(const Scalar& s) const { return CwiseScalarTypedLessReturnType(derived(), ConstantReturnType(rows(), cols(), s)); }

EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE const CwiseScalarTypedGreaterReturnType
cwiseTypedGreater(const Scalar& s) const { return CwiseScalarTypedGreaterReturnType(derived(), ConstantReturnType(rows(), cols(), s)); }

EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE const CwiseScalarTypedLessOrEqualReturnType
cwiseTypedLessOrEqual(const Scalar& s) const { return CwiseScalarTypedLessOrEqualReturnType(derived(), ConstantReturnType(rows(), cols(), s)); }

EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE const CwiseScalarTypedGreaterOrEqualReturnType
cwiseTypedGreaterOrEqual(const Scalar& s) const { return CwiseScalarTypedGreaterOrEqualReturnType(derived(), ConstantReturnType(rows(), cols(), s)); }




/** \returns an expression of the coefficient wise product of \c *this and \a other
  *
  * \sa MatrixBase::cwiseProduct
  */
template<typename OtherDerived>
EIGEN_DEVICE_FUNC
EIGEN_STRONG_INLINE const EIGEN_CWISE_BINARY_RETURN_TYPE(Derived,OtherDerived,product)
operator*(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived> &other) const
{
  return EIGEN_CWISE_BINARY_RETURN_TYPE(Derived,OtherDerived,product)(derived(), other.derived());
}

/** \returns an expression of the coefficient wise quotient of \c *this and \a other
  *
  * \sa MatrixBase::cwiseQuotient
  */
template<typename OtherDerived>
EIGEN_DEVICE_FUNC
EIGEN_STRONG_INLINE const CwiseBinaryOp<internal::scalar_quotient_op<Scalar,typename OtherDerived::Scalar>, const Derived, const OtherDerived>
operator/(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived> &other) const
{
  return CwiseBinaryOp<internal::scalar_quotient_op<Scalar,typename OtherDerived::Scalar>, const Derived, const OtherDerived>(derived(), other.derived());
}

/** \returns an expression of the coefficient-wise min of \c *this and \a other
  *
  * Example: \include Cwise_min.cpp
  * Output: \verbinclude Cwise_min.out
  *
  * \sa max()
  */
template <int NaNPropagation=PropagateFast, typename OtherDerived>
EIGEN_DEVICE_FUNC
EIGEN_STRONG_INLINE const CwiseBinaryOp<internal::scalar_min_op<Scalar,Scalar,NaNPropagation>, const Derived, const OtherDerived>
#ifdef EIGEN_PARSED_BY_DOXYGEN
min
#else
(min)
#endif
(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived> &other) const
{
  return CwiseBinaryOp<internal::scalar_min_op<Scalar,Scalar,NaNPropagation>, const Derived, const OtherDerived>(derived(), other.derived());
}

/** \returns an expression of the coefficient-wise min of \c *this and scalar \a other
  *
  * \sa max()
  */
template <int NaNPropagation=PropagateFast>
EIGEN_DEVICE_FUNC
EIGEN_STRONG_INLINE const CwiseBinaryOp<internal::scalar_min_op<Scalar,Scalar,NaNPropagation>, const Derived,
    const CwiseNullaryOp<internal::scalar_constant_op<Scalar>, PlainObject> >
#ifdef EIGEN_PARSED_BY_DOXYGEN
min
#else
(min)
#endif
(const Scalar &other) const
{
  return (min<NaNPropagation>)(Derived::PlainObject::Constant(rows(), cols(), other));
}

/** \returns an expression of the coefficient-wise max of \c *this and \a other
  *
  * Example: \include Cwise_max.cpp
  * Output: \verbinclude Cwise_max.out
  *
  * \sa min()
  */
template <int NaNPropagation=PropagateFast, typename OtherDerived>
EIGEN_DEVICE_FUNC
EIGEN_STRONG_INLINE const CwiseBinaryOp<internal::scalar_max_op<Scalar,Scalar,NaNPropagation>, const Derived, const OtherDerived>
#ifdef EIGEN_PARSED_BY_DOXYGEN
max
#else
(max)
#endif
(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived> &other) const
{
  return CwiseBinaryOp<internal::scalar_max_op<Scalar,Scalar,NaNPropagation>, const Derived, const OtherDerived>(derived(), other.derived());
}

/** \returns an expression of the coefficient-wise max of \c *this and scalar \a other
  *
  * \sa min()
  */
template <int NaNPropagation=PropagateFast>
EIGEN_DEVICE_FUNC
    EIGEN_STRONG_INLINE const CwiseBinaryOp<internal::scalar_max_op<Scalar,Scalar,NaNPropagation>, const Derived,
                                        const CwiseNullaryOp<internal::scalar_constant_op<Scalar>, PlainObject> >
#ifdef EIGEN_PARSED_BY_DOXYGEN
max
#else
(max)
#endif
(const Scalar &other) const
{
  return (max<NaNPropagation>)(Derived::PlainObject::Constant(rows(), cols(), other));
}

/** \returns an expression of the coefficient-wise absdiff of \c *this and \a other
  *
  * Example: \include Cwise_absolute_difference.cpp
  * Output: \verbinclude Cwise_absolute_difference.out
  *
  * \sa absolute_difference()
  */
EIGEN_MAKE_CWISE_BINARY_OP(absolute_difference,absolute_difference)

/** \returns an expression of the coefficient-wise absolute_difference of \c *this and scalar \a other
  *
  * \sa absolute_difference()
  */
EIGEN_DEVICE_FUNC
EIGEN_STRONG_INLINE const CwiseBinaryOp<internal::scalar_absolute_difference_op<Scalar,Scalar>, const Derived,
                                        const CwiseNullaryOp<internal::scalar_constant_op<Scalar>, PlainObject> >
#ifdef EIGEN_PARSED_BY_DOXYGEN
absolute_difference
#else
(absolute_difference)
#endif
(const Scalar &other) const
{
  return (absolute_difference)(Derived::PlainObject::Constant(rows(), cols(), other));
}

/** \returns an expression of the coefficient-wise power of \c *this to the given array of \a exponents.
  *
  * This function computes the coefficient-wise power.
  *
  * Example: \include Cwise_array_power_array.cpp
  * Output: \verbinclude Cwise_array_power_array.out
  */
EIGEN_MAKE_CWISE_BINARY_OP(pow,pow)

/** \returns an expression of the coefficient-wise atan2(\c *this, \a y), where \a y is the given array argument.
  *
  * This function computes the coefficient-wise atan2.
  *
  */
EIGEN_MAKE_CWISE_BINARY_OP(atan2,atan2)


// TODO code generating macros could be moved to Macros.h and could include generation of documentation
#define EIGEN_MAKE_CWISE_COMP_OP(OP, COMPARATOR) \
template<typename OtherDerived> \
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE const CwiseBinaryOp<internal::scalar_cmp_op<Scalar, typename OtherDerived::Scalar, internal::cmp_ ## COMPARATOR>, const Derived, const OtherDerived> \
OP(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived> &other) const \
{ \
  return CwiseBinaryOp<internal::scalar_cmp_op<Scalar, typename OtherDerived::Scalar, internal::cmp_ ## COMPARATOR>, const Derived, const OtherDerived>(derived(), other.derived()); \
}\
typedef CwiseBinaryOp<internal::scalar_cmp_op<Scalar,Scalar, internal::cmp_ ## COMPARATOR>, const Derived, const CwiseNullaryOp<internal::scalar_constant_op<Scalar>, PlainObject> > Cmp ## COMPARATOR ## ReturnType; \
typedef CwiseBinaryOp<internal::scalar_cmp_op<Scalar,Scalar, internal::cmp_ ## COMPARATOR>, const CwiseNullaryOp<internal::scalar_constant_op<Scalar>, PlainObject>, const Derived > RCmp ## COMPARATOR ## ReturnType; \
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE const Cmp ## COMPARATOR ## ReturnType \
OP(const Scalar& s) const { \
  return this->OP(Derived::PlainObject::Constant(rows(), cols(), s)); \
} \
EIGEN_DEVICE_FUNC friend EIGEN_STRONG_INLINE const RCmp ## COMPARATOR ## ReturnType \
OP(const Scalar& s, const EIGEN_CURRENT_STORAGE_BASE_CLASS<Derived>& d) { \
  return Derived::PlainObject::Constant(d.rows(), d.cols(), s).OP(d); \
}

#define EIGEN_MAKE_CWISE_COMP_R_OP(OP, R_OP, RCOMPARATOR) \
template<typename OtherDerived> \
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE const CwiseBinaryOp<internal::scalar_cmp_op<typename OtherDerived::Scalar, Scalar, internal::cmp_##RCOMPARATOR>, const OtherDerived, const Derived> \
OP(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived> &other) const \
{ \
  return CwiseBinaryOp<internal::scalar_cmp_op<typename OtherDerived::Scalar, Scalar, internal::cmp_##RCOMPARATOR>, const OtherDerived, const Derived>(other.derived(), derived()); \
} \
EIGEN_DEVICE_FUNC \
inline const RCmp ## RCOMPARATOR ## ReturnType \
OP(const Scalar& s) const { \
  return Derived::PlainObject::Constant(rows(), cols(), s).R_OP(*this); \
} \
friend inline const Cmp ## RCOMPARATOR ## ReturnType \
OP(const Scalar& s, const Derived& d) { \
  return d.R_OP(Derived::PlainObject::Constant(d.rows(), d.cols(), s)); \
}



/** \returns an expression of the coefficient-wise \< operator of *this and \a other
  *
  * Example: \include Cwise_less.cpp
  * Output: \verbinclude Cwise_less.out
  *
  * \sa all(), any(), operator>(), operator<=()
  */
EIGEN_MAKE_CWISE_COMP_OP(operator<, LT)

/** \returns an expression of the coefficient-wise \<= operator of *this and \a other
  *
  * Example: \include Cwise_less_equal.cpp
  * Output: \verbinclude Cwise_less_equal.out
  *
  * \sa all(), any(), operator>=(), operator<()
  */
EIGEN_MAKE_CWISE_COMP_OP(operator<=, LE)

/** \returns an expression of the coefficient-wise \> operator of *this and \a other
  *
  * Example: \include Cwise_greater.cpp
  * Output: \verbinclude Cwise_greater.out
  *
  * \sa all(), any(), operator>=(), operator<()
  */
EIGEN_MAKE_CWISE_COMP_R_OP(operator>, operator<, LT)

/** \returns an expression of the coefficient-wise \>= operator of *this and \a other
  *
  * Example: \include Cwise_greater_equal.cpp
  * Output: \verbinclude Cwise_greater_equal.out
  *
  * \sa all(), any(), operator>(), operator<=()
  */
EIGEN_MAKE_CWISE_COMP_R_OP(operator>=, operator<=, LE)

/** \returns an expression of the coefficient-wise == operator of *this and \a other
  *
  * \warning this performs an exact comparison, which is generally a bad idea with floating-point types.
  * In order to check for equality between two vectors or matrices with floating-point coefficients, it is
  * generally a far better idea to use a fuzzy comparison as provided by isApprox() and
  * isMuchSmallerThan().
  *
  * Example: \include Cwise_equal_equal.cpp
  * Output: \verbinclude Cwise_equal_equal.out
  *
  * \sa all(), any(), isApprox(), isMuchSmallerThan()
  */
EIGEN_MAKE_CWISE_COMP_OP(operator==, EQ)

/** \returns an expression of the coefficient-wise != operator of *this and \a other
  *
  * \warning this performs an exact comparison, which is generally a bad idea with floating-point types.
  * In order to check for equality between two vectors or matrices with floating-point coefficients, it is
  * generally a far better idea to use a fuzzy comparison as provided by isApprox() and
  * isMuchSmallerThan().
  *
  * Example: \include Cwise_not_equal.cpp
  * Output: \verbinclude Cwise_not_equal.out
  *
  * \sa all(), any(), isApprox(), isMuchSmallerThan()
  */
EIGEN_MAKE_CWISE_COMP_OP(operator!=, NEQ)


#undef EIGEN_MAKE_CWISE_COMP_OP
#undef EIGEN_MAKE_CWISE_COMP_R_OP

// scalar addition
#ifndef EIGEN_PARSED_BY_DOXYGEN
EIGEN_MAKE_SCALAR_BINARY_OP(operator+,sum)
#else
/** \returns an expression of \c *this with each coeff incremented by the constant \a scalar
  *
  * \tparam T is the scalar type of \a scalar. It must be compatible with the scalar type of the given expression.
  *
  * Example: \include Cwise_plus.cpp
  * Output: \verbinclude Cwise_plus.out
  *
  * \sa operator+=(), operator-()
  */
template<typename T>
const CwiseBinaryOp<internal::scalar_sum_op<Scalar,T>,Derived,Constant<T> > operator+(const T& scalar) const;
/** \returns an expression of \a expr with each coeff incremented by the constant \a scalar
  *
  * \tparam T is the scalar type of \a scalar. It must be compatible with the scalar type of the given expression.
  */
template<typename T> friend
const CwiseBinaryOp<internal::scalar_sum_op<T,Scalar>,Constant<T>,Derived> operator+(const T& scalar, const StorageBaseType& expr);
#endif

#ifndef EIGEN_PARSED_BY_DOXYGEN
EIGEN_MAKE_SCALAR_BINARY_OP(operator-,difference)
#else
/** \returns an expression of \c *this with each coeff decremented by the constant \a scalar
  *
  * \tparam T is the scalar type of \a scalar. It must be compatible with the scalar type of the given expression.
  *
  * Example: \include Cwise_minus.cpp
  * Output: \verbinclude Cwise_minus.out
  *
  * \sa operator+=(), operator-()
  */
template<typename T>
const CwiseBinaryOp<internal::scalar_difference_op<Scalar,T>,Derived,Constant<T> > operator-(const T& scalar) const;
/** \returns an expression of the constant matrix of value \a scalar decremented by the coefficients of \a expr
  *
  * \tparam T is the scalar type of \a scalar. It must be compatible with the scalar type of the given expression.
  */
template<typename T> friend
const CwiseBinaryOp<internal::scalar_difference_op<T,Scalar>,Constant<T>,Derived> operator-(const T& scalar, const StorageBaseType& expr);
#endif


#ifndef EIGEN_PARSED_BY_DOXYGEN
  EIGEN_MAKE_SCALAR_BINARY_OP_ONTHELEFT(operator/,quotient)
#else
  /**
    * \brief Component-wise division of the scalar \a s by array elements of \a a.
    *
    * \tparam Scalar is the scalar type of \a x. It must be compatible with the scalar type of the given array expression (\c Derived::Scalar).
    */
  template<typename T> friend
  inline const CwiseBinaryOp<internal::scalar_quotient_op<T,Scalar>,Constant<T>,Derived>
  operator/(const T& s,const StorageBaseType& a);
#endif

// NOTE disabled until we agree on argument order
#if 0
/** \cpp11 \returns an expression of the coefficient-wise polygamma function.
  *
  * \specialfunctions_module
  *
  * It returns the \a n -th derivative of the digamma(psi) evaluated at \c *this.
  *
  * \warning Be careful with the order of the parameters: x.polygamma(n) is equivalent to polygamma(n,x)
  *
  * \sa Eigen::polygamma()
  */
template<typename DerivedN>
inline const CwiseBinaryOp<internal::scalar_polygamma_op<Scalar>, const DerivedN, const Derived>
polygamma(const EIGEN_CURRENT_STORAGE_BASE_CLASS<DerivedN> &n) const
{
  return CwiseBinaryOp<internal::scalar_polygamma_op<Scalar>, const DerivedN, const Derived>(n.derived(), this->derived());
}
#endif

/** \returns an expression of the coefficient-wise zeta function.
  *
  * \specialfunctions_module
  *
  * It returns the Riemann zeta function of two arguments \c *this and \a q:
  *
  * \param q is the shift, it must be > 0
  *
  * \note *this is the exponent, it must be > 1.
  * \note This function supports only float and double scalar types. To support other scalar types, the user has
  * to provide implementations of zeta(T,T) for any scalar type T to be supported.
  *
  * This method is an alias for zeta(*this,q);
  *
  * \sa Eigen::zeta()
  */
template<typename DerivedQ>
inline const CwiseBinaryOp<internal::scalar_zeta_op<Scalar>, const Derived, const DerivedQ>
zeta(const EIGEN_CURRENT_STORAGE_BASE_CLASS<DerivedQ> &q) const
{
  return CwiseBinaryOp<internal::scalar_zeta_op<Scalar>, const Derived, const DerivedQ>(this->derived(), q.derived());
}


#   ifdef EIGEN_ARRAYBASE_PLUGIN
#     include EIGEN_ARRAYBASE_PLUGIN
#   endif
#undef EIGEN_CURRENT_STORAGE_BASE_CLASS
#undef EIGEN_DOC_UNARY_ADDONS

    /** Special case of the template operator=, in order to prevent the compiler
      * from generating a default operator= (issue hit with g++ 4.1)
      */
    EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
    Derived& operator=(const ArrayBase& other)
    {
      internal::call_assignment(derived(), other.derived());
      return derived();
    }
    
    /** Set all the entries to \a value.
      * \sa DenseBase::setConstant(), DenseBase::fill() */
    EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
    Derived& operator=(const Scalar &value)
    { Base::setConstant(value); return derived(); }

    EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
    Derived& operator+=(const Scalar& scalar);
    EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
    Derived& operator-=(const Scalar& scalar);

    template<typename OtherDerived>
    EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
    Derived& operator+=(const ArrayBase<OtherDerived>& other);
    template<typename OtherDerived>
    EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
    Derived& operator-=(const ArrayBase<OtherDerived>& other);

    template<typename OtherDerived>
    EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
    Derived& operator*=(const ArrayBase<OtherDerived>& other);

    template<typename OtherDerived>
    EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
    Derived& operator/=(const ArrayBase<OtherDerived>& other);

  public:
    EIGEN_DEVICE_FUNC
    ArrayBase<Derived>& array() { return *this; }
    EIGEN_DEVICE_FUNC
    const ArrayBase<Derived>& array() const { return *this; }

    /** \returns an \link Eigen::MatrixBase Matrix \endlink expression of this array
      * \sa MatrixBase::array() */
    EIGEN_DEVICE_FUNC
    MatrixWrapper<Derived> matrix() { return MatrixWrapper<Derived>(derived()); }
    EIGEN_DEVICE_FUNC
    const MatrixWrapper<const Derived> matrix() const { return MatrixWrapper<const Derived>(derived()); }

//     template<typename Dest>
//     inline void evalTo(Dest& dst) const { dst = matrix(); }

  protected:
    EIGEN_DEFAULT_COPY_CONSTRUCTOR(ArrayBase)
    EIGEN_DEFAULT_EMPTY_CONSTRUCTOR_AND_DESTRUCTOR(ArrayBase)

  private:
    explicit ArrayBase(Index);
    ArrayBase(Index,Index);
    template<typename OtherDerived> explicit ArrayBase(const ArrayBase<OtherDerived>&);
  protected:
    // mixing arrays and matrices is not legal
    template<typename OtherDerived> Derived& operator+=(const MatrixBase<OtherDerived>& )
    {EIGEN_STATIC_ASSERT(std::ptrdiff_t(sizeof(typename OtherDerived::Scalar))==-1,YOU_CANNOT_MIX_ARRAYS_AND_MATRICES); return *this;}
    // mixing arrays and matrices is not legal
    template<typename OtherDerived> Derived& operator-=(const MatrixBase<OtherDerived>& )
    {EIGEN_STATIC_ASSERT(std::ptrdiff_t(sizeof(typename OtherDerived::Scalar))==-1,YOU_CANNOT_MIX_ARRAYS_AND_MATRICES); return *this;}
};

/** replaces \c *this by \c *this - \a other.
  *
  * \returns a reference to \c *this
  */
template<typename Derived>
template<typename OtherDerived>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE Derived &
ArrayBase<Derived>::operator-=(const ArrayBase<OtherDerived> &other)
{
  call_assignment(derived(), other.derived(), internal::sub_assign_op<Scalar,typename OtherDerived::Scalar>());
  return derived();
}

/** replaces \c *this by \c *this + \a other.
  *
  * \returns a reference to \c *this
  */
template<typename Derived>
template<typename OtherDerived>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE Derived &
ArrayBase<Derived>::operator+=(const ArrayBase<OtherDerived>& other)
{
  call_assignment(derived(), other.derived(), internal::add_assign_op<Scalar,typename OtherDerived::Scalar>());
  return derived();
}

/** replaces \c *this by \c *this * \a other coefficient wise.
  *
  * \returns a reference to \c *this
  */
template<typename Derived>
template<typename OtherDerived>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE Derived &
ArrayBase<Derived>::operator*=(const ArrayBase<OtherDerived>& other)
{
  call_assignment(derived(), other.derived(), internal::mul_assign_op<Scalar,typename OtherDerived::Scalar>());
  return derived();
}

/** replaces \c *this by \c *this / \a other coefficient wise.
  *
  * \returns a reference to \c *this
  */
template<typename Derived>
template<typename OtherDerived>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE Derived &
ArrayBase<Derived>::operator/=(const ArrayBase<OtherDerived>& other)
{
  call_assignment(derived(), other.derived(), internal::div_assign_op<Scalar,typename OtherDerived::Scalar>());
  return derived();
}

} // end namespace Eigen

#endif // EIGEN_ARRAYBASE_H
