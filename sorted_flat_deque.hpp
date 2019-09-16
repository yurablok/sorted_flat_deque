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
// v0.2 16-Sep-19   Added set_max_size.

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
            return static_cast<position_t>(this - &parent->m_nodes.at_offset(0));
        }
        item_t item;
        //value_t value;
        position_t prevOffset;
        position_t nextOffset;
    };
public:
    sorted_flat_deque() {
        clear();
        set_comparator(nullptr);
        set_max_size(0);
    }
    sorted_flat_deque(const sorted_flat_deque<item_t>& other) {
        *this = other;
    }
    sorted_flat_deque(sorted_flat_deque<item_t>&& other) {
        *this = std::move(other);
    }
    template <typename ItemT = item_t, typename ValueT = value_t,
        typename = typename std::enable_if<
            std::is_same<ItemT, ValueT>::value == true>::type>
    sorted_flat_deque(const position_t max_size, const comparator_t comparator = nullptr) {
        clear();
        set_comparator(comparator);
        set_max_size(max_size);
    }
    template <typename ItemT = item_t, typename ValueT = value_t,
        typename = typename std::enable_if<
            std::is_same<ItemT, ValueT>::value == false>::type,
        typename = void> // Just for fix build error.
    sorted_flat_deque(const position_t max_size, const comparator_t comparator) {
        clear();
        set_comparator(comparator);
        set_max_size(max_size);
    }

    sorted_flat_deque<item_t>& operator=(const sorted_flat_deque<item_t>& other) {
        m_size = other.m_size;
        m_minOffset = other.m_minOffset;
        m_medianOffset = other.m_medianOffset;
        m_medianPos = other.m_medianPos;
        m_maxOffset = other.m_maxOffset;
        m_nodes = other.m_nodes;
        return *this;
    }
    sorted_flat_deque<item_t>& operator=(sorted_flat_deque<item_t>&& other) {
        m_size = other.m_size; other.m_size = 0;
        m_minOffset = other.m_minOffset; other.m_minOffset = position_max;
        m_medianOffset = other.m_medianOffset; m_medianOffset = position_max;
        m_medianPos = other.m_medianPos; m_medianPos = position_max;
        m_maxOffset = other.m_maxOffset; m_maxOffset = position_max;
        m_nodes = std::move(other.m_nodes);
        return *this;
    }

    template <typename ItemT = item_t, typename ValueT = value_t>
        typename std::enable_if<
            std::is_same<ItemT, ValueT>::value == true, void>::
    type set_comparator(const comparator_t comparator = nullptr) {
        if (comparator) {
            m_comparator = comparator;
        }
        else {
            m_comparator = [](const value_t& left, const value_t& right) { return left < right; };
        }
    }
    template <typename ItemT = item_t, typename ValueT = value_t>
        typename std::enable_if<
            std::is_same<ItemT, ValueT>::value == false, void>::
    type set_comparator(const comparator_t comparator) {
        m_comparator = comparator;
    }

    void set_max_size(const uint32_t max_size, const bool remove_from_front = true) {
        if (m_nodes.size() == max_size) {
            return;
        }
        if (m_nodes.size() > max_size) {
            if (remove_from_front) {
                while (m_size > max_size) {
                    pop_front();
                }
            }
            else {
                while (m_size > max_size) {
                    pop_back();
                }
            }
        }

        sorted_flat_deque<item_t, value_t> temp;
        temp.swap(*this);
        clear();
        m_comparator = temp.m_comparator;
        m_nodes.set_max_size(max_size, remove_from_front);
        for (auto it = temp.begin(); it != temp.end(); ++it) {
            push_back(std::move(it.extract()));
        }
    }
    void clear() {
        m_nodes.clear();
        m_size = 0;
        m_minOffset = position_max;
        m_medianOffset = position_max;
        m_medianPos = position_max;
        m_maxOffset = position_max;
        //m_sum = 0;
    }
    void shrink_to_fit() {
        m_nodes.shrink_to_fit();
    }
    void swap(sorted_flat_deque<item_t, value_t>& other) {
        std::swap(m_comparator, other.m_comparator);
        std::swap(m_nodes, other.m_nodes);
        std::swap(m_size, other.m_size);
        std::swap(m_minOffset, other.m_minOffset);
        std::swap(m_medianOffset, other.m_medianOffset);
        std::swap(m_medianPos, other.m_medianPos);
        std::swap(m_maxOffset, other.m_maxOffset);
        //std::swap(m_sum, other.m_sum);
    }

    void push_back(item_t&& item) {
        push_back_impl(std::move(item));
    }
    void push_back(const item_t& item) {
        push_back_impl(item);
    }

    void push_front(item_t&& value) {
        push_front_impl(std::move(value));
    }
    void push_front(const item_t& item) {
        push_front_impl(item);
    }

    item_t&& pop_front() {
        if (m_nodes.empty() || m_size == 0) {
            throw std::logic_error("m_nodes.empty()");
        }
        auto& to_remove = m_nodes.front();
        //m_sum -= to_remove.value;
        if (to_remove.prevOffset != position_max) {
            m_nodes.at_offset(to_remove.prevOffset).nextOffset = to_remove.nextOffset;
        }
        else { // left
            m_minOffset = to_remove.nextOffset;
        }
        if (to_remove.nextOffset != position_max) {
            m_nodes.at_offset(to_remove.nextOffset).prevOffset = to_remove.prevOffset;
        }
        else { // right
            m_maxOffset = to_remove.prevOffset;
        }
        m_size -= 1;

        if (m_medianOffset == m_nodes.front_offset()) {
            m_medianOffset = m_nodes.front().prevOffset;
            if (m_medianPos > 0) {
                m_medianPos -= 1;
            }
        }
        if (m_medianOffset != position_max) {
            if (m_comparator(to_remove.item, m_nodes.at_offset(m_medianOffset).item) // <=
                    && m_medianPos) {
                m_medianPos -= 1;
            }
            update_median_pos();
        }
        return std::move(m_nodes.pop_front().item);
    }
    item_t&& pop_back() {
        if (m_nodes.empty() || m_size == 0) {
            throw std::logic_error("m_nodes.empty()");
        }
        auto& to_remove = m_nodes.back();
        //m_sum -= to_remove.value;
        if (to_remove.prevOffset != position_max) {
            m_nodes.at_offset(to_remove.prevOffset).nextOffset = to_remove.nextOffset;
        }
        else { // left
            m_minOffset = to_remove.nextOffset;
        }
        if (to_remove.nextOffset != position_max) {
            m_nodes.at_offset(to_remove.nextOffset).prevOffset = to_remove.prevOffset;
        }
        else { // right
            m_maxOffset = to_remove.prevOffset;
        }
        m_size -= 1;

        if (m_medianOffset == m_nodes.back_offset()) {
            m_medianOffset = m_nodes.back().prevOffset;
            if (m_medianPos > 0) {
                m_medianPos -= 1;
            }
        }
        if (m_medianOffset != position_max) {
            if (m_comparator(to_remove.item, m_nodes.at_offset(m_medianOffset).item) // <=
                && m_medianPos) {
                m_medianPos -= 1;
            }
            update_median_pos();
        }
        return std::move(m_nodes.pop_back().item);
    }
    
    item_t& min() const {
        if (m_minOffset == position_max) {
            throw std::logic_error("m_min == position_max");
        }
        else {
            return m_nodes.at_offset(m_minOffset).item;
        }
    }
    item_t& median() const {
        if (m_medianOffset == position_max) {
            throw std::logic_error("m_middle == position_max");
        }
        else {
            return m_nodes.at_offset(m_medianOffset).item;
        }
    }
    item_t& max() const {
        if (m_maxOffset == position_max) {
            throw std::logic_error("m_max == position_max");
        }
        else {
            return m_nodes.at_offset(m_maxOffset).item;
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

    // BidirectionalIterator
    class iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;

        iterator() {}
        iterator(const position_t nodeIdx, sorted_flat_deque<item_t, value_t>* ptr) {
            assign(nodeIdx, ptr);
        }
        void assign(const position_t nodeIdx, sorted_flat_deque<item_t, value_t>* ptr) {
            m_nodeIdx = nodeIdx;
            m_ptr = ptr;
        }
        //TODO: make this iterator compatible with std::make_move_iterator
        item_t&& extract() {
            return std::move(m_ptr->m_nodes.at_offset(m_nodeIdx).item);
        }
        item_t& operator*() const {
            return m_ptr->m_nodes.at_offset(m_nodeIdx).item;
        }
        item_t* operator->() const {
            return &m_ptr->m_nodes.at_offset(m_nodeIdx).item;
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
            m_nodeIdx = m_ptr->m_nodes.at_offset(m_nodeIdx).nextOffset;
            return *this;
        }
        iterator operator++(int) { // Postfix increment
            iterator temp = *this;
            this->operator++();
            return temp;
        }
        iterator& operator--() { // Prefix decrement
            if (m_nodeIdx == position_max) {
                if (m_ptr->m_maxOffset == position_max) {
                    throw std::logic_error("m_nodeIdx == position_max && m_maxIdx == position_max");
                }
                // If it==end but the container is not empty
                m_nodeIdx = m_ptr->m_maxOffset;
                return *this;
            }
            if (m_ptr->m_nodes.at_offset(m_nodeIdx).prevOffset == position_max) {
                throw std::logic_error("prevOffset == position_max");
            }
            m_nodeIdx = m_ptr->m_nodes.at_offset(m_nodeIdx).prevOffset;
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
        using iterator_category = std::bidirectional_iterator_tag;

        const_iterator() {}
        const_iterator(const position_t nodeIdx, const sorted_flat_deque<item_t, value_t>* ptr) {
            assign(nodeIdx, ptr);
        }
        void assign(const position_t nodeIdx, const sorted_flat_deque<item_t, value_t>* ptr) {
            m_nodeIdx = nodeIdx;
            m_ptr = ptr;
        }
        const item_t& operator*() const {
            return m_ptr->m_nodes.at_offset(m_nodeIdx).item;
        }
        const item_t* operator->() const {
            return &m_ptr->m_nodes.at_offset(m_nodeIdx).item;
        }
        bool operator==(const const_iterator& other) const {
            return (m_nodeIdx == other.m_nodeIdx) && (m_ptr == other.m_ptr);
        }
        bool operator!=(const const_iterator& other) const {
            return (m_nodeIdx != other.m_nodeIdx) || (m_ptr != other.m_ptr);
        }

        const_iterator& operator++() { // Prefix increment
            if (m_nodeIdx == position_max) {
                throw std::logic_error("m_nodeIdx == position_max");
            }
            m_nodeIdx = m_ptr->m_nodes.at_offset(m_nodeIdx).nextOffset;
            return *this;
        }
        const_iterator operator++(int) { // Postfix increment
            const_iterator temp = *this;
            this->operator++();
            return temp;
        }
        const_iterator& operator--() { // Prefix decrement
            if (m_nodeIdx == position_max) {
                if (m_ptr->m_maxOffset == position_max) {
                    throw std::logic_error("m_nodeIdx == position_max && m_maxIdx == position_max");
                }
                // If it==end but the container is not empty
                m_nodeIdx = m_ptr->m_maxOffset;
                return *this;
            }
            if (m_ptr->m_nodes.at_offset(m_nodeIdx).prevOffset == position_max) {
                throw std::logic_error("prevOffset == position_max");
            }
            m_nodeIdx = m_ptr->m_nodes.at_offset(m_nodeIdx).prevOffset;
            return *this;
        }
        const_iterator operator--(int) { // Postfix decrement
            const_iterator temp = *this;
            this->operator--();
            return temp;
        }

    private:
        const sorted_flat_deque<item_t, value_t>* m_ptr = nullptr;
        position_t m_nodeIdx = position_max;
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
        return iterator(m_minOffset, this);
    }
    iterator median_it() {
        return iterator(m_medianOffset, this);
    }
    iterator end() {
        return iterator(position_max, this);
    }
    const_iterator begin() const {
        return const_iterator(m_minOffset, this);
    }
    const_iterator median_it() const {
        return const_iterator(m_medianOffset, this);
    }
    const_iterator end() const {
        return const_iterator(position_max, this);
    }
    const_iterator cbegin() const {
        return const_iterator(m_minOffset, this);
    }
    const_iterator cmedian_it() const {
        return const_iterator(m_medianOffset, this);
    }
    const_iterator cend() const {
        return const_iterator(position_max, this);
    }

private:
    template <typename ItemT>
    void push_back_impl(ItemT item) {
        if (max_size() == 0) {
            return;
        }
        //m_sum += m_accessor(value);
        while (size() >= max_size()) {
            pop_front();
        }
        m_nodes.push_back(node());

        auto& back = m_nodes.back();
        const auto back_offset = m_nodes.back_offset();
        if (m_medianOffset == position_max) {
            back.nextOffset = position_max;
            back.prevOffset = position_max;
            //back.value = m_accessor(value);
            back.item = std::move(item);

            m_size = 1;
            m_minOffset = back_offset;
            m_maxOffset = back_offset;
            m_medianOffset = back_offset;
            m_medianPos = 0;
            return;
        }

        // O OM
        // O N OM
        if (m_comparator(item, m_nodes.at_offset(m_medianOffset).item)) { // <=
            node* carriage = &m_nodes.at_offset(m_medianOffset);
            m_medianPos += 1;
            while (true) {
                if (!m_comparator(item, carriage->item)) { // >=
                    back.nextOffset = carriage->nextOffset;
                    back.prevOffset = carriage->idx(this);
                    //back.value = m_accessor(value);
                    back.item = std::move(item);

                    carriage->nextOffset = back_offset;
                    m_nodes.at_offset(back.nextOffset).prevOffset = back_offset;

                    m_size += 1;
                    break;
                }
                else if (carriage->prevOffset == position_max) { // left
                    carriage->prevOffset = back_offset;
                    back.nextOffset = carriage->idx(this);
                    back.prevOffset = position_max;
                    //back.value = m_accessor(value);
                    back.item = std::move(item);

                    m_size += 1;
                    m_minOffset = back_offset;
                    break;
                }
                carriage = &m_nodes.at_offset(carriage->prevOffset);
            }
        }
        // OM O
        // OM N O
        else {
            node* carriage = &m_nodes.at_offset(m_medianOffset);
            while (true) {
                if (m_comparator(item, carriage->item)) { // <=
                    back.nextOffset = carriage->idx(this);
                    back.prevOffset = carriage->prevOffset;
                    //back.value = m_accessor(value);
                    back.item = std::move(item);

                    carriage->prevOffset = back_offset;
                    m_nodes.at_offset(back.prevOffset).nextOffset = back_offset;

                    m_size += 1;
                    break;
                }
                if (carriage->nextOffset == position_max) { // right
                    carriage->nextOffset = m_nodes.back_offset();
                    back.nextOffset = position_max;
                    back.prevOffset = carriage->idx(this);
                    //back.value = m_accessor(value);
                    back.item = std::move(item);

                    m_size += 1;
                    m_maxOffset = back_offset;
                    break;
                }
                carriage = &m_nodes.at_offset(carriage->nextOffset);
            }
        }
        update_median_pos();
    }
    template <typename ItemT>
    void push_front_impl(ItemT item) {
        if (max_size() == 0) {
            return;
        }
        while (size() >= max_size()) {
            pop_back();
        }
        m_nodes.push_front(node());

        auto& front = m_nodes.front();
        const auto front_offset = m_nodes.front_offset();
        if (m_medianOffset == position_max) {
            front.nextOffset = position_max;
            front.prevOffset = position_max;
            front.item = std::move(item);

            m_size = 1;
            m_minOffset = front_offset;
            m_maxOffset = front_offset;
            m_medianOffset = front_offset;
            m_medianPos = 0;
            return;
        }

        // O OM
        // O N OM
        if (m_comparator(item, m_nodes.at_offset(m_medianOffset).item)) { // <=
            node* carriage = &m_nodes.at_offset(m_medianOffset);
            m_medianPos += 1;
            while (true) {
                if (!m_comparator(item, carriage->item)) { // >=
                    front.nextOffset = carriage->nextOffset;
                    front.prevOffset = carriage->idx(this);
                    //back.value = m_accessor(value);
                    front.item = std::move(item);

                    carriage->nextOffset = front_offset;
                    m_nodes.at_offset(front.nextOffset).prevOffset = front_offset;

                    m_size += 1;
                    break;
                }
                else if (carriage->prevOffset == position_max) { // left
                    carriage->prevOffset = front_offset;
                    front.nextOffset = carriage->idx(this);
                    front.prevOffset = position_max;
                    //back.value = m_accessor(value);
                    front.item = std::move(item);

                    m_size += 1;
                    m_minOffset = front_offset;
                    break;
                }
                carriage = &m_nodes.at_offset(carriage->prevOffset);
            }
        }
        // OM O
        // OM N O
        else {
            node* carriage = &m_nodes.at_offset(m_medianOffset);
            while (true) {
                if (m_comparator(item, carriage->item)) { // <=
                    front.nextOffset = carriage->idx(this);
                    front.prevOffset = carriage->prevOffset;
                    //back.value = m_accessor(value);
                    front.item = std::move(item);

                    carriage->prevOffset = front_offset;
                    m_nodes.at_offset(front.prevOffset).nextOffset = front_offset;

                    m_size += 1;
                    break;
                }
                if (carriage->nextOffset == position_max) { // right
                    carriage->nextOffset = m_nodes.front_offset();
                    front.nextOffset = position_max;
                    front.prevOffset = carriage->idx(this);
                    //back.value = m_accessor(value);
                    front.item = std::move(item);

                    m_size += 1;
                    m_maxOffset = front_offset;
                    break;
                }
                carriage = &m_nodes.at_offset(carriage->nextOffset);
            }
        }
        update_median_pos();
    }
    void update_median_pos() {
        const position_t desiredMedianPos = (size() ? size() - 1 : 0) >> 1;
        while (m_medianPos > desiredMedianPos) { // <-
            m_medianOffset = m_nodes.at_offset(m_medianOffset).prevOffset;
            m_medianPos -= 1;
        }
        while (m_medianPos < desiredMedianPos) { // ->
            m_medianOffset = m_nodes.at_offset(m_medianOffset).nextOffset;
            m_medianPos += 1;
        }
    }
    comparator_t m_comparator;
    mutable circular_buffer<node> m_nodes;
    position_t m_size = 0;
    position_t m_minOffset = position_max;
    position_t m_medianOffset = position_max;
    position_t m_medianPos = position_max;
    position_t m_maxOffset = position_max;
    //value_t m_sum = 0;
};
