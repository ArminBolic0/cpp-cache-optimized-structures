#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>


using Clock = std::chrono::high_resolution_clock;

/*/
When we use new[], the OS may scatter allocations across memory.
By making our own pool allocator, everything lives in one contiguous buffer which results in much better spatial locality and that improves cache hits.
By using new, heap memory can get fragmented on deallocation.
With pool allocator, you never free individual object, either we free all at once with batch deallocation, or recycle blocks.
Like this we avoid fragmentation entirely because allocation is always linear.
*/

template<typename T>
class PoolAllocator
{
    std::vector<T*> buffers;
    size_t capacity; // this is how many element we can store
    size_t offset;   // this is how many we already use
public:
    PoolAllocator(size_t cap) : capacity(cap), offset(0) {
        buffers.push_back(new T[cap]);
    }

    T* allocate(size_t n) {
        if (offset + n > capacity)
        {
            buffers.push_back(new T[capacity]);
            offset = 0;
        }
        T* ptr = buffers.back() + offset; // carve from the buffer
        offset += n;
        return ptr;
    }

    ~PoolAllocator() {
        for (auto buffer : buffers)
        {
            delete[] buffer; buffer = nullptr;
        }
    }
};

/*/
We use chunked vector to store values in multiple arrays of CHUNK_SIZE number of elements rather than one big continuous array.
Benefits we get from this are:
We avoid reallocating huge contiguous memory blocks as we would do with normal array,
Each chunk is contiguos, so traversing elements in a chunk is fast

Vector is alright if its not large, but if it is, it must allocate one huge contiguous block of memory, its slower and it might fragment memory.
Chunked vectors are cache friendly and we avoid huge allocations.
*/


//CHUNKED VECTOR WITH NEW[] 
template<typename T, size_t CHUNK_SIZE = 64>
class ChunkedVector {
    std::vector<T*> chunks;
    size_t size = 0;

public:
    ~ChunkedVector() {
        for (auto chunk : chunks) delete[] chunk;
    }

    void push_back(const T& value) {
        if (size % CHUNK_SIZE == 0)
            chunks.push_back(new T[CHUNK_SIZE]);
        chunks[size / CHUNK_SIZE][size % CHUNK_SIZE] = value;
        size++;
    }

    T& operator[](size_t index) {
        return chunks[index / CHUNK_SIZE][index % CHUNK_SIZE];
    }

    size_t get_size() const { return size; }
};


//CHUNKED VECTOR WITH POOL ALLOCATION

template<typename T, size_t CHUNK_SIZE = 64> // CHUNK_SIZE - number of elements in each chunk, can be modified 
class ChunkedVectorPoolAllocation
{
    std::vector<T*> chunks; // here each pointer points to a chunk
    size_t size = 0;
    PoolAllocator<T> allocator{ 2048 }; // we use our custom allocator

public:

    void push_back(const T& value)
    {
        if (size % CHUNK_SIZE == 0) chunks.push_back(allocator.allocate(CHUNK_SIZE)); // size is counter for elements in ChunkedVector, when we reach number thats divisible by CHUNK_SIZE we create new chunk
        chunks[size / CHUNK_SIZE][size % CHUNK_SIZE] = value;
        size++;
    }

    T& operator[](size_t index) {
        return chunks[index / CHUNK_SIZE][index % CHUNK_SIZE];
    }

    size_t get_size() const { return size; }
};


double benchmarkStdVector(size_t n = 10'000'000, int repeat = 5) {
    double bestTime = 1e300;
    for (size_t i = 0; i < repeat; i++)
    {
        auto start = Clock::now();
        std::vector<int> v;
        v.reserve(n);
        for (size_t k = 0; k < n; k++) v.push_back(k);
        volatile long long sum = 0;
        for (size_t k = 0; k < v.size(); i++) sum += v[i];
        auto end = Clock::now();
        std::chrono::duration<double> duration = end - start;
        bestTime = std::min(bestTime, duration.count());
    }
    return bestTime;
}

double benchmarkChunkedVector(size_t n = 10'000'000, int repeat = 5) {
    double bestTime;
    for (size_t i = 0; i < repeat; i++)
    {
        auto start = Clock::now();
        ChunkedVector<int> v;
        for (size_t k = 0; k < n; k++) v.push_back(k);
        volatile long long sum = 0;
        for (size_t k = 0; k < v.get_size(); k++) sum += v[k];
        auto end = Clock::now();
        std::chrono::duration<double> duration = end - start;
        bestTime = std::min(bestTime, duration.count());
    }
    return bestTime;
}

double benchmarkChunkedVectorPooled(size_t n) {
    double bestTime;
    for (size_t i = 0; i < repeat; i++)
    {
        auto start = Clock::now();
        ChunkedVectorPoolAllocation<int> v;
        for (size_t k = 0; k < n; k++) v.push_back(k);
        volatile long long sum = 0;
        for (size_t k = 0; k < v.get_size(); k++) sum += v[k];
        auto end = Clock::now();
        std::chrono::duration<double> duration = end - start;
        bestTime = std::min(bestTime, duration.count());
    }
    return bestTime;
}

int main()
{
    std::vector<size_t> testSizes = { 5'000'000, 10'000'000, 25'000'000 };

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Benchmark results (times in seconds)\n\n";

    std::cout << std::setw(12) << "N"
        << std::setw(15) << "std::vector"
        << std::setw(20) << "ChunkedVector"
        << std::setw(25) << "ChunkedVector (pooled)"
        << std::setw(20) << "Speedup (pooled/std)"
        << "\n";

    for (size_t N : testSizes) {
        double t1 = benchmarkStdVector(N);
        double t2 = benchmarkChunkedVector(N);
        double t3 = benchmarkChunkedVectorPooled(N);

        std::cout << std::setw(12) << N
            << std::setw(15) << t1
            << std::setw(20) << t2
            << std::setw(25) << t3
            << std::setw(20) << (t1 / t3) << "x"
            << "\n";
    }

    return 0;
}
