#ifndef I_LOCKFREE_RINGBUFFER_H
#define I_LOCKFREE_RINGBUFFER_H

#include <cstdint>

namespace lockfree_ringbuffer
{
    enum class STATUS
    {
        SUCCESS = 0,
        ERROR_BUFFER_FULL,
        ERROR_NOTHING_TO_READ,
        ERROR_ID
    };

    using ID = std::uint32_t;

    template <typename T>
    class IRingBuffer
    {
    public:
        virtual ~IRingBuffer(){};
        virtual ID addReader() = 0;
        virtual ID addWriter() = 0;

        virtual STATUS tryWrite(ID id, T t) = 0;
        virtual STATUS tryReadNext(ID id, T &t) = 0;
        virtual STATUS tryReadNewest(ID id, T &t) = 0;
        virtual void clean() = 0;
    };

} // namespace lockfree_ringbuffer

#endif // I_LOCKFREE_RINGBUFFER_H