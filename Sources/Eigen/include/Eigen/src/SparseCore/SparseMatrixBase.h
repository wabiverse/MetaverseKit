// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008-2014 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef EIGEN_SPARSEMATRIXBASE_H
#define EIGEN_SPARSEMATRIXBASE_H

// IWYU pragma: private
#include "./InternalHeaderCheck.h"

namespace Eigen { 

/** \ingroup SparseCore_Module
  *
  * \class SparseMatrixBase
  *
  * \brief Base class of any sparse matrices or sparse expressions
  *
  * \tparam Derived is the derived type, e.g. a sparse matrix type, or an expression, etc.
  *
  * This class can be extended with the help of the plugin mechanism described on the page
  * \ref TopicCustomizing_Plugins by defining the preprocessor symbol \c EIGEN_SPARSEMATRIXBASE_PLUGIN.
  */
template<typename Derived> class SparseMatrixBase
  : public EigenBase<Derived>
{
  public:

    typedef typename internal::traits<Derived>::Scalar Scalar;
    
    /** The numeric type of the expression' coefficients, e.g. float, double, int or std::complex<float>, etc.
      *
      * It is an alias for the Scalar type */
    typedef Scalar value_type;
    
    typedef typename internal::packet_traits<Scalar>::type PacketScalar;
    typedef typename internal::traits<Derived>::StorageKind StorageKind;

    /** The integer type used to \b store indices within a SparseMatrix.
      * For a \c SparseMatrix<Scalar,Options,IndexType> it an alias of the third template parameter \c IndexType. */
    typedef typename internal::traits<Derived>::StorageIndex StorageIndex;

    typedef typename internal::add_const_on_value_type_if_arithmetic<
                         typename internal::packet_traits<Scalar>::type
                     >::type PacketReturnType;

    typedef SparseMatrixBase StorageBaseType;

    typedef Matrix<StorageIndex,Dynamic,1> IndexVector;
    typedef Matrix<Scalar,Dynamic,1> ScalarVector;
    
    template<typename OtherDerived>
    Derived& operator=(const EigenBase<OtherDerived> &other);

    enum {

      RowsAtCompileTime = internal::traits<Derived>::RowsAtCompileTime,
        /**< The number of rows at compile-time. This is just a copy of the value provided
          * by the \a Derived type. If a value is not known at compile-time,
          * it is set to the \a Dynamic constant.
          * \sa MatrixBase::rows(), MatrixBase::cols(), ColsAtCompileTime, SizeAtCompileTime */

      ColsAtCompileTime = internal::traits<Derived>::ColsAtCompileTime,
        /**< The number of columns at compile-time. This is just a copy of the value provided
          * by the \a Derived type. If a value is not known at compile-time,
          * it is set to the \a Dynamic constant.
          * \sa MatrixBase::rows(), MatrixBase::cols(), RowsAtCompileTime, SizeAtCompileTime */


      SizeAtCompileTime = (internal::size_of_xpr_at_compile_time<Derived>::ret),
        /**< This is equal to the number of coefficients, i.e. the number of
          * rows times the number of columns, or to \a Dynamic if this is not
          * known at compile-time. \sa RowsAtCompileTime, ColsAtCompileTime */

      MaxRowsAtCompileTime = RowsAtCompileTime,
      MaxColsAtCompileTime = ColsAtCompileTime,

      MaxSizeAtCompileTime = internal::size_at_compile_time(MaxRowsAtCompileTime, MaxColsAtCompileTime),

      IsVectorAtCompileTime = RowsAtCompileTime == 1 || ColsAtCompileTime == 1,
        /**< This is set to true if either the number of rows or the number of
          * columns is known at compile-time to be equal to 1. Indeed, in that case,
          * we are dealing with a column-vector (if there is only one column) or with
          * a row-vector (if there is only one row). */

      NumDimensions = int(MaxSizeAtCompileTime) == 1 ? 0 : bool(IsVectorAtCompileTime) ? 1 : 2,
        /**< This value is equal to Tensor::NumDimensions, i.e. 0 for scalars, 1 for vectors,
         * and 2 for matrices.
         */

      Flags = internal::traits<Derived>::Flags,
        /**< This stores expression \ref flags flags which may or may not be inherited by new expressions
          * constructed from this one. See the \ref flags "list of flags".
          */

      IsRowMajor = Flags&RowMajorBit ? 1 : 0,
      
      InnerSizeAtCompileTime = int(IsVectorAtCompileTime) ? int(SizeAtCompileTime)
                             : int(IsRowMajor) ? int(ColsAtCompileTime) : int(RowsAtCompileTime),

      #ifndef EIGEN_PARSED_BY_DOXYGEN
      HasDirectAccess_ = (int(Flags)&DirectAccessBit) ? 1 : 0 // workaround sunCC
      #endif
    };

    /** \internal the return type of MatrixBase::adjoint() */
    typedef std::conditional_t<NumTraits<Scalar>::IsComplex,
                        CwiseUnaryOp<internal::scalar_conjugate_op<Scalar>, Eigen::Transpose<const Derived> >,
                        Transpose<const Derived>
                     > AdjointReturnType;
    typedef Transpose<Derived> TransposeReturnType;
    typedef Transpose<const Derived> ConstTransposeReturnType;

    // FIXME storage order do not match evaluator storage order
    typedef SparseMatrix<Scalar, Flags&RowMajorBit ? RowMajor : ColMajor, StorageIndex> PlainObject;

#ifndef EIGEN_PARSED_BY_DOXYGEN
    /** This is the "real scalar" type; if the \a Scalar type is already real numbers
      * (e.g. int, float or double) then \a RealScalar is just the same as \a Scalar. If
      * \a Scalar is \a std::complex<T> then RealScalar is \a T.
      *
      * \sa class NumTraits
      */
    typedef typename NumTraits<Scalar>::Real RealScalar;

    /** \internal the return type of coeff()
      */
    typedef std::conditional_t<HasDirectAccess_, const Scalar&, Scalar> CoeffReturnType;

    /** \internal Represents a matrix with all coefficients equal to one another*/
    typedef CwiseNullaryOp<internal::scalar_constant_op<Scalar>,Matrix<Scalar,Dynamic,Dynamic> > ConstantReturnType;

    /** type of the equivalent dense matrix */
    typedef Matrix<Scalar,RowsAtCompileTime,ColsAtCompileTime> DenseMatrixType;
    /** type of the equivalent square matrix */
    typedef Matrix<Scalar, internal::max_size_prefer_dynamic(RowsAtCompileTime, ColsAtCompileTime),
                           internal::max_size_prefer_dynamic(RowsAtCompileTime, ColsAtCompileTime)> SquareMatrixType;

    inline const Derived& derived() const { return *static_cast<const Derived*>(this); }
    inline Derived& derived() { return *static_cast<Derived*>(this); }
    inline Derived& const_cast_derived() const
    { return *static_cast<Derived*>(const_cast<SparseMatrixBase*>(this)); }

    typedef EigenBase<Derived> Base;

#endif // not EIGEN_PARSED_BY_DOXYGEN

#define EIGEN_CURRENT_STORAGE_BASE_CLASS Eigen::SparseMatrixBase
#ifdef EIGEN_PARSED_BY_DOXYGEN
#define EIGEN_DOC_UNARY_ADDONS(METHOD,OP)           /** <p>This method does not change the sparsity of \c *this: the OP is applied to explicitly stored coefficients only. \sa SparseCompressedBase::coeffs() </p> */
#define EIGEN_DOC_BLOCK_ADDONS_NOT_INNER_PANEL      /** <p> \warning This method returns a read-only expression for any sparse matrices. \sa \ref TutorialSparse_SubMatrices "Sparse block operations" </p> */
#define EIGEN_DOC_BLOCK_ADDONS_INNER_PANEL_IF(COND) /** <p> \warning This method returns a read-write expression for COND sparse matrices only. Otherwise, the returned expression is read-only. \sa \ref TutorialSparse_SubMatrices "Sparse block operations" </p> */
#else
#define EIGEN_DOC_UNARY_ADDONS(X,Y)
#define EIGEN_DOC_BLOCK_ADDONS_NOT_INNER_PANEL
#define EIGEN_DOC_BLOCK_ADDONS_INNER_PANEL_IF(COND)
#endif

// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008-2009 Gael Guennebaud <gael.guennebaud@inria.fr>
// Copyright (C) 2006-2008 Benoit Jacob <jacob.benoit.1@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

// This file is a base class plugin containing common coefficient wise functions.

#ifndef EIGEN_PARSED_BY_DOXYGEN

/** \internal the return type of conjugate() */
typedef std::conditional_t<NumTraits<Scalar>::IsComplex,
            const CwiseUnaryOp<internal::scalar_conjugate_op<Scalar>, const Derived>,
            const Derived&
          > ConjugateReturnType;
/** \internal the return type of real() const */
typedef std::conditional_t<NumTraits<Scalar>::IsComplex,
            const CwiseUnaryOp<internal::scalar_real_op<Scalar>, const Derived>,
            const Derived&
          > RealReturnType;
/** \internal the return type of real() */
typedef std::conditional_t<NumTraits<Scalar>::IsComplex,
            CwiseUnaryView<internal::scalar_real_ref_op<Scalar>, Derived>,
            Derived&
          > NonConstRealReturnType;
/** \internal the return type of imag() const */
typedef CwiseUnaryOp<internal::scalar_imag_op<Scalar>, const Derived> ImagReturnType;
/** \internal the return type of imag() */
typedef CwiseUnaryView<internal::scalar_imag_ref_op<Scalar>, Derived> NonConstImagReturnType;

typedef CwiseUnaryOp<internal::scalar_opposite_op<Scalar>, const Derived> NegativeReturnType;

#endif // not EIGEN_PARSED_BY_DOXYGEN

/// \returns an expression of the opposite of \c *this
///
EIGEN_DOC_UNARY_ADDONS(operator-,opposite)
///
EIGEN_DEVICE_FUNC
inline const NegativeReturnType
operator-() const { return NegativeReturnType(derived()); }


template<class NewType> struct CastXpr { typedef typename internal::cast_return_type<Derived,const CwiseUnaryOp<internal::core_cast_op<Scalar, NewType>, const Derived> >::type Type; };

/// \returns an expression of \c *this with the \a Scalar type casted to
/// \a NewScalar.
///
/// The template parameter \a NewScalar is the type we are casting the scalars to.
///
EIGEN_DOC_UNARY_ADDONS(cast,conversion function)
///
/// \sa class CwiseUnaryOp
///
template<typename NewType>
EIGEN_DEVICE_FUNC
typename CastXpr<NewType>::Type
cast() const
{
  return typename CastXpr<NewType>::Type(derived());
}

/// \returns an expression of the complex conjugate of \c *this.
///
EIGEN_DOC_UNARY_ADDONS(conjugate,complex conjugate)
///
/// \sa <a href="group__CoeffwiseMathFunctions.html#cwisetable_conj">Math functions</a>, MatrixBase::adjoint()
EIGEN_DEVICE_FUNC
inline ConjugateReturnType
conjugate() const
{
  return ConjugateReturnType(derived());
}

/// \returns an expression of the complex conjugate of \c *this if Cond==true, returns derived() otherwise.
///
EIGEN_DOC_UNARY_ADDONS(conjugate,complex conjugate)
///
/// \sa conjugate()
template<bool Cond>
EIGEN_DEVICE_FUNC
inline std::conditional_t<Cond,ConjugateReturnType,const Derived&>
conjugateIf() const
{
  typedef std::conditional_t<Cond,ConjugateReturnType,const Derived&> ReturnType;
  return ReturnType(derived());
}

/// \returns a read-only expression of the real part of \c *this.
///
EIGEN_DOC_UNARY_ADDONS(real,real part function)
///
/// \sa imag()
EIGEN_DEVICE_FUNC
inline RealReturnType
real() const { return RealReturnType(derived()); }

/// \returns an read-only expression of the imaginary part of \c *this.
///
EIGEN_DOC_UNARY_ADDONS(imag,imaginary part function)
///
/// \sa real()
EIGEN_DEVICE_FUNC
inline const ImagReturnType
imag() const { return ImagReturnType(derived()); }

/// \brief Apply a unary operator coefficient-wise
/// \param[in]  func  Functor implementing the unary operator
/// \tparam  CustomUnaryOp Type of \a func
/// \returns An expression of a custom coefficient-wise unary operator \a func of *this
///
/// The function \c ptr_fun() from the C++ standard library can be used to make functors out of normal functions.
///
/// Example:
/// \include class_CwiseUnaryOp_ptrfun.cpp
/// Output: \verbinclude class_CwiseUnaryOp_ptrfun.out
///
/// Genuine functors allow for more possibilities, for instance it may contain a state.
///
/// Example:
/// \include class_CwiseUnaryOp.cpp
/// Output: \verbinclude class_CwiseUnaryOp.out
///
EIGEN_DOC_UNARY_ADDONS(unaryExpr,unary function)
///
/// \sa unaryViewExpr, binaryExpr, class CwiseUnaryOp
///
template<typename CustomUnaryOp>
EIGEN_DEVICE_FUNC
inline const CwiseUnaryOp<CustomUnaryOp, const Derived>
unaryExpr(const CustomUnaryOp& func = CustomUnaryOp()) const
{
  return CwiseUnaryOp<CustomUnaryOp, const Derived>(derived(), func);
}

/// \returns an expression of a custom coefficient-wise unary operator \a func of *this
///
/// The template parameter \a CustomUnaryOp is the type of the functor
/// of the custom unary operator.
///
/// Example:
/// \include class_CwiseUnaryOp.cpp
/// Output: \verbinclude class_CwiseUnaryOp.out
///
EIGEN_DOC_UNARY_ADDONS(unaryViewExpr,unary function)
///
/// \sa unaryExpr, binaryExpr class CwiseUnaryOp
///
template<typename CustomViewOp>
EIGEN_DEVICE_FUNC
inline const CwiseUnaryView<CustomViewOp, const Derived>
unaryViewExpr(const CustomViewOp& func = CustomViewOp()) const
{
  return CwiseUnaryView<CustomViewOp, const Derived>(derived(), func);
}

/// \returns a non const expression of the real part of \c *this.
///
EIGEN_DOC_UNARY_ADDONS(real,real part function)
///
/// \sa imag()
EIGEN_DEVICE_FUNC
inline NonConstRealReturnType
real() { return NonConstRealReturnType(derived()); }

/// \returns a non const expression of the imaginary part of \c *this.
///
EIGEN_DOC_UNARY_ADDONS(imag,imaginary part function)
///
/// \sa real()
EIGEN_DEVICE_FUNC
inline NonConstImagReturnType
imag() { return NonConstImagReturnType(derived()); }


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


// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008-2010 Gael Guennebaud <gael.guennebaud@inria.fr>
// Copyright (C) 2006-2010 Benoit Jacob <jacob.benoit.1@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef EIGEN_PARSED_BY_DOXYGEN

/// \internal expression type of a column */
typedef Block<Derived, internal::traits<Derived>::RowsAtCompileTime, 1, !IsRowMajor> ColXpr;
typedef const Block<const Derived, internal::traits<Derived>::RowsAtCompileTime, 1, !IsRowMajor> ConstColXpr;
/// \internal expression type of a row */
typedef Block<Derived, 1, internal::traits<Derived>::ColsAtCompileTime, IsRowMajor> RowXpr;
typedef const Block<const Derived, 1, internal::traits<Derived>::ColsAtCompileTime, IsRowMajor> ConstRowXpr;
/// \internal expression type of a block of whole columns */
typedef Block<Derived, internal::traits<Derived>::RowsAtCompileTime, Dynamic, !IsRowMajor> ColsBlockXpr;
typedef const Block<const Derived, internal::traits<Derived>::RowsAtCompileTime, Dynamic, !IsRowMajor> ConstColsBlockXpr;
/// \internal expression type of a block of whole rows */
typedef Block<Derived, Dynamic, internal::traits<Derived>::ColsAtCompileTime, IsRowMajor> RowsBlockXpr;
typedef const Block<const Derived, Dynamic, internal::traits<Derived>::ColsAtCompileTime, IsRowMajor> ConstRowsBlockXpr;
/// \internal expression type of a block of whole columns */
template<int N> struct NColsBlockXpr { typedef Block<Derived, internal::traits<Derived>::RowsAtCompileTime, N, !IsRowMajor> Type; };
template<int N> struct ConstNColsBlockXpr { typedef const Block<const Derived, internal::traits<Derived>::RowsAtCompileTime, N, !IsRowMajor> Type; };
/// \internal expression type of a block of whole rows */
template<int N> struct NRowsBlockXpr { typedef Block<Derived, N, internal::traits<Derived>::ColsAtCompileTime, IsRowMajor> Type; };
template<int N> struct ConstNRowsBlockXpr { typedef const Block<const Derived, N, internal::traits<Derived>::ColsAtCompileTime, IsRowMajor> Type; };
/// \internal expression of a block */
typedef Block<Derived> BlockXpr;
typedef const Block<const Derived> ConstBlockXpr;
/// \internal expression of a block of fixed sizes */
template<int Rows, int Cols> struct FixedBlockXpr { typedef Block<Derived,Rows,Cols> Type; };
template<int Rows, int Cols> struct ConstFixedBlockXpr { typedef Block<const Derived,Rows,Cols> Type; };

typedef VectorBlock<Derived> SegmentReturnType;
typedef const VectorBlock<const Derived> ConstSegmentReturnType;
template<int Size> struct FixedSegmentReturnType { typedef VectorBlock<Derived, Size> Type; };
template<int Size> struct ConstFixedSegmentReturnType { typedef const VectorBlock<const Derived, Size> Type; };

/// \internal inner-vector
typedef Block<Derived,IsRowMajor?1:Dynamic,IsRowMajor?Dynamic:1,true>       InnerVectorReturnType;
typedef Block<const Derived,IsRowMajor?1:Dynamic,IsRowMajor?Dynamic:1,true> ConstInnerVectorReturnType;

/// \internal set of inner-vectors
typedef Block<Derived,Dynamic,Dynamic,true> InnerVectorsReturnType;
typedef Block<const Derived,Dynamic,Dynamic,true> ConstInnerVectorsReturnType;

#endif // not EIGEN_PARSED_BY_DOXYGEN

/// \returns an expression of a block in \c *this with either dynamic or fixed sizes.
///
/// \param  startRow  the first row in the block
/// \param  startCol  the first column in the block
/// \param  blockRows number of rows in the block, specified at either run-time or compile-time
/// \param  blockCols number of columns in the block, specified at either run-time or compile-time
/// \tparam NRowsType the type of the value handling the number of rows in the block, typically Index.
/// \tparam NColsType the type of the value handling the number of columns in the block, typically Index.
///
/// Example using runtime (aka dynamic) sizes: \include MatrixBase_block_int_int_int_int.cpp
/// Output: \verbinclude MatrixBase_block_int_int_int_int.out
///
/// \newin{3.4}:
///
/// The number of rows \a blockRows and columns \a blockCols can also be specified at compile-time by passing Eigen::fix<N>,
/// or Eigen::fix<N>(n) as arguments. In the later case, \c n plays the role of a runtime fallback value in case \c N equals Eigen::Dynamic.
/// Here is an example with a fixed number of rows \c NRows and dynamic number of columns \c cols:
/// \code
/// mat.block(i,j,fix<NRows>,cols)
/// \endcode
///
/// This function thus fully covers the features offered by the following overloads block<NRows,NCols>(Index, Index),
/// and block<NRows,NCols>(Index, Index, Index, Index) that are thus obsolete. Indeed, this generic version avoids
/// redundancy, it preserves the argument order, and prevents the need to rely on the template keyword in templated code.
///
/// but with less redundancy and more consistency as it does not modify the argument order
/// and seamlessly enable hybrid fixed/dynamic sizes.
///
/// \note Even in the case that the returned expression has dynamic size, in the case
/// when it is applied to a fixed-size matrix, it inherits a fixed maximal size,
/// which means that evaluating it does not cause a dynamic memory allocation.
///
EIGEN_DOC_BLOCK_ADDONS_NOT_INNER_PANEL
///
/// \sa class Block, fix, fix<N>(int)
///
template<typename NRowsType, typename NColsType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
typename FixedBlockXpr<internal::get_fixed_value<NRowsType>::value,internal::get_fixed_value<NColsType>::value>::Type
#else
typename FixedBlockXpr<...,...>::Type
#endif
block(Index startRow, Index startCol, NRowsType blockRows, NColsType blockCols)
{
  return typename FixedBlockXpr<internal::get_fixed_value<NRowsType>::value,internal::get_fixed_value<NColsType>::value>::Type(
            derived(), startRow, startCol, internal::get_runtime_value(blockRows), internal::get_runtime_value(blockCols));
}

/// This is the const version of block(Index,Index,NRowsType,NColsType)
template<typename NRowsType, typename NColsType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
const typename ConstFixedBlockXpr<internal::get_fixed_value<NRowsType>::value,internal::get_fixed_value<NColsType>::value>::Type
#else
const typename ConstFixedBlockXpr<...,...>::Type
#endif
block(Index startRow, Index startCol, NRowsType blockRows, NColsType blockCols) const
{
  return typename ConstFixedBlockXpr<internal::get_fixed_value<NRowsType>::value,internal::get_fixed_value<NColsType>::value>::Type(
            derived(), startRow, startCol, internal::get_runtime_value(blockRows), internal::get_runtime_value(blockCols));
}



/// \returns a expression of a top-right corner of \c *this with either dynamic or fixed sizes.
///
/// \param cRows the number of rows in the corner
/// \param cCols the number of columns in the corner
/// \tparam NRowsType the type of the value handling the number of rows in the block, typically Index.
/// \tparam NColsType the type of the value handling the number of columns in the block, typically Index.
///
/// Example with dynamic sizes: \include MatrixBase_topRightCorner_int_int.cpp
/// Output: \verbinclude MatrixBase_topRightCorner_int_int.out
///
/// The number of rows \a blockRows and columns \a blockCols can also be specified at compile-time by passing Eigen::fix<N>,
/// or Eigen::fix<N>(n) as arguments. See \link block(Index,Index,NRowsType,NColsType) block() \endlink for the details.
///
EIGEN_DOC_BLOCK_ADDONS_NOT_INNER_PANEL
///
/// \sa block(Index,Index,NRowsType,NColsType), class Block
///
template<typename NRowsType, typename NColsType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
typename FixedBlockXpr<internal::get_fixed_value<NRowsType>::value,internal::get_fixed_value<NColsType>::value>::Type
#else
typename FixedBlockXpr<...,...>::Type
#endif
topRightCorner(NRowsType cRows, NColsType cCols)
{
  return typename FixedBlockXpr<internal::get_fixed_value<NRowsType>::value,internal::get_fixed_value<NColsType>::value>::Type
            (derived(), 0, cols() - internal::get_runtime_value(cCols), internal::get_runtime_value(cRows), internal::get_runtime_value(cCols));
}

/// This is the const version of topRightCorner(NRowsType, NColsType).
template<typename NRowsType, typename NColsType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
const typename ConstFixedBlockXpr<internal::get_fixed_value<NRowsType>::value,internal::get_fixed_value<NColsType>::value>::Type
#else
const typename ConstFixedBlockXpr<...,...>::Type
#endif
topRightCorner(NRowsType cRows, NColsType cCols) const
{
  return typename ConstFixedBlockXpr<internal::get_fixed_value<NRowsType>::value,internal::get_fixed_value<NColsType>::value>::Type
            (derived(), 0, cols() - internal::get_runtime_value(cCols), internal::get_runtime_value(cRows), internal::get_runtime_value(cCols));
}

/// \returns an expression of a fixed-size top-right corner of \c *this.
///
/// \tparam CRows the number of rows in the corner
/// \tparam CCols the number of columns in the corner
///
/// Example: \include MatrixBase_template_int_int_topRightCorner.cpp
/// Output: \verbinclude MatrixBase_template_int_int_topRightCorner.out
///
EIGEN_DOC_BLOCK_ADDONS_NOT_INNER_PANEL
///
/// \sa class Block, block<int,int>(Index,Index)
///
template<int CRows, int CCols>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename FixedBlockXpr<CRows,CCols>::Type topRightCorner()
{
  return typename FixedBlockXpr<CRows,CCols>::Type(derived(), 0, cols() - CCols);
}

/// This is the const version of topRightCorner<int, int>().
template<int CRows, int CCols>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
const typename ConstFixedBlockXpr<CRows,CCols>::Type topRightCorner() const
{
  return typename ConstFixedBlockXpr<CRows,CCols>::Type(derived(), 0, cols() - CCols);
}

/// \returns an expression of a top-right corner of \c *this.
///
/// \tparam CRows number of rows in corner as specified at compile-time
/// \tparam CCols number of columns in corner as specified at compile-time
/// \param  cRows number of rows in corner as specified at run-time
/// \param  cCols number of columns in corner as specified at run-time
///
/// This function is mainly useful for corners where the number of rows is specified at compile-time
/// and the number of columns is specified at run-time, or vice versa. The compile-time and run-time
/// information should not contradict. In other words, \a cRows should equal \a CRows unless
/// \a CRows is \a Dynamic, and the same for the number of columns.
///
/// Example: \include MatrixBase_template_int_int_topRightCorner_int_int.cpp
/// Output: \verbinclude MatrixBase_template_int_int_topRightCorner_int_int.out
///
EIGEN_DOC_BLOCK_ADDONS_NOT_INNER_PANEL
///
/// \sa class Block
///
template<int CRows, int CCols>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename FixedBlockXpr<CRows,CCols>::Type topRightCorner(Index cRows, Index cCols)
{
  return typename FixedBlockXpr<CRows,CCols>::Type(derived(), 0, cols() - cCols, cRows, cCols);
}

/// This is the const version of topRightCorner<int, int>(Index, Index).
template<int CRows, int CCols>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
const typename ConstFixedBlockXpr<CRows,CCols>::Type topRightCorner(Index cRows, Index cCols) const
{
  return typename ConstFixedBlockXpr<CRows,CCols>::Type(derived(), 0, cols() - cCols, cRows, cCols);
}



/// \returns an expression of a top-left corner of \c *this  with either dynamic or fixed sizes.
///
/// \param cRows the number of rows in the corner
/// \param cCols the number of columns in the corner
/// \tparam NRowsType the type of the value handling the number of rows in the block, typically Index.
/// \tparam NColsType the type of the value handling the number of columns in the block, typically Index.
///
/// Example: \include MatrixBase_topLeftCorner_int_int.cpp
/// Output: \verbinclude MatrixBase_topLeftCorner_int_int.out
///
/// The number of rows \a blockRows and columns \a blockCols can also be specified at compile-time by passing Eigen::fix<N>,
/// or Eigen::fix<N>(n) as arguments. See \link block(Index,Index,NRowsType,NColsType) block() \endlink for the details.
///
EIGEN_DOC_BLOCK_ADDONS_NOT_INNER_PANEL
///
/// \sa block(Index,Index,NRowsType,NColsType), class Block
///
template<typename NRowsType, typename NColsType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
typename FixedBlockXpr<internal::get_fixed_value<NRowsType>::value,internal::get_fixed_value<NColsType>::value>::Type
#else
typename FixedBlockXpr<...,...>::Type
#endif
topLeftCorner(NRowsType cRows, NColsType cCols)
{
  return typename FixedBlockXpr<internal::get_fixed_value<NRowsType>::value,internal::get_fixed_value<NColsType>::value>::Type
            (derived(), 0, 0, internal::get_runtime_value(cRows), internal::get_runtime_value(cCols));
}

/// This is the const version of topLeftCorner(Index, Index).
template<typename NRowsType, typename NColsType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
const typename ConstFixedBlockXpr<internal::get_fixed_value<NRowsType>::value,internal::get_fixed_value<NColsType>::value>::Type
#else
const typename ConstFixedBlockXpr<...,...>::Type
#endif
topLeftCorner(NRowsType cRows, NColsType cCols) const
{
  return typename ConstFixedBlockXpr<internal::get_fixed_value<NRowsType>::value,internal::get_fixed_value<NColsType>::value>::Type
            (derived(), 0, 0, internal::get_runtime_value(cRows), internal::get_runtime_value(cCols));
}

/// \returns an expression of a fixed-size top-left corner of \c *this.
///
/// The template parameters CRows and CCols are the number of rows and columns in the corner.
///
/// Example: \include MatrixBase_template_int_int_topLeftCorner.cpp
/// Output: \verbinclude MatrixBase_template_int_int_topLeftCorner.out
///
EIGEN_DOC_BLOCK_ADDONS_NOT_INNER_PANEL
///
/// \sa block(Index,Index,NRowsType,NColsType), class Block
///
template<int CRows, int CCols>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename FixedBlockXpr<CRows,CCols>::Type topLeftCorner()
{
  return typename FixedBlockXpr<CRows,CCols>::Type(derived(), 0, 0);
}

/// This is the const version of topLeftCorner<int, int>().
template<int CRows, int CCols>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
const typename ConstFixedBlockXpr<CRows,CCols>::Type topLeftCorner() const
{
  return typename ConstFixedBlockXpr<CRows,CCols>::Type(derived(), 0, 0);
}

/// \returns an expression of a top-left corner of \c *this.
///
/// \tparam CRows number of rows in corner as specified at compile-time
/// \tparam CCols number of columns in corner as specified at compile-time
/// \param  cRows number of rows in corner as specified at run-time
/// \param  cCols number of columns in corner as specified at run-time
///
/// This function is mainly useful for corners where the number of rows is specified at compile-time
/// and the number of columns is specified at run-time, or vice versa. The compile-time and run-time
/// information should not contradict. In other words, \a cRows should equal \a CRows unless
/// \a CRows is \a Dynamic, and the same for the number of columns.
///
/// Example: \include MatrixBase_template_int_int_topLeftCorner_int_int.cpp
/// Output: \verbinclude MatrixBase_template_int_int_topLeftCorner_int_int.out
///
EIGEN_DOC_BLOCK_ADDONS_NOT_INNER_PANEL
///
/// \sa class Block
///
template<int CRows, int CCols>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename FixedBlockXpr<CRows,CCols>::Type topLeftCorner(Index cRows, Index cCols)
{
  return typename FixedBlockXpr<CRows,CCols>::Type(derived(), 0, 0, cRows, cCols);
}

/// This is the const version of topLeftCorner<int, int>(Index, Index).
template<int CRows, int CCols>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
const typename ConstFixedBlockXpr<CRows,CCols>::Type topLeftCorner(Index cRows, Index cCols) const
{
  return typename ConstFixedBlockXpr<CRows,CCols>::Type(derived(), 0, 0, cRows, cCols);
}



/// \returns an expression of a bottom-right corner of \c *this  with either dynamic or fixed sizes.
///
/// \param cRows the number of rows in the corner
/// \param cCols the number of columns in the corner
/// \tparam NRowsType the type of the value handling the number of rows in the block, typically Index.
/// \tparam NColsType the type of the value handling the number of columns in the block, typically Index.
///
/// Example: \include MatrixBase_bottomRightCorner_int_int.cpp
/// Output: \verbinclude MatrixBase_bottomRightCorner_int_int.out
///
/// The number of rows \a blockRows and columns \a blockCols can also be specified at compile-time by passing Eigen::fix<N>,
/// or Eigen::fix<N>(n) as arguments. See \link block(Index,Index,NRowsType,NColsType) block() \endlink for the details.
///
EIGEN_DOC_BLOCK_ADDONS_NOT_INNER_PANEL
///
/// \sa block(Index,Index,NRowsType,NColsType), class Block
///
template<typename NRowsType, typename NColsType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
typename FixedBlockXpr<internal::get_fixed_value<NRowsType>::value,internal::get_fixed_value<NColsType>::value>::Type
#else
typename FixedBlockXpr<...,...>::Type
#endif
bottomRightCorner(NRowsType cRows, NColsType cCols)
{
  return typename FixedBlockXpr<internal::get_fixed_value<NRowsType>::value,internal::get_fixed_value<NColsType>::value>::Type
            (derived(), rows() - internal::get_runtime_value(cRows), cols() - internal::get_runtime_value(cCols),
                        internal::get_runtime_value(cRows), internal::get_runtime_value(cCols));
}

/// This is the const version of bottomRightCorner(NRowsType, NColsType).
template<typename NRowsType, typename NColsType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
const typename ConstFixedBlockXpr<internal::get_fixed_value<NRowsType>::value,internal::get_fixed_value<NColsType>::value>::Type
#else
const typename ConstFixedBlockXpr<...,...>::Type
#endif
bottomRightCorner(NRowsType cRows, NColsType cCols) const
{
  return typename ConstFixedBlockXpr<internal::get_fixed_value<NRowsType>::value,internal::get_fixed_value<NColsType>::value>::Type
            (derived(), rows() - internal::get_runtime_value(cRows), cols() - internal::get_runtime_value(cCols),
                        internal::get_runtime_value(cRows), internal::get_runtime_value(cCols));
}

/// \returns an expression of a fixed-size bottom-right corner of \c *this.
///
/// The template parameters CRows and CCols are the number of rows and columns in the corner.
///
/// Example: \include MatrixBase_template_int_int_bottomRightCorner.cpp
/// Output: \verbinclude MatrixBase_template_int_int_bottomRightCorner.out
///
EIGEN_DOC_BLOCK_ADDONS_NOT_INNER_PANEL
///
/// \sa block(Index,Index,NRowsType,NColsType), class Block
///
template<int CRows, int CCols>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename FixedBlockXpr<CRows,CCols>::Type bottomRightCorner()
{
  return typename FixedBlockXpr<CRows,CCols>::Type(derived(), rows() - CRows, cols() - CCols);
}

/// This is the const version of bottomRightCorner<int, int>().
template<int CRows, int CCols>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
const typename ConstFixedBlockXpr<CRows,CCols>::Type bottomRightCorner() const
{
  return typename ConstFixedBlockXpr<CRows,CCols>::Type(derived(), rows() - CRows, cols() - CCols);
}

/// \returns an expression of a bottom-right corner of \c *this.
///
/// \tparam CRows number of rows in corner as specified at compile-time
/// \tparam CCols number of columns in corner as specified at compile-time
/// \param  cRows number of rows in corner as specified at run-time
/// \param  cCols number of columns in corner as specified at run-time
///
/// This function is mainly useful for corners where the number of rows is specified at compile-time
/// and the number of columns is specified at run-time, or vice versa. The compile-time and run-time
/// information should not contradict. In other words, \a cRows should equal \a CRows unless
/// \a CRows is \a Dynamic, and the same for the number of columns.
///
/// Example: \include MatrixBase_template_int_int_bottomRightCorner_int_int.cpp
/// Output: \verbinclude MatrixBase_template_int_int_bottomRightCorner_int_int.out
///
EIGEN_DOC_BLOCK_ADDONS_NOT_INNER_PANEL
///
/// \sa class Block
///
template<int CRows, int CCols>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename FixedBlockXpr<CRows,CCols>::Type bottomRightCorner(Index cRows, Index cCols)
{
  return typename FixedBlockXpr<CRows,CCols>::Type(derived(), rows() - cRows, cols() - cCols, cRows, cCols);
}

/// This is the const version of bottomRightCorner<int, int>(Index, Index).
template<int CRows, int CCols>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
const typename ConstFixedBlockXpr<CRows,CCols>::Type bottomRightCorner(Index cRows, Index cCols) const
{
  return typename ConstFixedBlockXpr<CRows,CCols>::Type(derived(), rows() - cRows, cols() - cCols, cRows, cCols);
}



/// \returns an expression of a bottom-left corner of \c *this  with either dynamic or fixed sizes.
///
/// \param cRows the number of rows in the corner
/// \param cCols the number of columns in the corner
/// \tparam NRowsType the type of the value handling the number of rows in the block, typically Index.
/// \tparam NColsType the type of the value handling the number of columns in the block, typically Index.
///
/// Example: \include MatrixBase_bottomLeftCorner_int_int.cpp
/// Output: \verbinclude MatrixBase_bottomLeftCorner_int_int.out
///
/// The number of rows \a blockRows and columns \a blockCols can also be specified at compile-time by passing Eigen::fix<N>,
/// or Eigen::fix<N>(n) as arguments. See \link block(Index,Index,NRowsType,NColsType) block() \endlink for the details.
///
EIGEN_DOC_BLOCK_ADDONS_NOT_INNER_PANEL
///
/// \sa block(Index,Index,NRowsType,NColsType), class Block
///
template<typename NRowsType, typename NColsType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
typename FixedBlockXpr<internal::get_fixed_value<NRowsType>::value,internal::get_fixed_value<NColsType>::value>::Type
#else
typename FixedBlockXpr<...,...>::Type
#endif
bottomLeftCorner(NRowsType cRows, NColsType cCols)
{
  return typename FixedBlockXpr<internal::get_fixed_value<NRowsType>::value,internal::get_fixed_value<NColsType>::value>::Type
            (derived(), rows() - internal::get_runtime_value(cRows), 0,
                        internal::get_runtime_value(cRows), internal::get_runtime_value(cCols));
}

/// This is the const version of bottomLeftCorner(NRowsType, NColsType).
template<typename NRowsType, typename NColsType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
typename ConstFixedBlockXpr<internal::get_fixed_value<NRowsType>::value,internal::get_fixed_value<NColsType>::value>::Type
#else
typename ConstFixedBlockXpr<...,...>::Type
#endif
bottomLeftCorner(NRowsType cRows, NColsType cCols) const
{
  return typename ConstFixedBlockXpr<internal::get_fixed_value<NRowsType>::value,internal::get_fixed_value<NColsType>::value>::Type
            (derived(), rows() - internal::get_runtime_value(cRows), 0,
                        internal::get_runtime_value(cRows), internal::get_runtime_value(cCols));
}

/// \returns an expression of a fixed-size bottom-left corner of \c *this.
///
/// The template parameters CRows and CCols are the number of rows and columns in the corner.
///
/// Example: \include MatrixBase_template_int_int_bottomLeftCorner.cpp
/// Output: \verbinclude MatrixBase_template_int_int_bottomLeftCorner.out
///
EIGEN_DOC_BLOCK_ADDONS_NOT_INNER_PANEL
///
/// \sa block(Index,Index,NRowsType,NColsType), class Block
///
template<int CRows, int CCols>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename FixedBlockXpr<CRows,CCols>::Type bottomLeftCorner()
{
  return typename FixedBlockXpr<CRows,CCols>::Type(derived(), rows() - CRows, 0);
}

/// This is the const version of bottomLeftCorner<int, int>().
template<int CRows, int CCols>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
const typename ConstFixedBlockXpr<CRows,CCols>::Type bottomLeftCorner() const
{
  return typename ConstFixedBlockXpr<CRows,CCols>::Type(derived(), rows() - CRows, 0);
}

/// \returns an expression of a bottom-left corner of \c *this.
///
/// \tparam CRows number of rows in corner as specified at compile-time
/// \tparam CCols number of columns in corner as specified at compile-time
/// \param  cRows number of rows in corner as specified at run-time
/// \param  cCols number of columns in corner as specified at run-time
///
/// This function is mainly useful for corners where the number of rows is specified at compile-time
/// and the number of columns is specified at run-time, or vice versa. The compile-time and run-time
/// information should not contradict. In other words, \a cRows should equal \a CRows unless
/// \a CRows is \a Dynamic, and the same for the number of columns.
///
/// Example: \include MatrixBase_template_int_int_bottomLeftCorner_int_int.cpp
/// Output: \verbinclude MatrixBase_template_int_int_bottomLeftCorner_int_int.out
///
EIGEN_DOC_BLOCK_ADDONS_NOT_INNER_PANEL
///
/// \sa class Block
///
template<int CRows, int CCols>
EIGEN_STRONG_INLINE
typename FixedBlockXpr<CRows,CCols>::Type bottomLeftCorner(Index cRows, Index cCols)
{
  return typename FixedBlockXpr<CRows,CCols>::Type(derived(), rows() - cRows, 0, cRows, cCols);
}

/// This is the const version of bottomLeftCorner<int, int>(Index, Index).
template<int CRows, int CCols>
EIGEN_STRONG_INLINE
const typename ConstFixedBlockXpr<CRows,CCols>::Type bottomLeftCorner(Index cRows, Index cCols) const
{
  return typename ConstFixedBlockXpr<CRows,CCols>::Type(derived(), rows() - cRows, 0, cRows, cCols);
}



/// \returns a block consisting of the top rows of \c *this.
///
/// \param n the number of rows in the block
/// \tparam NRowsType the type of the value handling the number of rows in the block, typically Index.
///
/// Example: \include MatrixBase_topRows_int.cpp
/// Output: \verbinclude MatrixBase_topRows_int.out
///
/// The number of rows \a n can also be specified at compile-time by passing Eigen::fix<N>,
/// or Eigen::fix<N>(n) as arguments.
/// See \link block(Index,Index,NRowsType,NColsType) block() \endlink for the details.
///
EIGEN_DOC_BLOCK_ADDONS_INNER_PANEL_IF(row-major)
///
/// \sa block(Index,Index,NRowsType,NColsType), class Block
///
template<typename NRowsType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
typename NRowsBlockXpr<internal::get_fixed_value<NRowsType>::value>::Type
#else
typename NRowsBlockXpr<...>::Type
#endif
topRows(NRowsType n)
{
  return typename NRowsBlockXpr<internal::get_fixed_value<NRowsType>::value>::Type
            (derived(), 0, 0, internal::get_runtime_value(n), cols());
}

/// This is the const version of topRows(NRowsType).
template<typename NRowsType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
const typename ConstNRowsBlockXpr<internal::get_fixed_value<NRowsType>::value>::Type
#else
const typename ConstNRowsBlockXpr<...>::Type
#endif
topRows(NRowsType n) const
{
  return typename ConstNRowsBlockXpr<internal::get_fixed_value<NRowsType>::value>::Type
            (derived(), 0, 0, internal::get_runtime_value(n), cols());
}

/// \returns a block consisting of the top rows of \c *this.
///
/// \tparam N the number of rows in the block as specified at compile-time
/// \param n the number of rows in the block as specified at run-time
///
/// The compile-time and run-time information should not contradict. In other words,
/// \a n should equal \a N unless \a N is \a Dynamic.
///
/// Example: \include MatrixBase_template_int_topRows.cpp
/// Output: \verbinclude MatrixBase_template_int_topRows.out
///
EIGEN_DOC_BLOCK_ADDONS_INNER_PANEL_IF(row-major)
///
/// \sa block(Index,Index,NRowsType,NColsType), class Block
///
template<int N>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename NRowsBlockXpr<N>::Type topRows(Index n = N)
{
  return typename NRowsBlockXpr<N>::Type(derived(), 0, 0, n, cols());
}

/// This is the const version of topRows<int>().
template<int N>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename ConstNRowsBlockXpr<N>::Type topRows(Index n = N) const
{
  return typename ConstNRowsBlockXpr<N>::Type(derived(), 0, 0, n, cols());
}



/// \returns a block consisting of the bottom rows of \c *this.
///
/// \param n the number of rows in the block
/// \tparam NRowsType the type of the value handling the number of rows in the block, typically Index.
///
/// Example: \include MatrixBase_bottomRows_int.cpp
/// Output: \verbinclude MatrixBase_bottomRows_int.out
///
/// The number of rows \a n can also be specified at compile-time by passing Eigen::fix<N>,
/// or Eigen::fix<N>(n) as arguments.
/// See \link block(Index,Index,NRowsType,NColsType) block() \endlink for the details.
///
EIGEN_DOC_BLOCK_ADDONS_INNER_PANEL_IF(row-major)
///
/// \sa block(Index,Index,NRowsType,NColsType), class Block
///
template<typename NRowsType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
typename NRowsBlockXpr<internal::get_fixed_value<NRowsType>::value>::Type
#else
typename NRowsBlockXpr<...>::Type
#endif
bottomRows(NRowsType n)
{
  return typename NRowsBlockXpr<internal::get_fixed_value<NRowsType>::value>::Type
            (derived(), rows() - internal::get_runtime_value(n), 0, internal::get_runtime_value(n), cols());
}

/// This is the const version of bottomRows(NRowsType).
template<typename NRowsType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
const typename ConstNRowsBlockXpr<internal::get_fixed_value<NRowsType>::value>::Type
#else
const typename ConstNRowsBlockXpr<...>::Type
#endif
bottomRows(NRowsType n) const
{
  return typename ConstNRowsBlockXpr<internal::get_fixed_value<NRowsType>::value>::Type
            (derived(), rows() - internal::get_runtime_value(n), 0, internal::get_runtime_value(n), cols());
}

/// \returns a block consisting of the bottom rows of \c *this.
///
/// \tparam N the number of rows in the block as specified at compile-time
/// \param n the number of rows in the block as specified at run-time
///
/// The compile-time and run-time information should not contradict. In other words,
/// \a n should equal \a N unless \a N is \a Dynamic.
///
/// Example: \include MatrixBase_template_int_bottomRows.cpp
/// Output: \verbinclude MatrixBase_template_int_bottomRows.out
///
EIGEN_DOC_BLOCK_ADDONS_INNER_PANEL_IF(row-major)
///
/// \sa block(Index,Index,NRowsType,NColsType), class Block
///
template<int N>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename NRowsBlockXpr<N>::Type bottomRows(Index n = N)
{
  return typename NRowsBlockXpr<N>::Type(derived(), rows() - n, 0, n, cols());
}

/// This is the const version of bottomRows<int>().
template<int N>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename ConstNRowsBlockXpr<N>::Type bottomRows(Index n = N) const
{
  return typename ConstNRowsBlockXpr<N>::Type(derived(), rows() - n, 0, n, cols());
}



/// \returns a block consisting of a range of rows of \c *this.
///
/// \param startRow the index of the first row in the block
/// \param n the number of rows in the block
/// \tparam NRowsType the type of the value handling the number of rows in the block, typically Index.
///
/// Example: \include DenseBase_middleRows_int.cpp
/// Output: \verbinclude DenseBase_middleRows_int.out
///
/// The number of rows \a n can also be specified at compile-time by passing Eigen::fix<N>,
/// or Eigen::fix<N>(n) as arguments.
/// See \link block(Index,Index,NRowsType,NColsType) block() \endlink for the details.
///
EIGEN_DOC_BLOCK_ADDONS_INNER_PANEL_IF(row-major)
///
/// \sa block(Index,Index,NRowsType,NColsType), class Block
///
template<typename NRowsType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
typename NRowsBlockXpr<internal::get_fixed_value<NRowsType>::value>::Type
#else
typename NRowsBlockXpr<...>::Type
#endif
middleRows(Index startRow, NRowsType n)
{
  return typename NRowsBlockXpr<internal::get_fixed_value<NRowsType>::value>::Type
            (derived(), startRow, 0, internal::get_runtime_value(n), cols());
}

/// This is the const version of middleRows(Index,NRowsType).
template<typename NRowsType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
const typename ConstNRowsBlockXpr<internal::get_fixed_value<NRowsType>::value>::Type
#else
const typename ConstNRowsBlockXpr<...>::Type
#endif
middleRows(Index startRow, NRowsType n) const
{
  return typename ConstNRowsBlockXpr<internal::get_fixed_value<NRowsType>::value>::Type
            (derived(), startRow, 0, internal::get_runtime_value(n), cols());
}

/// \returns a block consisting of a range of rows of \c *this.
///
/// \tparam N the number of rows in the block as specified at compile-time
/// \param startRow the index of the first row in the block
/// \param n the number of rows in the block as specified at run-time
///
/// The compile-time and run-time information should not contradict. In other words,
/// \a n should equal \a N unless \a N is \a Dynamic.
///
/// Example: \include DenseBase_template_int_middleRows.cpp
/// Output: \verbinclude DenseBase_template_int_middleRows.out
///
EIGEN_DOC_BLOCK_ADDONS_INNER_PANEL_IF(row-major)
///
/// \sa block(Index,Index,NRowsType,NColsType), class Block
///
template<int N>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename NRowsBlockXpr<N>::Type middleRows(Index startRow, Index n = N)
{
  return typename NRowsBlockXpr<N>::Type(derived(), startRow, 0, n, cols());
}

/// This is the const version of middleRows<int>().
template<int N>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename ConstNRowsBlockXpr<N>::Type middleRows(Index startRow, Index n = N) const
{
  return typename ConstNRowsBlockXpr<N>::Type(derived(), startRow, 0, n, cols());
}



/// \returns a block consisting of the left columns of \c *this.
///
/// \param n the number of columns in the block
/// \tparam NColsType the type of the value handling the number of columns in the block, typically Index.
///
/// Example: \include MatrixBase_leftCols_int.cpp
/// Output: \verbinclude MatrixBase_leftCols_int.out
///
/// The number of columns \a n can also be specified at compile-time by passing Eigen::fix<N>,
/// or Eigen::fix<N>(n) as arguments.
/// See \link block(Index,Index,NRowsType,NColsType) block() \endlink for the details.
///
EIGEN_DOC_BLOCK_ADDONS_INNER_PANEL_IF(column-major)
///
/// \sa block(Index,Index,NRowsType,NColsType), class Block
///
template<typename NColsType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
typename NColsBlockXpr<internal::get_fixed_value<NColsType>::value>::Type
#else
typename NColsBlockXpr<...>::Type
#endif
leftCols(NColsType n)
{
  return typename NColsBlockXpr<internal::get_fixed_value<NColsType>::value>::Type
            (derived(), 0, 0, rows(), internal::get_runtime_value(n));
}

/// This is the const version of leftCols(NColsType).
template<typename NColsType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
const typename ConstNColsBlockXpr<internal::get_fixed_value<NColsType>::value>::Type
#else
const typename ConstNColsBlockXpr<...>::Type
#endif
leftCols(NColsType n) const
{
  return typename ConstNColsBlockXpr<internal::get_fixed_value<NColsType>::value>::Type
            (derived(), 0, 0, rows(), internal::get_runtime_value(n));
}

/// \returns a block consisting of the left columns of \c *this.
///
/// \tparam N the number of columns in the block as specified at compile-time
/// \param n the number of columns in the block as specified at run-time
///
/// The compile-time and run-time information should not contradict. In other words,
/// \a n should equal \a N unless \a N is \a Dynamic.
///
/// Example: \include MatrixBase_template_int_leftCols.cpp
/// Output: \verbinclude MatrixBase_template_int_leftCols.out
///
EIGEN_DOC_BLOCK_ADDONS_INNER_PANEL_IF(column-major)
///
/// \sa block(Index,Index,NRowsType,NColsType), class Block
///
template<int N>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename NColsBlockXpr<N>::Type leftCols(Index n = N)
{
  return typename NColsBlockXpr<N>::Type(derived(), 0, 0, rows(), n);
}

/// This is the const version of leftCols<int>().
template<int N>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename ConstNColsBlockXpr<N>::Type leftCols(Index n = N) const
{
  return typename ConstNColsBlockXpr<N>::Type(derived(), 0, 0, rows(), n);
}



/// \returns a block consisting of the right columns of \c *this.
///
/// \param n the number of columns in the block
/// \tparam NColsType the type of the value handling the number of columns in the block, typically Index.
///
/// Example: \include MatrixBase_rightCols_int.cpp
/// Output: \verbinclude MatrixBase_rightCols_int.out
///
/// The number of columns \a n can also be specified at compile-time by passing Eigen::fix<N>,
/// or Eigen::fix<N>(n) as arguments.
/// See \link block(Index,Index,NRowsType,NColsType) block() \endlink for the details.
///
EIGEN_DOC_BLOCK_ADDONS_INNER_PANEL_IF(column-major)
///
/// \sa block(Index,Index,NRowsType,NColsType), class Block
///
template<typename NColsType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
typename NColsBlockXpr<internal::get_fixed_value<NColsType>::value>::Type
#else
typename NColsBlockXpr<...>::Type
#endif
rightCols(NColsType n)
{
  return typename NColsBlockXpr<internal::get_fixed_value<NColsType>::value>::Type
            (derived(), 0, cols() - internal::get_runtime_value(n), rows(), internal::get_runtime_value(n));
}

/// This is the const version of rightCols(NColsType).
template<typename NColsType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
const typename ConstNColsBlockXpr<internal::get_fixed_value<NColsType>::value>::Type
#else
const typename ConstNColsBlockXpr<...>::Type
#endif
rightCols(NColsType n) const
{
  return typename ConstNColsBlockXpr<internal::get_fixed_value<NColsType>::value>::Type
            (derived(), 0, cols() - internal::get_runtime_value(n), rows(), internal::get_runtime_value(n));
}

/// \returns a block consisting of the right columns of \c *this.
///
/// \tparam N the number of columns in the block as specified at compile-time
/// \param n the number of columns in the block as specified at run-time
///
/// The compile-time and run-time information should not contradict. In other words,
/// \a n should equal \a N unless \a N is \a Dynamic.
///
/// Example: \include MatrixBase_template_int_rightCols.cpp
/// Output: \verbinclude MatrixBase_template_int_rightCols.out
///
EIGEN_DOC_BLOCK_ADDONS_INNER_PANEL_IF(column-major)
///
/// \sa block(Index,Index,NRowsType,NColsType), class Block
///
template<int N>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename NColsBlockXpr<N>::Type rightCols(Index n = N)
{
  return typename NColsBlockXpr<N>::Type(derived(), 0, cols() - n, rows(), n);
}

/// This is the const version of rightCols<int>().
template<int N>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename ConstNColsBlockXpr<N>::Type rightCols(Index n = N) const
{
  return typename ConstNColsBlockXpr<N>::Type(derived(), 0, cols() - n, rows(), n);
}



/// \returns a block consisting of a range of columns of \c *this.
///
/// \param startCol the index of the first column in the block
/// \param numCols the number of columns in the block
/// \tparam NColsType the type of the value handling the number of columns in the block, typically Index.
///
/// Example: \include DenseBase_middleCols_int.cpp
/// Output: \verbinclude DenseBase_middleCols_int.out
///
/// The number of columns \a n can also be specified at compile-time by passing Eigen::fix<N>,
/// or Eigen::fix<N>(n) as arguments.
/// See \link block(Index,Index,NRowsType,NColsType) block() \endlink for the details.
///
EIGEN_DOC_BLOCK_ADDONS_INNER_PANEL_IF(column-major)
///
/// \sa block(Index,Index,NRowsType,NColsType), class Block
///
template<typename NColsType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
typename NColsBlockXpr<internal::get_fixed_value<NColsType>::value>::Type
#else
typename NColsBlockXpr<...>::Type
#endif
middleCols(Index startCol, NColsType numCols)
{
  return typename NColsBlockXpr<internal::get_fixed_value<NColsType>::value>::Type
            (derived(), 0, startCol, rows(), internal::get_runtime_value(numCols));
}

/// This is the const version of middleCols(Index,NColsType).
template<typename NColsType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
const typename ConstNColsBlockXpr<internal::get_fixed_value<NColsType>::value>::Type
#else
const typename ConstNColsBlockXpr<...>::Type
#endif
middleCols(Index startCol, NColsType numCols) const
{
  return typename ConstNColsBlockXpr<internal::get_fixed_value<NColsType>::value>::Type
            (derived(), 0, startCol, rows(), internal::get_runtime_value(numCols));
}

/// \returns a block consisting of a range of columns of \c *this.
///
/// \tparam N the number of columns in the block as specified at compile-time
/// \param startCol the index of the first column in the block
/// \param n the number of columns in the block as specified at run-time
///
/// The compile-time and run-time information should not contradict. In other words,
/// \a n should equal \a N unless \a N is \a Dynamic.
///
/// Example: \include DenseBase_template_int_middleCols.cpp
/// Output: \verbinclude DenseBase_template_int_middleCols.out
///
EIGEN_DOC_BLOCK_ADDONS_INNER_PANEL_IF(column-major)
///
/// \sa block(Index,Index,NRowsType,NColsType), class Block
///
template<int N>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename NColsBlockXpr<N>::Type middleCols(Index startCol, Index n = N)
{
  return typename NColsBlockXpr<N>::Type(derived(), 0, startCol, rows(), n);
}

/// This is the const version of middleCols<int>().
template<int N>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename ConstNColsBlockXpr<N>::Type middleCols(Index startCol, Index n = N) const
{
  return typename ConstNColsBlockXpr<N>::Type(derived(), 0, startCol, rows(), n);
}



/// \returns a fixed-size expression of a block of \c *this.
///
/// The template parameters \a NRows and \a NCols are the number of
/// rows and columns in the block.
///
/// \param startRow the first row in the block
/// \param startCol the first column in the block
///
/// Example: \include MatrixBase_block_int_int.cpp
/// Output: \verbinclude MatrixBase_block_int_int.out
///
/// \note The usage of of this overload is discouraged from %Eigen 3.4, better used the generic
/// block(Index,Index,NRowsType,NColsType), here is the one-to-one equivalence:
/// \code
/// mat.template block<NRows,NCols>(i,j)  <-->  mat.block(i,j,fix<NRows>,fix<NCols>)
/// \endcode
///
/// \note since block is a templated member, the keyword template has to be used
/// if the matrix type is also a template parameter: \code m.template block<3,3>(1,1); \endcode
///
EIGEN_DOC_BLOCK_ADDONS_NOT_INNER_PANEL
///
/// \sa block(Index,Index,NRowsType,NColsType), class Block
///
template<int NRows, int NCols>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename FixedBlockXpr<NRows,NCols>::Type block(Index startRow, Index startCol)
{
  return typename FixedBlockXpr<NRows,NCols>::Type(derived(), startRow, startCol);
}

/// This is the const version of block<>(Index, Index). */
template<int NRows, int NCols>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
const typename ConstFixedBlockXpr<NRows,NCols>::Type block(Index startRow, Index startCol) const
{
  return typename ConstFixedBlockXpr<NRows,NCols>::Type(derived(), startRow, startCol);
}

/// \returns an expression of a block of \c *this.
///
/// \tparam NRows number of rows in block as specified at compile-time
/// \tparam NCols number of columns in block as specified at compile-time
/// \param  startRow  the first row in the block
/// \param  startCol  the first column in the block
/// \param  blockRows number of rows in block as specified at run-time
/// \param  blockCols number of columns in block as specified at run-time
///
/// This function is mainly useful for blocks where the number of rows is specified at compile-time
/// and the number of columns is specified at run-time, or vice versa. The compile-time and run-time
/// information should not contradict. In other words, \a blockRows should equal \a NRows unless
/// \a NRows is \a Dynamic, and the same for the number of columns.
///
/// Example: \include MatrixBase_template_int_int_block_int_int_int_int.cpp
/// Output: \verbinclude MatrixBase_template_int_int_block_int_int_int_int.out
///
/// \note The usage of of this overload is discouraged from %Eigen 3.4, better used the generic
/// block(Index,Index,NRowsType,NColsType), here is the one-to-one complete equivalence:
/// \code
/// mat.template block<NRows,NCols>(i,j,rows,cols)     <-->  mat.block(i,j,fix<NRows>(rows),fix<NCols>(cols))
/// \endcode
/// If we known that, e.g., NRows==Dynamic and NCols!=Dynamic, then the equivalence becomes:
/// \code
/// mat.template block<Dynamic,NCols>(i,j,rows,NCols)  <-->  mat.block(i,j,rows,fix<NCols>)
/// \endcode
///
EIGEN_DOC_BLOCK_ADDONS_NOT_INNER_PANEL
///
/// \sa block(Index,Index,NRowsType,NColsType), class Block
///
template<int NRows, int NCols>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename FixedBlockXpr<NRows,NCols>::Type block(Index startRow, Index startCol,
                                                  Index blockRows, Index blockCols)
{
  return typename FixedBlockXpr<NRows,NCols>::Type(derived(), startRow, startCol, blockRows, blockCols);
}

/// This is the const version of block<>(Index, Index, Index, Index).
template<int NRows, int NCols>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
const typename ConstFixedBlockXpr<NRows,NCols>::Type block(Index startRow, Index startCol,
                                                              Index blockRows, Index blockCols) const
{
  return typename ConstFixedBlockXpr<NRows,NCols>::Type(derived(), startRow, startCol, blockRows, blockCols);
}

/// \returns an expression of the \a i-th column of \c *this. Note that the numbering starts at 0.
///
/// Example: \include MatrixBase_col.cpp
/// Output: \verbinclude MatrixBase_col.out
///
EIGEN_DOC_BLOCK_ADDONS_INNER_PANEL_IF(column-major)
/**
  * \sa row(), class Block */
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
ColXpr col(Index i)
{
  return ColXpr(derived(), i);
}

/// This is the const version of col().
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
ConstColXpr col(Index i) const
{
  return ConstColXpr(derived(), i);
}

/// \returns an expression of the \a i-th row of \c *this. Note that the numbering starts at 0.
///
/// Example: \include MatrixBase_row.cpp
/// Output: \verbinclude MatrixBase_row.out
///
EIGEN_DOC_BLOCK_ADDONS_INNER_PANEL_IF(row-major)
/**
  * \sa col(), class Block */
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
RowXpr row(Index i)
{
  return RowXpr(derived(), i);
}

/// This is the const version of row(). */
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
ConstRowXpr row(Index i) const
{
  return ConstRowXpr(derived(), i);
}

/// \returns an expression of a segment (i.e. a vector block) in \c *this with either dynamic or fixed sizes.
///
/// \only_for_vectors
///
/// \param start the first coefficient in the segment
/// \param n the number of coefficients in the segment
/// \tparam NType the type of the value handling the number of coefficients in the segment, typically Index.
///
/// Example: \include MatrixBase_segment_int_int.cpp
/// Output: \verbinclude MatrixBase_segment_int_int.out
///
/// The number of coefficients \a n can also be specified at compile-time by passing Eigen::fix<N>,
/// or Eigen::fix<N>(n) as arguments.
/// See \link block(Index,Index,NRowsType,NColsType) block() \endlink for the details.
///
/// \note Even in the case that the returned expression has dynamic size, in the case
/// when it is applied to a fixed-size vector, it inherits a fixed maximal size,
/// which means that evaluating it does not cause a dynamic memory allocation.
///
/// \sa block(Index,Index,NRowsType,NColsType), fix<N>, fix<N>(int), class Block
///
template<typename NType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
typename FixedSegmentReturnType<internal::get_fixed_value<NType>::value>::Type
#else
typename FixedSegmentReturnType<...>::Type
#endif
segment(Index start, NType n)
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return typename FixedSegmentReturnType<internal::get_fixed_value<NType>::value>::Type
            (derived(), start, internal::get_runtime_value(n));
}


/// This is the const version of segment(Index,NType).
template<typename NType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
const typename ConstFixedSegmentReturnType<internal::get_fixed_value<NType>::value>::Type
#else
const typename ConstFixedSegmentReturnType<...>::Type
#endif
segment(Index start, NType n) const
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return typename ConstFixedSegmentReturnType<internal::get_fixed_value<NType>::value>::Type
            (derived(), start, internal::get_runtime_value(n));
}

