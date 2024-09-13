// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2007-2010 Benoit Jacob <jacob.benoit.1@gmail.com>
// Copyright (C) 2008-2010 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef EIGEN_DENSEBASE_H
#define EIGEN_DENSEBASE_H

// IWYU pragma: private
#include "./InternalHeaderCheck.h"

namespace Eigen {

// The index type defined by EIGEN_DEFAULT_DENSE_INDEX_TYPE must be a signed type.
EIGEN_STATIC_ASSERT(NumTraits<DenseIndex>::IsSigned,THE_INDEX_TYPE_MUST_BE_A_SIGNED_TYPE)

/** \class DenseBase
  * \ingroup Core_Module
  *
  * \brief Base class for all dense matrices, vectors, and arrays
  *
  * This class is the base that is inherited by all dense objects (matrix, vector, arrays,
  * and related expression types). The common Eigen API for dense objects is contained in this class.
  *
  * \tparam Derived is the derived type, e.g., a matrix type or an expression.
  *
  * This class can be extended with the help of the plugin mechanism described on the page
  * \ref TopicCustomizing_Plugins by defining the preprocessor symbol \c EIGEN_DENSEBASE_PLUGIN.
  *
  * \sa \blank \ref TopicClassHierarchy
  */
template<typename Derived> class DenseBase
#ifndef EIGEN_PARSED_BY_DOXYGEN
  : public DenseCoeffsBase<Derived, internal::accessors_level<Derived>::value>
#else
  : public DenseCoeffsBase<Derived,DirectWriteAccessors>
#endif // not EIGEN_PARSED_BY_DOXYGEN
{
  public:

    /** Inner iterator type to iterate over the coefficients of a row or column.
      * \sa class InnerIterator
      */
    typedef Eigen::InnerIterator<Derived> InnerIterator;

    typedef typename internal::traits<Derived>::StorageKind StorageKind;

    /**
      * \brief The type used to store indices
      * \details This typedef is relevant for types that store multiple indices such as
      *          PermutationMatrix or Transpositions, otherwise it defaults to Eigen::Index
      * \sa \blank \ref TopicPreprocessorDirectives, Eigen::Index, SparseMatrixBase.
     */
    typedef typename internal::traits<Derived>::StorageIndex StorageIndex;

    /** The numeric type of the expression' coefficients, e.g. float, double, int or std::complex<float>, etc. */
    typedef typename internal::traits<Derived>::Scalar Scalar;

    /** The numeric type of the expression' coefficients, e.g. float, double, int or std::complex<float>, etc.
      *
      * It is an alias for the Scalar type */
    typedef Scalar value_type;

    typedef typename NumTraits<Scalar>::Real RealScalar;
    typedef DenseCoeffsBase<Derived, internal::accessors_level<Derived>::value> Base;

    using Base::derived;
    using Base::const_cast_derived;
    using Base::rows;
    using Base::cols;
    using Base::size;
    using Base::rowIndexByOuterInner;
    using Base::colIndexByOuterInner;
    using Base::coeff;
    using Base::coeffByOuterInner;
    using Base::operator();
    using Base::operator[];
    using Base::x;
    using Base::y;
    using Base::z;
    using Base::w;
    using Base::stride;
    using Base::innerStride;
    using Base::outerStride;
    using Base::rowStride;
    using Base::colStride;
    typedef typename Base::CoeffReturnType CoeffReturnType;

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

      MaxRowsAtCompileTime = internal::traits<Derived>::MaxRowsAtCompileTime,
        /**< This value is equal to the maximum possible number of rows that this expression
          * might have. If this expression might have an arbitrarily high number of rows,
          * this value is set to \a Dynamic.
          *
          * This value is useful to know when evaluating an expression, in order to determine
          * whether it is possible to avoid doing a dynamic memory allocation.
          *
          * \sa RowsAtCompileTime, MaxColsAtCompileTime, MaxSizeAtCompileTime
          */

      MaxColsAtCompileTime = internal::traits<Derived>::MaxColsAtCompileTime,
        /**< This value is equal to the maximum possible number of columns that this expression
          * might have. If this expression might have an arbitrarily high number of columns,
          * this value is set to \a Dynamic.
          *
          * This value is useful to know when evaluating an expression, in order to determine
          * whether it is possible to avoid doing a dynamic memory allocation.
          *
          * \sa ColsAtCompileTime, MaxRowsAtCompileTime, MaxSizeAtCompileTime
          */

      MaxSizeAtCompileTime = internal::size_at_compile_time(internal::traits<Derived>::MaxRowsAtCompileTime,
                                                            internal::traits<Derived>::MaxColsAtCompileTime),
        /**< This value is equal to the maximum possible number of coefficients that this expression
          * might have. If this expression might have an arbitrarily high number of coefficients,
          * this value is set to \a Dynamic.
          *
          * This value is useful to know when evaluating an expression, in order to determine
          * whether it is possible to avoid doing a dynamic memory allocation.
          *
          * \sa SizeAtCompileTime, MaxRowsAtCompileTime, MaxColsAtCompileTime
          */

      IsVectorAtCompileTime = internal::traits<Derived>::RowsAtCompileTime == 1
                           || internal::traits<Derived>::ColsAtCompileTime == 1,
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

      IsRowMajor = int(Flags) & RowMajorBit, /**< True if this expression has row-major storage order. */

      InnerSizeAtCompileTime = int(IsVectorAtCompileTime) ? int(SizeAtCompileTime)
                             : int(IsRowMajor) ? int(ColsAtCompileTime) : int(RowsAtCompileTime),

      InnerStrideAtCompileTime = internal::inner_stride_at_compile_time<Derived>::ret,
      OuterStrideAtCompileTime = internal::outer_stride_at_compile_time<Derived>::ret
    };

    typedef typename internal::find_best_packet<Scalar,SizeAtCompileTime>::type PacketScalar;

    enum { IsPlainObjectBase = 0 };

    /** The plain matrix type corresponding to this expression.
      * \sa PlainObject */
    typedef Matrix<typename internal::traits<Derived>::Scalar,
                internal::traits<Derived>::RowsAtCompileTime,
                internal::traits<Derived>::ColsAtCompileTime,
                AutoAlign | (internal::traits<Derived>::Flags&RowMajorBit ? RowMajor : ColMajor),
                internal::traits<Derived>::MaxRowsAtCompileTime,
                internal::traits<Derived>::MaxColsAtCompileTime
          > PlainMatrix;

    /** The plain array type corresponding to this expression.
      * \sa PlainObject */
    typedef Array<typename internal::traits<Derived>::Scalar,
                internal::traits<Derived>::RowsAtCompileTime,
                internal::traits<Derived>::ColsAtCompileTime,
                AutoAlign | (internal::traits<Derived>::Flags&RowMajorBit ? RowMajor : ColMajor),
                internal::traits<Derived>::MaxRowsAtCompileTime,
                internal::traits<Derived>::MaxColsAtCompileTime
          > PlainArray;

    /** \brief The plain matrix or array type corresponding to this expression.
      *
      * This is not necessarily exactly the return type of eval(). In the case of plain matrices,
      * the return type of eval() is a const reference to a matrix, not a matrix! It is however guaranteed
      * that the return type of eval() is either PlainObject or const PlainObject&.
      */
    typedef std::conditional_t<internal::is_same<typename internal::traits<Derived>::XprKind,MatrixXpr >::value,
                                 PlainMatrix, PlainArray> PlainObject;

    /** \returns the outer size.
      *
      * \note For a vector, this returns just 1. For a matrix (non-vector), this is the major dimension
      * with respect to the \ref TopicStorageOrders "storage order", i.e., the number of columns for a
      * column-major matrix, and the number of rows for a row-major matrix. */
    EIGEN_DEVICE_FUNC EIGEN_CONSTEXPR
    Index outerSize() const
    {
      return IsVectorAtCompileTime ? 1
           : int(IsRowMajor) ? this->rows() : this->cols();
    }

    /** \returns the inner size.
      *
      * \note For a vector, this is just the size. For a matrix (non-vector), this is the minor dimension
      * with respect to the \ref TopicStorageOrders "storage order", i.e., the number of rows for a
      * column-major matrix, and the number of columns for a row-major matrix. */
    EIGEN_DEVICE_FUNC EIGEN_CONSTEXPR
    Index innerSize() const
    {
      return IsVectorAtCompileTime ? this->size()
           : int(IsRowMajor) ? this->cols() : this->rows();
    }

