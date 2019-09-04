// sorted_flat_deque
// C++11, STL-like API, bidirectional iterator, one memory allocation in the circular buffer.
//
// push - O(n/2)
// pop - O(1)
// min - O(1)
// median - O(1)
// max - O(1)
// average - O(1)
//
// Author: Yurii Blok
// License: MIT
// https://github.com/yurablok/sorted_flat_deque
// History:
// v0.1 06-Sep-19   First release.

#pragma once
#include <functional>
#include "circular_buffer.hpp"


template <typename item_t, typename value_t = item_t>
class sorted_flat_deque {
public:
    using position_t = uint32_t;
    static const position_t position_max = -1;
    using item_type = item_t;
    using value_type = value_t;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    //using accessor_t = std::function<const value_t& (const item_t& item)>;
    using comparator_t = std::function<bool(const item_t& left, const item_t& right)>;
private:
    struct node {
        position_t idx(sorted_flat_deque<item_t, value_t>* parent) {
            return static_cast<position_t>(this - &parent->m_nodes.at_idx(0));
        }
        item_t item;
        //value_t value;
        position_t prevIdx;
        position_t nextIdx;
    };
public:
    template <typename ItemT = item_t, typename ValueT = value_t,
        typename = typename std::enable_if<
            std::is_same<ItemT, ValueT>::value == true>::type>
    sorted_flat_deque() {
        init(0);
    }
    template <typename ItemT = item_t, typename ValueT = value_t,
        typename = typename std::enable_if<
            std::is_same<ItemT, ValueT>::value == false>::type,
        typename = ItemT> // Just for fix build error.
    sorted_flat_deque() {
        init(0, nullptr);
    }
    sorted_flat_deque(const sorted_flat_deque<item_t>& other) {
        *this = other;
    }
    sorted_flat_deque(sorted_flat_deque<item_t>&& other) {
        *this = std::move(other);
    }
    sorted_flat_deque(const position_t max_size) {
        init(max_size);
    }
    sorted_flat_deque(const position_t max_size, const comparator_t comparator) {
        init(max_size, comparator);
    }

    //TODO: init -> resize

    template <typename ItemT = item_t, typename ValueT = value_t>
    typename std::enable_if<
        std::is_same<ItemT, ValueT>::value == true,
        void>::type
    init(const position_t max_size) {
        m_comparator = [](const value_t& left, const value_t& right) { return left < right; };
        m_size = 0;
        m_minIdx = position_max;
        m_medianIdx = position_max;
        m_medianPos = position_max;
        m_maxIdx = position_max;
        m_nodes.init(max_size);
    }
    template <typename ItemT = item_t, typename ValueT = value_t>
    typename std::enable_if<
        std::is_same<ItemT, ValueT>::value == false,
        void>::type
    init(const position_t max_size, const comparator_t comparator) {
        m_comparator = comparator;
        m_size = 0;
        m_minIdx = position_max;
        m_medianIdx = position_max;
        m_medianPos = position_max;
        m_maxIdx = position_max;
        m_nodes.init(max_size);
    }

    sorted_flat_deque<item_t>& operator=(const sorted_flat_deque<item_t>& other) {
        m_size = other.m_size;
        m_minIdx = other.m_minIdx;
        m_medianIdx = other.m_medianIdx;
        m_medianPos = other.m_medianPos;
        m_maxIdx = other.m_maxIdx;
        m_nodes = other.m_nodes;
    }
    sorted_flat_deque<item_t>& operator=(sorted_flat_deque<item_t>&& other) {
        m_size = other.m_size; other.m_size = 0;
        m_minIdx = other.m_minIdx; other.m_minIdx = position_max;
        m_medianIdx = other.m_medianIdx; m_medianIdx = position_max;
        m_medianPos = other.m_medianPos; m_medianPos = position_max;
        m_maxIdx = other.m_maxIdx; m_maxIdx = position_max;
        m_nodes = std::move(other.m_nodes);
    }

    //void set_max_size(const position_t max_size) {
    //    //TODO: set_max_size
    //}