/// \returns an expression of the first coefficients of \c *this with either dynamic or fixed sizes.
///
/// \only_for_vectors
///
/// \param n the number of coefficients in the segment
/// \tparam NType the type of the value handling the number of coefficients in the segment, typically Index.
///
/// Example: \include MatrixBase_start_int.cpp
/// Output: \verbinclude MatrixBase_start_int.out
///
/// The number of coefficients \a n can also be specified at compile-time by passing Eigen::fix<N>,
/// or Eigen::fix<N>(n) as arguments.
/// See \link block(Index,Index,NRowsType,NColsType) block() \endlink for the details.
///
/// \note Even in the case that the returned expression has dynamic size, in the case
/// when it is applied to a fixed-size vector, it inherits a fixed maximal size,
/// which means that evaluating it does not cause a dynamic memory allocation.
///
/// \sa class Block, block(Index,Index)
///
template<typename NType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
typename FixedSegmentReturnType<internal::get_fixed_value<NType>::value>::Type
#else
typename FixedSegmentReturnType<...>::Type
#endif
head(NType n)
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return typename FixedSegmentReturnType<internal::get_fixed_value<NType>::value>::Type
              (derived(), 0, internal::get_runtime_value(n));
}

/// This is the const version of head(NType).
template<typename NType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
const typename ConstFixedSegmentReturnType<internal::get_fixed_value<NType>::value>::Type
#else
const typename ConstFixedSegmentReturnType<...>::Type
#endif
head(NType n) const
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return typename ConstFixedSegmentReturnType<internal::get_fixed_value<NType>::value>::Type
            (derived(), 0, internal::get_runtime_value(n));
}

