#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include <lockfree_ringbuffer/lockfree_swsr_ringbuffer.h>
#include <thread>
#include <chrono>

constexpr std::size_t MAX_VALUES = 10000;
struct MemoryHog
{
    int values[MAX_VALUES];
};

constexpr std::uint32_t MAX_CYCLES = 100;
constexpr std::uint32_t MAX_RETRIES = 10;

using namespace lockfree;
using namespace std::chrono_literals;

constexpr std::size_t RING_BUFFER_SIZE = 4;

SCENARIO("One producer and no consumer")
{
    GIVEN("A ringbuffer")
    {
        // Assemble
        SWSRRingBuffer<MemoryHog, RING_BUFFER_SIZE> ring_buffer;
        STATUS result{};
        WHEN("A producer writes to it continously")
        {
            // Act
            auto id = ring_buffer.addWriter();
            for (std::uint32_t j{0}; j < MAX_CYCLES; j++)
            {
                MemoryHog memory_hog{};
                for (std::uint32_t t{0}; t < MAX_RETRIES; t++)
                {
                    result = ring_buffer.tryWrite(id, memory_hog);
                    if (result == STATUS::SUCCESS)
                        break;
                }
            }
            // Assert
            THEN("the buffer fills up")
            {
                REQUIRE(result == STATUS::ERROR_BUFFER_FULL);
            }
        }
    }
}

SCENARIO("No producer and one consumer")
{
    STATUS result{};
    GIVEN("A ringbuffer")
    {
        // Assemble
        SWSRRingBuffer<MemoryHog, RING_BUFFER_SIZE> ring_buffer;
        MemoryHog memory_hog{};
        // Act
        WHEN("consumer reads the ringbuffer")
        {
            auto id = ring_buffer.addReader();
            result = ring_buffer.tryRead(id, memory_hog);
            // Assert
            THEN("there is nothing to read")
            {
                REQUIRE(result == STATUS::ERROR_NOTHING_TO_READ);
            }
        }
    }
}

std::chrono::duration<double> elapsed_seconds_since(const std::chrono::time_point<std::chrono::system_clock> &start)
{
    auto now = std::chrono::system_clock::now();
    return now - start;
}

void producer_fn(lockfree::SWSRRingBuffer<MemoryHog, RING_BUFFER_SIZE> &ring_buffer, const std::chrono::milliseconds delay)
{

    auto id = ring_buffer.addWriter();
    std::uint32_t i{0};
    for (std::uint32_t j{0}; j < MAX_CYCLES; j++)
    {
        MemoryHog memory_hog{};
        for (std::uint32_t k = 0; k < MAX_VALUES; k++)
        {
            i++;
            memory_hog.values[k] = i;
        }

        std::this_thread::sleep_for(delay);

        auto start = std::chrono::system_clock::now();
        while (elapsed_seconds_since(start) < 2s)
        {
            if (ring_buffer.tryWrite(id, memory_hog) == STATUS::SUCCESS)
                break;
        }
    }
}

void consumer_fn(lockfree::SWSRRingBuffer<MemoryHog, RING_BUFFER_SIZE> &ring_buffer, const std::chrono::milliseconds delay)
{
    auto id = ring_buffer.addReader();
    std::uint32_t i{0};
    for (std::uint32_t j{0}; j < MAX_CYCLES; j++)
    {
        MemoryHog memory_hog;

        auto start = std::chrono::system_clock::now();
        while (elapsed_seconds_since(start) < 2s)
        {
            if (ring_buffer.tryRead(id, memory_hog) == STATUS::SUCCESS)
                break;
        }
        for (std::uint32_t k = 0; k < MAX_VALUES; k++)
        {
            i++;
            REQUIRE(memory_hog.values[k] == i);
        }
        std::this_thread::sleep_for(delay);
    }
}

