// Copyright 2026 Matus Sabol
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
// OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT 
// OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include "common/types.h"
#include "common/grammar.h"
#include <iostream>

namespace ptlib::utility {

class NullStream : public std::ostream
{
public:
    NullStream() 
    : std::ostream(nullptr)
    {}

    NullStream(const NullStream&)
    : std::ostream(nullptr)
    {}

    template <class T>
    friend NullStream& operator<<(NullStream& os, const T& value)
    {
        return os;
    }
} inline nullout;

#ifdef PTLIB_VERBOSE_LOGGING
#define ptlib_out std::cout
#else
#define ptlib_out ptlib::utility::nullout
#endif

void printLRItemSet(const common::Grammar& grammar, const common::LRItemSet& itemSet)
{
    for (const auto& item : itemSet)
    {
        ptlib::common::Production p = grammar.productions[item.first];

        ptlib_out << *p.first << " -> ";

        bool dotPrinted = false;
        for (int i = 0; i < p.second.size(); ++i)
        {
            if (i == item.second)
            {
                ptlib_out << ". ";
                dotPrinted = true;
            }

            ptlib_out << *p.second[i];
        }

        if (!dotPrinted)
        {
            ptlib_out << ".";
        }

        ptlib_out << std::endl;
    }
}

} // namespace ptlib::utility