//
// Created by 11048 on 2023/6/7.
//
#include <stdexcept>
#include <cassert>
#ifndef STL_STUDY_EXCEPTDEF_H
#define STL_STUDY_EXCEPTDEF_H

namespace mystl
{

#define MYSTL_DEBUG(expr) \
  assert(expr)

#define THROW_LENGTH_ERROR_IF(expr, what) \
  if ((expr)) throw std::length_error(what)

#define THROW_OUT_OF_RANGE_IF(expr, what) \
  if ((expr)) throw std::out_of_range(what)

#define THROW_RUNTIME_ERROR_IF(expr, what) \
  if ((expr)) throw std::runtime_error(what)

}
#endif //STL_STUDY_EXCEPTDEF_H
