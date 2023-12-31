//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenEXR Project.
//

// clang-format off

#ifndef _PyImathMatrix_h_
#define _PyImathMatrix_h_

#include <Python.h>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/python.hpp>
#include <OpenEXR/ImathMatrix.h>
#include <OpenEXR/ImathMatrixAlgo.h>
#include "PyImath.h"

namespace PyImath {

template <class T> boost::python::class_<IMATH_NAMESPACE::Matrix22<T> > register_Matrix22();
template <class T> boost::python::class_<IMATH_NAMESPACE::Matrix33<T> > register_Matrix33();
template <class T> boost::python::class_<IMATH_NAMESPACE::Matrix44<T> > register_Matrix44();
template <class T> boost::python::class_<FixedArray<IMATH_NAMESPACE::Matrix44<T> > > register_M44Array();
template <class T> boost::python::class_<FixedArray<IMATH_NAMESPACE::Matrix33<T> > > register_M33Array();
template <class T> boost::python::class_<FixedArray<IMATH_NAMESPACE::Matrix22<T> > > register_M22Array();
typedef FixedArray<IMATH_NAMESPACE::Matrix22<float> >  M22fArray;
typedef FixedArray<IMATH_NAMESPACE::Matrix22<double> >  M22dArray;
typedef FixedArray<IMATH_NAMESPACE::Matrix33<float> >  M33fArray;
typedef FixedArray<IMATH_NAMESPACE::Matrix33<double> >  M33dArray;
typedef FixedArray<IMATH_NAMESPACE::Matrix44<float> >  M44fArray;
typedef FixedArray<IMATH_NAMESPACE::Matrix44<double> >  M44dArray;

//

// Other code in the Zeno code base assumes the existance of a class with the
// same name as the Imath class, and with static functions wrap() and
// convert() to produce a PyImath object from an Imath object and vice-versa,
// respectively.  The class Boost generates from the Imath class does not
// have these properties, so we define a companion class here.
// The template argument, T, is the element type (e.g.,float, double).

template <class T>
class M22 {
  public:
    static PyObject *	wrap (const IMATH_NAMESPACE::Matrix22<T> &m);
    static int		convert (PyObject *p, IMATH_NAMESPACE::Matrix22<T> *m);
};

template <class T>
class M33 {
  public:
    static PyObject *	wrap (const IMATH_NAMESPACE::Matrix33<T> &m);
    static int		convert (PyObject *p, IMATH_NAMESPACE::Matrix33<T> *m);
};

template <class T>
class M44 {
  public:
    static PyObject *	wrap (const IMATH_NAMESPACE::Matrix44<T> &m);
    static int		convert (PyObject *p, IMATH_NAMESPACE::Matrix44<T> *m);
};

template <class T>
PyObject *
M22<T>::wrap (const IMATH_NAMESPACE::Matrix22<T> &m)
{
    typename boost::python::return_by_value::apply < IMATH_NAMESPACE::Matrix22<T> >::type converter;
    PyObject *p = converter (m);
    return p;
}

template <class T>
PyObject *
M33<T>::wrap (const IMATH_NAMESPACE::Matrix33<T> &m)
{
    typename boost::python::return_by_value::apply < IMATH_NAMESPACE::Matrix33<T> >::type converter;
    PyObject *p = converter (m);
    return p;
}

template <class T>
PyObject *
M44<T>::wrap (const IMATH_NAMESPACE::Matrix44<T> &m)
{
    typename boost::python::return_by_value::apply < IMATH_NAMESPACE::Matrix44<T> >::type converter;
    PyObject *p = converter (m);
    return p;
}

template <class T>
int
M22<T>::convert (PyObject *p, IMATH_NAMESPACE::Matrix22<T> *m)
{
    boost::python::extract <IMATH_NAMESPACE::M22f> extractorMf (p);
    if (extractorMf.check())
    {
        IMATH_NAMESPACE::M22f e = extractorMf();
        m->setValue (e);
        return 1;
    }

    boost::python::extract <IMATH_NAMESPACE::M22d> extractorMd (p);
    if (extractorMd.check())
    {
        IMATH_NAMESPACE::M22d e = extractorMd();
        m->setValue (e);
        return 1;
    }

    return 0;
}

template <class T>
int
M33<T>::convert (PyObject *p, IMATH_NAMESPACE::Matrix33<T> *m)
{
    boost::python::extract <IMATH_NAMESPACE::M33f> extractorMf (p);
    if (extractorMf.check())
    {
        IMATH_NAMESPACE::M33f e = extractorMf();
        m->setValue (e);
        return 1;
    }

    boost::python::extract <IMATH_NAMESPACE::M33d> extractorMd (p);
    if (extractorMd.check())
    {
        IMATH_NAMESPACE::M33d e = extractorMd();
        m->setValue (e);
        return 1;
    }

    return 0;
}

template <class T>
int
M44<T>::convert (PyObject *p, IMATH_NAMESPACE::Matrix44<T> *m)
{
    boost::python::extract <IMATH_NAMESPACE::M44f> extractorMf (p);
    if (extractorMf.check())
    {
        IMATH_NAMESPACE::M44f e = extractorMf();
        m->setValue (e);
        return 1;
    }

    boost::python::extract <IMATH_NAMESPACE::M44d> extractorMd (p);
    if (extractorMd.check())
    {
        IMATH_NAMESPACE::M44d e = extractorMd();
        m->setValue (e);
        return 1;
    }

    return 0;
}

template <class Matrix>
boost::python::tuple
jacobiEigensolve(const Matrix& m)
{
    typedef typename Matrix::BaseType T;
    typedef typename Matrix::BaseVecType Vec;

    // For the C++ version, we just assume that the passed-in matrix is
    // symmetric, but we assume that many of our script users are less
    // sophisticated and might get tripped up by this.  Also, the cost
    // of doing this check is likely miniscule compared to the Pythonic
    // overhead.

    // Give a fairly generous tolerance to account for possible epsilon drift:
    const int d = Matrix::dimensions();
    const T tol = std::sqrt(std::numeric_limits<T>::epsilon());
    for (int i = 0; i < d; ++i)
    {
        for (int j = i+1; j < d; ++j)
        {
            const T Aij = m[i][j],
                    Aji = m[j][i];
            if (std::abs(Aij - Aji) >= tol){
              throw std::invalid_argument
              ("Symmetric eigensolve requires a symmetric matrix (matrix[i][j] == matrix[j][i]).");
            }
        }
    }

    Matrix tmp = m;
    Matrix Q;
    Vec S;
    IMATH_NAMESPACE::jacobiEigenSolver (tmp, S, Q);
    return boost::python::make_tuple (Q, S);
}

typedef M22<float>	M22f;
typedef M22<double>	M22d;

typedef M33<float>	M33f;
typedef M33<double>	M33d;

typedef M44<float>	M44f;
typedef M44<double>	M44d;

}

#endif
