#include "lockfree_ringbuffer/lockfree_ringbuffer.h"
#include <cstring>

namespace lockfree_ringbuffer
{
    template <typename T, std::size_t S>
    RingBuffer<T, S>::RingBuffer()
    {
        buffer_ = std::make_unique<Element<T>[]>(S); // heap allocated array
        clean();
    }

    template <typename T, std::size_t S>
    ID RingBuffer<T, S>::addReader()
    {
        return 0;
    }

    template <typename T, std::size_t S>
    ID RingBuffer<T, S>::addWriter()
    {
        return 0;
    }

    static inline std::size_t modulo_power_of2(const std::size_t &dividend, const std::size_t &divisor)
    {
        return dividend & (divisor - 1);
    }

    template <typename T, std::size_t S>
    STATUS RingBuffer<T, S>::tryWrite(ID id, T t)
    {
        std::size_t writer_index = modulo_power_of2(writer_position_ + 1, S);
        std::size_t reader_index = modulo_power_of2(reader_position_, S);

        if (writer_index == reader_index)
        {
            return STATUS::ERROR_BUFFER_FULL;
        }

        buffer_[writer_index].fully_written = false;

        writer_position_++;

        buffer_[writer_index].content = t;
        buffer_[writer_index].fully_written = true;
        return STATUS::SUCCESS;
    }

    template <typename T, std::size_t S>
    STATUS RingBuffer<T, S>::tryReadNext(ID id, T &t)
    {
        if (reader_position_ == writer_position_)
        {
            return STATUS::ERROR_NOTHING_TO_READ;
        }
        std::size_t writer_index = modulo_power_of2(writer_position_, S);
        std::size_t reader_index = modulo_power_of2(reader_position_ + 1, S);

        if (reader_index == writer_index && !buffer_[reader_index].fully_written)
        {
            return STATUS::ERROR_NOTHING_TO_READ;
        }

        t = buffer_[reader_index].content;
        reader_position_++;
        return STATUS::SUCCESS;
    }

    template <typename T, std::size_t S>
    STATUS RingBuffer<T, S>::tryReadNewest(ID id, T &t)
    {
    }

    //     if (reader_position_ == writer_position_)
    //     {
    //         return STATUS::ERROR_NOTHING_TO_READ;
    //     }
    //     std::size_t index = modulo_power_of2(writer_position_, S);
    //     if (buffer_[index].fully_written)
    //     {
    //         t = buffer_[index].content;
    //         std::size_t tmp = writer_position_;
    //         reader_position_ = tmp;
    //     }
    //     else
    //     {
    //         index = (writer_position_ - 1) & (S - 1);
    //         t = buffer_[index].content;
    //         std::size_t tmp = writer_position_ - 1;
    //         reader_position_ = tmp;
    //     }
    //     return STATUS::SUCCESS;
    // }

    template <typename T, std::size_t S>
    void RingBuffer<T, S>::clean()
    {
        std::memset(buffer_.get(), 0, S * sizeof(Element<T>));
        reader_position_ = 0;
        writer_position_ = 0;
    }

} // namespace lockfree_ringbuffer