    /** Only plain matrices/arrays, not expressions, may be resized; therefore the only useful resize methods are
      * Matrix::resize() and Array::resize(). The present method only asserts that the new size equals the old size, and does
      * nothing else.
      */
    EIGEN_DEVICE_FUNC
    void resize(Index newSize)
    {
      EIGEN_ONLY_USED_FOR_DEBUG(newSize);
      eigen_assert(newSize == this->size()
                && "DenseBase::resize() does not actually allow to resize.");
    }
    /** Only plain matrices/arrays, not expressions, may be resized; therefore the only useful resize methods are
      * Matrix::resize() and Array::resize(). The present method only asserts that the new size equals the old size, and does
      * nothing else.
      */
    EIGEN_DEVICE_FUNC
    void resize(Index rows, Index cols)
    {
      EIGEN_ONLY_USED_FOR_DEBUG(rows);
      EIGEN_ONLY_USED_FOR_DEBUG(cols);
      eigen_assert(rows == this->rows() && cols == this->cols()
                && "DenseBase::resize() does not actually allow to resize.");
    }

#ifndef EIGEN_PARSED_BY_DOXYGEN
    /** \internal Represents a matrix with all coefficients equal to one another*/
    typedef CwiseNullaryOp<internal::scalar_constant_op<Scalar>,PlainObject> ConstantReturnType;
    /** \internal \deprecated Represents a vector with linearly spaced coefficients that allows sequential access only. */
    EIGEN_DEPRECATED typedef CwiseNullaryOp<internal::linspaced_op<Scalar>,PlainObject> SequentialLinSpacedReturnType;
    /** \internal Represents a vector with linearly spaced coefficients that allows random access. */
    typedef CwiseNullaryOp<internal::linspaced_op<Scalar>,PlainObject> RandomAccessLinSpacedReturnType;
    /** \internal Represents a vector with equally spaced coefficients that allows random access. */
    typedef CwiseNullaryOp<internal::equalspaced_op<Scalar>, PlainObject> RandomAccessEqualSpacedReturnType;
    /** \internal the return type of MatrixBase::eigenvalues() */
    typedef Matrix<typename NumTraits<typename internal::traits<Derived>::Scalar>::Real, internal::traits<Derived>::ColsAtCompileTime, 1> EigenvaluesReturnType;

#endif // not EIGEN_PARSED_BY_DOXYGEN

    /** Copies \a other into *this. \returns a reference to *this. */
    template<typename OtherDerived>
    EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
    Derived& operator=(const DenseBase<OtherDerived>& other);

    /** Special case of the template operator=, in order to prevent the compiler
      * from generating a default operator= (issue hit with g++ 4.1)
      */
    EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
    Derived& operator=(const DenseBase& other);

    template<typename OtherDerived>
    EIGEN_DEVICE_FUNC
    Derived& operator=(const EigenBase<OtherDerived> &other);

    template<typename OtherDerived>
    EIGEN_DEVICE_FUNC
    Derived& operator+=(const EigenBase<OtherDerived> &other);

    template<typename OtherDerived>
    EIGEN_DEVICE_FUNC
    Derived& operator-=(const EigenBase<OtherDerived> &other);

    template<typename OtherDerived>
    EIGEN_DEVICE_FUNC
    Derived& operator=(const ReturnByValue<OtherDerived>& func);

    /** \internal
      * Copies \a other into *this without evaluating other. \returns a reference to *this. */
    template<typename OtherDerived>
    /** \deprecated */
    EIGEN_DEPRECATED EIGEN_DEVICE_FUNC
    Derived& lazyAssign(const DenseBase<OtherDerived>& other);

    EIGEN_DEVICE_FUNC
    CommaInitializer<Derived> operator<< (const Scalar& s);

    template<unsigned int Added,unsigned int Removed>
    /** \deprecated it now returns \c *this */
    EIGEN_DEPRECATED
    const Derived& flagged() const
    { return derived(); }

    template<typename OtherDerived>
    EIGEN_DEVICE_FUNC
    CommaInitializer<Derived> operator<< (const DenseBase<OtherDerived>& other);

    typedef Transpose<Derived> TransposeReturnType;
    EIGEN_DEVICE_FUNC
    TransposeReturnType transpose();
    typedef Transpose<const Derived> ConstTransposeReturnType;
    EIGEN_DEVICE_FUNC
    const ConstTransposeReturnType transpose() const;
    EIGEN_DEVICE_FUNC
    void transposeInPlace();

    EIGEN_DEVICE_FUNC static const ConstantReturnType
    Constant(Index rows, Index cols, const Scalar& value);
    EIGEN_DEVICE_FUNC static const ConstantReturnType
    Constant(Index size, const Scalar& value);
    EIGEN_DEVICE_FUNC static const ConstantReturnType
    Constant(const Scalar& value);

    EIGEN_DEPRECATED EIGEN_DEVICE_FUNC static const RandomAccessLinSpacedReturnType
    LinSpaced(Sequential_t, Index size, const Scalar& low, const Scalar& high);
    EIGEN_DEPRECATED EIGEN_DEVICE_FUNC static const RandomAccessLinSpacedReturnType
    LinSpaced(Sequential_t, const Scalar& low, const Scalar& high);

    EIGEN_DEVICE_FUNC static const RandomAccessLinSpacedReturnType
    LinSpaced(Index size, const Scalar& low, const Scalar& high);
    EIGEN_DEVICE_FUNC static const RandomAccessLinSpacedReturnType
    LinSpaced(const Scalar& low, const Scalar& high);

    EIGEN_DEVICE_FUNC static const RandomAccessEqualSpacedReturnType
    EqualSpaced(Index size, const Scalar& low, const Scalar& step);
    EIGEN_DEVICE_FUNC static const RandomAccessEqualSpacedReturnType
    EqualSpaced(const Scalar& low, const Scalar& step);

    template<typename CustomNullaryOp> EIGEN_DEVICE_FUNC
    static const CwiseNullaryOp<CustomNullaryOp, PlainObject>
    NullaryExpr(Index rows, Index cols, const CustomNullaryOp& func);
    template<typename CustomNullaryOp> EIGEN_DEVICE_FUNC
    static const CwiseNullaryOp<CustomNullaryOp, PlainObject>
    NullaryExpr(Index size, const CustomNullaryOp& func);
    template<typename CustomNullaryOp> EIGEN_DEVICE_FUNC
    static const CwiseNullaryOp<CustomNullaryOp, PlainObject>
    NullaryExpr(const CustomNullaryOp& func);

    EIGEN_DEVICE_FUNC static const ConstantReturnType Zero(Index rows, Index cols);
    EIGEN_DEVICE_FUNC static const ConstantReturnType Zero(Index size);
    EIGEN_DEVICE_FUNC static const ConstantReturnType Zero();
    EIGEN_DEVICE_FUNC static const ConstantReturnType Ones(Index rows, Index cols);
    EIGEN_DEVICE_FUNC static const ConstantReturnType Ones(Index size);
    EIGEN_DEVICE_FUNC static const ConstantReturnType Ones();

    EIGEN_DEVICE_FUNC void fill(const Scalar& value);
    EIGEN_DEVICE_FUNC Derived& setConstant(const Scalar& value);
    EIGEN_DEVICE_FUNC Derived& setLinSpaced(Index size, const Scalar& low, const Scalar& high);
    EIGEN_DEVICE_FUNC Derived& setLinSpaced(const Scalar& low, const Scalar& high);
    EIGEN_DEVICE_FUNC Derived& setEqualSpaced(Index size, const Scalar& low, const Scalar& step);
    EIGEN_DEVICE_FUNC Derived& setEqualSpaced(const Scalar& low, const Scalar& step);
    EIGEN_DEVICE_FUNC Derived& setZero();
    EIGEN_DEVICE_FUNC Derived& setOnes();
    EIGEN_DEVICE_FUNC Derived& setRandom();

