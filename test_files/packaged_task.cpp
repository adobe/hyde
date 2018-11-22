/*
    Copyright 2015 Adobe
    Distributed under the Boost Software License, Version 1.0.
    (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/

/**************************************************************************************************/

#ifndef HYDE_TEST_FILE_PACKAGED_TASK
#define HYDE_TEST_FILE_PACKAGED_TASK

#include <initializer_list>
#include <memory>

/**************************************************************************************************/

namespace stlab {

/**************************************************************************************************/

inline namespace v1 {

/**************************************************************************************************/

template <typename...>
class packaged_task;

/**************************************************************************************************/

namespace detail {

/**************************************************************************************************/

template <typename>
struct packaged_task_from_signature;

template <typename R, typename... Args>
struct packaged_task_from_signature<R(Args...)> {
    using type = packaged_task<Args...>;
};

template <typename T>
using packaged_task_from_signature_t = typename packaged_task_from_signature<T>::type;

/**************************************************************************************************/

} // namespace detail

/**************************************************************************************************/

template <typename... Args>
class packaged_task {
    using ptr_t = std::weak_ptr<int>;

    ptr_t _p;

    explicit packaged_task(ptr_t p) : _p(std::move(p)) {}

    template <typename Signature, typename E, typename F>
    friend auto package(E, F&&) -> std::pair<detail::packaged_task_from_signature_t<Signature>,
                                              int>;

    template <typename Signature, typename E, typename F>
    friend auto package_with_broken_promise(E, F&&)
        -> std::pair<detail::packaged_task_from_signature_t<Signature>,
                     int>;

public:
    packaged_task() = default;

    ~packaged_task() {
        auto p = _p.lock();
        if (p) p->remove_promise();
    }

    packaged_task(const packaged_task& x) : _p(x._p) {
        auto p = _p.lock();
        if (p) p->add_promise();
    }

    packaged_task(packaged_task&&) noexcept = default;
    packaged_task& operator=(const packaged_task& x) {
        auto tmp = x;
        *this = std::move(tmp);
        return *this;
    }
    packaged_task& operator=(packaged_task&& x) noexcept = default;

    template <typename... A>
    void operator()(A&&... args) const {
        auto p = _p.lock();
        if (p) (*p)(std::forward<A>(args)...);
    }

    void set_exception(std::exception_ptr error) {
        auto p = _p.lock();
        if (p) p->set_error(std::move(error));
    }
};

/**************************************************************************************************/

} // namespace v1

/**************************************************************************************************/

} // namespace stlab

/**************************************************************************************************/

#endif // HYDE_TEST_FILE_PACKAGED_TASK
