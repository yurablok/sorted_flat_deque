#include <iostream>
#include <cassert>
#include <random>
#include <array>
#include <vector>
#include <chrono>

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
        value = 0;
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

        // decrease  fxxxb000 -> fxxxb0
        buf.clear();
        buf.set_max_size(8);
        buf.push_back(1);
        buf.push_back(2);
        buf.push_back(3);
        buf.push_back(4);
        buf.push_back(5);
        buf.set_max_size(6);
        assert(buf.at_offset(0) == 1);
        assert(buf.at_offset(4) == 5);
        assert(buf.front() == 1);
        assert(buf.back() == 5);
        assert(buf.size() == 5);

        // decrease  000fxxxb -> 0fxxxb
        buf.clear();
        buf.set_max_size(8);
        buf.push_back(1);
        buf.push_back(2);
        buf.push_back(3);
        buf.push_back(4);
        buf.push_back(5);
        buf.push_back(6);
        buf.push_back(7);
        buf.push_back(8);
        buf.pop_front();
        buf.pop_front();
        buf.pop_front();
        buf.set_max_size(6);
        assert(buf.at_offset(1) == 4);
        assert(buf.at_offset(5) == 8);
        assert(buf.front() == 4);
        assert(buf.back() == 8);
        assert(buf.size() == 5);

        // decrease  xb000fxx -> xb0fxx
        buf.clear();
        buf.set_max_size(8);
        buf.push_back(1); // 1
        buf.push_back(2); // 1 2
        buf.push_back(3); // 1 2 3
        buf.push_back(4); // 1 2 3 4
        buf.push_back(5); // 1 2 3 4 5
        buf.push_back(6); // 1 2 3 4 5 6
        buf.push_back(7); // 1 2 3 4 5 6 7
        buf.push_back(8); // 1 2 3 4 5 6 7 8
        buf.push_back(9); // 9 2 3 4 5 6 7 8
        buf.push_back(10); // 9 10 3 4 5 6 7 8
        buf.pop_front(); // 9 10 0 4 5 6 7 8
        buf.pop_front(); // 9 10 0 0 5 6 7 8
        buf.pop_front(); // 9 10 0 0 0 6 7 8
        buf.set_max_size(6); // 9 10 0 6 7 8
        assert(buf.at_offset(1) == 10);
        assert(buf.at_offset(3) == 6);
        assert(buf.front() == 6);
        assert(buf.back() == 10);
        assert(buf.size() == 5);
        //for (auto it = buf.rbegin(); it != buf.rend(); ++it) {
        //    std::cout << *it << " __ ";
        //}
        //std::cout << std::endl;

        // increase  x -> 0x
        buf.clear();
        buf.set_max_size(1);
        buf.set_max_size(2);
        buf.set_max_size(0);
        buf.set_max_size(1);
        buf.push_back(100);
        buf.set_max_size(2);
        assert(buf.at_offset(1) == 100);
        assert(buf.front() == 100);
        assert(buf.back() == 100);
        assert(buf.size() == 1);
        buf.set_max_size(0);
        assert(buf.size() == 0);

        // increase  fxxb00 -> fxxb0000
        buf.clear();
        buf.set_max_size(6);
        buf.push_back(1);
        buf.push_back(2);
        buf.push_back(3);
        buf.push_back(4);
        buf.set_max_size(8);
        assert(buf.at_offset(0) == 1);
        assert(buf.at_offset(3) == 4);
        assert(buf.front() == 1);
        assert(buf.back() == 4);
        assert(buf.size() == 4);

        // increase  0fxxb0 -> 0fxxb000
        buf.clear();
        buf.set_max_size(6);
        buf.push_back(1);
        buf.push_back(2);
        buf.push_back(3);
        buf.push_back(4);
        buf.push_back(5);
        buf.pop_front();
        buf.set_max_size(8);
        assert(buf.at_offset(1) == 2);
        assert(buf.at_offset(4) == 5);
        assert(buf.front() == 2);
        assert(buf.back() == 5);
        assert(buf.size() == 4);

        // increase  00x -> 000x
        buf.clear();
        buf.set_max_size(3);
        buf.push_back(1);
        buf.push_back(2);
        buf.push_back(3);
        buf.pop_front();
        buf.pop_front();
        buf.set_max_size(4);
        assert(buf.at_offset(3) == 3);
        assert(buf.front() == 3);
        assert(buf.back() == 3);
        assert(buf.size() == 1);

        // increase  00fxxb -> 0000fxxb
        buf.clear();
        buf.set_max_size(6);
        buf.push_back(1);
        buf.push_back(2);
        buf.push_back(3);
        buf.push_back(4);
        buf.push_back(5);
        buf.push_back(6); // 1 2 3 4 5 6
        buf.pop_front();
        buf.pop_front(); // 0 0 3 4 5 6
        buf.set_max_size(8); // 0 0 0 0 3 4 5 6
        assert(buf.at_offset(4) == 3);
        assert(buf.at_offset(7) == 6);
        assert(buf.front() == 3);
        assert(buf.back() == 6);
        assert(buf.size() == 4);

        // increase  xb00fx -> xb0000fx
        buf.clear();
        buf.set_max_size(6);
        buf.push_back(1);
        buf.push_back(2);
        buf.push_back(3);
        buf.push_back(4);
        buf.push_back(5);
        buf.push_back(6);
        buf.push_back(7);
        buf.push_back(8); // 7 8 3 4 5 6
        buf.pop_front();
        buf.pop_front(); // 7 8 0 0 5 6
        buf.set_max_size(8); // 7 8 0 0 0 0 5 6
        assert(buf.at_offset(1) == 8);
        assert(buf.at_offset(6) == 5);
        assert(buf.front() == 5);
        assert(buf.back() == 8);
        assert(buf.size() == 4);
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