    template<typename OtherDerived> EIGEN_DEVICE_FUNC
    bool isApprox(const DenseBase<OtherDerived>& other,
                  const RealScalar& prec = NumTraits<Scalar>::dummy_precision()) const;
    EIGEN_DEVICE_FUNC
    bool isMuchSmallerThan(const RealScalar& other,
                           const RealScalar& prec = NumTraits<Scalar>::dummy_precision()) const;
    template<typename OtherDerived> EIGEN_DEVICE_FUNC
    bool isMuchSmallerThan(const DenseBase<OtherDerived>& other,
                           const RealScalar& prec = NumTraits<Scalar>::dummy_precision()) const;

    EIGEN_DEVICE_FUNC bool isApproxToConstant(const Scalar& value, const RealScalar& prec = NumTraits<Scalar>::dummy_precision()) const;
    EIGEN_DEVICE_FUNC bool isConstant(const Scalar& value, const RealScalar& prec = NumTraits<Scalar>::dummy_precision()) const;
    EIGEN_DEVICE_FUNC bool isZero(const RealScalar& prec = NumTraits<Scalar>::dummy_precision()) const;
    EIGEN_DEVICE_FUNC bool isOnes(const RealScalar& prec = NumTraits<Scalar>::dummy_precision()) const;

    EIGEN_DEVICE_FUNC inline bool hasNaN() const;
    EIGEN_DEVICE_FUNC inline bool allFinite() const;

    EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
    Derived& operator*=(const Scalar& other);
    EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
    Derived& operator/=(const Scalar& other);

    typedef internal::add_const_on_value_type_t<typename internal::eval<Derived>::type> EvalReturnType;
    /** \returns the matrix or vector obtained by evaluating this expression.
      *
      * Notice that in the case of a plain matrix or vector (not an expression) this function just returns
      * a const reference, in order to avoid a useless copy.
      *
      * \warning Be careful with eval() and the auto C++ keyword, as detailed in this \link TopicPitfalls_auto_keyword page \endlink.
      */
    EIGEN_DEVICE_FUNC
    EIGEN_STRONG_INLINE EvalReturnType eval() const
    {
      // Even though MSVC does not honor strong inlining when the return type
      // is a dynamic matrix, we desperately need strong inlining for fixed
      // size types on MSVC.
      return typename internal::eval<Derived>::type(derived());
    }

    /** swaps *this with the expression \a other.
      *
      */
    template<typename OtherDerived>
    EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
    void swap(const DenseBase<OtherDerived>& other)
    {
      EIGEN_STATIC_ASSERT(!OtherDerived::IsPlainObjectBase,THIS_EXPRESSION_IS_NOT_A_LVALUE__IT_IS_READ_ONLY);
      eigen_assert(rows()==other.rows() && cols()==other.cols());
      call_assignment(derived(), other.const_cast_derived(), internal::swap_assign_op<Scalar>());
    }

    /** swaps *this with the matrix or array \a other.
      *
      */
    template<typename OtherDerived>
    EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
    void swap(PlainObjectBase<OtherDerived>& other)
    {
      eigen_assert(rows()==other.rows() && cols()==other.cols());
      call_assignment(derived(), other.derived(), internal::swap_assign_op<Scalar>());
    }

    EIGEN_DEVICE_FUNC inline const NestByValue<Derived> nestByValue() const;
    EIGEN_DEVICE_FUNC inline const ForceAlignedAccess<Derived> forceAlignedAccess() const;
    EIGEN_DEVICE_FUNC inline ForceAlignedAccess<Derived> forceAlignedAccess();
    template<bool Enable> EIGEN_DEVICE_FUNC
    inline const std::conditional_t<Enable,ForceAlignedAccess<Derived>,Derived&> forceAlignedAccessIf() const;
    template<bool Enable> EIGEN_DEVICE_FUNC
    inline std::conditional_t<Enable,ForceAlignedAccess<Derived>,Derived&> forceAlignedAccessIf();

    EIGEN_DEVICE_FUNC Scalar sum() const;
    EIGEN_DEVICE_FUNC Scalar mean() const;
    EIGEN_DEVICE_FUNC Scalar trace() const;

    EIGEN_DEVICE_FUNC Scalar prod() const;

    template<int NaNPropagation>
    EIGEN_DEVICE_FUNC typename internal::traits<Derived>::Scalar minCoeff() const;
    template<int NaNPropagation>
    EIGEN_DEVICE_FUNC typename internal::traits<Derived>::Scalar maxCoeff() const;


    // By default, the fastest version with undefined NaN propagation semantics is
    // used.
    // TODO(rmlarsen): Replace with default template argument when we move to
    // c++11 or beyond.
    EIGEN_DEVICE_FUNC inline typename internal::traits<Derived>::Scalar minCoeff() const {
      return minCoeff<PropagateFast>();
    }
    EIGEN_DEVICE_FUNC inline typename internal::traits<Derived>::Scalar maxCoeff() const {
      return maxCoeff<PropagateFast>();
    }

    template<int NaNPropagation, typename IndexType>
    EIGEN_DEVICE_FUNC
    typename internal::traits<Derived>::Scalar minCoeff(IndexType* row, IndexType* col) const;
    template<int NaNPropagation, typename IndexType>
    EIGEN_DEVICE_FUNC
    typename internal::traits<Derived>::Scalar maxCoeff(IndexType* row, IndexType* col) const;
    template<int NaNPropagation, typename IndexType>
    EIGEN_DEVICE_FUNC
    typename internal::traits<Derived>::Scalar minCoeff(IndexType* index) const;
    template<int NaNPropagation, typename IndexType>
    EIGEN_DEVICE_FUNC
    typename internal::traits<Derived>::Scalar maxCoeff(IndexType* index) const;

    // TODO(rmlarsen): Replace these methods with a default template argument.
    template<typename IndexType>
    EIGEN_DEVICE_FUNC inline
    typename internal::traits<Derived>::Scalar minCoeff(IndexType* row, IndexType* col) const {
      return minCoeff<PropagateFast>(row, col);
    }
    template<typename IndexType>
    EIGEN_DEVICE_FUNC inline
    typename internal::traits<Derived>::Scalar maxCoeff(IndexType* row, IndexType* col) const {
      return maxCoeff<PropagateFast>(row, col);
    }
    template<typename IndexType>
     EIGEN_DEVICE_FUNC inline
    typename internal::traits<Derived>::Scalar minCoeff(IndexType* index) const {
      return minCoeff<PropagateFast>(index);
    }
    template<typename IndexType>
    EIGEN_DEVICE_FUNC inline
    typename internal::traits<Derived>::Scalar maxCoeff(IndexType* index) const {
      return maxCoeff<PropagateFast>(index);
    }
  
    template<typename BinaryOp>
    EIGEN_DEVICE_FUNC
    Scalar redux(const BinaryOp& func) const;

    template<typename Visitor>
    EIGEN_DEVICE_FUNC
    void visit(Visitor& func) const;

    /** \returns a WithFormat proxy object allowing to print a matrix the with given
      * format \a fmt.
      *
      * See class IOFormat for some examples.
      *
      * \sa class IOFormat, class WithFormat
      */
    inline const WithFormat<Derived> format(const IOFormat& fmt) const
    {
      return WithFormat<Derived>(derived(), fmt);
    }

    /** \returns the unique coefficient of a 1x1 expression */
    EIGEN_DEVICE_FUNC
    CoeffReturnType value() const
    {
      EIGEN_STATIC_ASSERT_SIZE_1x1(Derived)
      eigen_assert(this->rows() == 1 && this->cols() == 1);
      return derived().coeff(0,0);
    }

    EIGEN_DEVICE_FUNC bool all() const;
    EIGEN_DEVICE_FUNC bool any() const;
    EIGEN_DEVICE_FUNC Index count() const;

    typedef VectorwiseOp<Derived, Horizontal> RowwiseReturnType;
    typedef const VectorwiseOp<const Derived, Horizontal> ConstRowwiseReturnType;
    typedef VectorwiseOp<Derived, Vertical> ColwiseReturnType;
    typedef const VectorwiseOp<const Derived, Vertical> ConstColwiseReturnType;

