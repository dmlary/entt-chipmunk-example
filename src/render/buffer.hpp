#pragma once
#include <initializer_list>
#include <sokol_gfx.h>
#include <vector>

namespace render {

template <typename T>
class Buffer {
public:
    inline const void *data() const { return m_data.data(); }
    inline size_t size() const { return sizeof(T) * m_data.size(); }
    inline size_t stride() const { return sizeof(T); }
    inline size_t elements() const { return m_data.size(); }
    inline void push_back(const T&& v) { m_data.push_back(v); }
    inline void append(std::initializer_list<T>ilist) {
        m_data.insert(m_data.end(), ilist);
    }
    inline void clear() { m_data.clear(); }
    inline void reserve(size_t count) { m_data.reserve(count); }

    inline T& at(size_t pos) {
        assert(pos < m_data.size());
        return m_data[pos];
    }

    virtual const sg_range sg_range() const {
        return { .ptr = data(), .size = size() };
    }
private:
    std::vector<T> m_data;
};

using IndexBuffer = Buffer<uint16_t>;

} // namespace render
