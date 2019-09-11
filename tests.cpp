#include <iostream>
#include <cassert>

#include "circular_buffer.hpp"
#include "sorted_flat_deque.hpp"

struct data_t {
    data_t() {
        std::cout << this << " c-tor" << std::endl;
    }
    data_t(const uint32_t value_) {
        std::cout << this << " c-tor new:" << value_ << std::endl;
        value = value_;
    }
    data_t(const data_t& other) {
        std::cout << this << " c-tor_copy old:" << value << " new:"
            << other.value << std::endl;
        value = other.value;
    }
    data_t(data_t&& other) {
        std::cout << this << " c-tor_move old:" << value << " new:"
            << other.value << std::endl;
        value = other.value;
        other.value = 0;
    }
    data_t& operator=(const data_t& other) {
        std::cout << this << " assign_copy old:" << value << " new:"
            << other.value << std::endl;
        value = other.value;
        return *this;
    }
    data_t& operator=(data_t&& other) {
        std::cout << this << " assign_move old:" << value << " new:"
            << other.value << std::endl;
        value = other.value;
        other.value = 0;
        return *this;
    }
    ~data_t() {
        std::cout << this << " d-tor old:" << value << std::endl;
    }
    uint32_t value = 0;
};

void test_circular_buffer() {
    { // basic
        circular_buffer<int32_t> buf;
        assert(buf.size() == 0);
        assert(buf.max_size() == 0);
        buf.set_max_size(0);
        buf.push_back(11);
        assert(buf.size() == 0);
        assert(buf.max_size() == 0);

        buf.clear();
        buf.set_max_size(1);
        assert(buf.size() == 0);
        assert(buf.max_size() == 1);
        buf.push_back(11);
        assert(buf.size() == 1);
        assert(buf.front() == 11);
        assert(buf.back() == 11);
        assert(buf.at(0) == 11);
        buf.push_back(22);
        assert(buf.size() == 1);
        assert(buf.front() == 22);
        assert(buf.back() == 22);
        assert(buf.at(0) == 22);

        buf.clear();
        buf.set_max_size(3);
        assert(buf.size() == 0);
        assert(buf.max_size() == 3);
        assert(buf.begin() == buf.end());
        buf.push_back(11); // 11
        assert(buf.begin() != buf.end());
        buf.push_back(22); // 11 22
        buf.push_back(33); // 11 22 33
        buf.push_back(44); // 22 33 44
        assert(buf.size() == 3);
        assert(buf.begin() != buf.end());
        auto it = buf.begin();
        assert(*it == 22);
        ++it;
        assert(it != buf.end());
        assert(*it == 33);
        ++it;
        assert(it != buf.end());
        assert(*it == 44);
        ++it;
        assert(it == buf.end());
        int32_t sum = 0;
        for (const auto& value : buf) {
            sum += value;
        }
        assert(sum == 99);
        buf.push_front(55); // 55 22 33
        assert(buf.at(0) == 55);
        assert(*(buf.end() - 1) == 33);
    } // basic

    { // complex type
        circular_buffer<data_t> buf;
        buf.set_max_size(3); // c-tor x3
        buf.push_back(data_t(11));
        buf[0] = data_t(22); // assign move
        data_t data; // c-tor
        buf[0] = data; // assign copy
        data.value = 33;
        buf[0] = data; // assign copy
        buf.pop_back(); // d-tor
    } // complex type

    { // set_max_size
        circular_buffer<int32_t> buf;
        buf.set_max_size(3);
        buf.push_back(1); // 1 _ _
        buf.push_back(2); // 1 2 _
        buf.push_back(3); // 1 2 3
        buf.pop_front(); // _ 2 3
        buf.set_max_size(5); // _ 2 3 _ _
        assert(buf[0] == 2);
        assert(buf[1] == 3);
        buf.set_max_size(4); // 2 3 _ _
        assert(buf[0] == 2);
        assert(buf[1] == 3);
        buf.push_back(4); // 2 3 4 _
        buf.push_back(5); // 2 3 4 5
        buf.set_max_size(3); // 3 4 5
        assert(buf[0] == 3);
        assert(buf[1] == 4);
        assert(buf[2] == 5);
    } // set_max_size
}

template <typename value_t>
value_t test_sum(const std::initializer_list<value_t> sequence) {
    value_t sum = 0;
    for (const auto& value : sequence) {
        sum = (sum + value) * 2;
    }
    return sum;
}