    /** \returns a VectorwiseOp wrapper of *this for broadcasting and partial reductions
    *
    * Example: \include MatrixBase_rowwise.cpp
    * Output: \verbinclude MatrixBase_rowwise.out
    *
    * \sa colwise(), class VectorwiseOp, \ref TutorialReductionsVisitorsBroadcasting
    */
    //Code moved here due to a CUDA compiler bug
    EIGEN_DEVICE_FUNC inline ConstRowwiseReturnType rowwise() const {
      return ConstRowwiseReturnType(derived());
    }
    EIGEN_DEVICE_FUNC RowwiseReturnType rowwise();

    /** \returns a VectorwiseOp wrapper of *this broadcasting and partial reductions
    *
    * Example: \include MatrixBase_colwise.cpp
    * Output: \verbinclude MatrixBase_colwise.out
    *
    * \sa rowwise(), class VectorwiseOp, \ref TutorialReductionsVisitorsBroadcasting
    */
    EIGEN_DEVICE_FUNC inline ConstColwiseReturnType colwise() const {
      return ConstColwiseReturnType(derived());
    }
    EIGEN_DEVICE_FUNC ColwiseReturnType colwise();

    typedef CwiseNullaryOp<internal::scalar_random_op<Scalar>,PlainObject> RandomReturnType;
    static const RandomReturnType Random(Index rows, Index cols);
    static const RandomReturnType Random(Index size);
    static const RandomReturnType Random();

    template <typename ThenDerived, typename ElseDerived>
    inline EIGEN_DEVICE_FUNC
        CwiseTernaryOp<internal::scalar_boolean_select_op<typename DenseBase<ThenDerived>::Scalar,
                           typename DenseBase<ElseDerived>::Scalar, Scalar>,
        ThenDerived, ElseDerived, Derived>
        select(const DenseBase<ThenDerived>& thenMatrix, const DenseBase<ElseDerived>& elseMatrix) const;

    template <typename ThenDerived>
    inline EIGEN_DEVICE_FUNC
        CwiseTernaryOp<internal::scalar_boolean_select_op<typename DenseBase<ThenDerived>::Scalar,
                           typename DenseBase<ThenDerived>::Scalar, Scalar>,
        ThenDerived, typename DenseBase<ThenDerived>::ConstantReturnType, Derived>
        select(const DenseBase<ThenDerived>& thenMatrix,
               const typename DenseBase<ThenDerived>::Scalar& elseScalar) const;

    template <typename ElseDerived>
    inline EIGEN_DEVICE_FUNC
        CwiseTernaryOp<internal::scalar_boolean_select_op<typename DenseBase<ElseDerived>::Scalar,
                           typename DenseBase<ElseDerived>::Scalar, Scalar>,
        typename DenseBase<ElseDerived>::ConstantReturnType, ElseDerived, Derived>
        select(const typename DenseBase<ElseDerived>::Scalar& thenScalar,
               const DenseBase<ElseDerived>& elseMatrix) const;

    template<int p> RealScalar lpNorm() const;

    template<int RowFactor, int ColFactor>
    EIGEN_DEVICE_FUNC
    const Replicate<Derived,RowFactor,ColFactor> replicate() const;
    /**
    * \return an expression of the replication of \c *this
    *
    * Example: \include MatrixBase_replicate_int_int.cpp
    * Output: \verbinclude MatrixBase_replicate_int_int.out
    *
    * \sa VectorwiseOp::replicate(), DenseBase::replicate<int,int>(), class Replicate
    */
    //Code moved here due to a CUDA compiler bug
    EIGEN_DEVICE_FUNC
    const Replicate<Derived, Dynamic, Dynamic> replicate(Index rowFactor, Index colFactor) const
    {
      return Replicate<Derived, Dynamic, Dynamic>(derived(), rowFactor, colFactor);
    }

    typedef Reverse<Derived, BothDirections> ReverseReturnType;
    typedef const Reverse<const Derived, BothDirections> ConstReverseReturnType;
    EIGEN_DEVICE_FUNC ReverseReturnType reverse();
    /** This is the const version of reverse(). */
    //Code moved here due to a CUDA compiler bug
    EIGEN_DEVICE_FUNC ConstReverseReturnType reverse() const
    {
      return ConstReverseReturnType(derived());
    }
    EIGEN_DEVICE_FUNC void reverseInPlace();

    #ifdef EIGEN_PARSED_BY_DOXYGEN
    /** STL-like <a href="https://en.cppreference.com/w/cpp/named_req/RandomAccessIterator">RandomAccessIterator</a>
      * iterator type as returned by the begin() and end() methods.
      */
    typedef random_access_iterator_type iterator;
    /** This is the const version of iterator (aka read-only) */
    typedef random_access_iterator_type const_iterator;
    #else
    typedef std::conditional_t< (Flags&DirectAccessBit)==DirectAccessBit,
                                     internal::pointer_based_stl_iterator<Derived>,
                                     internal::generic_randaccess_stl_iterator<Derived>
                                   > iterator_type;

    typedef std::conditional_t< (Flags&DirectAccessBit)==DirectAccessBit,
                                     internal::pointer_based_stl_iterator<const Derived>,
                                     internal::generic_randaccess_stl_iterator<const Derived>
                                   > const_iterator_type;

    // Stl-style iterators are supported only for vectors.

    typedef std::conditional_t<IsVectorAtCompileTime, iterator_type, void> iterator;

    typedef std::conditional_t<IsVectorAtCompileTime, const_iterator_type, void> const_iterator;
    #endif

    inline iterator begin();
    inline const_iterator begin() const;
    inline const_iterator cbegin() const;
    inline iterator end();
    inline const_iterator end() const;
    inline const_iterator cend() const;

#define EIGEN_CURRENT_STORAGE_BASE_CLASS Eigen::DenseBase
#define EIGEN_DOC_BLOCK_ADDONS_NOT_INNER_PANEL
#define EIGEN_DOC_BLOCK_ADDONS_INNER_PANEL_IF(COND)
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


// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2017 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.


#if !defined(EIGEN_PARSED_BY_DOXYGEN)

protected:
// define some aliases to ease readability

template <typename Indices>
using IvcRowType = typename internal::IndexedViewCompatibleType<Indices, RowsAtCompileTime>::type;

template <typename Indices>
using IvcColType = typename internal::IndexedViewCompatibleType<Indices, ColsAtCompileTime>::type;

template <typename Indices>
using IvcType = typename internal::IndexedViewCompatibleType<Indices, SizeAtCompileTime>::type;

typedef typename internal::IndexedViewCompatibleType<Index, 1>::type IvcIndex;

template <typename Indices>
inline IvcRowType<Indices> ivcRow(const Indices& indices) const {
  return internal::makeIndexedViewCompatible(
      indices, internal::variable_if_dynamic<Index, RowsAtCompileTime>(derived().rows()), Specialized);
}

template <typename Indices>
inline IvcColType<Indices> ivcCol(const Indices& indices) const {
  return internal::makeIndexedViewCompatible(
      indices, internal::variable_if_dynamic<Index, ColsAtCompileTime>(derived().cols()), Specialized);
}

template <typename Indices>
inline IvcType<Indices> ivcSize(const Indices& indices) const {
  return internal::makeIndexedViewCompatible(
      indices, internal::variable_if_dynamic<Index, SizeAtCompileTime>(derived().size()), Specialized);
}

// this helper class assumes internal::valid_indexed_view_overload<RowIndices, ColIndices>::value == true
template <typename RowIndices, typename ColIndices,
          bool UseSymbolic = internal::traits<IndexedView<Derived, IvcRowType<RowIndices>, IvcColType<ColIndices>>>::ReturnAsScalar,
          bool UseBlock = internal::traits<IndexedView<Derived, IvcRowType<RowIndices>, IvcColType<ColIndices>>>::ReturnAsBlock,
          bool UseGeneric = internal::traits<IndexedView<Derived, IvcRowType<RowIndices>, IvcColType<ColIndices>>>::ReturnAsIndexedView>
struct IndexedViewSelector;

// Generic
template <typename RowIndices, typename ColIndices>
struct IndexedViewSelector<RowIndices, ColIndices, false, false, true> {
  using ReturnType = IndexedView<Derived, IvcRowType<RowIndices>, IvcColType<ColIndices>>;
  using ConstReturnType = IndexedView<const Derived, IvcRowType<RowIndices>, IvcColType<ColIndices>>;

