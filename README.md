# sorted_flat_deque

C++11, STL-like API, bidirectional iterator, one memory allocation in the circular buffer.

push - O(n/2)  
pop - O(1)  
min - O(1)  
median - O(1)  
max - O(1)  

### Principle of usage:
1. Container initialization
```cpp
sorted_flat_deque<int32_t> deque;
                       ╔═══╤═══╤═══╤═══╗
deque.set_max_size(4); ║   │   │   │   ║  // internal circular buffer
                       ╚═══╧═══╧═══╧═══╝
```
2. Pushing items
```cpp
                    ╔═══╤═══╤═══╤═══╗
deque.push_back(7); ║ 7 │   │   │   ║
                    ╚═══╧═══╧═══╧═══╝
                    ╔═══╤═══╤═══╤═══╗
deque.push_back(2); ║ 7f│ 2b│   │   ║ // b - back position, f - front position
                    ╚═══╧═══╧═══╧═══╝
                    ╔═══╤═══╤═══╤═══╗
deque.push_back(6); ║ 7f│ 2 │ 6b│   ║
                    ╚═══╧═══╧═══╧═══╝
                    ╔═══╤═══╤═══╤═══╗
deque.push_back(1); ║ 7f│ 2 │ 6 │ 1b║
                    ╚═══╧═══╧═══╧═══╝
                    ╔═══╤═══╤═══╤═══╗
deque.push_back(3); ║ 3b│ 2f│ 6 │ 1 ║ // element '7' was automatically removed from front
                    ╚═══╧═══╧═══╧═══╝
```
3. Accessing to `min`, `max` or `median` elements
```cpp
for (auto it = deque.cbegin(); it != deque.cend(); ++it) {
    std::cout << *it << " ";
}
std::cout << std::endl; // 1 2 3 6
deque.min();    // 1
deque.median(); // 2
deque.max();    // 6
```
