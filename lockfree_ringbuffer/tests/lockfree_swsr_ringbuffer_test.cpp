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

constexpr std::uint32_t MAX_CYCLES = 10;
constexpr std::uint32_t MAX_RETRIES = 10;

SCENARIO("One producer and no consumer")
{
    GIVEN("A ringbuffer")
    {
        // Assemble
        using namespace lockfree;
        SWSRRingBuffer<MemoryHog, 4> ring_buffer;
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
    using namespace lockfree;
    STATUS result{};
    GIVEN("A ringbuffer")
    {
        // Assemble
        SWSRRingBuffer<MemoryHog, 4> ring_buffer;
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

SCENARIO("One producer and one consumer")
{
    GIVEN("A ringbuffer")
    {
        using namespace lockfree;
        SWSRRingBuffer<MemoryHog, 8> ring_buffer;

        WHEN("A producer is filling it")
        {
            std::thread producer([&ring_buffer]() {
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

                    using namespace std::chrono_literals;
                    auto start = std::chrono::system_clock::now();
                    while (elapsed_seconds_since(start) < 2s)
                    {
                        if (ring_buffer.tryWrite(id, memory_hog) == STATUS::SUCCESS)
                            break;
                    }
                }
            });
            THEN("A consumer reads the ringbuffer")
            {
                std::thread consumer([&ring_buffer]() {
                    auto id = ring_buffer.addReader();
                    std::uint32_t i{0};
                    for (std::uint32_t j{0}; j < MAX_CYCLES; j++)
                    {
                        MemoryHog memory_hog;

                        using namespace std::chrono_literals;
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
                    }
                });
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
        using namespace lockfree;
        SWSRRingBuffer<MemoryHog, 8> ring_buffer;
        WHEN("A slow producer fills it")
        {
            std::thread producer([&ring_buffer]() {
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

                    using namespace std::chrono_literals;
                    auto start = std::chrono::system_clock::now();
                    while (elapsed_seconds_since(start) < 2s)
                    {
                        std::this_thread::sleep_for(1ms);
                        if (ring_buffer.tryWrite(id, memory_hog) == STATUS::SUCCESS)
                            break;
                    }
                }
            });
            THEN("A consumer reads the ringbuffer")
            {
                std::thread consumer([&ring_buffer]() {
                    auto id = ring_buffer.addReader();
                    std::uint32_t i{0};
                    for (std::uint32_t j{0}; j < MAX_CYCLES; j++)
                    {
                        MemoryHog memory_hog;

                        using namespace std::chrono_literals;
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
                    }
                });
                producer.join();
                consumer.join();
            }
        }
    }
}