void test_sorted_flat_deque() {
    {
        sorted_flat_deque<int32_t> sorted;
        assert(sorted.size() == 0);
        sorted.push_back(0);
        assert(sorted.size() == 0);
        sorted.init(1);
        sorted.push_back(0);
        assert(sorted.size() == 1);
        sorted.push_back(0);
        assert(sorted.size() == 1);

        sorted.init(2);
        assert(sorted.size() == 0);
        sorted.push_back(2);
        sorted.push_back(1);
        assert(sorted.size() == 2);
        sorted.pop_front();
        sorted.pop_front();
        assert(sorted.size() == 0);
        bool is_throw_catched = false;
        try {
            sorted.pop_front();
        }
        catch (...) {
            is_throw_catched = true;
        }
        assert(is_throw_catched == true);
        assert(sorted.size() == 0);

        sorted.init(7);
        sorted.push_back(75); // 75M
        sorted.push_back(37); // 37M 75
        sorted.push_back(83); // 37 75M 83
        sorted.push_back(92); // 37 75M 83 92
        sorted.push_back(59); // 37 59 75M 83 92
        sorted.push_back(96); // 37 59 75M 83 92 96
        sorted.push_back(57); // 37 57 59 75M 83 92 96
        sorted.push_back(80); // 37 57 59 80M 83 92 96 -75
        sorted.push_back(65); // 57 59 65 80M 83 92 96 -37
        sorted.push_back(55); // 55 57 59 65M 80 92 96 -83
        sorted.push_back(74); // 55 57 59 65M 74 80 96 -92
        sorted.push_back(67); // 55 57 65 67M 74 80 96 -59
        sorted.push_back(15); // 15 55 57 65M 67 74 80 -96
        sorted.push_back(76); // 15 55 65 67M 74 76 80 -57
        sorted.push_back(59); // 15 55 59 65M 67 74 76 -80
        sorted.push_back(30); // 15 30 55 59M 67 74 76 -65
        assert(sorted.min() == 15);
        assert(sorted.median() == 59);
        assert(sorted.max() == 76);

        int32_t sum = 0;
        for (auto it = sorted.begin(); it != sorted.end(); ++it) {
            std::cout << *it;
            if (it == sorted.it_median()) {
                std::cout << "M ";
            }
            else {
                std::cout << " ";
            }
            sum = (sum + *it) * 2;
        }
        std::cout << std::endl;
        assert(sum == test_sum<int32_t>({ 15, 30, 55, 59, 67, 74, 76 }));
    }
    {
        sorted_flat_deque<data_t, uint32_t> sorted;
        sorted.push_back(data_t(6));
        assert(sorted.size() == 0);
        sorted.init(1, [](const data_t& left, const data_t& right) {
            return left.value < right.value; });
        sorted.push_back(data_t(66));
        for (auto it = sorted.begin(); it != sorted.end(); ++it) {
            std::cout << it->value << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    test_circular_buffer();
    test_sorted_flat_deque();
    std::cout << "success" << std::endl;

//    std::mt19937 rng;
//    std::chrono::high_resolution_clock::time_point begin, end;
//    constexpr uint32_t maxSize = 313;
//    //constexpr uint32_t reps = 1350;
//    constexpr uint32_t reps = 1770;
//    //constexpr uint32_t reps = 1000000;
//    system("pause");
//    {
//        int32_t chsum = 0;
//        rng.seed();
//
//        std::array<int32_t, maxSize> temp;
//        circular_buffer<int32_t> testA;
//        testA.init(maxSize);
//
//        begin = std::chrono::high_resolution_clock::now();
//
//        for (uint32_t i = 0; i < reps; ++i) {
//            //if (i == 1336) {
//            //    __nop();
//            //}
//            int32_t newValue = rng() % 2000 - 1000;
//            testA.push_back(newValue);
//            for (uint32_t n = 0; n < testA.size(); ++n) {
//                temp[n] = testA[n];
//            }
//            std::sort(temp.begin(), temp.begin() + testA.size());
//            chsum += temp[(testA.size() - 1) >> 1];
//
//            //if (1766 < i) {
//                std::cout << i << ": " << temp[(testA.size() - 1) >> 1] << std::endl;
//            //}
//        }
//
//        end = std::chrono::high_resolution_clock::now();
//
//        std::cout << "TestA: time=" << std::chrono::duration_cast<std::chrono::microseconds>(
//            end - begin).count() << " mcs, chsum=" << chsum << std::endl;
//    }
//    system("pause");
//    {
//        int32_t chsum = 0;
//        rng.seed();
//
//        sorted_flat_deque<int32_t> testB;
//        testB.init(maxSize);
//
//        begin = std::chrono::high_resolution_clock::now();
//
//        for (uint32_t i = 0; i < reps; ++i) {
//            if (i == 1766) {
//                __nop();
//                //testB.debug_print();
//                //testB.debug_indexes();
//            }
//            int32_t newValue = rng() % 2000 - 1000;
//            testB.push_back(newValue);
//            chsum += testB.median();
//
//            //if (1766 < i) {
//                std::cout << i << ": " << testB.median() << std::endl;
//            //}
//            if (i == 1766) {
//                //std::cout << "newValue=" << newValue << std::endl;
//                //testB.debug_print();
//                //testB.debug_indexes();
//            }
//        }
//
//        end = std::chrono::high_resolution_clock::now();
//
//        std::cout << "TestA: time=" << std::chrono::duration_cast<std::chrono::microseconds>(
//            end - begin).count() << " mcs, chsum=" << chsum << std::endl;
//    }
//    system("pause");
}
