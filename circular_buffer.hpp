#pragma once
#include <vector>

template <typename T>
class circular_buffer {
public:
    circular_buffer() {}
    circular_buffer(const circular_buffer& other) {
        init(0);
    }
    circular_buffer(const uint32_t max_size) {
        init(max_size);
    }
    void init(const uint32_t max_size) {
        m_buffer.clear();
        m_readPos = 0;
        m_writePos = 0;
        m_size = 0;
        m_buffer.resize(max_size);
        //set_max_size(max_size);
    }
    //void set_max_size(const uint32_t new_max_size)
    //{
    //    if (m_buffer.size() == new_max_size) {
    //        return;
    //    }
    //    else if (m_buffer.size() < new_max_size) {
    //        // 001111 
    //        // xxx    new_max_size=3
    //        // 000111 pop_back
    //        // 111000 <-
    //        // 111    resize
    //        while (m_size > new_max_size) {
    //            pop();
    //        }
    //        m_buffer.resize(new_max_size);
    //    }
    //    else if (m_buffer.size() > new_max_size) {
    //        m_buffer.resize(new_max_size);
    //    }
    //}
    void push(const T& value) {
        while (m_size >= m_buffer.size()) {
            pop();
        }
        m_buffer[m_writePos] = value;
        ++m_writePos;
        if (m_writePos >= m_buffer.size()) {
            m_writePos = 0;
        }
        ++m_size;
    }
    void push() {
        T t;
        push(t);
    }
    T pop() {
        const auto ret = m_buffer[m_readPos];
        if (m_size == 0) {
            return ret;
        }
        if (m_size > 0) {
            --m_size;
        }
        ++m_readPos;
        if (m_readPos >= m_buffer.size()) {
            m_readPos = 0;
        }
        return ret;
    }
    T& at(const uint32_t index) {
        uint32_t realIndex = m_readPos + index;
        if (realIndex >= m_buffer.size()) {
            realIndex -= m_buffer.size();
        }
        return m_buffer.at(realIndex);
    }
    const T& at(const uint32_t index) const {
        return const_cast<circular_buffer<T>*>(this)->at(index);
    }
    T& operator[](const uint32_t index) {
        return this->at(index);
    }
    const T& operator[](const uint32_t index) const {
        return this->at(index);
    }
    T& at_idx(const uint32_t offset) {
        return m_buffer[offset];
    }
    const T& at_idx(const uint32_t offset) const {
        return const_cast<circular_buffer<T>*>(this)->at_idx(offset);
    }
    T& front() {
        return this->at(0);
    }
    const T& front() const {
        return const_cast<circular_buffer<T>*>(this)->front();
    }
    uint32_t front_idx() const {
        return m_readPos;
    }
    T& back() {
        return this->at(size() - 1);
    }
    const T& back() const {
        return const_cast<circular_buffer<T>*>(this)->back();
    }
    uint32_t back_idx() const {
        uint32_t realIndex = m_readPos + m_size - 1;
        if (realIndex >= m_buffer.size()) {
            realIndex -= m_buffer.size();
        }
        return realIndex;
    }
    uint32_t maxSize() const {
        return m_buffer.size();
    }
    uint32_t size() const {
        return m_size;
    }
    bool empty() const {
        return size() == 0;
    }

    class iterator {
    };
    class const_iterator {
    };

    iterator begin() {
    }
    iterator end() {
    }

    const_iterator cbegin() {
    }
    const_iterator cend() {
    }

private:
    std::vector<T>   m_buffer;
    uint32_t         m_readPos;
    uint32_t         m_writePos;
    uint32_t         m_size;
};