/// \returns an expression of a last coefficients of \c *this with either dynamic or fixed sizes.
///
/// \only_for_vectors
///
/// \param n the number of coefficients in the segment
/// \tparam NType the type of the value handling the number of coefficients in the segment, typically Index.
///
/// Example: \include MatrixBase_end_int.cpp
/// Output: \verbinclude MatrixBase_end_int.out
///
/// The number of coefficients \a n can also be specified at compile-time by passing Eigen::fix<N>,
/// or Eigen::fix<N>(n) as arguments.
/// See \link block(Index,Index,NRowsType,NColsType) block() \endlink for the details.
///
/// \note Even in the case that the returned expression has dynamic size, in the case
/// when it is applied to a fixed-size vector, it inherits a fixed maximal size,
/// which means that evaluating it does not cause a dynamic memory allocation.
///
/// \sa class Block, block(Index,Index)
///
template<typename NType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
typename FixedSegmentReturnType<internal::get_fixed_value<NType>::value>::Type
#else
typename FixedSegmentReturnType<...>::Type
#endif
tail(NType n)
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return typename FixedSegmentReturnType<internal::get_fixed_value<NType>::value>::Type
            (derived(), this->size() - internal::get_runtime_value(n), internal::get_runtime_value(n));
}

/// This is the const version of tail(Index).
template<typename NType>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
#ifndef EIGEN_PARSED_BY_DOXYGEN
const typename ConstFixedSegmentReturnType<internal::get_fixed_value<NType>::value>::Type
#else
const typename ConstFixedSegmentReturnType<...>::Type
#endif
tail(NType n) const
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return typename ConstFixedSegmentReturnType<internal::get_fixed_value<NType>::value>::Type
            (derived(), this->size() - internal::get_runtime_value(n), internal::get_runtime_value(n));
}