    void push_back(item_t&& item) {
        if (max_size() == 0) {
            return;
        }
        //m_sum += m_accessor(value);
        while (size() >= max_size()) {
            pop_front();
        }
        m_nodes.push_back(node());

        auto& back = m_nodes.back();
        const auto back_idx = m_nodes.back_idx();
        if (m_medianIdx == position_max) {
            back.nextIdx = position_max;
            back.prevIdx = position_max;
            //back.value = m_accessor(value);
            back.item = std::move(item);

            m_size = 1;
            m_minIdx = back_idx;
            m_maxIdx = back_idx;
            m_medianIdx = back_idx;
            m_medianPos = 0;
            return;
        }

        // O OM
        // O N OM
        if (m_comparator(item, m_nodes.at_idx(m_medianIdx).item)) { // <=
            node* carriage = &m_nodes.at_idx(m_medianIdx);
            m_medianPos += 1;
            while (true) {
                if (!m_comparator(item, carriage->item)) { // >=
                    back.nextIdx = carriage->nextIdx;
                    back.prevIdx = carriage->idx(this);
                    //back.value = m_accessor(value);
                    back.item = std::move(item);

                    carriage->nextIdx = back_idx;
                    m_nodes.at_idx(back.nextIdx).prevIdx = back_idx;

                    m_size += 1;
                    break;
                }
                else if (carriage->prevIdx == position_max) { // left
                    carriage->prevIdx = back_idx;
                    back.nextIdx = carriage->idx(this);
                    back.prevIdx = position_max;
                    //back.value = m_accessor(value);
                    back.item = std::move(item);

                    m_size += 1;
                    m_minIdx = back_idx;
                    break;
                }
                carriage = &m_nodes.at_idx(carriage->prevIdx);
            }
        }
        // OM O
        // OM N O
        else {
            node* carriage = &m_nodes.at_idx(m_medianIdx);
            while (true) {
                if (m_comparator(item, carriage->item)) { // <=
                    back.nextIdx = carriage->idx(this);
                    back.prevIdx = carriage->prevIdx;
                    //back.value = m_accessor(value);
                    back.item = std::move(item);

                    carriage->prevIdx = back_idx;
                    m_nodes.at_idx(back.prevIdx).nextIdx = back_idx;

                    m_size += 1;
                    break;
                }
                if (carriage->nextIdx == position_max) { // right
                    carriage->nextIdx = m_nodes.back_idx();
                    back.nextIdx = position_max;
                    back.prevIdx = carriage->idx(this);
                    //back.value = m_accessor(value);
                    back.item = std::move(item);

                    m_size += 1;
                    m_maxIdx = back_idx;
                    break;
                }
                carriage = &m_nodes.at_idx(carriage->nextIdx);
            }
        }
        const position_t desiredMedianPos = (size() ? size() - 1 : 0) >> 1;
        while (m_medianPos > desiredMedianPos) { // <-
            m_medianIdx = m_nodes.at_idx(m_medianIdx).prevIdx;
            m_medianPos -= 1;
        }
        while (m_medianPos < desiredMedianPos) { // ->
            m_medianIdx = m_nodes.at_idx(m_medianIdx).nextIdx;
            m_medianPos += 1;
        }
    }
    void push_back(const item_t& item) {
        push_back(std::move(item));
    }

    //void push_front(const item_t& item) {
    //    //TODO: push_front copy
    //}
    //void push_front(item_t&& value) {
    //    //TODO: push_front move
    //}

    item_t&& pop_front() {
        if (m_nodes.empty() || m_size == 0) {
            throw std::logic_error("m_nodes.empty()");
        }
        auto& to_remove = m_nodes.front();
        //m_sum -= to_remove.value;
        if (to_remove.prevIdx != position_max) {
            m_nodes.at_idx(to_remove.prevIdx).nextIdx = to_remove.nextIdx;
        }
        else { // left
            m_minIdx = to_remove.nextIdx;
        }
        if (to_remove.nextIdx != position_max) {
            m_nodes.at_idx(to_remove.nextIdx).prevIdx = to_remove.prevIdx;
        }
        else { // right
            m_maxIdx = to_remove.prevIdx;
        }
        m_size -= 1;

        if (m_medianIdx == m_nodes.front_idx()) {
            m_medianIdx = m_nodes.front().prevIdx;
            if (m_medianPos > 0) {
                m_medianPos -= 1;
            }
        }
        if (m_medianIdx != position_max) {
            if (m_comparator(to_remove.item, m_nodes.at_idx(m_medianIdx).item) // <=
                    && m_medianPos) {
                m_medianPos -= 1;
            }
            const position_t desiredMedianPos = (size() ? size() - 1 : 0) >> 1;
            while (m_medianPos > desiredMedianPos) { // <-
                m_medianIdx = m_nodes.at_idx(m_medianIdx).prevIdx;
                m_medianPos -= 1;
            }
            while (m_medianPos < desiredMedianPos) { // ->
                m_medianIdx = m_nodes.at_idx(m_medianIdx).nextIdx;
                m_medianPos += 1;
            }
        }
        return std::move(m_nodes.pop_front().item);
    }
    //item_t&& pop_back() {
    //    //TODO: pop_back
    //}
    
