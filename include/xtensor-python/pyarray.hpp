/***************************************************************************
* Copyright (c) 2016, Johan Mabille and Sylvain Corlay                     *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef PY_ARRAY_HPP
#define PY_ARRAY_HPP

#include <cstddef>
#include <algorithm>

#include "pybind11/numpy.h"

#include "xtensor/xexpression.hpp"
#include "xtensor/xsemantic.hpp"
#include "xtensor/xiterator.hpp"

namespace xt
{

    using pybind_array = pybind11::array;
    using buffer_info = pybind11::buffer_info;

    /***********************
     * pyarray declaration *
     ***********************/

    template <class T, int ExtraFlags>
    class pyarray;

    template <class T, int ExtraFlags>
    struct array_inner_types<pyarray<T, ExtraFlags>>
    {
        using temporary_type = pyarray<T, ExtraFlags>;
    };

    template <class A>
    class pyarray_backstrides
    {

    public:

        using array_type = A;
        using value_type = typename array_type::size_type;
        using size_type = typename array_type::size_type;

        pyarray_backstrides(const A& a);

        value_type operator[](size_type i) const;

    private:

        const pybind_array* p_a;
    };

    /**
     * @class pyarray
     * @brief Wrapper on the Python buffer protocol.
     */
    template <class T, int ExtraFlags = pybind_array::forcecast>
    class pyarray : public pybind_array,
                     public xarray_semantic<pyarray<T, ExtraFlags>>
    {

    public:

        using self_type = pyarray<T, ExtraFlags>;
        using base_type = pybind_array;
        using semantic_base = xarray_semantic<self_type>;
        using value_type = T;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;

        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        using stepper = xstepper<self_type>;
        using const_stepper = xstepper<const self_type>;

        using iterator = xiterator<stepper>;
        using const_iterator = xiterator<const_stepper>;
        
        using storage_iterator = T*;
        using const_storage_iterator = const T*;

        using shape_type = xshape<size_type>;
        using strides_type = xstrides<size_type>;
        using backstrides_type = pyarray_backstrides<self_type>;

        using closure_type = const self_type&;

        PYBIND11_OBJECT_CVT(pyarray, pybind_array, is_non_null, m_ptr = ensure_(m_ptr));

        pyarray();

        explicit pyarray(const buffer_info& info);

        pyarray(const xshape<size_type>& shape,
                const xstrides<size_type>& strides, 
                const T* ptr = nullptr,
                handle base = handle());

        explicit pyarray(const xshape<size_type>& shape, 
                         const T* ptr = nullptr,
                         handle base = handle());

        explicit pyarray(size_type count,
                         const T* ptr = nullptr,
                         handle base = handle());

        size_type dimension() const;
        const shape_type& shape() const;
        const strides_type& strides() const;
        backstrides_type backstrides() const;

        void reshape(const shape_type& shape);
        void reshape(const shape_type& shape, layout l);
        void reshape(const shape_type& shape, const strides_type& strides);

        template<typename... Args>
        reference operator()(Args... args);

        template<typename... Args>
        const_reference operator()(Args... args) const;

        template<typename... Args>
        pointer data(Args... args);

        template<typename... Args>
        const_pointer data(Args... args) const;

        bool broadcast_shape(shape_type& shape) const;
        bool is_trivial_broadcast(const strides_type& strides) const;

        iterator begin();
        iterator end();

        const_iterator begin() const;
        const_iterator end() const;
        const_iterator cbegin() const;
        const_iterator cend() const;

        iterator xbegin(const shape_type& shape);
        iterator xend(const shape_type& shape);

        const_iterator xbegin(const shape_type& shape) const;
        const_iterator xend(const shape_type& shape) const;
        const_iterator cxbegin(const shape_type& shape) const;
        const_iterator cxend(const shape_type& shape) const;

        stepper stepper_begin(const shape_type& shape);
        stepper stepper_end(const shape_type& shape);

        const_stepper stepper_begin(const shape_type& shape) const;
        const_stepper stepper_end(const shape_type& shape) const;

        storage_iterator storage_begin();
        storage_iterator storage_end();

        const_storage_iterator storage_begin() const;
        const_storage_iterator storage_end() const;

        template <class E>
        pyarray(const xexpression<E>& e);

        template <class E>
        pyarray& operator=(const xexpression<E>& e);

    private:

        template<typename... Args>
        auto index_at(Args... args) const -> size_type;

        static constexpr auto itemsize() -> size_type;

        static bool is_non_null(PyObject* ptr);

        static PyObject *ensure_(PyObject* ptr);

        mutable shape_type m_shape;
        mutable strides_type m_strides;

    };
    
    /**************************************
     * pyarray_backstrides implementation *
     **************************************/

    template <class A>
    inline pyarray_backstrides<A>::pyarray_backstrides(const A& a)
        : p_a(&a)
    {
    }

    template <class A>
    inline auto pyarray_backstrides<A>::operator[](size_type i) const -> value_type
    {
        value_type sh = p_a->shape()[i];
        value_type res = sh == 1 ? 0 : (sh - 1) * p_a->strides()[i] / sizeof(typename A::value_type);
        return  res;
    }

    /**************************
     * pyarray implementation *
     **************************/

    template <class T, int ExtraFlags>
    inline pyarray<T, ExtraFlags>::pyarray()
         : pybind_array()
    {
    }

    template <class T, int ExtraFlags>
    inline pyarray<T, ExtraFlags>::pyarray(const buffer_info& info)
        : pybind_array(info)
    {
    }

    template <class T, int ExtraFlags>
    inline pyarray<T, ExtraFlags>::pyarray(const xshape<size_type>& shape,
                                           const xstrides<size_type>& strides, 
                                           const T *ptr,
                                           handle base)
        : pybind_array(shape, strides, ptr, base)
    {
    }

    template <class T, int ExtraFlags>
    inline pyarray<T, ExtraFlags>::pyarray(const xshape<size_type>& shape, 
                                           const T* ptr,
                                           handle base)
        : pybind_array(shape, ptr, base)
    {
    }

    template <class T, int ExtraFlags>
    inline pyarray<T, ExtraFlags>::pyarray(size_type count,
                                           const T* ptr,
                                           handle base)
        : pybind_array(count, ptr, base)
    {
    }

    template <class T, int ExtraFlags>
    inline auto pyarray<T, ExtraFlags>::dimension() const -> size_type
    {
        return pybind_array::ndim();
    }

    template <class T, int ExtraFlags>
    inline auto pyarray<T, ExtraFlags>::shape() const -> const shape_type&
    {
        // Until we have the CRTP on shape types, we copy the shape.
        m_shape.resize(dimension());
        std::copy(pybind_array::shape(), pybind_array::shape() + dimension(), m_shape.begin());
        return m_shape;
    }

    template <class T, int ExtraFlags>
    inline auto pyarray<T, ExtraFlags>::strides() const -> const strides_type&
    {
        m_strides.resize(dimension());
        std::transform(pybind_array::strides(), pybind_array::strides() + dimension(), m_strides.begin(),
            [](size_type str) { return str / sizeof(value_type); });
        return m_strides;
    }

    template <class T, int ExtraFlags>
    inline auto pyarray<T, ExtraFlags>::backstrides() const -> backstrides_type
    {
        backstrides_type tmp(*this);
        return tmp;
    }

    template <class T, int ExtraFlags>
    void pyarray<T, ExtraFlags>::reshape(const shape_type& shape)
    {
        if (!m_ptr || shape.size() != dimension() || !std::equal(shape.begin(), shape.end(), pybind_array::shape()))
        {
            reshape(shape, layout::row_major);
        }
    }

    template <class T, int ExtraFlags>
    void pyarray<T, ExtraFlags>::reshape(const shape_type& shape, layout l)
    {
        strides_type strides(shape.size());
        size_type data_size = sizeof(value_type);
        if (l == layout::row_major)
        {
            for (size_type i = strides.size(); i != 0; --i)
            {
                strides[i - 1] = data_size;
                data_size = strides[i - 1] * shape[i - 1];
                if (shape[i - 1] == 1)
                {
                    strides[i - 1] = 0;
                }
            }
        }
        else
        {
            for (size_type i = 0; i < strides.size(); ++i)
            {
                strides[i] = data_size;
                data_size = strides[i] * shape[i];
                if (shape[i] == 1)
                {
                    strides[i] = 0;
                }
            }
        }
        reshape(shape, strides);
    }

    template <class T, int ExtraFlags>
    void pyarray<T, ExtraFlags>::reshape(const shape_type& shape, const strides_type& strides)
    {
        self_type tmp(shape, strides);
        *this = std::move(tmp);
    }

    template <class T, int ExtraFlags>
    template<typename... Args> 
    inline auto pyarray<T, ExtraFlags>::operator()(Args... args) -> reference
    {
        if (sizeof...(args) != dimension())
        {
            pybind_array::fail_dim_check(sizeof...(args), "index dimension mismatch");
        }
        // not using pybind_array::offset_at() / index_at() here so as to avoid another dimension check.
        return *(static_cast<pointer>(pybind_array::mutable_data()) + pybind_array::get_byte_offset(args...) / itemsize());
    }

    template <class T, int ExtraFlags>
    template<typename... Args> 
    inline auto pyarray<T, ExtraFlags>::operator()(Args... args) const -> const_reference
    {
        if (sizeof...(args) != dimension())
        {
            pybind_array::fail_dim_check(sizeof...(args), "index dimension mismatch");
        }
        // not using pybind_array::offset_at() / index_at() here so as to avoid another dimension check.
        return *(static_cast<const_pointer>(pybind_array::data()) + pybind_array::get_byte_offset(args...) / itemsize());
    }

    template <class T, int ExtraFlags>
    template<typename... Args> 
    inline auto pyarray<T, ExtraFlags>::data(Args... args) -> pointer
    {
        return static_cast<pointer>(pybind_array::mutable_data(args...));
    }

    template <class T, int ExtraFlags>
    template<typename... Args>
    inline auto pyarray<T, ExtraFlags>::data(Args... args) const -> const_pointer
    {
        return static_cast<const T*>(pybind_array::data(args...));
    }

    template <class T, int ExtraFlags>
    bool pyarray<T, ExtraFlags>::broadcast_shape(shape_type& shape) const
    {
        return xt::broadcast_shape(this->shape(), shape);
    }

    template <class T, int ExtraFlags>
    bool pyarray<T, ExtraFlags>::is_trivial_broadcast(const strides_type& strides) const
    {
        return strides.size() == dimension() &&
            std::equal(strides.begin(), strides.end(), this->strides().begin());
    }

    template <class T, int ExtraFlags>
    inline auto pyarray<T, ExtraFlags>::begin() -> iterator
    {
        return xbegin(shape());
    }

    template <class T, int ExtraFlags>
    inline auto pyarray<T, ExtraFlags>::end() -> iterator
    {
        return xend(shape());
    }

    template <class T, int ExtraFlags>
    inline auto pyarray<T, ExtraFlags>::begin() const -> const_iterator
    {
        return xbegin(shape());
    }

    template <class T, int ExtraFlags>
    inline auto pyarray<T, ExtraFlags>::end() const -> const_iterator
    {
        return xend(shape());
    }

    template <class T, int ExtraFlags>
    inline auto pyarray<T, ExtraFlags>::cbegin() const -> const_iterator
    {
        return begin();
    }

    template <class T, int ExtraFlags>
    inline auto pyarray<T, ExtraFlags>::cend() const -> const_iterator
    {
        return end();
    }

    template <class T, int ExtraFlags>
    inline auto pyarray<T, ExtraFlags>::xbegin(const shape_type& shape) -> iterator
    {
        return iterator(stepper_begin(shape), shape);
    }

    template <class T, int ExtraFlags>
    inline auto pyarray<T, ExtraFlags>::xend(const shape_type& shape) -> iterator
    {
        return iterator(stepper_end(shape), shape);
    }

    template <class T, int ExtraFlags>
    inline auto pyarray<T, ExtraFlags>::xbegin(const shape_type& shape) const -> const_iterator
    {
        return const_iterator(stepper_begin(shape), shape);
    }

    template <class T, int ExtraFlags>
    inline auto pyarray<T, ExtraFlags>::xend(const shape_type& shape) const -> const_iterator
    {
        return const_iterator(stepper_end(shape), shape);
    }

    template <class T, int ExtraFlags>
    inline auto pyarray<T, ExtraFlags>::cxbegin(const shape_type& shape) const -> const_iterator
    {
        return xbegin(shape);
    }

    template <class T, int ExtraFlags>
    inline auto pyarray<T, ExtraFlags>::cxend(const shape_type& shape) const -> const_iterator
    {
        return xend(shape);
    }

    template <class T, int ExtraFlags>
    inline auto pyarray<T, ExtraFlags>::stepper_begin(const shape_type& shape) -> stepper
    {
        size_type offset = shape.size() - dimension();
        return stepper(this, storage_begin(), offset);
    }

    template <class T, int ExtraFlags>
    inline auto pyarray<T, ExtraFlags>::stepper_end(const shape_type& shape) -> stepper
    {
        size_type offset = shape.size() - dimension();
        return stepper(this, storage_end(), offset);
    }

    template <class T, int ExtraFlags>
    inline auto pyarray<T, ExtraFlags>::stepper_begin(const shape_type& shape) const -> const_stepper
    {
        size_type offset = shape.size() - dimension();
        return const_stepper(this, storage_begin(), offset);
    }

    template <class T, int ExtraFlags>
    inline auto pyarray<T, ExtraFlags>::stepper_end(const shape_type& shape) const -> const_stepper
    {
        size_type offset = shape.size() - dimension();
        return const_stepper(this, storage_end(), offset);
    }

    template <class T, int ExtraFlags>
    inline auto pyarray<T, ExtraFlags>::storage_begin() -> storage_iterator
    {
        return reinterpret_cast<storage_iterator>(PyArray_GET_(m_ptr, data));
    }

    template <class T, int ExtraFlags>
    inline auto pyarray<T, ExtraFlags>::storage_end() -> storage_iterator
    {
        return storage_begin() + pybind_array::size();
    }

    template <class T, int ExtraFlags>
    inline auto pyarray<T, ExtraFlags>::storage_begin() const -> const_storage_iterator
    {
        return reinterpret_cast<const_storage_iterator>(PyArray_GET_(m_ptr, data));
    }

    template <class T, int ExtraFlags>
    inline auto pyarray<T, ExtraFlags>::storage_end() const -> const_storage_iterator
    {
        return storage_begin() + pybind_array::size();
    }

    template <class T, int ExtraFlags>
    template <class E>
    inline pyarray<T, ExtraFlags>::pyarray(const xexpression<E>& e)
         : pybind_array()
    {
        semantic_base::assign(e);
    }

    template <class T, int ExtraFlags>
    template <class E>
    inline auto pyarray<T, ExtraFlags>::operator=(const xexpression<E>& e) -> self_type&
    {
        return semantic_base::operator=(e);
    }

    // Private methods

    template <class T, int ExtraFlags>
    template<typename... Args> 
    inline auto pyarray<T, ExtraFlags>::index_at(Args... args) const -> size_type
    {
        return pybind_array::offset_at(args...) / itemsize();
    }

    template <class T, int ExtraFlags>
    constexpr auto pyarray<T, ExtraFlags>::itemsize() -> size_type
    {
        return sizeof(value_type);
    }

    template <class T, int ExtraFlags>
    inline bool pyarray<T, ExtraFlags>::is_non_null(PyObject* ptr)
    {
        return ptr != nullptr;
    }

    template <class T, int ExtraFlags>
    inline PyObject* pyarray<T, ExtraFlags>::ensure_(PyObject* ptr)
    {
        if (ptr == nullptr)
        {
            return nullptr;
        }
        auto& api = pybind11::detail::npy_api::get();
        PyObject *result = api.PyArray_FromAny_(ptr, pybind11::dtype::of<T>().release().ptr(), 0, 0,
                                                pybind11::detail::npy_api::NPY_ENSURE_ARRAY_ | ExtraFlags, nullptr);
        if (!result)
        {
            PyErr_Clear();
        }
        Py_DECREF(ptr);
        return result;
    }

}

#endif


