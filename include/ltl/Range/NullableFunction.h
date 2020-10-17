#pragma once

#include <cassert>
#include <cstddef>
#include "../ltl.h"
#include "../invoke.h"

namespace ltl {

#if defined LTL_ALLOW_UNDEFINED_BEHAVIOUR
template <typename F, typename = void>
#else
template <typename F>
#endif
struct NullableFunction {
    NullableFunction() = default;
    NullableFunction(F f) : m_function{new (&m_buffer) F{std::move(f)}} {}

    ~NullableFunction() { deleteFunction(); }

    NullableFunction(const NullableFunction &f) { copyFunction(f); }
    NullableFunction(NullableFunction &&f) { moveFunction(std::move(f)); }

    NullableFunction &operator=(NullableFunction f) {
        deleteFunction();
        moveFunction(std::move(f));
        return *this;
    }

    template <typename... Args>
    decltype(auto) operator()(Args &&... args) const {
        assert(m_function);
        if constexpr (std::is_member_pointer_v<F>)
            return ltl::invoke(*m_function, FWD(args)...);
        else
            return (*m_function)(FWD(args)...);
    }

  private:
    void deleteFunction() {
        if (m_function)
            m_function->~F();
    }

    void copyFunction(const NullableFunction &f) {
        if (f.m_function) {
            m_function = new (&m_buffer) F{*f.m_function};
        } else {
            m_function = nullptr;
        }
    }

    void moveFunction(NullableFunction &&f) {
        if (f.m_function) {
            m_function = new (&m_buffer) F{std::move(*f.m_function)};
        } else {
            m_function = nullptr;
        }
    }

    alignas(F) std::byte m_buffer[sizeof(F)];
    F *m_function = nullptr;
};

#if defined LTL_ALLOW_UNDEFINED_BEHAVIOUR

template <typename F>
struct NullableFunction<F, std::enable_if_t<std::is_empty_v<F>>> {
    NullableFunction() = default;
    NullableFunction(F) {}

    template <typename... Args>
    decltype(auto) operator()(Args &&... args) const {
        return (*(F *)(nullptr))(FWD(args)...);
    }
};

#endif

struct Nothing {};

} // namespace ltl
