#ifndef __PARSER_H__
#define __PARSER_H__

#include <vector>
#include <variant>
#include <optional>
#include <utility>
#include <unordered_map>
#include <iostream>

namespace simple_parser
{

enum symbol_type
{
    SYMBOL_TERMINAL,
    SYMBOL_NONTERMINAL
};

template<class T>
class rule;

template<class T>
class symbol 
{
public:
    T value;
    symbol_type type;
public:
    symbol(T v)
    : value(v), type(SYMBOL_TERMINAL)
    { }
    symbol(T v, symbol_type t)
    : value(v), type(t)
    { }
    ~symbol()
    { }
public:
};

template <class T>
using production_t = std::vector<symbol<T>>;

template <class T>
class production
{
public:
    production_t<T> m_production;
    std::optional<production_t<T>> m_semantic;
public:
    production()
    : m_semantic()
    { }
    production(production_t<T> p)
    : m_production(p), m_semantic()
    {  }
};

template<class T>
constexpr std::ostream& operator<<(std::ostream& os, production_t<T>& p)
{
    for (const auto& it : p)
    {
        os << it.value << " ";
    }
    
    return os;
}

template<class T>
class rule
{
public:
    std::string name;
    production<T> cur_production;
    std::vector<production<T>> productions;
public:
    rule(std::string n)
    : name(n)
    { }
    ~rule()
    { }
public:
    inline void operator()()
    {
        std::cout << "rule " << name << ":" << std::endl;
        unsigned int cnt = 0;
        for (const auto& prod : productions)
        {
            std::cout << "  production " << cnt << std::endl;
            for (const auto& s : prod.m_production)
            {
                std::cout << "    ";
                std::cout << ( s.type == SYMBOL_NONTERMINAL ? "rule " : "" );
                std::cout << s.value << std::endl;
            }
            if (prod.m_semantic)
            {
                std::cout << "  semantic " << cnt << std::endl;
                for (const auto& s : prod.m_semantic.value())
                {
                    std::cout << "    ";
                    std::cout << ( s.type == SYMBOL_NONTERMINAL ? "rule " : "" );
                    std::cout << s.value << std::endl;
                }
            }

            ++cnt;
        }
        std::cout << std::endl;
    }

    constexpr rule<T>& operator=(rule<T> r)
    {
        cur_production = r.cur_production;
        productions = r.productions;

        if (productions.empty() && cur_production.m_production.empty())
        {
            cur_production.m_production.emplace_back(r.name, SYMBOL_NONTERMINAL);
            productions.push_back(cur_production);
        }
        else
        {
            productions.push_back(cur_production);
            cur_production.m_production.clear();
        }

        return *this;
    }
    constexpr rule<T>& operator=(const T& v)
    {
        cur_production.m_production.insert(cur_production.m_production.cbegin(), v);

        productions.push_back(cur_production);
        cur_production.m_production.clear();

        return *this;
    }

    constexpr rule<T>& operator|=(rule<T> r)
    {
        if (!r.cur_production.m_production.empty())
        {
            productions.push_back(r.cur_production);
        }
        else
        {
            if (r.productions.empty())
            {
                cur_production.m_production.emplace_back(r.name, SYMBOL_NONTERMINAL);
                productions.push_back(cur_production);
            }
        }

        productions.insert(productions.end(), r.productions.begin(), r.productions.end());

        return *this;
    }
    constexpr rule<T>& operator|=(const T& v)
    {
        cur_production.m_production.push_back(v);
        productions.push_back(cur_production);

        return *this;
    }

    constexpr rule<T> operator>>(const T& v)
    {
        rule<T> rn(this->name);
        rn.cur_production = this->cur_production;
        if (rn.cur_production.m_production.empty())
        {
            rn.cur_production.m_production.emplace_back(this->name, SYMBOL_NONTERMINAL);
        }
        rn.cur_production.m_production.push_back(v);

        return rn;
    }

    constexpr rule<T> operator>>(rule<T> r)
    {
        rule<T> rn(this->name);
        rn.cur_production = this->cur_production;
        if (rn.cur_production.m_production.empty())
        {
            rn.cur_production.m_production.emplace_back(this->name, SYMBOL_NONTERMINAL);
        }
        rn.cur_production.m_production.emplace_back(r.name, SYMBOL_NONTERMINAL);

        return rn;
    }

    template <class R>
    friend constexpr rule<R> operator>>(const R& v, rule<R> r);

    constexpr rule<T> operator[](rule<T> r)
    {
        std::cout << "running [" << r.cur_production.m_production << "] on " << this->cur_production.m_production << std::endl;

        if (r.cur_production.m_semantic)
        {
            std::cout << "old semantic: " << r.cur_production.m_semantic.value() << std::endl;
        }
        r.cur_production.m_semantic = r.cur_production.m_production;
        std::cout << "new semantic: " << r.cur_production.m_semantic.value() << std::endl;

        return *this;
    }
};

template <class R>
constexpr rule<R> operator>>(const R& v, rule<R> r)
{
    rule<R> rn(r.name);
    rn.cur_production.m_production.push_back(v);
    rn.cur_production.m_production.emplace_back(r.name, SYMBOL_NONTERMINAL);

    return rn;
}

} // namespace parser

#endif