    item_t& min() const {
        if (m_minIdx == position_max) {
            throw std::logic_error("m_min == position_max");
        }
        else {
            return m_nodes.at_idx(m_minIdx).item;
        }
    }
    item_t& median() const {
        if (m_medianIdx == position_max) {
            throw std::logic_error("m_middle == position_max");
        }
        else {
            return m_nodes.at_idx(m_medianIdx).item;
        }
    }
    item_t& max() const {
        if (m_maxIdx == position_max) {
            throw std::logic_error("m_max == position_max");
        }
        else {
            return m_nodes.at_idx(m_maxIdx).item;
        }
    }
    //value_t average() const {
    //    return m_sum / static_cast<value_t>(m_nodes.size());
    //}
    position_t size() const {
        return m_size;
    }
    position_t max_size() const {
        return m_nodes.max_size();
    }
    bool empty() const {
        return m_size == 0;
    }
    void clear() {
        m_nodes.clear();
        m_size = 0;
        m_minIdx = position_max;
        m_medianIdx = position_max;
        m_medianPos = position_max;
        m_maxIdx = position_max;
        //m_sum = 0;
    }
    void swap(sorted_flat_deque<item_t, value_t>& other) {
        std::swap(m_comparator, other.m_comparator);
        std::swap(m_nodes, other.m_nodes);
        std::swap(m_size, other.m_size);
        std::swap(m_minIdx, other.m_minIdx);
        std::swap(m_medianIdx, other.m_medianIdx);
        std::swap(m_medianPos, other.m_medianPos);
        std::swap(m_maxIdx, other.m_maxIdx);
        //std::swap(m_sum, other.m_sum);
    }
    void shrink_to_fit() {
        m_nodes.shrink_to_fit();
    }

    // BidirectionalIterator
    class iterator {
    public:
        iterator() {}
        iterator(const position_t nodeIdx, sorted_flat_deque<item_t, value_t>* ptr) {
            assign(nodeIdx, ptr);
        }
        void assign(const position_t nodeIdx, sorted_flat_deque<item_t, value_t>* ptr) {
            m_nodeIdx = nodeIdx;
            m_ptr = ptr;
        }
        item_t& operator*() const {
            return m_ptr->m_nodes.at_idx(m_nodeIdx).item;
        }
        item_t* operator->() const {
            return &m_ptr->m_nodes.at_idx(m_nodeIdx).item;
        }
        bool operator==(const iterator& other) const {
            return (m_nodeIdx == other.m_nodeIdx) && (m_ptr == other.m_ptr);
        }
        bool operator!=(const iterator& other) const {
            return (m_nodeIdx != other.m_nodeIdx) || (m_ptr != other.m_ptr);
        }

        iterator& operator++() { // Prefix increment
            if (m_nodeIdx == position_max) {
                throw std::logic_error("m_nodeIdx == position_max");
            }
            m_nodeIdx = m_ptr->m_nodes.at_idx(m_nodeIdx).nextIdx;
            return *this;
        }
        iterator operator++(int) { // Postfix increment
            iterator temp = *this;
            this->operator++();
            return temp;
        }
        iterator& operator--() { // Prefix decrement
            if (m_nodeIdx == position_max) {
                if (m_ptr->m_maxIdx == position_max) {
                    throw std::logic_error("m_nodeIdx == position_max && m_maxIdx == position_max");
                }
                // If it==end but the container is not empty
                m_nodeIdx = m_ptr->m_maxIdx;
                return *this;
            }
            if (m_ptr->m_nodes.at_idx(m_nodeIdx).prevIdx == position_max) {
                throw std::logic_error("prevIdx == position_max");
            }
            m_nodeIdx = m_ptr->m_nodes.at_idx(m_nodeIdx).prevIdx;
            return *this;
        }
        iterator operator--(int) { // Postfix decrement
            iterator temp = *this;
            this->operator--();
            return temp;
        }

