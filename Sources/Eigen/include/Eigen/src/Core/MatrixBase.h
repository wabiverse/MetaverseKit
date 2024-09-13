// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2006-2009 Benoit Jacob <jacob.benoit.1@gmail.com>
// Copyright (C) 2008 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef EIGEN_MATRIXBASE_H
#define EIGEN_MATRIXBASE_H

// IWYU pragma: private
#include "./InternalHeaderCheck.h"

namespace Eigen {

/** \class MatrixBase
  * \ingroup Core_Module
  *
  * \brief Base class for all dense matrices, vectors, and expressions
  *
  * This class is the base that is inherited by all matrix, vector, and related expression
  * types. Most of the Eigen API is contained in this class, and its base classes. Other important
  * classes for the Eigen API are Matrix, and VectorwiseOp.
  *
  * Note that some methods are defined in other modules such as the \ref LU_Module LU module
  * for all functions related to matrix inversions.
  *
  * \tparam Derived is the derived type, e.g. a matrix type, or an expression, etc.
  *
  * When writing a function taking Eigen objects as argument, if you want your function
  * to take as argument any matrix, vector, or expression, just let it take a
  * MatrixBase argument. As an example, here is a function printFirstRow which, given
  * a matrix, vector, or expression \a x, prints the first row of \a x.
  *
  * \code
    template<typename Derived>
    void printFirstRow(const Eigen::MatrixBase<Derived>& x)
    {
      cout << x.row(0) << endl;
    }
  * \endcode
  *
  * This class can be extended with the help of the plugin mechanism described on the page
  * \ref TopicCustomizing_Plugins by defining the preprocessor symbol \c EIGEN_MATRIXBASE_PLUGIN.
  *
  * \sa \blank \ref TopicClassHierarchy
  */
template<typename Derived> class MatrixBase
  : public DenseBase<Derived>
{
  public:
#ifndef EIGEN_PARSED_BY_DOXYGEN
    typedef MatrixBase StorageBaseType;
    typedef typename internal::traits<Derived>::StorageKind StorageKind;
    typedef typename internal::traits<Derived>::StorageIndex StorageIndex;
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
    using Base::eval;
    using Base::operator-;
    using Base::operator+=;
    using Base::operator-=;
    using Base::operator*=;
    using Base::operator/=;

    typedef typename Base::CoeffReturnType CoeffReturnType;
    typedef typename Base::ConstTransposeReturnType ConstTransposeReturnType;
    typedef typename Base::RowXpr RowXpr;
    typedef typename Base::ColXpr ColXpr;
#endif // not EIGEN_PARSED_BY_DOXYGEN



#ifndef EIGEN_PARSED_BY_DOXYGEN
    /** type of the equivalent square matrix */
    typedef Matrix<Scalar, internal::max_size_prefer_dynamic(RowsAtCompileTime, ColsAtCompileTime),
                           internal::max_size_prefer_dynamic(RowsAtCompileTime, ColsAtCompileTime)> SquareMatrixType;
#endif // not EIGEN_PARSED_BY_DOXYGEN

    /** \returns the size of the main diagonal, which is min(rows(),cols()).
      * \sa rows(), cols(), SizeAtCompileTime. */
    EIGEN_DEVICE_FUNC
    inline Index diagonalSize() const { return (numext::mini)(rows(),cols()); }

    typedef typename Base::PlainObject PlainObject;

#ifndef EIGEN_PARSED_BY_DOXYGEN
    /** \internal Represents a matrix with all coefficients equal to one another*/
    typedef CwiseNullaryOp<internal::scalar_constant_op<Scalar>,PlainObject> ConstantReturnType;
    /** \internal the return type of MatrixBase::adjoint() */
    typedef std::conditional_t<NumTraits<Scalar>::IsComplex,
               CwiseUnaryOp<internal::scalar_conjugate_op<Scalar>, ConstTransposeReturnType>,
               ConstTransposeReturnType
            > AdjointReturnType;
    /** \internal Return type of eigenvalues() */
    typedef Matrix<std::complex<RealScalar>, internal::traits<Derived>::ColsAtCompileTime, 1, ColMajor> EigenvaluesReturnType;
    /** \internal the return type of identity */
    typedef CwiseNullaryOp<internal::scalar_identity_op<Scalar>,PlainObject> IdentityReturnType;
    /** \internal the return type of unit vectors */
    typedef Block<const CwiseNullaryOp<internal::scalar_identity_op<Scalar>, SquareMatrixType>,
                  internal::traits<Derived>::RowsAtCompileTime,
                  internal::traits<Derived>::ColsAtCompileTime> BasisReturnType;
#endif // not EIGEN_PARSED_BY_DOXYGEN

#define EIGEN_CURRENT_STORAGE_BASE_CLASS Eigen::MatrixBase
#define EIGEN_DOC_UNARY_ADDONS(X,Y)

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


#   ifdef EIGEN_MATRIXBASE_PLUGIN
#     include EIGEN_MATRIXBASE_PLUGIN
#   endif
#undef EIGEN_CURRENT_STORAGE_BASE_CLASS
#undef EIGEN_DOC_UNARY_ADDONS

    /** Special case of the template operator=, in order to prevent the compiler
      * from generating a default operator= (issue hit with g++ 4.1)
      */
    EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
    Derived& operator=(const MatrixBase& other);

    // We cannot inherit here via Base::operator= since it is causing
    // trouble with MSVC.

    template <typename OtherDerived>
    EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
    Derived& operator=(const DenseBase<OtherDerived>& other);

    template <typename OtherDerived>
    EIGEN_DEVICE_FUNC
    Derived& operator=(const EigenBase<OtherDerived>& other);

    template<typename OtherDerived>
    EIGEN_DEVICE_FUNC
    Derived& operator=(const ReturnByValue<OtherDerived>& other);

    template<typename OtherDerived>
    EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
    Derived& operator+=(const MatrixBase<OtherDerived>& other);
    template<typename OtherDerived>
    EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
    Derived& operator-=(const MatrixBase<OtherDerived>& other);

    template<typename OtherDerived>
    EIGEN_DEVICE_FUNC
    const Product<Derived,OtherDerived>
    operator*(const MatrixBase<OtherDerived> &other) const;

    template<typename OtherDerived>
    EIGEN_DEVICE_FUNC
    const Product<Derived,OtherDerived,LazyProduct>
    lazyProduct(const MatrixBase<OtherDerived> &other) const;

    template<typename OtherDerived>
    Derived& operator*=(const EigenBase<OtherDerived>& other);

    template<typename OtherDerived>
    void applyOnTheLeft(const EigenBase<OtherDerived>& other);

    template<typename OtherDerived>
    void applyOnTheRight(const EigenBase<OtherDerived>& other);

    template<typename DiagonalDerived>
    EIGEN_DEVICE_FUNC
    const Product<Derived, DiagonalDerived, LazyProduct>
    operator*(const DiagonalBase<DiagonalDerived> &diagonal) const;

    template<typename SkewDerived>
    EIGEN_DEVICE_FUNC
    const Product<Derived, SkewDerived, LazyProduct>
    operator*(const SkewSymmetricBase<SkewDerived> &skew) const;

    template<typename OtherDerived>
    EIGEN_DEVICE_FUNC
    typename ScalarBinaryOpTraits<typename internal::traits<Derived>::Scalar,typename internal::traits<OtherDerived>::Scalar>::ReturnType
    dot(const MatrixBase<OtherDerived>& other) const;

    EIGEN_DEVICE_FUNC RealScalar squaredNorm() const;
    EIGEN_DEVICE_FUNC RealScalar norm() const;
    RealScalar stableNorm() const;
    RealScalar blueNorm() const;
    RealScalar hypotNorm() const;
    EIGEN_DEVICE_FUNC const PlainObject normalized() const;
    EIGEN_DEVICE_FUNC const PlainObject stableNormalized() const;
    EIGEN_DEVICE_FUNC void normalize();
    EIGEN_DEVICE_FUNC void stableNormalize();

    EIGEN_DEVICE_FUNC const AdjointReturnType adjoint() const;
    EIGEN_DEVICE_FUNC void adjointInPlace();

    typedef Diagonal<Derived> DiagonalReturnType;
    EIGEN_DEVICE_FUNC
    DiagonalReturnType diagonal();

    typedef Diagonal<const Derived> ConstDiagonalReturnType;
    EIGEN_DEVICE_FUNC
    const ConstDiagonalReturnType diagonal() const;

    template<int Index>
    EIGEN_DEVICE_FUNC
    Diagonal<Derived, Index> diagonal();

    template<int Index>
    EIGEN_DEVICE_FUNC
    const Diagonal<const Derived, Index> diagonal() const;

    EIGEN_DEVICE_FUNC
    Diagonal<Derived, DynamicIndex> diagonal(Index index);
    EIGEN_DEVICE_FUNC
    const Diagonal<const Derived, DynamicIndex> diagonal(Index index) const;

    template<unsigned int Mode> struct TriangularViewReturnType { typedef TriangularView<Derived, Mode> Type; };
    template<unsigned int Mode> struct ConstTriangularViewReturnType { typedef const TriangularView<const Derived, Mode> Type; };

    template<unsigned int Mode>
    EIGEN_DEVICE_FUNC
    typename TriangularViewReturnType<Mode>::Type triangularView();
    template<unsigned int Mode>
    EIGEN_DEVICE_FUNC
    typename ConstTriangularViewReturnType<Mode>::Type triangularView() const;

    template<unsigned int UpLo> struct SelfAdjointViewReturnType { typedef SelfAdjointView<Derived, UpLo> Type; };
    template<unsigned int UpLo> struct ConstSelfAdjointViewReturnType { typedef const SelfAdjointView<const Derived, UpLo> Type; };

    template<unsigned int UpLo>
    EIGEN_DEVICE_FUNC
    typename SelfAdjointViewReturnType<UpLo>::Type selfadjointView();
    template<unsigned int UpLo>
    EIGEN_DEVICE_FUNC
    typename ConstSelfAdjointViewReturnType<UpLo>::Type selfadjointView() const;

    const SparseView<Derived> sparseView(const Scalar& m_reference = Scalar(0),
                                         const typename NumTraits<Scalar>::Real& m_epsilon = NumTraits<Scalar>::dummy_precision()) const;
    EIGEN_DEVICE_FUNC static const IdentityReturnType Identity();
    EIGEN_DEVICE_FUNC static const IdentityReturnType Identity(Index rows, Index cols);
    EIGEN_DEVICE_FUNC static const BasisReturnType Unit(Index size, Index i);
    EIGEN_DEVICE_FUNC static const BasisReturnType Unit(Index i);
    EIGEN_DEVICE_FUNC static const BasisReturnType UnitX();
    EIGEN_DEVICE_FUNC static const BasisReturnType UnitY();
    EIGEN_DEVICE_FUNC static const BasisReturnType UnitZ();
    EIGEN_DEVICE_FUNC static const BasisReturnType UnitW();

    EIGEN_DEVICE_FUNC
    const DiagonalWrapper<const Derived> asDiagonal() const;
    const PermutationWrapper<const Derived> asPermutation() const;
    EIGEN_DEVICE_FUNC
    const SkewSymmetricWrapper<const Derived> asSkewSymmetric() const;

    EIGEN_DEVICE_FUNC
    Derived& setIdentity();
    EIGEN_DEVICE_FUNC
    Derived& setIdentity(Index rows, Index cols);
    EIGEN_DEVICE_FUNC Derived& setUnit(Index i);
    EIGEN_DEVICE_FUNC Derived& setUnit(Index newSize, Index i);

    bool isIdentity(const RealScalar& prec = NumTraits<Scalar>::dummy_precision()) const;
    bool isDiagonal(const RealScalar& prec = NumTraits<Scalar>::dummy_precision()) const;

    bool isUpperTriangular(const RealScalar& prec = NumTraits<Scalar>::dummy_precision()) const;
    bool isLowerTriangular(const RealScalar& prec = NumTraits<Scalar>::dummy_precision()) const;

    bool isSkewSymmetric(const RealScalar& prec = NumTraits<Scalar>::dummy_precision()) const;

    template<typename OtherDerived>
    bool isOrthogonal(const MatrixBase<OtherDerived>& other,
                      const RealScalar& prec = NumTraits<Scalar>::dummy_precision()) const;
    bool isUnitary(const RealScalar& prec = NumTraits<Scalar>::dummy_precision()) const;

    /** \returns true if each coefficients of \c *this and \a other are all exactly equal.
      * \warning When using floating point scalar values you probably should rather use a
      *          fuzzy comparison such as isApprox()
      * \sa isApprox(), operator!= */
    template<typename OtherDerived>
    EIGEN_DEVICE_FUNC inline bool operator==(const MatrixBase<OtherDerived>& other) const
    { return cwiseEqual(other).all(); }

    /** \returns true if at least one pair of coefficients of \c *this and \a other are not exactly equal to each other.
      * \warning When using floating point scalar values you probably should rather use a
      *          fuzzy comparison such as isApprox()
      * \sa isApprox(), operator== */
    template<typename OtherDerived>
    EIGEN_DEVICE_FUNC inline bool operator!=(const MatrixBase<OtherDerived>& other) const
    { return cwiseNotEqual(other).any(); }

    NoAlias<Derived,Eigen::MatrixBase > EIGEN_DEVICE_FUNC noalias();

    // TODO forceAlignedAccess is temporarily disabled
    // Need to find a nicer workaround.
    inline const Derived& forceAlignedAccess() const { return derived(); }
    inline Derived& forceAlignedAccess() { return derived(); }
    template<bool Enable> inline const Derived& forceAlignedAccessIf() const { return derived(); }
    template<bool Enable> inline Derived& forceAlignedAccessIf() { return derived(); }

    EIGEN_DEVICE_FUNC Scalar trace() const;

    template<int p> EIGEN_DEVICE_FUNC RealScalar lpNorm() const;

    EIGEN_DEVICE_FUNC MatrixBase<Derived>& matrix() { return *this; }
    EIGEN_DEVICE_FUNC const MatrixBase<Derived>& matrix() const { return *this; }

    /** \returns an \link Eigen::ArrayBase Array \endlink expression of this matrix
      * \sa ArrayBase::matrix() */
    EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE ArrayWrapper<Derived> array() { return ArrayWrapper<Derived>(derived()); }
    /** \returns a const \link Eigen::ArrayBase Array \endlink expression of this matrix
      * \sa ArrayBase::matrix() */
    EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE const ArrayWrapper<const Derived> array() const { return ArrayWrapper<const Derived>(derived()); }

/////////// LU module ///////////

    template<typename PermutationIndex = DefaultPermutationIndex> inline const FullPivLU<PlainObject, PermutationIndex> fullPivLu() const;
    template<typename PermutationIndex = DefaultPermutationIndex> inline const PartialPivLU<PlainObject, PermutationIndex> partialPivLu() const;

    template<typename PermutationIndex = DefaultPermutationIndex> inline const PartialPivLU<PlainObject, PermutationIndex> lu() const;

    EIGEN_DEVICE_FUNC
    inline const Inverse<Derived> inverse() const;

    template<typename ResultType>
    inline void computeInverseAndDetWithCheck(
      ResultType& inverse,
      typename ResultType::Scalar& determinant,
      bool& invertible,
      const RealScalar& absDeterminantThreshold = NumTraits<Scalar>::dummy_precision()
    ) const;

    template<typename ResultType>
    inline void computeInverseWithCheck(
      ResultType& inverse,
      bool& invertible,
      const RealScalar& absDeterminantThreshold = NumTraits<Scalar>::dummy_precision()
    ) const;

    EIGEN_DEVICE_FUNC
    Scalar determinant() const;

/////////// Cholesky module ///////////

    inline const LLT<PlainObject>  llt() const;
    inline const LDLT<PlainObject> ldlt() const;

/////////// QR module ///////////

    inline const HouseholderQR<PlainObject> householderQr() const;
    template<typename PermutationIndex = DefaultPermutationIndex> inline const ColPivHouseholderQR<PlainObject, PermutationIndex> colPivHouseholderQr() const;
    template<typename PermutationIndex = DefaultPermutationIndex> inline const FullPivHouseholderQR<PlainObject, PermutationIndex> fullPivHouseholderQr() const;
    template<typename PermutationIndex = DefaultPermutationIndex> inline const CompleteOrthogonalDecomposition<PlainObject, PermutationIndex> completeOrthogonalDecomposition() const;

/////////// Eigenvalues module ///////////

    inline EigenvaluesReturnType eigenvalues() const;
    inline RealScalar operatorNorm() const;

/////////// SVD module ///////////

    template<int Options = 0>
    inline JacobiSVD<PlainObject, Options> jacobiSvd() const;
    template<int Options = 0>
    EIGEN_DEPRECATED
    inline JacobiSVD<PlainObject, Options> jacobiSvd(unsigned int computationOptions) const;

    template<int Options = 0>
    inline BDCSVD<PlainObject, Options> bdcSvd() const;
    template<int Options = 0>
    EIGEN_DEPRECATED
    inline BDCSVD<PlainObject, Options> bdcSvd(unsigned int computationOptions) const;

/////////// Geometry module ///////////

    template<typename OtherDerived>
    EIGEN_DEVICE_FUNC
    inline typename internal::cross_impl<Derived, OtherDerived>::return_type
    cross(const MatrixBase<OtherDerived>& other) const;

    template<typename OtherDerived>
    EIGEN_DEVICE_FUNC
    inline PlainObject cross3(const MatrixBase<OtherDerived>& other) const;

    EIGEN_DEVICE_FUNC
    inline PlainObject unitOrthogonal(void) const;

    EIGEN_DEPRECATED EIGEN_DEVICE_FUNC
    inline Matrix<Scalar,3,1> eulerAngles(Index a0, Index a1, Index a2) const;

    EIGEN_DEVICE_FUNC
    inline Matrix<Scalar,3,1> canonicalEulerAngles(Index a0, Index a1, Index a2) const;

    // put this as separate enum value to work around possible GCC 4.3 bug (?)
    enum { HomogeneousReturnTypeDirection = ColsAtCompileTime==1&&RowsAtCompileTime==1 ? ((internal::traits<Derived>::Flags&RowMajorBit)==RowMajorBit ? Horizontal : Vertical)
                                          : ColsAtCompileTime==1 ? Vertical : Horizontal };
    typedef Homogeneous<Derived, HomogeneousReturnTypeDirection> HomogeneousReturnType;
    EIGEN_DEVICE_FUNC
    inline HomogeneousReturnType homogeneous() const;

    enum {
      SizeMinusOne = SizeAtCompileTime==Dynamic ? Dynamic : SizeAtCompileTime-1
    };
    typedef Block<const Derived,
                  internal::traits<Derived>::ColsAtCompileTime==1 ? SizeMinusOne : 1,
                  internal::traits<Derived>::ColsAtCompileTime==1 ? 1 : SizeMinusOne> ConstStartMinusOne;
    typedef EIGEN_EXPR_BINARYOP_SCALAR_RETURN_TYPE(ConstStartMinusOne,Scalar,quotient) HNormalizedReturnType;
    EIGEN_DEVICE_FUNC
    inline const HNormalizedReturnType hnormalized() const;

////////// Householder module ///////////

    EIGEN_DEVICE_FUNC
    void makeHouseholderInPlace(Scalar& tau, RealScalar& beta);
    template<typename EssentialPart>
    EIGEN_DEVICE_FUNC
    void makeHouseholder(EssentialPart& essential,
                         Scalar& tau, RealScalar& beta) const;
    template<typename EssentialPart>
    EIGEN_DEVICE_FUNC
    void applyHouseholderOnTheLeft(const EssentialPart& essential,
                                   const Scalar& tau,
                                   Scalar* workspace);
    template<typename EssentialPart>
    EIGEN_DEVICE_FUNC
    void applyHouseholderOnTheRight(const EssentialPart& essential,
                                    const Scalar& tau,
                                    Scalar* workspace);

///////// Jacobi module /////////

    template<typename OtherScalar>
    EIGEN_DEVICE_FUNC
    void applyOnTheLeft(Index p, Index q, const JacobiRotation<OtherScalar>& j);
    template<typename OtherScalar>
    EIGEN_DEVICE_FUNC
    void applyOnTheRight(Index p, Index q, const JacobiRotation<OtherScalar>& j);

///////// SparseCore module /////////

    template<typename OtherDerived>
    EIGEN_STRONG_INLINE const typename SparseMatrixBase<OtherDerived>::template CwiseProductDenseReturnType<Derived>::Type
    cwiseProduct(const SparseMatrixBase<OtherDerived> &other) const
    {
      return other.cwiseProduct(derived());
    }

///////// MatrixFunctions module /////////

    typedef typename internal::stem_function<Scalar>::type StemFunction;
#define EIGEN_MATRIX_FUNCTION(ReturnType, Name, Description) \
    /** \returns an expression of the matrix Description of \c *this. \brief This function requires the <a href="unsupported/group__MatrixFunctions__Module.html"> unsupported MatrixFunctions module</a>. To compute the coefficient-wise Description use ArrayBase::##Name . */ \
    const ReturnType<Derived> Name() const;
#define EIGEN_MATRIX_FUNCTION_1(ReturnType, Name, Description, Argument) \
    /** \returns an expression of the matrix Description of \c *this. \brief This function requires the <a href="unsupported/group__MatrixFunctions__Module.html"> unsupported MatrixFunctions module</a>. To compute the coefficient-wise Description use ArrayBase::##Name . */ \
    const ReturnType<Derived> Name(Argument) const;

    EIGEN_MATRIX_FUNCTION(MatrixExponentialReturnValue, exp, exponential)
    /** \brief Helper function for the <a href="unsupported/group__MatrixFunctions__Module.html"> unsupported MatrixFunctions module</a>.*/
    const MatrixFunctionReturnValue<Derived> matrixFunction(StemFunction f) const;
    EIGEN_MATRIX_FUNCTION(MatrixFunctionReturnValue, cosh, hyperbolic cosine)
    EIGEN_MATRIX_FUNCTION(MatrixFunctionReturnValue, sinh, hyperbolic sine)
    EIGEN_MATRIX_FUNCTION(MatrixFunctionReturnValue, atanh, inverse hyperbolic cosine)
    EIGEN_MATRIX_FUNCTION(MatrixFunctionReturnValue, acosh, inverse hyperbolic cosine)
    EIGEN_MATRIX_FUNCTION(MatrixFunctionReturnValue, asinh, inverse hyperbolic sine)
    EIGEN_MATRIX_FUNCTION(MatrixFunctionReturnValue, cos, cosine)
    EIGEN_MATRIX_FUNCTION(MatrixFunctionReturnValue, sin, sine)
    EIGEN_MATRIX_FUNCTION(MatrixSquareRootReturnValue, sqrt, square root)
    EIGEN_MATRIX_FUNCTION(MatrixLogarithmReturnValue, log, logarithm)
    EIGEN_MATRIX_FUNCTION_1(MatrixPowerReturnValue,        pow, power to \c p, const RealScalar& p)
    EIGEN_MATRIX_FUNCTION_1(MatrixComplexPowerReturnValue, pow, power to \c p, const std::complex<RealScalar>& p)

  protected:
    EIGEN_DEFAULT_COPY_CONSTRUCTOR(MatrixBase)
    EIGEN_DEFAULT_EMPTY_CONSTRUCTOR_AND_DESTRUCTOR(MatrixBase)

  private:
    EIGEN_DEVICE_FUNC explicit MatrixBase(int);
    EIGEN_DEVICE_FUNC MatrixBase(int,int);
    template<typename OtherDerived> EIGEN_DEVICE_FUNC explicit MatrixBase(const MatrixBase<OtherDerived>&);
  protected:
    // mixing arrays and matrices is not legal
    template<typename OtherDerived> Derived& operator+=(const ArrayBase<OtherDerived>& )
    {EIGEN_STATIC_ASSERT(std::ptrdiff_t(sizeof(typename OtherDerived::Scalar))==-1,YOU_CANNOT_MIX_ARRAYS_AND_MATRICES); return *this;}
    // mixing arrays and matrices is not legal
    template<typename OtherDerived> Derived& operator-=(const ArrayBase<OtherDerived>& )
    {EIGEN_STATIC_ASSERT(std::ptrdiff_t(sizeof(typename OtherDerived::Scalar))==-1,YOU_CANNOT_MIX_ARRAYS_AND_MATRICES); return *this;}
};


/***************************************************************************
* Implementation of matrix base methods
***************************************************************************/

/** replaces \c *this by \c *this * \a other.
  *
  * \returns a reference to \c *this
  *
  * Example: \include MatrixBase_applyOnTheRight.cpp
  * Output: \verbinclude MatrixBase_applyOnTheRight.out
  */
template<typename Derived>
template<typename OtherDerived>
inline Derived&
MatrixBase<Derived>::operator*=(const EigenBase<OtherDerived> &other)
{
  other.derived().applyThisOnTheRight(derived());
  return derived();
}

/** replaces \c *this by \c *this * \a other. It is equivalent to MatrixBase::operator*=().
  *
  * Example: \include MatrixBase_applyOnTheRight.cpp
  * Output: \verbinclude MatrixBase_applyOnTheRight.out
  */
template<typename Derived>
template<typename OtherDerived>
inline void MatrixBase<Derived>::applyOnTheRight(const EigenBase<OtherDerived> &other)
{
  other.derived().applyThisOnTheRight(derived());
}

/** replaces \c *this by \a other * \c *this.
  *
  * Example: \include MatrixBase_applyOnTheLeft.cpp
  * Output: \verbinclude MatrixBase_applyOnTheLeft.out
  */
template<typename Derived>
template<typename OtherDerived>
inline void MatrixBase<Derived>::applyOnTheLeft(const EigenBase<OtherDerived> &other)
{
  other.derived().applyThisOnTheLeft(derived());
}

} // end namespace Eigen

#endif // EIGEN_MATRIXBASE_H
