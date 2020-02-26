#ifndef TEEBUF_H
#define TEEBUF_H

#include <iostream>

template <typename C, typename T = std::char_traits<C>>
struct basic_teebuf : public std::basic_streambuf<C, T>
{
    using streambuf_type = std::basic_streambuf<C, T>;
    using int_type = typename T::int_type;

    basic_teebuf(streambuf_type* buff_a, streambuf_type* buff_b)
        : first(buff_a), second(buff_b) {}

protected:
    virtual int_type overflow(int_type c) override
    {
        constexpr int_type eof = T::eof();
        if (T::eq_int_type(c, eof)) {
            return T::not_eof(c);
        }
        else {
            const C ch = T::to_char_type(c);
            if (T::eq_int_type(first->sputc(ch), eof) ||
                T::eq_int_type(second->sputc(ch), eof)) {
                return eof;
            }
            else {
                return c;
            }
        }
    }

    virtual int sync() override
    {
        return !first->pubsync() && !second->pubsync() ? 0 : -1;
    }

private:
    streambuf_type* first;
    streambuf_type* second;
};

template <typename C, typename T = std::char_traits<C>>
struct basic_teestream : public std::basic_ostream<C, T>
{
    typedef std::basic_ostream<C, T> stream_type;
    typedef basic_teebuf<C, T> streambuff_type;

    basic_teestream(stream_type& first, stream_type& second)
        : stream_type(&stmbuf), stmbuf(first.rdbuf(), second.rdbuf()) {}

    basic_teestream(streambuff_type* first, streambuff_type* second)
        : stream_type(&stmbuf), stmbuf(first, second) {}

    ~basic_teestream() { stmbuf.pubsync(); }

private:
    streambuff_type stmbuf;
};

#endif