  static inline ReturnType run(Derived& derived, const RowIndices& rowIndices, const ColIndices& colIndices) {
    return ReturnType(derived, derived.ivcRow(rowIndices), derived.ivcCol(colIndices));
  }
  static inline ConstReturnType run(const Derived& derived, const RowIndices& rowIndices,
                                    const ColIndices& colIndices) {
    return ConstReturnType(derived, derived.ivcRow(rowIndices), derived.ivcCol(colIndices));
  }
};

// Block
template <typename RowIndices, typename ColIndices>
struct IndexedViewSelector<RowIndices, ColIndices, false, true, false> {
  using IndexedViewType = IndexedView<Derived, IvcRowType<RowIndices>, IvcColType<ColIndices>>;
  using ConstIndexedViewType = IndexedView<const Derived, IvcRowType<RowIndices>, IvcColType<ColIndices>>;
  using ReturnType = typename internal::traits<IndexedViewType>::BlockType;
  using ConstReturnType = typename internal::traits<ConstIndexedViewType>::BlockType;

  static inline ReturnType run(Derived& derived, const RowIndices& rowIndices, const ColIndices& colIndices) {
    IvcRowType<RowIndices> actualRowIndices = derived.ivcRow(rowIndices);
    IvcColType<ColIndices> actualColIndices = derived.ivcCol(colIndices);
    return ReturnType(derived, internal::first(actualRowIndices), internal::first(actualColIndices),
                      internal::index_list_size(actualRowIndices), internal::index_list_size(actualColIndices));
  }
  static inline ConstReturnType run(const Derived& derived, const RowIndices& rowIndices,
                                    const ColIndices& colIndices) {
    IvcRowType<RowIndices> actualRowIndices = derived.ivcRow(rowIndices);
    IvcColType<ColIndices> actualColIndices = derived.ivcCol(colIndices);
    return ConstReturnType(derived, internal::first(actualRowIndices), internal::first(actualColIndices),
                           internal::index_list_size(actualRowIndices), internal::index_list_size(actualColIndices));
  }
};

// Symbolic
template <typename RowIndices, typename ColIndices>
struct IndexedViewSelector<RowIndices, ColIndices, true, false, false> {
  using ReturnType = typename DenseBase<Derived>::Scalar&;
  using ConstReturnType = typename DenseBase<Derived>::CoeffReturnType;

  static inline ReturnType run(Derived& derived, const RowIndices& rowIndices, const ColIndices& colIndices) {
    return derived(internal::eval_expr_given_size(rowIndices, derived.rows()),
                   internal::eval_expr_given_size(colIndices, derived.cols()));
  }
  static inline ConstReturnType run(const Derived& derived, const RowIndices& rowIndices,
                                    const ColIndices& colIndices) {
    return derived(internal::eval_expr_given_size(rowIndices, derived.rows()),
                   internal::eval_expr_given_size(colIndices, derived.cols()));
  }
};

// this helper class assumes internal::is_valid_index_type<Indices>::value == false
template <typename Indices, 
          bool UseSymbolic = symbolic::is_symbolic<Indices>::value,
          bool UseBlock = !UseSymbolic && internal::get_compile_time_incr<IvcType<Indices>>::value == 1,
          bool UseGeneric = !UseSymbolic && !UseBlock>
struct VectorIndexedViewSelector;

// Generic
template <typename Indices>
struct VectorIndexedViewSelector<Indices, false, false, true> {

  static constexpr bool IsRowMajor = DenseBase<Derived>::IsRowMajor;

  using RowMajorReturnType = IndexedView<Derived, IvcIndex, IvcType<Indices>>;
  using ConstRowMajorReturnType = IndexedView<const Derived, IvcIndex, IvcType<Indices>>;

  using ColMajorReturnType = IndexedView<Derived, IvcType<Indices>, IvcIndex>;
  using ConstColMajorReturnType = IndexedView<const Derived, IvcType<Indices>, IvcIndex>;

  using ReturnType = typename internal::conditional<IsRowMajor, RowMajorReturnType, ColMajorReturnType>::type;
  using ConstReturnType =
      typename internal::conditional<IsRowMajor, ConstRowMajorReturnType, ConstColMajorReturnType>::type;

  template <bool UseRowMajor = IsRowMajor, std::enable_if_t<UseRowMajor, bool> = true>
  static inline RowMajorReturnType run(Derived& derived, const Indices& indices) {
    return RowMajorReturnType(derived, IvcIndex(0), derived.ivcCol(indices));
  }
  template <bool UseRowMajor = IsRowMajor, std::enable_if_t<UseRowMajor, bool> = true>
  static inline ConstRowMajorReturnType run(const Derived& derived, const Indices& indices) {
    return ConstRowMajorReturnType(derived, IvcIndex(0), derived.ivcCol(indices));
  }
  template <bool UseRowMajor = IsRowMajor, std::enable_if_t<!UseRowMajor, bool> = true>
  static inline ColMajorReturnType run(Derived& derived, const Indices& indices) {
    return ColMajorReturnType(derived, derived.ivcRow(indices), IvcIndex(0));
  }
  template <bool UseRowMajor = IsRowMajor, std::enable_if_t<!UseRowMajor, bool> = true>
  static inline ConstColMajorReturnType run(const Derived& derived, const Indices& indices) {
    return ConstColMajorReturnType(derived, derived.ivcRow(indices), IvcIndex(0));
  }
};

// Block
template <typename Indices>
struct VectorIndexedViewSelector<Indices, false, true, false> {

  using ReturnType = VectorBlock<Derived, internal::array_size<Indices>::value>;
  using ConstReturnType = VectorBlock<const Derived, internal::array_size<Indices>::value>;

  static inline ReturnType run(Derived& derived, const Indices& indices) {
    IvcType<Indices> actualIndices = derived.ivcSize(indices);
    return ReturnType(derived, internal::first(actualIndices), internal::index_list_size(actualIndices));
  }
  static inline ConstReturnType run(const Derived& derived, const Indices& indices) {
    IvcType<Indices> actualIndices = derived.ivcSize(indices);
    return ConstReturnType(derived, internal::first(actualIndices), internal::index_list_size(actualIndices));
  }
};

// Symbolic
template <typename Indices>
struct VectorIndexedViewSelector<Indices, true, false, false> {

  using ReturnType = typename DenseBase<Derived>::Scalar&;
  using ConstReturnType = typename DenseBase<Derived>::CoeffReturnType;

