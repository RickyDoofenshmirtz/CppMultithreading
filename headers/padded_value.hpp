#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

template <typename T>
class padded_value
{
public:
    using value_type = T;

    constexpr padded_value() noexcept = default;

    explicit constexpr padded_value(value_type value) noexcept
        : m_value(value)
    {
    }

    [[nodiscard]] constexpr auto get() -> value_type& { return m_value; }
    [[nodiscard]] constexpr auto get() const -> value_type { return m_value; }

    [[nodiscard]]
    constexpr auto padding_size() const -> std::size_t
    {
        return m_padding.size();
    }

    [[nodiscard]] constexpr auto size() const -> std::size_t
    {
        return sizeof(value_type) + m_padding.size();
    }

private:
    value_type m_value{};
    std::array<std::uint8_t, 64 - sizeof(value_type)> m_padding{};
};

[[maybe_unused]]
static constexpr auto VALUE_TYPE_SIZE = sizeof(padded_value<int>);