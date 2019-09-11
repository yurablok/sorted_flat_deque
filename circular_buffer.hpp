#pragma once
#include <vector>

//NOTE: Destructors of stored items may be called several times.
template <typename T>
class circular_buffer {
public:
    using position_t = uint32_t;
    using value_type = T;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    circular_buffer() {
        clear();
        set_max_size(0);
    }
    circular_buffer(const circular_buffer<T>& other) {
        *this = other;
    }
    circular_buffer(circular_buffer<T>&& other) {
        *this = std::move(other);
    }
    circular_buffer(const position_t max_size) {
        clear();
        set_max_size(max_size);
    }

    circular_buffer<T>& operator=(const circular_buffer<T>& other) {
        m_buffer = other.m_buffer;
        m_frontOffset = other.m_frontOffset;
        m_size = other.m_size;
    }
    circular_buffer<T>& operator=(circular_buffer<T>&& other) {
        m_buffer = std::move(other.m_buffer);
        m_frontOffset = other.m_frontOffset; other.m_frontOffset = 0;
        m_size = other.m_size; other.m_size = 0;
    }

    void set_max_size(
        const uint32_t new_max_size,
        const bool remove_from_front = true)
    {
        if (m_buffer.size() == new_max_size) {
            return;
        }
        else if (m_buffer.size() > new_max_size) {
            // 001111 
            // xxx    new_max_size=3
            // 000111 pop_front
            // 111000 <-
            // 111    resize
            if (remove_from_front) {
                while (m_size > new_max_size) {
                    pop_front();
                }
            }
            else {
                while (m_size > new_max_size) {
                    pop_back();
                }
            }

            if (m_frontOffset != 0) {
                for (uint32_t i = 0; i < m_size; ++i) {
                    std::swap(m_buffer[i + m_frontOffset], m_buffer[i]);
                }
                m_frontOffset = 0;
            }
            m_buffer.resize(new_max_size);
        }
        else if (m_buffer.size() < new_max_size) {
            m_buffer.resize(new_max_size);
        }
    }
    void clear() {
        m_buffer.clear();
        m_frontOffset = 0;
        m_size = 0;
    }
    void shrink_to_fit() {
        m_buffer.shrink_to_fit();
    }
    void swap(circular_buffer<T>& other) {
        std::swap(m_buffer, other.m_buffer);
        std::swap(m_frontOffset, other.m_frontOffset);
        std::swap(m_size, other.m_size);
    }

    void push_back(T&& value) {
        if (m_buffer.empty()) {
            return;
        }
        while (m_size >= m_buffer.size()) {
            pop_front();
        }
        ++m_size;
        m_buffer.at(backOffset()) = value;
    }
    void push_back(const T& value) {
        push_back(std::move(value));
    }

    void push_front(T&& value) {
        if (m_buffer.empty()) {
            return;
        }
        while (m_size >= m_buffer.size()) {
            pop_back();
        }
        if (m_frontOffset == 0) {
            m_frontOffset = static_cast<position_t>(m_buffer.size() - 1);
        }
        else {
            --m_frontOffset;
        }
        m_buffer.at(m_frontOffset) = value;
        ++m_size;
    }
    void push_front(const T& value) {
        push_front(std::move(value));
    }

    T&& pop_back() {
        if (m_size == 0) {
            T t;
            return std::move(t);
        }
        const position_t posToPop = backOffset();
        --m_size;
        return std::move(m_buffer.at(posToPop));
    }
    T&& pop_front() {
        if (m_size == 0) {
            T t;
            return std::move(t);
        }
        const position_t posToPop = m_frontOffset;
        ++m_frontOffset;
        if (m_frontOffset >= m_buffer.size()) {
            m_frontOffset = 0;
        }
        --m_size;
        return std::move(m_buffer.at(posToPop));
    }

    T& at(const position_t pos) {
        const position_t size = static_cast<position_t>(m_buffer.size());
        position_t realIndex = m_frontOffset + pos;
        if (realIndex >= size) {
            realIndex -= size;
        }
        return m_buffer.at(realIndex);
    }
    const T& at(const position_t pos) const {
        return const_cast<circular_buffer<T>*>(this)->at(pos);
    }
    T& operator[](const position_t index) {
        return this->at(index);
    }
    const T& operator[](const position_t index) const {
        return this->at(index);
    }
    T& at_idx(const position_t offset) {
        return m_buffer.at(offset);
    }
    const T& at_idx(const position_t offset) const {
        return const_cast<circular_buffer<T>*>(this)->at_idx(offset);
    }
    T& front() {
        return this->at_idx(m_frontOffset);
    }
    const T& front() const {
        return const_cast<circular_buffer<T>*>(this)->front();
    }
    position_t front_idx() const {
        return m_frontOffset;
    }
    T& back() {
        return this->at_idx(backOffset());
    }
    const T& back() const {
        return const_cast<circular_buffer<T>*>(this)->back();
    }
    position_t back_idx() const {
        return backOffset();
    }

    position_t max_size() const {
        return static_cast<position_t>(m_buffer.size());
    }
    position_t size() const {
        return m_size;
    }
    bool empty() const {
        return size() == 0;
    }

