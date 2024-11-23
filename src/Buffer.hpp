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
#include <memory>

#define DEFAULT_BUFFER_SIZE 512

class Buffer;

using CharArray = std::vector<char>;
using BufferPtr = std::shared_ptr<Buffer>;

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
        _fake_index = _read_index;
        _write_index = ReadableBytes();
    }

public:
    Buffer(uint32_t size = DEFAULT_BUFFER_SIZE)
        : _buffer(size, 0), _read_index(0), _fake_index(0), _write_index(0)
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
        if (len == 0) return;
        
        EnsureWritableBytes(len);
        std::copy(buf, buf + len, _buffer.begin() + _write_index);
        _write_index += len;
    }

    void Write(const std::string buf)
    {
        Write(buf.c_str(), buf.size());
    }

    void Read(char* buf, uint32_t len)
    {
        assert(ReadableBytes() >= len);
        std::copy(_buffer.begin() + _read_index, 
                  _buffer.begin() + _read_index + len,
                  buf);
        _read_index += len;
        _fake_index = _read_index;
    }

    std::string ReadLine()
    {
        auto iter = std::find(_buffer.begin() + _read_index, _buffer.begin() + _write_index, '\n'); // 注意查找的位置
        if (iter == _buffer.end()) { return ""; }

        std::string str(_buffer.begin() + _read_index, iter); // 注意构造的位置
        _read_index = _fake_index = (iter- _buffer.begin() + 1);
        return str;
    }

    std::string ReadAllContent()
    {
        if (!ReadableBytes()) return "";

        char buf[ReadableBytes() + 1] = {'\0'};
        Read(buf, ReadableBytes());

        return buf;
    }

    // 只看不读出，放置数据不完全
    void Peek(char* buf, uint32_t len)
    {
        assert(ReadableBytes() >= len);
        std::copy(_buffer.begin() + _read_index, 
                  _buffer.begin() + _read_index + len,
                  buf);
        _fake_index += len;
    }

    std::string PeekLine()
    {
        auto iter = std::find(_buffer.begin() + _read_index, _buffer.begin() + _write_index, '\n');
        if (iter == _buffer.end()) { return ""; }

        std::string str(_buffer.begin() + _read_index, iter); 
        _fake_index = (iter- _buffer.begin() + 1);
        return str;
    }

    std::string PeekAllContent()
    {
        if (!ReadableBytes()) return "";

        char buf[ReadableBytes() + 1] = {'\0'};
        Peek(buf, ReadableBytes());

        return buf;
    }

    // 读指针实际移动
    void Update()
    {
        _read_index = _fake_index;
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
    uint32_t  _fake_index;  // 读了但不更新
    uint32_t  _write_index; // 写下标
};