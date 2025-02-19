#if !defined(__linux__)
// for some reason, including the top level Eigen include
// on Linux might crash the swift AST parser, so we don't
// include it here.
# include <Eigen/Eigen.h>
#endif