  static inline ReturnType run(Derived& derived, const Indices& id) {
    return derived(internal::eval_expr_given_size(id, derived.size()));
  }
  static inline ConstReturnType run(const Derived& derived, const Indices& id) {
    return derived(internal::eval_expr_given_size(id, derived.size()));
  }
};

// SFINAE dummy types

template <typename RowIndices, typename ColIndices>
using EnableOverload = std::enable_if_t<
    internal::valid_indexed_view_overload<RowIndices, ColIndices>::value && internal::is_lvalue<Derived>::value, bool>;

template <typename RowIndices, typename ColIndices>
using EnableConstOverload =
    std::enable_if_t<internal::valid_indexed_view_overload<RowIndices, ColIndices>::value, bool>;

template <typename Indices>
using EnableVectorOverload =
    std::enable_if_t<!internal::is_valid_index_type<Indices>::value && internal::is_lvalue<Derived>::value, bool>;

template <typename Indices>
using EnableConstVectorOverload = std::enable_if_t<!internal::is_valid_index_type<Indices>::value, bool>;

public:

// Public API for 2D matrices/arrays

// non-const versions

template <typename RowIndices, typename ColIndices>
using IndexedViewType = typename IndexedViewSelector<RowIndices, ColIndices>::ReturnType;

template <typename RowIndices, typename ColIndices, EnableOverload<RowIndices, ColIndices> = true>
IndexedViewType<RowIndices, ColIndices> operator()(const RowIndices& rowIndices, const ColIndices& colIndices) {
  return IndexedViewSelector<RowIndices, ColIndices>::run(derived(), rowIndices, colIndices);
}

template <typename RowType, size_t RowSize, typename ColIndices, typename RowIndices = Array<RowType, RowSize, 1>,
          EnableOverload<RowIndices, ColIndices> = true>
IndexedViewType<RowIndices, ColIndices> operator()(const RowType (&rowIndices)[RowSize], const ColIndices& colIndices) {
  return IndexedViewSelector<RowIndices, ColIndices>::run(derived(), RowIndices{rowIndices}, colIndices);
}

template <typename RowIndices, typename ColType, size_t ColSize, typename ColIndices = Array<ColType, ColSize, 1>,
          EnableOverload<RowIndices, ColIndices> = true>
IndexedViewType<RowIndices, ColIndices> operator()(const RowIndices& rowIndices, const ColType (&colIndices)[ColSize]) {
  return IndexedViewSelector<RowIndices, ColIndices>::run(derived(), rowIndices, ColIndices{colIndices});
}

template <typename RowType, size_t RowSize, typename ColType, size_t ColSize,
          typename RowIndices = Array<RowType, RowSize, 1>, typename ColIndices = Array<ColType, ColSize, 1>,
          EnableOverload<RowIndices, ColIndices> = true>
IndexedViewType<RowIndices, ColIndices> operator()(const RowType (&rowIndices)[RowSize],
                                                   const ColType (&colIndices)[ColSize]) {
  return IndexedViewSelector<RowIndices, ColIndices>::run(derived(), RowIndices{rowIndices}, ColIndices{colIndices});
}

// const versions

template <typename RowIndices, typename ColIndices>
using ConstIndexedViewType = typename IndexedViewSelector<RowIndices, ColIndices>::ConstReturnType;

template <typename RowIndices, typename ColIndices, EnableConstOverload<RowIndices, ColIndices> = true>
ConstIndexedViewType<RowIndices, ColIndices> operator()(const RowIndices& rowIndices,
                                                        const ColIndices& colIndices) const {
  return IndexedViewSelector<RowIndices, ColIndices>::run(derived(), rowIndices, colIndices);
}

template <typename RowType, size_t RowSize, typename ColIndices, typename RowIndices = Array<RowType, RowSize, 1>,
          EnableConstOverload<RowIndices, ColIndices> = true>
ConstIndexedViewType<RowIndices, ColIndices> operator()(const RowType (&rowIndices)[RowSize],
                                                        const ColIndices& colIndices) const {
  return IndexedViewSelector<RowIndices, ColIndices>::run(derived(), RowIndices{rowIndices}, colIndices);
}

template <typename RowIndices, typename ColType, size_t ColSize, typename ColIndices = Array<ColType, ColSize, 1>,
          EnableConstOverload<RowIndices, ColIndices> = true>
ConstIndexedViewType<RowIndices, ColIndices> operator()(const RowIndices& rowIndices,
                                                        const ColType (&colIndices)[ColSize]) const {
  return IndexedViewSelector<RowIndices, ColIndices>::run(derived(), rowIndices, ColIndices{colIndices});
}

template <typename RowType, size_t RowSize, typename ColType, size_t ColSize,
          typename RowIndices = Array<RowType, RowSize, 1>, typename ColIndices = Array<ColType, ColSize, 1>,
          EnableConstOverload<RowIndices, ColIndices> = true>
ConstIndexedViewType<RowIndices, ColIndices> operator()(const RowType (&rowIndices)[RowSize],
                                                        const ColType (&colIndices)[ColSize]) const {
  return IndexedViewSelector<RowIndices, ColIndices>::run(derived(), RowIndices{rowIndices}, ColIndices{colIndices});
}

// Public API for 1D vectors/arrays

// non-const versions

template <typename Indices>
using VectorIndexedViewType = typename VectorIndexedViewSelector<Indices>::ReturnType;

template <typename Indices, EnableVectorOverload<Indices> = true>
VectorIndexedViewType<Indices> operator()(const Indices& indices) {
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return VectorIndexedViewSelector<Indices>::run(derived(), indices);
}

template <typename IndexType, size_t Size, typename Indices = Array<IndexType, Size, 1>,
          EnableVectorOverload<Indices> = true>
VectorIndexedViewType<Indices> operator()(const IndexType (&indices)[Size]) {
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return VectorIndexedViewSelector<Indices>::run(derived(), Indices{indices});
}

// const versions

template <typename Indices>
using ConstVectorIndexedViewType = typename VectorIndexedViewSelector<Indices>::ConstReturnType;

template <typename Indices, EnableConstVectorOverload<Indices> = true>
ConstVectorIndexedViewType<Indices> operator()(const Indices& indices) const {
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return VectorIndexedViewSelector<Indices>::run(derived(), indices);
}

template <typename IndexType, size_t Size, typename Indices = Array<IndexType, Size, 1>,
          EnableConstVectorOverload<Indices> = true>
ConstVectorIndexedViewType<Indices> operator()(const IndexType (&indices)[Size]) const {
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived)
  return VectorIndexedViewSelector<Indices>::run(derived(), Indices{indices});
}

#else // EIGEN_PARSED_BY_DOXYGEN

/**
  * \returns a generic submatrix view defined by the rows and columns indexed \a rowIndices and \a colIndices respectively.
  *
  * Each parameter must either be:
  *  - An integer indexing a single row or column
  *  - Eigen::placeholders::all indexing the full set of respective rows or columns in increasing order
  *  - An ArithmeticSequence as returned by the Eigen::seq and Eigen::seqN functions
  *  - Any %Eigen's vector/array of integers or expressions
  *  - Plain C arrays: \c int[N]
  *  - And more generally any type exposing the following two member functions:
  * \code
  * <integral type> operator[](<integral type>) const;
  * <integral type> size() const;
  * \endcode
  * where \c <integral \c type>  stands for any integer type compatible with Eigen::Index (i.e. \c std::ptrdiff_t).
  *
  * The last statement implies compatibility with \c std::vector, \c std::valarray, \c std::array, many of the Range-v3's ranges, etc.
  *
  * If the submatrix can be represented using a starting position \c (i,j) and positive sizes \c (rows,columns), then this
  * method will returns a Block object after extraction of the relevant information from the passed arguments. This is the case
  * when all arguments are either:
  *  - An integer
  *  - Eigen::placeholders::all
  *  - An ArithmeticSequence with compile-time increment strictly equal to 1, as returned by Eigen::seq(a,b), and Eigen::seqN(a,N).
  *
  * Otherwise a more general IndexedView<Derived,RowIndices',ColIndices'> object will be returned, after conversion of the inputs
  * to more suitable types \c RowIndices' and \c ColIndices'.
  *
  * For 1D vectors and arrays, you better use the operator()(const Indices&) overload, which behave the same way but taking a single parameter.
  *
  * See also this <a href="https://stackoverflow.com/questions/46110917/eigen-replicate-items-along-one-dimension-without-useless-allocations">question</a> and its answer for an example of how to duplicate coefficients.
  *
  * \sa operator()(const Indices&), class Block, class IndexedView, DenseBase::block(Index,Index,Index,Index)
  */
template<typename RowIndices, typename ColIndices>
IndexedView_or_Block
operator()(const RowIndices& rowIndices, const ColIndices& colIndices);

/** This is an overload of operator()(const RowIndices&, const ColIndices&) for 1D vectors or arrays
  *
  * \only_for_vectors
  */
template<typename Indices>
IndexedView_or_VectorBlock
operator()(const Indices& indices);

#endif  // EIGEN_PARSED_BY_DOXYGEN



#ifdef EIGEN_PARSED_BY_DOXYGEN

