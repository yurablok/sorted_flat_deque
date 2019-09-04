#pragma once

// push - O(n/2)
// pop - O(1)
// min - O(1)
// median - O(1)
// max - O(1)
// average - O(1)
template <typename T>
class sorted_flat_deque {
    struct node {
        T value;
        uint32_t prev;
        uint32_t next;
    };
public:
    void init(const uint32_t max_size) {
        m_size = 0;
        m_min = UINT32_MAX;
        m_median = UINT32_MAX;
        m_max = UINT32_MAX;
        m_nodes.init(max_size);
    }
    void set_max_size(const uint32_t max_size) {
        m_nodes;
    }
    void push(const T& value) {
        if (size_max() == 0) {
            return;
        }
        m_sum += value;
        while (size() >= size_max()) {
            pop();
        }
        m_nodes.push_back(node());

        auto& back = m_nodes.back();
        const auto back_idx = m_nodes.back_idx();
        if (m_median == UINT32_MAX) {
            back.next = UINT32_MAX;
            back.prev = UINT32_MAX;
            back.value = value;

            m_size = 1;
            m_min = back_idx;
            m_max = back_idx;
            m_median = back_idx;
            m_medianPos = 0;
            return;
        }

        // O OM
        // O N OM
        if (value <= m_nodes.at_idx(m_median).value) {
            node* carriage = &m_nodes.at_idx(m_median);
            m_medianPos += 1;
            while (true) {
                if (value >= carriage->value) {
                    back.next = carriage->next;
                    const auto distance = carriage - &m_nodes.at_idx(0);
                    back.prev = distance;
                    back.value = value;

                    carriage->next = back_idx;
                    m_nodes.at_idx(back.next).prev = back_idx;

                    m_size += 1;
                    break;
                }
                else if (carriage->prev == UINT32_MAX) { // left
                    carriage->prev = back_idx;
                    const auto distance = carriage - &m_nodes.at_idx(0);
                    back.next = distance;
                    back.prev = UINT32_MAX;
                    back.value = value;

                    m_size += 1;
                    m_min = back_idx;
                    break;
                }
                carriage = &m_nodes.at_idx(carriage->prev);
            }
        }
        // OM O
        // OM N O
        else {
            node* carriage = &m_nodes.at_idx(m_median);
            while (true) {
                if (value <= carriage->value) {
                    const auto distance = carriage - &m_nodes.at_idx(0);
                    back.next = distance;
                    back.prev = carriage->prev;
                    back.value = value;

                    carriage->prev = back_idx;
                    m_nodes.at_idx(back.prev).next = back_idx;

                    m_size += 1;
                    break;
                }
                if (carriage->next == UINT32_MAX) { // right
                    carriage->next = m_nodes.back_idx();
                    back.next = UINT32_MAX;
                    const auto distance = carriage - &m_nodes.at_idx(0);
                    back.prev = distance;
                    back.value = value;

                    m_size += 1;
                    m_max = back_idx;
                    break;
                }
                carriage = &m_nodes.at_idx(carriage->next);
            }
        }
        const uint32_t desiredMedianPos = (size() ? size() - 1 : 0) >> 1;
        while (m_medianPos > desiredMedianPos) { // <-
            m_median = m_nodes.at_idx(m_median).prev;
            m_medianPos -= 1;
        }
        while (m_medianPos < desiredMedianPos) { // ->
            m_median = m_nodes.at_idx(m_median).next;
            m_medianPos += 1;
        }
    }
    void pop() {
        if (m_nodes.empty()) {
            return;
        }
        auto& to_remove = m_nodes.front();
        m_sum -= to_remove.value;
        if (to_remove.prev != UINT32_MAX) {
            m_nodes.at_idx(to_remove.prev).next = to_remove.next;
        }
        else { // left
            m_min = to_remove.next;
        }
        if (to_remove.next != UINT32_MAX) {
            m_nodes.at_idx(to_remove.next).prev = to_remove.prev;
        }
        else { // right
            m_max = to_remove.prev;
        }
        m_size -= 1;

        if (m_median == m_nodes.front_idx()) {
            m_median = m_nodes.front().prev;
            if (m_medianPos > 0) {
                m_medianPos -= 1;
            }
        }
        if (m_median != UINT32_MAX) {
            //TODO: <= <
            if (to_remove.value <= m_nodes.at_idx(m_median).value && m_medianPos) {
                m_medianPos -= 1;
            }
            const uint32_t desiredMedianPos = (size() ? size() - 1 : 0) >> 1;
            while (m_medianPos > desiredMedianPos) { // <-
                m_median = m_nodes.at_idx(m_median).prev;
                m_medianPos -= 1;
            }
            while (m_medianPos < desiredMedianPos) { // ->
                m_median = m_nodes.at_idx(m_median).next;
                m_medianPos += 1;
            }
        }
        m_nodes.pop_front();
    }
    T& min() const {
        if (m_min == UINT32_MAX) {
            throw std::logic_error("m_min == UINT32_MAX");
        }
        else {
            return m_nodes.at_idx(m_min).value;
        }
    }
    T& median() const {
        if (m_median == UINT32_MAX) {
            throw std::logic_error("m_middle == UINT32_MAX");
        }
        else {
            return m_nodes.at_idx(m_median).value;
        }
    }
    T& max() const {
        if (m_max == UINT32_MAX) {
            throw std::logic_error("m_max == UINT32_MAX");
        }
        else {
            return m_nodes.at_idx(m_max).value;
        }
    }
    T average() const {
        return m_sum / static_cast<T>(m_nodes.size());
    }
    uint32_t size() const {
        return m_size;
    }
    bool empty() const {
        return m_size == 0;
    }
    uint32_t max_size() const {
        return m_nodes.max_size();
    }
    void clear() {
        //TODO: void clear()
    }
    //void debug_print() const {
    //    if (m_min == UINT32_MAX) {
    //        return;
    //    }
    //    std::cout << "{";
    //    const node *n = &m_nodes.at_idx(m_min);
    //    uint32_t i = 0;
    //    while (true) {
    //        const auto distance = n - &m_nodes.at_idx(0);
    //        std::cout << " " << n->value
    //            << "[" << distance
    //            << "](" << (int32_t)n->prev
    //            << ")(" << (int32_t)n->next << ")";
    //        if (distance == m_median) {
    //            std::cout << "M";
    //        }
    //        if (i++ == m_medianPos) {
    //            std::cout << "D";
    //        }
    //        if (n->next != UINT32_MAX) {
    //            n = &m_nodes.at_idx(n->next);
    //        }
    //        else {
    //            break;
    //        }
    //    }
    //    std::cout << " }" << std::endl;
    //}

private:
    mutable circular_buffer<node> m_nodes;
    uint32_t m_size = 0;
    uint32_t m_min = UINT32_MAX;
    uint32_t m_median = UINT32_MAX;
    uint32_t m_medianPos = UINT32_MAX;
    uint32_t m_max = UINT32_MAX;
    T m_sum = 0;
};