        // We cannot use <,<=,>,>= with a comparator returning bool.
        // To do this, we need to use a comparator that returns int.
        // But if we use a comparator that returns int, then the default
        // comparator will not work for unsigned int elements.
        // it  < it  -> ?
        // it  < end -> true
        // end < it  -> false
        // end < end -> false
        //bool operator<(const iterator& other) const;
        // it  <= it  -> ?
        // it  <= end -> true
        // end <= it  -> false
        // end <= end -> true
        //bool operator<=(const iterator& other) const;
        // it  > it  -> ?
        // it  > end -> false
        // end > it  -> true
        // end > end -> false
        //bool operator>(const iterator& other) const;
        // it  >= it  -> ?
        // it  >= end -> false
        // end >= it  -> true
        // end >= end -> true
        //bool operator>=(const iterator& other) const;
    private:
        sorted_flat_deque<item_t, value_t>* m_ptr = nullptr;
        position_t m_nodeIdx = -1;
    };
    // BidirectionalIterator
    class const_iterator {
    public:
        const_iterator() {}
        const_iterator(const position_t nodeIdx, sorted_flat_deque<item_t, value_t>* ptr) {
            assign(nodeIdx, ptr);
        }
        void assign(const position_t nodeIdx, sorted_flat_deque<item_t, value_t>* ptr) {
            m_nodeIdx = nodeIdx;
            m_ptr = ptr;
        }
        const item_t& operator*() const {
            return m_ptr->m_nodes.at_idx(m_nodeIdx).item;
        }
        const item_t* operator->() const {
            return &m_ptr->m_nodes.at_idx(m_nodeIdx).item;
        }
        bool operator==(const iterator& other) const {
            return (m_nodeIdx == other.m_nodeIdx) && (m_ptr == other.m_ptr);
        }
        bool operator!=(const iterator& other) const {
            return (m_nodeIdx != other.m_nodeIdx) || (m_ptr != other.m_ptr);
        }

        iterator& operator++() { // Prefix increment
            if (m_nodeIdx == position_max) {
                throw std::logic_error("m_nodeIdx == position_max");
            }
            m_nodeIdx = m_ptr->m_nodes.at_idx(m_nodeIdx).nextIdx;
            return *this;
        }
        iterator operator++(int) { // Postfix increment
            iterator temp = *this;
            this->operator++();
            return temp;
        }
        iterator& operator--() { // Prefix decrement
            if (m_nodeIdx == position_max) {
                if (m_ptr->m_maxIdx == position_max) {
                    throw std::logic_error("m_nodeIdx == position_max && m_maxIdx == position_max");
                }
                // If it==end but the container is not empty
                m_nodeIdx = m_ptr->m_maxIdx;
                return *this;
            }
            if (m_ptr->m_nodes.at_idx(m_nodeIdx).prevIdx == position_max) {
                throw std::logic_error("prevIdx == position_max");
            }
            m_nodeIdx = m_ptr->m_nodes.at_idx(m_nodeIdx).prevIdx;
            return *this;
        }
        iterator operator--(int) { // Postfix decrement
            iterator temp = *this;
            this->operator--();
            return temp;
        }

    private:
        const sorted_flat_deque<item_t, value_t>* m_ptr = nullptr;
        position_t m_nodeIdx = -1;
    };
    //class reverse_iterator {
    //    //TODO: reverse_iterator
    //    //std::reverse_iterator<iterator>
    //};
    //class const_reverse_iterator {
    //    //TODO: const_reverse_iterator
    //    //std::reverse_iterator<const_iterator>
    //};

    iterator begin() {
        return iterator(m_minIdx, this);
    }
    iterator it_median() {
        return iterator(m_medianIdx, this);
    }
    iterator end() {
        return iterator(position_max, this);
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
    comparator_t m_comparator;
    mutable circular_buffer<node> m_nodes;
    position_t m_size = 0;
    position_t m_minIdx = position_max;
    position_t m_medianIdx = position_max;
    position_t m_medianPos = position_max;
    position_t m_maxIdx = position_max;
    //value_t m_sum = 0;
};