/// \returns an expression of \c *this with reshaped sizes.
///
/// \param nRows the number of rows in the reshaped expression, specified at either run-time or compile-time, or AutoSize
/// \param nCols the number of columns in the reshaped expression, specified at either run-time or compile-time, or AutoSize
/// \tparam Order specifies whether the coefficients should be processed in column-major-order (ColMajor), in row-major-order (RowMajor),
///               or follows the \em natural order of the nested expression (AutoOrder). The default is ColMajor.
/// \tparam NRowsType the type of the value handling the number of rows, typically Index.
/// \tparam NColsType the type of the value handling the number of columns, typically Index.
///
/// Dynamic size example: \include MatrixBase_reshaped_int_int.cpp
/// Output: \verbinclude MatrixBase_reshaped_int_int.out
///
/// The number of rows \a nRows and columns \a nCols can also be specified at compile-time by passing Eigen::fix<N>,
/// or Eigen::fix<N>(n) as arguments. In the later case, \c n plays the role of a runtime fallback value in case \c N equals Eigen::Dynamic.
/// Here is an example with a fixed number of rows and columns:
/// \include MatrixBase_reshaped_fixed.cpp
/// Output: \verbinclude MatrixBase_reshaped_fixed.out
///
/// Finally, one of the sizes parameter can be automatically deduced from the other one by passing AutoSize as in the following example:
/// \include MatrixBase_reshaped_auto.cpp
/// Output: \verbinclude MatrixBase_reshaped_auto.out
/// AutoSize does preserve compile-time sizes when possible, i.e., when the sizes of the input are known at compile time \b and
/// that the other size is passed at compile-time using Eigen::fix<N> as above.
///
/// \sa class Reshaped, fix, fix<N>(int)
///
template<int Order = ColMajor, typename NRowsType, typename NColsType>
EIGEN_DEVICE_FUNC
inline Reshaped<Derived,...>
reshaped(NRowsType nRows, NColsType nCols);

/// This is the const version of reshaped(NRowsType,NColsType).
template<int Order = ColMajor, typename NRowsType, typename NColsType>
EIGEN_DEVICE_FUNC
inline const Reshaped<const Derived,...>
reshaped(NRowsType nRows, NColsType nCols) const;

/// \returns an expression of \c *this with columns (or rows) stacked to a linear column vector
///
/// \tparam Order specifies whether the coefficients should be processed in column-major-order (ColMajor), in row-major-order (RowMajor),
///               or follows the \em natural order of the nested expression (AutoOrder). The default is ColMajor.
///
/// This overloads is essentially a shortcut for `A.reshaped<Order>(AutoSize,fix<1>)`.
///
/// - If `Order==ColMajor` (the default), then it returns a column-vector from the stacked columns of \c *this.
/// - If `Order==RowMajor`, then it returns a column-vector from the stacked rows of \c *this.
/// - If `Order==AutoOrder`, then it returns a column-vector with elements stacked following the storage order of \c *this.
///   This mode is the recommended one when the particular ordering of the element is not relevant.
///
/// Example:
/// \include MatrixBase_reshaped_to_vector.cpp
/// Output: \verbinclude MatrixBase_reshaped_to_vector.out
///
/// If you want more control, you can still fall back to reshaped(NRowsType,NColsType).
///
/// \sa reshaped(NRowsType,NColsType), class Reshaped
///
template<int Order = ColMajor>
EIGEN_DEVICE_FUNC
inline Reshaped<Derived,...>
reshaped();

/// This is the const version of reshaped().
template<int Order = ColMajor>
EIGEN_DEVICE_FUNC
inline const Reshaped<const Derived,...>
reshaped() const;

#else

// This file is automatically included twice to generate const and non-const versions

#ifndef EIGEN_RESHAPED_METHOD_2ND_PASS
#define EIGEN_RESHAPED_METHOD_CONST const
#else
#define EIGEN_RESHAPED_METHOD_CONST
#endif

#ifndef EIGEN_RESHAPED_METHOD_2ND_PASS

// This part is included once

#endif

template<typename NRowsType, typename NColsType>
EIGEN_DEVICE_FUNC
inline Reshaped<EIGEN_RESHAPED_METHOD_CONST Derived,
                internal::get_compiletime_reshape_size<NRowsType,NColsType,SizeAtCompileTime>::value,
                internal::get_compiletime_reshape_size<NColsType,NRowsType,SizeAtCompileTime>::value>
reshaped(NRowsType nRows, NColsType nCols) EIGEN_RESHAPED_METHOD_CONST
{
  return Reshaped<EIGEN_RESHAPED_METHOD_CONST Derived,
                  internal::get_compiletime_reshape_size<NRowsType,NColsType,SizeAtCompileTime>::value,
                  internal::get_compiletime_reshape_size<NColsType,NRowsType,SizeAtCompileTime>::value>
                (derived(),
                 internal::get_runtime_reshape_size(nRows,internal::get_runtime_value(nCols),size()),
                 internal::get_runtime_reshape_size(nCols,internal::get_runtime_value(nRows),size()));
}

template<int Order, typename NRowsType, typename NColsType>
EIGEN_DEVICE_FUNC
inline Reshaped<EIGEN_RESHAPED_METHOD_CONST Derived,
                internal::get_compiletime_reshape_size<NRowsType,NColsType,SizeAtCompileTime>::value,
                internal::get_compiletime_reshape_size<NColsType,NRowsType,SizeAtCompileTime>::value,
                internal::get_compiletime_reshape_order(Flags, Order)>
reshaped(NRowsType nRows, NColsType nCols) EIGEN_RESHAPED_METHOD_CONST
{
  return Reshaped<EIGEN_RESHAPED_METHOD_CONST Derived,
                  internal::get_compiletime_reshape_size<NRowsType,NColsType,SizeAtCompileTime>::value,
                  internal::get_compiletime_reshape_size<NColsType,NRowsType,SizeAtCompileTime>::value,
                  internal::get_compiletime_reshape_order(Flags, Order)>
                (derived(),
                 internal::get_runtime_reshape_size(nRows,internal::get_runtime_value(nCols),size()),
                 internal::get_runtime_reshape_size(nCols,internal::get_runtime_value(nRows),size()));
}

// Views as linear vectors

EIGEN_DEVICE_FUNC
inline Reshaped<EIGEN_RESHAPED_METHOD_CONST Derived,SizeAtCompileTime,1>
reshaped() EIGEN_RESHAPED_METHOD_CONST
{
  return Reshaped<EIGEN_RESHAPED_METHOD_CONST Derived,SizeAtCompileTime,1>(derived(),size(),1);
}

template<int Order>
EIGEN_DEVICE_FUNC
inline Reshaped<EIGEN_RESHAPED_METHOD_CONST Derived, SizeAtCompileTime, 1,
                internal::get_compiletime_reshape_order(Flags, Order)>
reshaped() EIGEN_RESHAPED_METHOD_CONST
{
  EIGEN_STATIC_ASSERT(Order==RowMajor || Order==ColMajor || Order==AutoOrder, INVALID_TEMPLATE_PARAMETER);
  return Reshaped<EIGEN_RESHAPED_METHOD_CONST Derived, SizeAtCompileTime, 1,
                  internal::get_compiletime_reshape_order(Flags, Order)>
                (derived(), size(), 1);
}

#undef EIGEN_RESHAPED_METHOD_CONST

#ifndef EIGEN_RESHAPED_METHOD_2ND_PASS
#define EIGEN_RESHAPED_METHOD_2ND_PASS

#ifdef EIGEN_PARSED_BY_DOXYGEN

/// \returns an expression of \c *this with reshaped sizes.
///
/// \param nRows the number of rows in the reshaped expression, specified at either run-time or compile-time, or AutoSize
/// \param nCols the number of columns in the reshaped expression, specified at either run-time or compile-time, or AutoSize
/// \tparam Order specifies whether the coefficients should be processed in column-major-order (ColMajor), in row-major-order (RowMajor),
///               or follows the \em natural order of the nested expression (AutoOrder). The default is ColMajor.
/// \tparam NRowsType the type of the value handling the number of rows, typically Index.
/// \tparam NColsType the type of the value handling the number of columns, typically Index.
///
/// Dynamic size example: \include MatrixBase_reshaped_int_int.cpp
/// Output: \verbinclude MatrixBase_reshaped_int_int.out
///
/// The number of rows \a nRows and columns \a nCols can also be specified at compile-time by passing Eigen::fix<N>,
/// or Eigen::fix<N>(n) as arguments. In the later case, \c n plays the role of a runtime fallback value in case \c N equals Eigen::Dynamic.
/// Here is an example with a fixed number of rows and columns:
/// \include MatrixBase_reshaped_fixed.cpp
/// Output: \verbinclude MatrixBase_reshaped_fixed.out
///
/// Finally, one of the sizes parameter can be automatically deduced from the other one by passing AutoSize as in the following example:
/// \include MatrixBase_reshaped_auto.cpp
/// Output: \verbinclude MatrixBase_reshaped_auto.out
/// AutoSize does preserve compile-time sizes when possible, i.e., when the sizes of the input are known at compile time \b and
/// that the other size is passed at compile-time using Eigen::fix<N> as above.
///
/// \sa class Reshaped, fix, fix<N>(int)
///
template<int Order = ColMajor, typename NRowsType, typename NColsType>
EIGEN_DEVICE_FUNC
inline Reshaped<Derived,...>
reshaped(NRowsType nRows, NColsType nCols);