template <typename deque_t>
void print(const deque_t& deque) {
    for (auto it = deque.cbegin(); it != deque.cend(); ++it) {
        std::cout << "[" << it.offset() << "]" << *it;
        if (it == deque.cmedian_it()) {
            std::cout << "M";
        }
        std::cout << " ";
    }
    std::cout << std::endl;
}

void test_sorted_flat_deque() {
    { // basic
        sorted_flat_deque<int32_t> sorted;
        assert(sorted.size() == 0);
        sorted.push_back(0);
        assert(sorted.size() == 0);

        sorted.clear();
        sorted.set_max_size(1);
        sorted.push_back(0);
        assert(sorted.size() == 1);
        sorted.push_back(0);
        assert(sorted.size() == 1);

        sorted.clear();
        sorted.set_max_size(2);
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

        sorted.clear();
        sorted.set_max_size(7);
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
            if (it == sorted.median_it()) {
                std::cout << "M ";
            }
            else {
                std::cout << " ";
            }
            sum = (sum + *it) * 2;
        }
        std::cout << std::endl;
        assert(sum == test_sum<int32_t>({ 15, 30, 55, 59, 67, 74, 76 }));
    } // basic

    { // push_front push_back pop_front pop_back
        sorted_flat_deque<int32_t> sorted;
        sorted.set_max_size(5);
        sorted.push_back(1); // 1
        sorted.push_back(2); // 1 2
        sorted.push_front(3); // 3 1 2
        assert(sorted.min() == 1);
        assert(sorted.median() == 2);
        assert(sorted.max() == 3);
        sorted.push_back(4); // 3 1 2 4
        sorted.push_front(5); // 5 3 1 2 4
        assert(sorted.min() == 1);
        assert(sorted.median() == 3);
        assert(sorted.max() == 5);
        sorted.push_front(6); // 6 5 3 1 2 _ 1 2 3 5 6
        assert(sorted.min() == 1);
        assert(sorted.median() == 3);
        assert(sorted.max() == 6);
        sorted.push_back(7); // 5 3 1 2 7 _ 1 2 3 5 7
        assert(sorted.min() == 1);
        assert(sorted.median() == 3);
        assert(sorted.max() == 7);
        sorted.pop_front(); // 3 1 2 7
        sorted.pop_front(); // 1 2 7
        sorted.pop_front(); // 2 7
        assert(sorted.min() == 2);
        assert(sorted.median() == 2);
        assert(sorted.max() == 7);
        assert(sorted.size() == 2);
        sorted.push_front(-6); // -6 2 7
        sorted.push_front(-5); // -5 -6 2 7
        sorted.push_front(-4); // -4 -5 -6 2 7
        sorted.push_front(-3); // -3 -4 -5 -6 2
        sorted.push_front(-2); // -2 -3 -4 -5 -6
        sorted.push_front(-1); // -1 -2 -3 -4 -5
        assert(sorted.min() == -5);
        assert(sorted.median() == -3);
        assert(sorted.max() == -1);
        assert(sorted.size() == 5);
    } // push_front push_back pop_front pop_back

    { // set_max_size
        sorted_flat_deque<data_t, uint32_t> sorted;
        sorted.push_back(data_t(6));
        assert(sorted.size() == 0);
        sorted.set_max_size(1);
        sorted.set_comparator([](const data_t& left, const data_t& right) {
            return left.value < right.value; });
        sorted.push_back(data_t(66)); // 66

        sorted.set_max_size(3);
        assert(sorted.min().value == 66);
        assert(sorted.median().value == 66);
        assert(sorted.max().value == 66);
        assert(sorted.size() == 1);

        sorted.push_back(data_t(24)); // 66 24
        sorted.push_back(data_t(42)); // 66 24 42
        sorted.push_back(data_t(64)); // 64 24 42
        sorted.pop_front();
        sorted.push_front(data_t(1));

        for (auto it = sorted.begin(); it != sorted.end(); ++it) {
            std::cout << it->value << " ";
        }
        std::cout << std::endl;

        sorted.set_max_size(5);
        sorted.push_back(data_t(13));
        sorted.push_back(data_t(333));
        sorted.push_back(data_t(103));

        for (auto it = sorted.begin(); it != sorted.end(); ++it) {
            std::cout << it->value << " ";
        }
        std::cout << std::endl;

        sorted.set_max_size(4);

        for (auto it = sorted.begin(); it != sorted.end(); ++it) {
            std::cout << it->value << " ";
        }
        std::cout << std::endl;
    } // set_max_size

    {
        sorted_flat_deque<int32_t> sorted;
        sorted.set_max_size(2);
        sorted.push_back(612); // 612
        assert(sorted.median() == 612);
        sorted.push_back(302); // 302 612
        assert(sorted.median() == 302);
        sorted.push_back(-266);// -266  302
        assert(sorted.median() == -266);
        sorted.push_back(-415);// -415 -266
        assert(sorted.median() == -415);
        sorted.push_back(-796);// -796 -415
        assert(sorted.median() == -796);
        sorted.push_back(391); // -796 391
        assert(sorted.median() == -796);
        sorted.push_back(429); // 391 429
        assert(sorted.median() == 391);
        sorted.push_back(985); // 429 985
        assert(sorted.median() == 429);
        sorted.push_back(-702);// -702 985
        assert(sorted.median() == -702);
        sorted.push_back(403); // -702 403
        assert(sorted.median() == -702);
    }
}