    // RandomAccessIterator
    class iterator {
    public:
        iterator() {}
        iterator(const position_t pos, circular_buffer<T>* ptr) {
            assign(pos, ptr);
        }
        void assign(const position_t pos, circular_buffer<T>* ptr) {
            m_pos = pos;
            m_ptr = ptr;
        }
        T& operator*() const {
            return m_ptr->at(m_pos);
        }
        T* operator->() const {
            return &m_ptr->at(m_pos);
        }
        bool operator==(const iterator& other) const {
            return (m_pos == other.m_pos) && (m_ptr == other.m_ptr);
        }
        bool operator!=(const iterator& other) const {
            return (m_pos != other.m_pos) || (m_ptr != other.m_ptr);
        }
        iterator& operator+=(const position_t offset) {
            m_pos += offset;
            return *this;
        }
        iterator& operator-=(const position_t offset) {
            m_pos -= offset;
            return *this;
        }
        iterator& operator++() { // Prefix increment
            this->operator+=(1);
            return *this;
        }
        iterator operator++(int) { // Postfix increment
            iterator temp = *this;
            this->operator++();
            return temp;
        }
        iterator& operator--() { // Prefix decrement
            this->operator-=(1);
            return *this;
        }
        iterator operator--(int) { // Postfix decrement
            iterator temp = *this;
            this->operator--();
            return temp;
        }
        iterator operator+(const position_t offset) const {
            iterator temp = *this;
            return temp += offset;
        }
        iterator operator-(const position_t offset) const {
            iterator temp = *this;
            return temp -= offset;
        }
        T& operator[](const position_t pos) const {
            return m_ptr->at(pos);
        }
        bool operator<(const iterator& other) const {
            return (m_ptr == other.m_ptr) && (m_pos < other.m_pos);
        }
        bool operator<=(const iterator& other) const {
            return (m_ptr == other.m_ptr) && (m_pos <= other.m_pos);
        }
        bool operator>(const iterator& other) const {
            return (m_ptr == other.m_ptr) && (m_pos > other.m_pos);
        }
        bool operator>=(const iterator& other) const {
            return (m_ptr == other.m_ptr) && (m_pos >= other.m_pos);
        }
    private:
        circular_buffer<T>* m_ptr = nullptr;
        position_t m_pos = 0;
    };
    // RandomAccessIterator
    class const_iterator {
    public:
        const_iterator() {} // construct with null vector pointer
        const_iterator(const position_t pos, const circular_buffer<T>* ptr) {
            assign(pos, ptr);
        }
        void assign(const position_t pos, const circular_buffer<T>* ptr) {
            m_pos = pos;
            m_ptr = ptr;
        }
        const T& operator*() const {
            return m_ptr->at(m_pos);
        }
        const T* operator->() const {
            return &m_ptr->at(m_pos);
        }
        bool operator==(const iterator& other) const {
            return (m_pos == other.m_pos) && (m_ptr == other.m_ptr);
        }
        bool operator!=(const iterator& other) const {
            return (m_pos != other.m_pos) || (m_ptr != other.m_ptr);
        }
        iterator& operator+=(const position_t offset) {
            m_pos += offset;
            return *this;
        }
        iterator& operator-=(const position_t offset) {
            m_pos -= offset;
            return *this;
        }
        iterator& operator++() { // Prefix increment
            this->operator+=(1);
            return *this;
        }
        iterator operator++(int) { // Postfix increment
            iterator temp = *this;
            this->operator++();
            return temp;
        }
        iterator& operator--() { // Prefix decrement
            this->operator-=(1);
            return *this;
        }
        iterator operator--(int) { // Postfix decrement
            iterator temp = *this;
            this->operator--();
            return temp;
        }
        iterator operator+(const position_t offset) const {
            iterator temp = *this;
            return temp += offset;
        }
        iterator operator-(const position_t offset) const {
            iterator temp = *this;
            return temp -= offset;
        }
        const T& operator[](const position_t pos) const {
            return m_ptr->at(pos);
        }
        bool operator<(const iterator& other) const {
            return (m_ptr == other.m_ptr) && (m_pos < other.m_pos);
        }
        bool operator<=(const iterator& other) const {
            return (m_ptr == other.m_ptr) && (m_pos <= other.m_pos);
        }
        bool operator>(const iterator& other) const {
            return (m_ptr == other.m_ptr) && (m_pos > other.m_pos);
        }
        bool operator>=(const iterator& other) const {
            return (m_ptr == other.m_ptr) && (m_pos >= other.m_pos);
        }
    private:
        const circular_buffer<value_type>* m_ptr = nullptr;
        position_t m_pos = 0;
    };
    class reverse_iterator {
        //TODO: circular_buffer reverse_iterator
        //std::reverse_iterator<iterator>
    };
    class const_reverse_iterator {
        //TODO: circular_buffer const_reverse_iterator
        //std::reverse_iterator<const_iterator>
    };

    iterator begin() {
        return iterator(0, this);
    }
    iterator end() {
        return iterator(m_size, this);
    }
    const_iterator begin() const {
        return const_iterator(0, this);
    }
    const_iterator end() const {
        return const_iterator(m_size, this);
    }
    const_iterator cbegin() const {
        return const_iterator(0, this);
    }
    const_iterator cend() const {
        return const_iterator(m_size, this);
    }

private:
    std::vector<T>   m_buffer;
    position_t       m_frontOffset;
    position_t backOffset() const {
        // size=0  front=0 back=0
        // size=1  front=0 back=0
        // size=2  front=0 back=1 || front=1 back=0
        if (m_size <= 1) {
            return m_frontOffset;
        }
        else {
            return m_frontOffset + m_size <= static_cast<position_t>(m_buffer.size())
                ? m_frontOffset + m_size - 1
                : m_frontOffset + m_size - 1 - static_cast<position_t>(m_buffer.size());
        }
    }
    position_t       m_size;
};