/// This is the const version of reshaped(NRowsType,NColsType).
template<int Order = ColMajor, typename NRowsType, typename NColsType>
EIGEN_DEVICE_FUNC
inline const Reshaped<const Derived,...>
reshaped(NRowsType nRows, NColsType nCols) const;

/// \returns an expression of \c *this with columns (or rows) stacked to a linear column vector
///
/// \tparam Order specifies whether the coefficients should be processed in column-major-order (ColMajor), in row-major-order (RowMajor),
///               or follows the \em natural order of the nested expression (AutoOrder). The default is ColMajor.
///
/// This overloads is essentially a shortcut for `A.reshaped<Order>(AutoSize,fix<1>)`.
///
/// - If `Order==ColMajor` (the default), then it returns a column-vector from the stacked columns of \c *this.
/// - If `Order==RowMajor`, then it returns a column-vector from the stacked rows of \c *this.
/// - If `Order==AutoOrder`, then it returns a column-vector with elements stacked following the storage order of \c *this.
///   This mode is the recommended one when the particular ordering of the element is not relevant.
///
/// Example:
/// \include MatrixBase_reshaped_to_vector.cpp
/// Output: \verbinclude MatrixBase_reshaped_to_vector.out
///
/// If you want more control, you can still fall back to reshaped(NRowsType,NColsType).
///
/// \sa reshaped(NRowsType,NColsType), class Reshaped
///
template<int Order = ColMajor>
EIGEN_DEVICE_FUNC
inline Reshaped<Derived,...>
reshaped();

/// This is the const version of reshaped().
template<int Order = ColMajor>
EIGEN_DEVICE_FUNC
inline const Reshaped<const Derived,...>
reshaped() const;

#else

// This file is automatically included twice to generate const and non-const versions

#ifndef EIGEN_RESHAPED_METHOD_2ND_PASS
#define EIGEN_RESHAPED_METHOD_CONST const
#else
#define EIGEN_RESHAPED_METHOD_CONST
#endif

#ifndef EIGEN_RESHAPED_METHOD_2ND_PASS

// This part is included once

#endif

template<typename NRowsType, typename NColsType>
EIGEN_DEVICE_FUNC
inline Reshaped<EIGEN_RESHAPED_METHOD_CONST Derived,
                internal::get_compiletime_reshape_size<NRowsType,NColsType,SizeAtCompileTime>::value,
                internal::get_compiletime_reshape_size<NColsType,NRowsType,SizeAtCompileTime>::value>
reshaped(NRowsType nRows, NColsType nCols) EIGEN_RESHAPED_METHOD_CONST
{
  return Reshaped<EIGEN_RESHAPED_METHOD_CONST Derived,
                  internal::get_compiletime_reshape_size<NRowsType,NColsType,SizeAtCompileTime>::value,
                  internal::get_compiletime_reshape_size<NColsType,NRowsType,SizeAtCompileTime>::value>
                (derived(),
                 internal::get_runtime_reshape_size(nRows,internal::get_runtime_value(nCols),size()),
                 internal::get_runtime_reshape_size(nCols,internal::get_runtime_value(nRows),size()));
}

template<int Order, typename NRowsType, typename NColsType>
EIGEN_DEVICE_FUNC
inline Reshaped<EIGEN_RESHAPED_METHOD_CONST Derived,
                internal::get_compiletime_reshape_size<NRowsType,NColsType,SizeAtCompileTime>::value,
                internal::get_compiletime_reshape_size<NColsType,NRowsType,SizeAtCompileTime>::value,
                internal::get_compiletime_reshape_order(Flags, Order)>
reshaped(NRowsType nRows, NColsType nCols) EIGEN_RESHAPED_METHOD_CONST
{
  return Reshaped<EIGEN_RESHAPED_METHOD_CONST Derived,
                  internal::get_compiletime_reshape_size<NRowsType,NColsType,SizeAtCompileTime>::value,
                  internal::get_compiletime_reshape_size<NColsType,NRowsType,SizeAtCompileTime>::value,
                  internal::get_compiletime_reshape_order(Flags, Order)>
                (derived(),
                 internal::get_runtime_reshape_size(nRows,internal::get_runtime_value(nCols),size()),
                 internal::get_runtime_reshape_size(nCols,internal::get_runtime_value(nRows),size()));
}

// Views as linear vectors

EIGEN_DEVICE_FUNC
inline Reshaped<EIGEN_RESHAPED_METHOD_CONST Derived,SizeAtCompileTime,1>
reshaped() EIGEN_RESHAPED_METHOD_CONST
{
  return Reshaped<EIGEN_RESHAPED_METHOD_CONST Derived,SizeAtCompileTime,1>(derived(),size(),1);
}

template<int Order>
EIGEN_DEVICE_FUNC
inline Reshaped<EIGEN_RESHAPED_METHOD_CONST Derived, SizeAtCompileTime, 1,
                internal::get_compiletime_reshape_order(Flags, Order)>
reshaped() EIGEN_RESHAPED_METHOD_CONST
{
  EIGEN_STATIC_ASSERT(Order==RowMajor || Order==ColMajor || Order==AutoOrder, INVALID_TEMPLATE_PARAMETER);
  return Reshaped<EIGEN_RESHAPED_METHOD_CONST Derived, SizeAtCompileTime, 1,
                  internal::get_compiletime_reshape_order(Flags, Order)>
                (derived(), size(), 1);
}

#undef EIGEN_RESHAPED_METHOD_CONST

#endif // EIGEN_PARSED_BY_DOXYGEN

#undef EIGEN_RESHAPED_METHOD_2ND_PASS
#endif

#endif // EIGEN_PARSED_BY_DOXYGEN

#   ifdef EIGEN_DENSEBASE_PLUGIN
#     include EIGEN_DENSEBASE_PLUGIN
#   endif
#undef EIGEN_CURRENT_STORAGE_BASE_CLASS
#undef EIGEN_DOC_BLOCK_ADDONS_NOT_INNER_PANEL
#undef EIGEN_DOC_BLOCK_ADDONS_INNER_PANEL_IF
#undef EIGEN_DOC_UNARY_ADDONS

    // disable the use of evalTo for dense objects with a nice compilation error
    template<typename Dest>
    EIGEN_DEVICE_FUNC
    inline void evalTo(Dest& ) const
    {
      EIGEN_STATIC_ASSERT((internal::is_same<Dest,void>::value),THE_EVAL_EVALTO_FUNCTION_SHOULD_NEVER_BE_CALLED_FOR_DENSE_OBJECTS);
    }

  protected:
    EIGEN_DEFAULT_COPY_CONSTRUCTOR(DenseBase)
    /** Default constructor. Do nothing. */
    EIGEN_DEVICE_FUNC constexpr DenseBase() {
      /* Just checks for self-consistency of the flags.
       * Only do it when debugging Eigen, as this borders on paranoia and could slow compilation down
       */
#ifdef EIGEN_INTERNAL_DEBUGGING
      EIGEN_STATIC_ASSERT((internal::check_implication(MaxRowsAtCompileTime==1 && MaxColsAtCompileTime!=1, int(IsRowMajor))
                        && internal::check_implication(MaxColsAtCompileTime==1 && MaxRowsAtCompileTime!=1, int(!IsRowMajor))),
                          INVALID_STORAGE_ORDER_FOR_THIS_VECTOR_EXPRESSION)
#endif
    }

  private:
    EIGEN_DEVICE_FUNC explicit DenseBase(int);
    EIGEN_DEVICE_FUNC DenseBase(int,int);
    template<typename OtherDerived> EIGEN_DEVICE_FUNC explicit DenseBase(const DenseBase<OtherDerived>&);
};

} // end namespace Eigen

#endif // EIGEN_DENSEBASE_H
