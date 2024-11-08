/*
* CopyRight Limuyou
* Date 24_11_3
* brief 实现了一个缓冲区，用于高效地对报文的读写操作
*/

#pragma once
#include <assert.h>
#include <iostream>
#include <vector>
#include <string>

#define DEFAULT_BUFFER_SIZE 10
using CharArray = std::vector<char>;

class Buffer
{
private:
    uint32_t FrontAvailable()
    {
        return _buffer.size() - _write_index;
    }

    uint32_t BackAvailable()
    {
        return _read_index;
    }

    void AdjustSpace()
    {
        std::copy(_buffer.begin() + _read_index, 
                  _buffer.begin() + _read_index + ReadableBytes(),
                  _buffer.begin());

        _read_index = 0;
        _write_index = _read_index + ReadableBytes();
    }

public:
    Buffer(uint32_t size = DEFAULT_BUFFER_SIZE)
        : _buffer(size, 0), _read_index(0), _write_index(0)
    {}

    uint32_t ReadableBytes()
    {
        return _write_index - _read_index;
    }

    uint32_t WritableBytes()
    {
        return FrontAvailable() + BackAvailable();
    }

    void EnsureWritableBytes(uint32_t len)
    {
        if (FrontAvailable() >= len) return;
        else if (WritableBytes() >= len)
        {
            AdjustSpace();
            return;
        }
        else
        {
            _buffer.resize(_write_index + len);
            return;
        }
    }

    void Write(const char* buf, uint32_t len)
    {
        EnsureWritableBytes(len);
        std::copy(buf, buf + len, _buffer.begin() + _write_index);
        _write_index += len;
    }

    void Read(char* buf, uint32_t len)
    {
        assert(ReadableBytes() >= len);
        std::copy(_buffer.begin() + _read_index, 
                  _buffer.begin() + _read_index + len,
                  buf);
        _read_index += len;
    }

    std::string ToString()
    {
        if (!ReadableBytes()) return "";

        char buf[ReadableBytes() + 1] = {'\0'};
        Read(buf, ReadableBytes());

        return buf;
    }

    void Clear()
    {
        _read_index = _write_index = 0;
    }

    bool Empty()
    {
        return _write_index == 0;
    }



private:
    CharArray _buffer;      // 缓冲区
    uint32_t  _read_index;  // 读下标
    uint32_t  _write_index; // 写下标
};