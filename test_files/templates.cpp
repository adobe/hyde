/*
Copyright 2018 Adobe
All Rights Reserved.

NOTICE: Adobe permits you to use, modify, and distribute this file in
accordance with the terms of the Adobe license agreement accompanying
it. If you have received this file from a source other than Adobe,
then your use, modification, or distribution of it requires the prior
written permission of Adobe. 
*/

// AST dump with
// clang --std=c++1z -Xclang -ast-dump -fsyntax-only ./test_files/templates.cpp

#include <functional>

/**************************************************************************************************/
#ifndef ADOBE_TOOL_HYDE
template <std::size_t N, class... EventArgs>
struct banana {
    template <std::size_t M, class... Args>
    bool operator()(Args&&... args) const {
        return _f(std::forward<Args>(args)...);
    }

private:
    std::function<bool(EventArgs...)> _f;
};

/**************************************************************************************************/

template <class Seq1, class Seq2>
struct index_sequence_cat;

template <std::size_t... N1, std::size_t... N2>
struct index_sequence_cat<std::index_sequence<N1...>, std::index_sequence<N2...>> {
    using type = std::index_sequence<N1..., N2...>;
};

template <class Seq1, class Seq2>
using index_sequence_cat_t = typename index_sequence_cat<Seq1, Seq2>::type;

/**************************************************************************************************/

template <class Seq>
struct index_sequence_to_array;

template <std::size_t... N>
struct index_sequence_to_array<std::index_sequence<N...>> {
    static constexpr std::array<std::size_t, sizeof...(N)> value{{N...}};
};

/**************************************************************************************************/

template <class Seq, template<std::size_t> class F, std::size_t Index, std::size_t Count>
struct index_sequence_transform;

template <class Seq, template<std::size_t> class F, std::size_t Index = 0, std::size_t Count = Seq::size()>
using index_sequence_transform_t = typename index_sequence_transform<Seq, F, Index, Count>::type;

template <class Seq, template<std::size_t> class F, std::size_t Index, std::size_t Count>
struct index_sequence_transform {
    using type = index_sequence_cat_t<index_sequence_transform_t<Seq, F, Index, Count / 2>,
        index_sequence_transform_t<Seq, F, Index + Count / 2, Count - Count / 2>>;
};

template <class Seq, template<std::size_t> class F, std::size_t Index>
struct index_sequence_transform<Seq, F, Index, 0> {
    using type = std::index_sequence<>;
};

template <class Seq, template<std::size_t> class F, std::size_t Index>
struct index_sequence_transform<Seq, F, Index, 1> {
    using type = typename F<index_sequence_to_array<Seq>::value[Index]>::type;
};

/**************************************************************************************************/

template <bool, class T>
struct move_if_helper;

template <class T>
struct move_if_helper<true, T> {
    using type = std::remove_reference_t<T>&&;
};

template <class T>
struct move_if_helper<false, T> {
    using type = std::remove_reference_t<T>&;
};

template <bool P, class T>
using move_if_helper_t = typename move_if_helper<P, T>::type;

#else
template <typename T>
struct cow {
    using element_type = T;
    operator const element_type& () const;
    std::string to_string(const element_type& x) const;
    template <typename U>
    std::string to_string(const element_type& x, U&& y) const;
};
#endif
/**************************************************************************************************/