/// \returns a fixed-size expression of a segment (i.e. a vector block) in \c *this
///
/// \only_for_vectors
///
/// \tparam N the number of coefficients in the segment as specified at compile-time
/// \param start the index of the first element in the segment
/// \param n the number of coefficients in the segment as specified at compile-time
///
/// The compile-time and run-time information should not contradict. In other words,
/// \a n should equal \a N unless \a N is \a Dynamic.
///
/// Example: \include MatrixBase_template_int_segment.cpp
/// Output: \verbinclude MatrixBase_template_int_segment.out
///
/// \sa segment(Index,NType), class Block
///
template<int N>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename FixedSegmentReturnType<N>::Type segment(Index start, Index n = N)
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return typename FixedSegmentReturnType<N>::Type(derived(), start, n);
}

/// This is the const version of segment<int>(Index).
template<int N>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename ConstFixedSegmentReturnType<N>::Type segment(Index start, Index n = N) const
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return typename ConstFixedSegmentReturnType<N>::Type(derived(), start, n);
}

/// \returns a fixed-size expression of the first coefficients of \c *this.
///
/// \only_for_vectors
///
/// \tparam N the number of coefficients in the segment as specified at compile-time
/// \param  n the number of coefficients in the segment as specified at run-time
///
/// The compile-time and run-time information should not contradict. In other words,
/// \a n should equal \a N unless \a N is \a Dynamic.
///
/// Example: \include MatrixBase_template_int_start.cpp
/// Output: \verbinclude MatrixBase_template_int_start.out
///
/// \sa head(NType), class Block
///
template<int N>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename FixedSegmentReturnType<N>::Type head(Index n = N)
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return typename FixedSegmentReturnType<N>::Type(derived(), 0, n);
}