int main() {
    test_circular_buffer();
    test_sorted_flat_deque();
    std::cout << "success" << std::endl;

    std::mt19937 rng;
    constexpr uint32_t maxSize = 2048;
    constexpr uint32_t maxSeeds = 50;
    constexpr uint32_t reps = 10000;
    std::chrono::high_resolution_clock::time_point begin, end;
    std::array<int32_t, reps> chsums;
    int64_t minDurationA_mcs = INT64_MAX;
    int64_t minDurationB_mcs = INT64_MAX;
    for (uint32_t seed = 0; seed < maxSeeds; ++seed) {
        int32_t chsumA = 0;
        {
            rng.seed(seed);

            std::array<int32_t, maxSize> temp = { 0 };
            circular_buffer<int32_t> testA;
            testA.set_max_size(maxSize);

            begin = std::chrono::high_resolution_clock::now();

            for (uint32_t i = 0; i < reps; ++i) {
                int32_t newValue = rng() % 200 - 100;
                testA.push_back(newValue);
                for (uint32_t n = 0; n < testA.size(); ++n) {
                    temp[n] = testA[n];
                }
                std::sort(temp.begin(), temp.begin() + testA.size());
                chsumA += temp[(testA.size() - 1) >> 1];
                chsums[i] = chsumA;
            }

            end = std::chrono::high_resolution_clock::now();

            const int64_t durationA_mcs = std::chrono::duration_cast<
                std::chrono::microseconds>(end - begin).count();
            minDurationA_mcs = std::min(minDurationA_mcs, durationA_mcs);
            std::cout << "TestA: seed=" << seed << " time="
                << durationA_mcs << " mcs, chsum=" << chsumA << std::endl;
        }
        int32_t chsumB = 0;
        {
            rng.seed(seed);

            sorted_flat_deque<int32_t> testB;
            testB.set_max_size(maxSize);

            begin = std::chrono::high_resolution_clock::now();

            for (uint32_t i = 0; i < reps; ++i) {
                int32_t newValue = rng() % 200 - 100;
                testB.push_back(newValue);
                chsumB += testB.median();
            }

            end = std::chrono::high_resolution_clock::now();

            const int64_t durationB_mcs = std::chrono::duration_cast<
                std::chrono::microseconds>(end - begin).count();
            minDurationB_mcs = std::min(minDurationB_mcs, durationB_mcs);
            std::cout << "TestB: seed=" << seed << " time="
                << durationB_mcs << " mcs, chsum=" << chsumB << std::endl;
        }
        //if (chsumA != chsumB) {
        //    [] {};
        //}
        assert(chsumA == chsumB);
    }
    std::cout << "minDurationA=" << minDurationA_mcs
        << " minDurationB=" << minDurationB_mcs << std::endl;
    system("pause");
}
