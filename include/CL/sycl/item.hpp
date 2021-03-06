/*
 * This file is part of hipSYCL, a SYCL implementation based on CUDA/HIP
 *
 * Copyright (c) 2018, 2019 Aksel Alpay and contributors
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef HIPSYCL_ITEM_HPP
#define HIPSYCL_ITEM_HPP

#include "id.hpp"
#include "range.hpp"
#include "detail/thread_hierarchy.hpp"
#include "backend/backend.hpp"

#include <type_traits>

namespace cl {
namespace sycl {

template <int dimensions, bool with_offset>
struct item;

namespace detail {

template <int dimensions>
struct item_base
{
  __device__ item_base(const id<dimensions>& my_id,
    const sycl::range<dimensions>& global_size)
    : global_id{my_id}, global_size(global_size)
  {}

  /* -- common interface members -- */

  __device__ sycl::range<dimensions> get_range() const
  { return global_size; }

  __device__ size_t get_range(int dimension) const
  { return global_size[dimension]; }

protected:
  id<dimensions> global_id;
  sycl::range<dimensions> global_size;
};

template <int dimensions>
__device__
item<dimensions, true> make_item(const id<dimensions>& my_id,
  const sycl::range<dimensions>& global_size, const id<dimensions>& offset)
{
  return item<dimensions, true>{my_id, global_size, offset};
}

template <int dimensions>
__device__
item<dimensions, false> make_item(const id<dimensions>& my_id,
  const sycl::range<dimensions>& global_size)
{
  return item<dimensions, false>{my_id, global_size};
}

} // namespace detail

template <int dimensions = 1, bool with_offset = true>
struct item : detail::item_base<dimensions>
{};

template <int dimensions>
struct item<dimensions, true> : detail::item_base<dimensions>
{
  /* -- common interface members -- */

  __device__ id<dimensions> get_id() const
  {
    return this->global_id + offset;
  }

  __device__ size_t get_id(int dimension) const
  {
    return this->global_id[dimension] + offset[dimension];
  }

  __device__ size_t operator[](int dimension)
  {
    return this->global_id[dimension] + offset[dimension];
  }

  __device__ size_t get_linear_id() const
  {
    return detail::linear_id<dimensions>::get(this->global_id + offset,
      this->global_size);
  }

  __device__ id<dimensions> get_offset() const
  {
    return offset;
  }

private:
  template<int d>
  using _range = sycl::range<d>; // workaround for nvcc
  friend __device__ item<dimensions, true> detail::make_item<dimensions>(
    const id<dimensions>&, const _range<dimensions>&, const id<dimensions>&);

  __device__ item(const id<dimensions>& my_id,
    const sycl::range<dimensions>& global_size, const id<dimensions>& offset)
    : detail::item_base<dimensions>(my_id, global_size), offset{offset}
  {}

  const id<dimensions> offset;
};

template <int dimensions>
struct item<dimensions, false> : detail::item_base<dimensions>
{
  /* -- common interface members -- */

  __device__ id<dimensions> get_id() const
  {
    return this->global_id;
  }

  __device__ size_t get_id(int dimension) const
  {
    return this->global_id[dimension];
  }

  __device__ size_t operator[](int dimension)
  {
    return this->global_id[dimension];
  }

  __device__ size_t get_linear_id() const
  {
    return detail::linear_id<dimensions>::get(this->global_id,
      this->global_size);
  }

  __device__ operator item<dimensions, true>() const
  {
    return detail::make_item<dimensions>(this->global_id, this->global_size,
      id<dimensions>{});
  }

private:
  template<int d>
  using _range = sycl::range<d>; // workaround for nvcc
  friend __device__ item<dimensions, false> detail::make_item<dimensions>(
    const id<dimensions>&, const _range<dimensions>&);

  __device__ item(const id<dimensions>& my_id,
    const sycl::range<dimensions>& global_size)
    : detail::item_base<dimensions>(my_id, global_size)
  {}
};

}
}

#endif