SCENARIO("One producer and one consumer")
{
    GIVEN("A ringbuffer")
    {
        SWSRRingBuffer<MemoryHog, RING_BUFFER_SIZE> ring_buffer;

        WHEN("A producer is filling it")
        {
            std::thread producer(producer_fn, std::ref(ring_buffer), 0ms);
            THEN("A consumer reads the ringbuffer")
            {
                std::thread consumer(consumer_fn, std::ref(ring_buffer), 0ms);
                producer.join();
                consumer.join();
            }
        }
    }
}

SCENARIO("One slow producer and one consumer")
{
    GIVEN("A ringbuffer")
    {
        SWSRRingBuffer<MemoryHog, RING_BUFFER_SIZE> ring_buffer;

        WHEN("A slow producer is filling it")
        {
            std::thread producer(producer_fn, std::ref(ring_buffer), 1ms);
            THEN("A consumer reads the ringbuffer")
            {
                std::thread consumer(consumer_fn, std::ref(ring_buffer), 0ms);
                producer.join();
                consumer.join();
            }
        }
    }
}

SCENARIO("One producer and one slow consumer")
{
    GIVEN("A ringbuffer")
    {
        SWSRRingBuffer<MemoryHog, RING_BUFFER_SIZE> ring_buffer;

        WHEN("A producer is filling it")
        {
            std::thread producer(producer_fn, std::ref(ring_buffer), 0ms);
            THEN("A slow consumer reads the ringbuffer")
            {
                std::thread consumer(consumer_fn, std::ref(ring_buffer), 1ms);
                producer.join();
                consumer.join();
            }
        }
    }
}

void fill_twice_producer_fn(lockfree::SWSRRingBuffer<MemoryHog, RING_BUFFER_SIZE> &ring_buffer, const std::chrono::milliseconds delay)
{

    auto id = ring_buffer.addWriter();
    std::uint32_t i{0};
    MemoryHog memory_hog{};
    // fill ring buffer twice
    auto start = std::chrono::system_clock::now();
    for (std::uint32_t j{0}; j <= 2 * RING_BUFFER_SIZE; j++)
    {

        for (std::uint32_t k = 0; k < MAX_VALUES; k++)
        {
            i++;
            memory_hog.values[k] = i;
        }

        std::this_thread::sleep_for(delay);

        start = std::chrono::system_clock::now();
        while (elapsed_seconds_since(start) < 2s)
        {
            if (ring_buffer.tryWrite(id, memory_hog) == STATUS::SUCCESS)
                break;
        }
    }
    // write magic end
    start = std::chrono::system_clock::now();
    memory_hog.values[0] = 0xE0F;
    while (elapsed_seconds_since(start) < 2s)
    {
        if (ring_buffer.tryWrite(id, memory_hog) == STATUS::SUCCESS)
            break;
    }
}

void consumer_tryReadNewest_fn(lockfree::SWSRRingBuffer<MemoryHog, RING_BUFFER_SIZE> &ring_buffer, const std::chrono::milliseconds delay)
{
    auto id = ring_buffer.addReader();
    std::uint32_t i{0};
    std::this_thread::sleep_for(delay);
    for (std::uint32_t j{0}; j < MAX_CYCLES; j++)
    {
        MemoryHog memory_hog;

        auto start = std::chrono::system_clock::now();
        while (elapsed_seconds_since(start) < 2s)
        {
            if (ring_buffer.tryReadNewest(id, memory_hog) == STATUS::SUCCESS)
                break;
        }

        if (memory_hog.values[0] == 0xE0F) // allow the thread to join on magic last entry
            break;
    }
}

SCENARIO("One producer and one consumer using tryReadNewest")
{
    GIVEN("A ringbuffer")
    {
        SWSRRingBuffer<MemoryHog, RING_BUFFER_SIZE> ring_buffer;

        WHEN("A producer is filling it")
        {
            std::thread producer(fill_twice_producer_fn, std::ref(ring_buffer), 0ms);
            THEN("A consumer reads the ringbuffer using tryReadNewest")
            {
                std::thread consumer(consumer_tryReadNewest_fn, std::ref(ring_buffer), 5ms);
                producer.join();
                consumer.join();
            }
        }
    }
}