/// This is the const version of head<int>().
template<int N>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename ConstFixedSegmentReturnType<N>::Type head(Index n = N) const
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return typename ConstFixedSegmentReturnType<N>::Type(derived(), 0, n);
}

/// \returns a fixed-size expression of the last coefficients of \c *this.
///
/// \only_for_vectors
///
/// \tparam N the number of coefficients in the segment as specified at compile-time
/// \param  n the number of coefficients in the segment as specified at run-time
///
/// The compile-time and run-time information should not contradict. In other words,
/// \a n should equal \a N unless \a N is \a Dynamic.
///
/// Example: \include MatrixBase_template_int_end.cpp
/// Output: \verbinclude MatrixBase_template_int_end.out
///
/// \sa tail(NType), class Block
///
template<int N>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename FixedSegmentReturnType<N>::Type tail(Index n = N)
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return typename FixedSegmentReturnType<N>::Type(derived(), size() - n);
}

/// This is the const version of tail<int>.
template<int N>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
typename ConstFixedSegmentReturnType<N>::Type tail(Index n = N) const
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return typename ConstFixedSegmentReturnType<N>::Type(derived(), size() - n);
}

/// \returns the \a outer -th column (resp. row) of the matrix \c *this if \c *this
/// is col-major (resp. row-major).
///
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
InnerVectorReturnType innerVector(Index outer)
{ return InnerVectorReturnType(derived(), outer); }

