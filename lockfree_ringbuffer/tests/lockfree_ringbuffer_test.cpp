#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include <lockfree_ringbuffer/lockfree_ringbuffer.h>
#include <thread>

constexpr std::size_t MAX_VALUES = 10000;

struct MemoryHog
{
    int values[MAX_VALUES];
};

SCENARIO("One producer and one consumer")
{
    GIVEN("A produces fills a ringbuffer")
    {
        using namespace lockfree_ringbuffer;
        RingBuffer<MemoryHog, 8> ring_buffer;

        std::thread producer([&ring_buffer]() {
            auto id = ring_buffer.addProducer();
            std::uint32_t i = 0;
            for (std::uint32_t j = 0; j < 10; j++)
            {
                MemoryHog memory_hog{};
                for (std::uint32_t k = 0; k < MAX_VALUES; k++)
                {
                    i++;
                    memory_hog.values[k] = i;
                }
                ring_buffer.write(id, memory_hog);
            }
        });
        THEN("A consumer reads the ringbuffer")
        {
            std::thread consumer([&ring_buffer]() {
                auto id = ring_buffer.addConsumer();
                std::uint32_t i = 0;
                for (std::uint32_t j = 0; j < 10; j++)
                {
                    MemoryHog memory_hog{};
                    for (std::uint32_t t = 0; t < 3; t++)
                    {
                        if (ring_buffer.readNewest(id, memory_hog) == STATUS::SUCCESS)
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