/// \returns the \a outer -th column (resp. row) of the matrix \c *this if \c *this
/// is col-major (resp. row-major). Read-only.
///
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
const ConstInnerVectorReturnType innerVector(Index outer) const
{ return ConstInnerVectorReturnType(derived(), outer); }

/// \returns the \a outer -th column (resp. row) of the matrix \c *this if \c *this
/// is col-major (resp. row-major).
///
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
InnerVectorsReturnType
innerVectors(Index outerStart, Index outerSize)
{
  return Block<Derived,Dynamic,Dynamic,true>(derived(),
                                             IsRowMajor ? outerStart : 0, IsRowMajor ? 0 : outerStart,
                                             IsRowMajor ? outerSize : rows(), IsRowMajor ? cols() : outerSize);

}

/// \returns the \a outer -th column (resp. row) of the matrix \c *this if \c *this
/// is col-major (resp. row-major). Read-only.
///
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
const ConstInnerVectorsReturnType
innerVectors(Index outerStart, Index outerSize) const
{
  return Block<const Derived,Dynamic,Dynamic,true>(derived(),
                                                  IsRowMajor ? outerStart : 0, IsRowMajor ? 0 : outerStart,
                                                  IsRowMajor ? outerSize : rows(), IsRowMajor ? cols() : outerSize);

}

/** \returns the i-th subvector (column or vector) according to the \c Direction
  * \sa subVectors()
  */
template<DirectionType Direction>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
std::conditional_t<Direction==Vertical,ColXpr,RowXpr>
subVector(Index i)
{
  return std::conditional_t<Direction==Vertical,ColXpr,RowXpr>(derived(),i);
}

/** This is the const version of subVector(Index) */
template<DirectionType Direction>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
std::conditional_t<Direction==Vertical,ConstColXpr,ConstRowXpr>
subVector(Index i) const
{
  return std::conditional_t<Direction==Vertical,ConstColXpr,ConstRowXpr>(derived(),i);
}

/** \returns the number of subvectors (rows or columns) in the direction \c Direction
  * \sa subVector(Index)
  */
template<DirectionType Direction>
EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE EIGEN_CONSTEXPR
Index subVectors() const
{ return (Direction==Vertical)?cols():rows(); }

#   ifdef EIGEN_SPARSEMATRIXBASE_PLUGIN
#     include EIGEN_SPARSEMATRIXBASE_PLUGIN
#   endif
#undef EIGEN_CURRENT_STORAGE_BASE_CLASS
#undef EIGEN_DOC_UNARY_ADDONS
#undef EIGEN_DOC_BLOCK_ADDONS_NOT_INNER_PANEL
#undef EIGEN_DOC_BLOCK_ADDONS_INNER_PANEL_IF

    /** \returns the number of rows. \sa cols() */
    inline Index rows() const { return derived().rows(); }
    /** \returns the number of columns. \sa rows() */
    inline Index cols() const { return derived().cols(); }
    /** \returns the number of coefficients, which is \a rows()*cols().
      * \sa rows(), cols(). */
    inline Index size() const { return rows() * cols(); }
    /** \returns true if either the number of rows or the number of columns is equal to 1.
      * In other words, this function returns
      * \code rows()==1 || cols()==1 \endcode
      * \sa rows(), cols(), IsVectorAtCompileTime. */
    inline bool isVector() const { return rows()==1 || cols()==1; }
    /** \returns the size of the storage major dimension,
      * i.e., the number of columns for a columns major matrix, and the number of rows otherwise */
    Index outerSize() const { return (int(Flags)&RowMajorBit) ? this->rows() : this->cols(); }
    /** \returns the size of the inner dimension according to the storage order,
      * i.e., the number of rows for a columns major matrix, and the number of cols otherwise */
    Index innerSize() const { return (int(Flags)&RowMajorBit) ? this->cols() : this->rows(); }

    bool isRValue() const { return m_isRValue; }
    Derived& markAsRValue() { m_isRValue = true; return derived(); }

    SparseMatrixBase() : m_isRValue(false) { /* TODO check flags */ }

    
    template<typename OtherDerived>
    Derived& operator=(const ReturnByValue<OtherDerived>& other);

    template<typename OtherDerived>
    inline Derived& operator=(const SparseMatrixBase<OtherDerived>& other);

    inline Derived& operator=(const Derived& other);

  protected:

    template<typename OtherDerived>
    inline Derived& assign(const OtherDerived& other);

    template<typename OtherDerived>
    inline void assignGeneric(const OtherDerived& other);

  public:
#ifndef EIGEN_NO_IO
    friend std::ostream & operator << (std::ostream & s, const SparseMatrixBase& m)
    {
      typedef typename Derived::Nested Nested;
      typedef internal::remove_all_t<Nested> NestedCleaned;

      if (Flags&RowMajorBit)
      {
        Nested nm(m.derived());
        internal::evaluator<NestedCleaned> thisEval(nm);
        for (Index row=0; row<nm.outerSize(); ++row)
        {
          Index col = 0;
          for (typename internal::evaluator<NestedCleaned>::InnerIterator it(thisEval, row); it; ++it)
          {
            for ( ; col<it.index(); ++col)
              s << "0 ";
            s << it.value() << " ";
            ++col;
          }
          for ( ; col<m.cols(); ++col)
            s << "0 ";
          s << std::endl;
        }
      }
      else
      {
        Nested nm(m.derived());
        internal::evaluator<NestedCleaned> thisEval(nm);
        if (m.cols() == 1) {
          Index row = 0;
          for (typename internal::evaluator<NestedCleaned>::InnerIterator it(thisEval, 0); it; ++it)
          {
            for ( ; row<it.index(); ++row)
              s << "0" << std::endl;
            s << it.value() << std::endl;
            ++row;
          }
          for ( ; row<m.rows(); ++row)
            s << "0" << std::endl;
        }
        else
        {
          SparseMatrix<Scalar, RowMajorBit, StorageIndex> trans = m;
          s << static_cast<const SparseMatrixBase<SparseMatrix<Scalar, RowMajorBit, StorageIndex> >&>(trans);
        }
      }
      return s;
    }
#endif

    template<typename OtherDerived>
    Derived& operator+=(const SparseMatrixBase<OtherDerived>& other);
    template<typename OtherDerived>
    Derived& operator-=(const SparseMatrixBase<OtherDerived>& other);
    
    template<typename OtherDerived>
    Derived& operator+=(const DiagonalBase<OtherDerived>& other);
    template<typename OtherDerived>
    Derived& operator-=(const DiagonalBase<OtherDerived>& other);

    template<typename OtherDerived>
    Derived& operator+=(const EigenBase<OtherDerived> &other);
    template<typename OtherDerived>
    Derived& operator-=(const EigenBase<OtherDerived> &other);

    Derived& operator*=(const Scalar& other);
    Derived& operator/=(const Scalar& other);

    template<typename OtherDerived> struct CwiseProductDenseReturnType {
      typedef CwiseBinaryOp<internal::scalar_product_op<typename ScalarBinaryOpTraits<
                                                          typename internal::traits<Derived>::Scalar,
                                                          typename internal::traits<OtherDerived>::Scalar
                                                        >::ReturnType>,
                            const Derived,
                            const OtherDerived
                          > Type;
    };

    template<typename OtherDerived>
    EIGEN_STRONG_INLINE const typename CwiseProductDenseReturnType<OtherDerived>::Type
    cwiseProduct(const MatrixBase<OtherDerived> &other) const;

    // sparse * diagonal
    template<typename OtherDerived>
    const Product<Derived,OtherDerived>
    operator*(const DiagonalBase<OtherDerived> &other) const
    { return Product<Derived,OtherDerived>(derived(), other.derived()); }

    // diagonal * sparse
    template<typename OtherDerived> friend
    const Product<OtherDerived,Derived>
    operator*(const DiagonalBase<OtherDerived> &lhs, const SparseMatrixBase& rhs)
    { return Product<OtherDerived,Derived>(lhs.derived(), rhs.derived()); }
    
    // sparse * sparse
    template<typename OtherDerived>
    const Product<Derived,OtherDerived,AliasFreeProduct>
    operator*(const SparseMatrixBase<OtherDerived> &other) const;
    
    // sparse * dense
    template<typename OtherDerived>
    const Product<Derived,OtherDerived>
    operator*(const MatrixBase<OtherDerived> &other) const
    { return Product<Derived,OtherDerived>(derived(), other.derived()); }
    
    // dense * sparse
    template<typename OtherDerived> friend
    const Product<OtherDerived,Derived>
    operator*(const MatrixBase<OtherDerived> &lhs, const SparseMatrixBase& rhs)
    { return Product<OtherDerived,Derived>(lhs.derived(), rhs.derived()); }
    
     /** \returns an expression of P H P^-1 where H is the matrix represented by \c *this */
    SparseSymmetricPermutationProduct<Derived,Upper|Lower> twistedBy(const PermutationMatrix<Dynamic,Dynamic,StorageIndex>& perm) const
    {
      return SparseSymmetricPermutationProduct<Derived,Upper|Lower>(derived(), perm);
    }

    template<typename OtherDerived>
    Derived& operator*=(const SparseMatrixBase<OtherDerived>& other);

    template<int Mode>
    inline const TriangularView<const Derived, Mode> triangularView() const;
    
    template<unsigned int UpLo> struct SelfAdjointViewReturnType { typedef SparseSelfAdjointView<Derived, UpLo> Type; };
    template<unsigned int UpLo> struct ConstSelfAdjointViewReturnType { typedef const SparseSelfAdjointView<const Derived, UpLo> Type; };

    template<unsigned int UpLo> inline 
    typename ConstSelfAdjointViewReturnType<UpLo>::Type selfadjointView() const;
    template<unsigned int UpLo> inline
    typename SelfAdjointViewReturnType<UpLo>::Type selfadjointView();

    template<typename OtherDerived> Scalar dot(const MatrixBase<OtherDerived>& other) const;
    template<typename OtherDerived> Scalar dot(const SparseMatrixBase<OtherDerived>& other) const;
    RealScalar squaredNorm() const;
    RealScalar norm()  const;
    RealScalar blueNorm() const;

    TransposeReturnType transpose() { return TransposeReturnType(derived()); }
    const ConstTransposeReturnType transpose() const { return ConstTransposeReturnType(derived()); }
    const AdjointReturnType adjoint() const { return AdjointReturnType(transpose()); }

    DenseMatrixType toDense() const
    {
      return DenseMatrixType(derived());
    }

    template<typename OtherDerived>
    bool isApprox(const SparseMatrixBase<OtherDerived>& other,
                  const RealScalar& prec = NumTraits<Scalar>::dummy_precision()) const;

    template<typename OtherDerived>
    bool isApprox(const MatrixBase<OtherDerived>& other,
                  const RealScalar& prec = NumTraits<Scalar>::dummy_precision()) const
    { return toDense().isApprox(other,prec); }

    /** \returns the matrix or vector obtained by evaluating this expression.
      *
      * Notice that in the case of a plain matrix or vector (not an expression) this function just returns
      * a const reference, in order to avoid a useless copy.
      */
    inline const typename internal::eval<Derived>::type eval() const
    { return typename internal::eval<Derived>::type(derived()); }

    Scalar sum() const;
    
    inline const SparseView<Derived>
    pruned(const Scalar& reference = Scalar(0), const RealScalar& epsilon = NumTraits<Scalar>::dummy_precision()) const;

  protected:

    bool m_isRValue;

    static inline StorageIndex convert_index(const Index idx) {
      return internal::convert_index<StorageIndex>(idx);
    }
  private:
    template<typename Dest> void evalTo(Dest &) const;
};

} // end namespace Eigen

#endif // EIGEN_SPARSEMATRIXBASE_H
