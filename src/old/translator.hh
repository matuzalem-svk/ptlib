// Copyright 2019 Matúš Sabol
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

#ifndef __TRANSLATOR_H__
#define __TRANSLATOR_H__

#include <algorithm>
#include <stack>
#include <map>
#include <vector>
#include <memory>
#include <sstream>
#include <cstdio>
#include <iostream>
#include <regex>

namespace ptlib
{

namespace lexer
{

/**
 * @brief Describes various kinds of symbols a parser can work with.
 */
enum symbol_type_t
{
    NONE,
    TERMINAL,
    NONTERMINAL
}; // enum symbol_type_t

/**
 * @brief A class representing a single symbol of a language: terminal, nonterminal, or special.
 */
template <class T>
class symbol 
{
public:
   	T m_value;              ///< Actual value held by the symbol.
    symbol_type_t m_type;   ///< Type of the symbol (e.g. a teminal, a nonterminal, etc.).
    std::string idx_name;   ///< A special group name, if the symbol belongs to one, e.g. "$id" for identifiers
public:
    /**
     * @brief Default constructor. Leaves members uninitialized.
     */
    symbol()
    { }

    /**
     * @brief Initializes symbol's value and type.
     * 
     * @param[in] value Value held by the symbol.
     * @param[in] type Type of the symbol.
     */
    symbol(T value, symbol_type_t type)
    : m_value(value), m_type(type), idx_name(value)
    { }

    /**
     * @brief Initializes symbol's value and type.
     * 
     * @param[in] value Value held by the symbol.
     * @param[in] type Type of the symbol.
     * @param[in] idx Symbol's special index.
     */
    symbol(T value, symbol_type_t type, std::string idx)
    : m_value(value), m_type(type), idx_name(idx)
    { }

    /**
     * @brief Default destructor.
     */
    virtual ~symbol()
    { }

public:
    /**
     * @brief Read-only getter for the symbol value.
     * 
     * @returns Symbol value.
     */
    constexpr T value() const
    {
        return m_value;
    }

    /**
     * @brief Read-only getter for the symbol type.
     * 
     * @returns Symbol type.
     */
    constexpr symbol_type_t type() const
    {
        return m_type;
    }

}; // class symbol

template <class T>
bool operator<(symbol<T> lhs, symbol<T> rhs)
{
    return lhs.type() == rhs.type() && lhs.value() < rhs.value();
}

template <class T>
bool operator>(symbol<T> lhs, symbol<T> rhs)
{
    return lhs.type() == rhs.type() && lhs.value() > rhs.value();
}

template <class T>
bool operator==(symbol<T> lhs, symbol<T> rhs)
{
    return lhs.type() == rhs.type() && lhs.value() == rhs.value();
}

template <class T>
bool operator!=(symbol<T> lhs, symbol<T> rhs)
{
    return !(lhs == rhs);
}

template <class T>
bool operator==(symbol<T> lhs, T rhs)
{
    return lhs.value() == rhs;
}

/**
 * @brief Encapsulates a string of language symbols.
 */
template <class T>
class string 
{
public:
    std::vector<symbol<T>> m_symbols;   ///< String of symbols.

public:
    /**
     * @brief Default constructor.
     */
    string()
    { }

    /**
     * @brief Single symbol constructor.
     */
    string(const symbol<T>& s)
    {
        append(s);
    }

    /**
     * @brief Symbol string copy constructor.
     */
    string(const string<T>& str)
    { 
        append(str);
    }

public:
    /**
     * @brief Appends a symbol by constructing in place.
     * 
     * @param[in] v Symbol value.
     * @param[in] t Symbol type.
     */
    void append(T v, symbol_type_t t)
    {
        m_symbols.emplace_back(v, t);
    }

    /**
     * @brief Appends a symbol by constructing in place.
     * 
     * @param[in] v Symbol value.
     * @param[in] t Symbol type.
     * @param[in] i Symbol special index.
     */
    void append(T v, symbol_type_t t, std::string i)
    {
        m_symbols.emplace_back(v, t, i);
    }

    /**
     * @brief Appends a copy of a single symbol.
     * 
     * @param[in] s Symbol to be appended.
     */
    void append(const symbol<T>& s)
    {
        m_symbols.push_back(s);
    }

    /**
     * @brief Appends a copy of a symbol string.
     * 
     * @param[in] str Symbol string to be appended.
     */
    void append(const string<T>& str)
    {
        m_symbols.insert(m_symbols.end(), str.symbols().begin(), str.symbols().end());
    }

    /**
     * @brief A read-only getter for symbols.
     * 
     * @returns Symbols vector.
     */
    inline std::vector<symbol<T>> symbols() const
    {
        return m_symbols;
    }
}; // class string

template <class T>
constexpr std::ostream& operator<<(std::ostream& os, const string<T>& str)
{
    for (const auto& s : str.symbols())
        os << s;

    return os;
}

/**
 * @brief A simple lexer that reads tokens from configured inputs in a way
 *        defined by regexes.
 */
class string_reader
{
private:
    std::string m_input;                                        ///< The input string being parsed.
    std::vector<std::pair<std::regex, std::string>> m_regex_wl; ///< A collection of regexes specifying strings we want to tokenize.
    std::vector<std::regex> m_regex_bl;                         ///< A collection of regexes specifying strings that are to be ignored.
public:
    /**
     * @brief Default constructor.
     */
    string_reader()
    { }

    /**
     * @brief Regex collection constructor.
     * 
     * @param[in] wl A vector of pairs (regex, token id) to be parsed from input.
     * @param[in] bl A vector of regexes to be ignored from the input when parsing.
     */
    string_reader(std::vector<std::pair<std::regex, std::string>> wl, 
                  std::vector<std::regex> bl)
    : m_regex_wl(wl), m_regex_bl(bl)
    { }

    /**
     * @brief Default destructor.
     */
    ~string_reader()
    { }
private:
    /**
     * @brief Internal - ignores configured strings and retrieves the next valid token.
     * 
     * @param[in] peeking Flag saying whether the input is only peeked, or read and removed from input.
     * 
     * @return The next token from input.
     */
    symbol<std::string> _next_token(bool peeking)
    {
        std::smatch m;

        // first remove all ignored matches
        bool all_ignored = false;
        while (!all_ignored)
        {
            all_ignored = true;
            for (auto bl : m_regex_bl)
            {
                if (std::regex_search(m_input, m, bl))
                {
                    m_input.erase(0, m[0].str().size());
                    all_ignored = false;
                }
            }
        }

        // acquire first token
        for (auto wl : m_regex_wl)
        {
            if (std::regex_search(m_input, m, wl.first))
            {
                lexer::symbol<std::string> ret(m[0].str(), lexer::TERMINAL, wl.second);

                if (!peeking)
                {
                    m_input.erase(0, m[0].str().size());
                }

                return ret;
            }
        }

        return lexer::symbol<std::string>("", lexer::NONE);
    }
public:
    /**
     * @brief Sets the input for the lexer.
     * 
     * @param[in] in The input string.
     */
    void set_input(std::string in)
    {
        m_input = in;
    }

    /**
     * @brief Returns whether the input is empty or not.
     * 
     * @return True when the input is empty, false otherwise.
     */
    bool is_input_empty()
    {
        return m_input.empty();
    }

    /**
     * @brief Adds a regex to the "ignoring" collection.
     * 
     * @param[in] r A string containing the regex in Extended POSIX format.
     */
    void add_regex_ignore(std::string r)
    {
        std::string full_pattern = "^" + r;
        std::regex rx(full_pattern, std::regex_constants::extended);
        m_regex_bl.push_back(rx);
    }

    /**
     * @brief Adds a regex to the "valid" collection
     * 
     * @param[in] r A string containing the regex in Extended POSIX format.
     * @param[in] i Symbol group name. See class symbol for more info.
     */
    void add_regex_use(std::string r, std::string i)
    {
        std::string full_pattern = "^" + r;
        std::regex rx(full_pattern, std::regex_constants::extended);
        m_regex_wl.emplace_back(rx, i);
    }

    /**
     * @brief Peeks the next token from the input.
     * 
     * @return Next token from input.
     */
    symbol<std::string> peek_next_token()
    {
        return _next_token(true);
    }

    /**
     * @brief Reads the next token from the input.
     * 
     * @return Next token from input.
     */
    symbol<std::string> get_next_token()
    {
        return _next_token(false);
    }
};
} // namespace lexer

namespace parser
{

/**
 * @brief Encapsulates CFG productions.
 */
struct production
{
    lexer::symbol<std::string> head;
    lexer::string<std::string> body;
}; // class production

/**
 * @brief Representation of a parser action.
 * 
 * E.g. a reduce action in a LR(1) parser.
 */
template <class T, class A>
struct action_t
{
    T type;
    A arg;
}; // struct action_t

/**
 * @brief A class specifying a common interface for all parsers.
 */
template <class T>
class _parser
{
public:
    _parser() 
    { }

    virtual ~_parser() 
    { }
public:
    virtual bool parse(std::string in) = 0;
}; // class _parser

/**
 * @brief LR(1) parser.
 * 
 * An extension of the generic _parser class. A very rudimentary parser that lacks the ability to 
 * construct its own parsing tables from productions, and therefore needs both specified in a 
 * particular format.
 */
class lr1 : public _parser<std::string>
{
public:
    /**
     * @brief Possible actions doable by a LR(1) parser.
     */
    enum action_type
    {
        ACTION_SHIFT,
        ACTION_REDUCE,
        ACTION_GOTO,
        ACTION_ACCEPT,
        ACTION_REJECT
    };

    /**
     * @brief A single row of the LR(1) parsing table.
     */
    struct item_table_t
    {
        std::map<std::string, action_t<action_type, size_t>> actions;   ///< A sparse row of LR(1) ACTION items.
        std::map<std::string, size_t> gotos;                            ///< A sparse row of LR(1) GOTO items.
    };

    typedef std::vector<item_table_t> lr_parse_table;

private:
    std::vector<lexer::symbol<std::string>> m_terminals;    ///< A list of terminals.
    std::vector<lexer::symbol<std::string>> m_nonterminals; ///< A list of nonterminals.

    lr_parse_table m_parsing_table;                         ///< LR(1) parsing table.
    std::stack<lexer::symbol<std::string>> m_symbol_stack;  ///< Working stack containing terminal and nonterminal symbols.
    std::stack<lr_parse_table::iterator> m_lr_stack;        ///< Working stack containing parsing table references.
public:
    lexer::string_reader* reader;
    std::vector<production> productions;                    ///< A list of productions.
public:
    /**
     * @brief Default constructor.
     */
    lr1()
    { }

    /**
     * @brief Lexer constructor
     * 
     * @param[in] r Refernce to a lexer.
     */
    lr1(lexer::string_reader& r)
    : reader(&r)
    { }

    /**
     * @brief Default destructor.
     */
    ~lr1()
    { }
public:
    void set_reader(lexer::string_reader& r)
    {
        reader = &r;
    }
    /**
     * @brief Loads the parsing table from a string input.
     * 
     * The input string needs to be in a specific format for the table to load properly.
     * The table is spread across multiple lines. Each two lines represent a single 
     * row of the parsing table, with the first of the two rows being the ACTION part
     * and the second being the GOTO part of the row. Lines must not contain any 
     * leading or trailing whitespace. The string must not contain any leading or
     * trailing empty lines. All items must be separated by a single space.
     * 
     * The first table row contains all the terminals and nonterminals the parser will use.
     * The following rows contain ACTION items in form z or zY, where z is 's', 'r', 'a' for 
     * shift, reduce and accept, respectively. Y is the argument of the action, where applicable.
     * For shift, Y denotes the parsing row to push on the parsig stack after shift, while for
     * reduce, Y denotes the production to reduce the stack with. Invalid table positions are 
     * marked by the letter 'x'.
     * 
     * @param[in] table_str A C++ string containing the parsing table.
     * 
     * @returns True if parsing table is loaded successfuly, false if an error occurs.
     */
    bool load_parsing_table(std::string table_str)
    {
        std::istringstream ss_table(table_str);

        // load in terminals - 1st line of input
        {
            std::string str_terminals; 
            std::getline(ss_table, str_terminals);
            std::istringstream ss_terminals(str_terminals);
            std::string term;

            size_t termno = 0;
            while (std::getline(ss_terminals, term, ' '))
            {
                m_terminals.emplace_back(term, lexer::TERMINAL, term);

                ++termno;
            }
        }

        // load in nonterminals - 2nd line of input
        {
            std::string str_nonterminals; 
            std::getline(ss_table, str_nonterminals);
            std::istringstream ss_nonterminals(str_nonterminals);
            std::string nonterm;
            while (std::getline(ss_nonterminals, nonterm, ' '))
            {
                m_nonterminals.emplace_back(nonterm, lexer::NONTERMINAL, nonterm);
            }
        }

        // load in the LR(1) table - ACTION entries on even lines, GOTO entries on odd ones, 
        // rX - reduce using production X
        // sX - shift input character and put table X on stack
        // x - invalid state
        // a - accept
        int lineno = 0;
        parser::lr1::item_table_t lr1_row;
        for (std::string line; std::getline(ss_table, line); ) 
        {
            std::istringstream line_stream(line);
            if (lineno % 2 == 0)
            {
                lr1_row.actions.clear();
                lr1_row.gotos.clear();

                size_t symbol_idx = 0;
                size_t action_arg;
                for (std::string str_action; std::getline(line_stream, str_action, ' '); )
                {
                    // load in ACTION entries
                    switch (str_action[0])
                    {
                        case 'x':
                            break;
                        case 's':
                            action_arg = std::stoi(str_action.substr(1));
                            lr1_row.actions[m_terminals[symbol_idx].value()] = {parser::lr1::ACTION_SHIFT, action_arg};
                            break;
                        case 'r':
                            action_arg = std::stoi(str_action.substr(1));
                            lr1_row.actions[m_terminals[symbol_idx].value()] = {parser::lr1::ACTION_REDUCE, action_arg};
                            break;
                        case 'a':
                            lr1_row.actions[m_terminals[symbol_idx].value()] = {parser::lr1::ACTION_ACCEPT, 0};
                            break;
                        default:
                            printf("ERROR: unexpected ACTION item '%s' on line %d, terminal %s.\n", str_action.c_str(), lineno, m_terminals[symbol_idx].value().c_str());
                            return false;
                            break;
                    }
                    ++symbol_idx;
                }
            }
            else
            {
                size_t symbol_idx = 0;
                size_t goto_arg;
                for (std::string str_goto; std::getline(line_stream, str_goto, ' '); )
                {
                    // load in GOTO entries
                    switch (str_goto[0])
                    {
                        case 'x':
                            break;
                        default:
                            goto_arg = std::stoi(str_goto);
                            lr1_row.gotos[m_nonterminals[symbol_idx].value()] = goto_arg;
                            break;
                    }
                    ++symbol_idx;
                }

                m_parsing_table.push_back(lr1_row);
            }

            ++lineno;
        }

        return true;
    }

    /**
     * @brief Loads production rules from a string input.
     * 
     * The input string needs to be in a specific format for productions to load properly.
     * Productions are spread across multiple lines. Each line represent a single production.
     * Lines must not contain any leading or trailing whitespace. The string must not contain 
     * any leading or trailing empty lines. All items must be separated by a single space.
     * 
     * Each row contains a production rule in form 'A = B', where A is a single nonterminal
     * and B is a string of symbols separated by a single space. Please note the spaces between
     * A, =, and B are required as well.
     * 
     * @param[in] table_str A C++ string containing the production rules.
     * 
     * @returns True if production rules are loaded successfuly, false if an error occurs.
     */
    bool load_production_rules(std::string productions_str)
    {
        std::istringstream productions_stream(productions_str);

        unsigned int lineno = 0;
        productions.reserve(100);
        for (std::string prod_str; std::getline(productions_stream, prod_str); )
        {
            productions.emplace_back();

            std::istringstream pss(prod_str);
            std::string tok;

            std::getline(pss, tok, ' ');
            productions[lineno].head = lexer::symbol<std::string>(tok, lexer::NONTERMINAL, tok);

            std::getline(pss, tok, ' ');
            if (tok != "=")
            {
                printf("ERROR: expecting '=' on rule %u, symbol 1\n", lineno);
                return false;
            }

            unsigned int sidx = 2;
            while (std::getline(pss, tok, ' '))
            {
                if (std::find(m_terminals.begin(), m_terminals.end(), lexer::symbol<std::string>(tok, lexer::TERMINAL)) != m_terminals.end())
                {
                    productions[lineno].body.append(tok, lexer::TERMINAL, tok);
                }
                else if (std::find(m_nonterminals.begin(), m_nonterminals.end(), lexer::symbol<std::string>(tok, lexer::NONTERMINAL)) != m_nonterminals.end())
                {
                    productions[lineno].body.append(tok, lexer::NONTERMINAL, tok);
                }
                else
                {
                    printf("ERROR: unknown symbol %s on production %u, symbol %u.\n", tok.c_str(), lineno, sidx);
                    return false;
                }

                ++sidx;
            }

            ++lineno;
        }

        return true;
    }

public:
    /**
     * @brief Initializes the parser for parsing.
     * 
     * @param[in] in Parser input string.
     * 
     * @returns True if everything initializes successfuly, false otherwise.
     */
    bool parse_init(std::string in) 
    {
        // prepare input
        reader->set_input(in);

        // clear and initialize the parsing stacks
        m_symbol_stack = std::stack<lexer::symbol<std::string>>();
        m_lr_stack = std::stack<lr_parse_table::iterator>();
        m_lr_stack.push(m_parsing_table.begin());

        return true;
    };

    /**
     * @brief Performs a single parse step.
     * 
     * @param[out] o_symbols A list of terminals belonging to special groups (e.g. '$id') that were reduced, in order.
     * 
     * @returns An action performed by the parser.
     */
    action_t<action_type, size_t> parse_step(lexer::string<std::string>* o_symbols)
    {
        // gather working symbols and parse rows
        lr_parse_table::iterator it_ptab = m_lr_stack.top();
        item_table_t table_item = *it_ptab;

        lexer::symbol<std::string> top_symbol("", lexer::NONE);
        if (!m_symbol_stack.empty())
        {
            top_symbol = m_symbol_stack.top();
        }

        // handle output
        if(o_symbols)
        {
            o_symbols->m_symbols.clear();
        }

#ifdef DEBUG
printf("==== CURRENT STACK STATUS ====\n");
{
    std::stack new_stack(m_symbol_stack);
    std::stack<lexer::symbol<std::string>> rev_stack;
    printf("m_symbol_stack size: %lu\n", new_stack.size());

    while (!new_stack.empty())
    {
        rev_stack.push(new_stack.top());
        new_stack.pop();
    }

    printf("[");
    while (!rev_stack.empty())
    {
        printf(" %s", rev_stack.top().value().c_str());
        rev_stack.pop();
    }
    printf(" ]\n");
}
{
    std::stack new_stack(m_lr_stack);
    std::stack<lr_parse_table::iterator> rev_stack;
    printf("m_lr_stack size: %lu\n", new_stack.size());

    while (!new_stack.empty())
    {
        rev_stack.push(new_stack.top());
        new_stack.pop();
    }

    printf("[");
    while (!rev_stack.empty())
    {
        auto it = rev_stack.top();
        printf(" %ld", (it - m_parsing_table.begin()));
        rev_stack.pop();
    }
    printf(" ]\n");
}
printf("======= CURRENT ACTION =======\n");
#endif
        // lookup next token
        lexer::symbol<std::string> token = reader->peek_next_token();

        auto lr_stack_size = m_lr_stack.size();
        auto symbol_stack_size = m_symbol_stack.size();

        // decide on the next action
        if (lr_stack_size > symbol_stack_size)
        {
            // parsing table is on top of stack, search ACTION table
            auto new_action_it = table_item.actions.find(token.idx_name); 
            if (new_action_it == table_item.actions.end())
            {
                printf("ERROR: no next course of action found for input '%s'.\n", token.value().c_str());
                return {ACTION_REJECT, 0};
            }

            switch ((*new_action_it).second.type)
            {
                case ACTION_ACCEPT:
#ifdef DEBUG
printf("ACCEPT input\n");
printf("==============================\n\n");
#endif
                    break;
                case ACTION_SHIFT:
                    token = reader->get_next_token();
                    m_symbol_stack.push(token);
                    m_lr_stack.push(m_parsing_table.begin() + (*new_action_it).second.arg);
#ifdef DEBUG
printf("SHIFT '%s' on stack, GOTO '%lu'\n", token.value().c_str(), (*new_action_it).second.arg);
#endif
                    break;
                case ACTION_REDUCE:
                    {
                        production& p = productions.at((*new_action_it).second.arg - 1);
                        for (int i = p.body.symbols().size() - 1; i >= 0; --i)
                        {
                            if (symbol_stack_size < p.body.symbols().size()
                            ||  lr_stack_size <= p.body.symbols().size())
                            {
                                printf("ERROR: insufficient number of symbols on stack - %lu needed, have (%lu/%lu).\n", p.body.symbols().size(), symbol_stack_size, lr_stack_size);
                                return {ACTION_REJECT, 0};
                            }

                            if (m_symbol_stack.top().idx_name != p.body.symbols()[i].value())
                            {
                                printf("ERROR: symbol on stack '%s'(%s) not expected by production (%s).\n", 
                                    m_symbol_stack.top().value().c_str(), 
                                    m_symbol_stack.top().idx_name.c_str(),
                                    p.body.symbols()[i].value().c_str());
                                return {ACTION_REJECT, 0};
                            }

                            if (o_symbols && m_symbol_stack.top().idx_name == "$id")
                            {
                                o_symbols->append(m_symbol_stack.top());
                            }

                            m_symbol_stack.pop();
                            m_lr_stack.pop();
                        }

                        m_symbol_stack.push(p.head);
#ifdef DEBUG
printf("REDUCE using production '%lu', put '%s' on stack\n", (*new_action_it).second.arg, p.head.value().c_str());
#endif
                    }
                    break;
                default:
                    printf("ERROR: encountered an unsupported type of action on token %s.\n", token.value().c_str());
                    return {ACTION_REJECT, 0};
                    break;
            }
            return (*new_action_it).second;
        }
        else
        {
            // nonterminal is on top of stack, search GOTO table
            auto new_goto_it = table_item.gotos.find(top_symbol.value()); 
            if (new_goto_it == table_item.gotos.end())
            {
                printf("ERROR: no next GOTO found on nonterminal '%s' for input '%s'.\n", 
                    top_symbol.idx_name.c_str(),
                    token.idx_name.c_str());
                return {ACTION_REJECT, 0};
            }

            // push new parsing table reference to the LR stack
            m_lr_stack.push(m_parsing_table.begin() + (*new_goto_it).second);
            return {ACTION_GOTO, (*new_goto_it).second};
#ifdef DEBUG
printf("GOTO table '%lu'\n", (*new_goto_it).second);
#endif
        }
#ifdef DEBUG
printf("==============================\n\n");
#endif

        return {ACTION_REJECT, 0};
    };

public:
    /**
     * @brief 
     * 
     * @param in 
     * 
     * @returns
     */
    bool parse(std::string in) 
    {
        parse_init(in);

        while (true)
        {
            auto action = parse_step(nullptr);

            switch (action.type)
            {
                case ACTION_REJECT:
                    return false;
                    break;
                case ACTION_ACCEPT:
                    return true;
                    break;
                default:
                    break;
            }
            
        }

        return true;
    };
}; // class lr1

} // namespace parser

namespace translator
{
/**
 * @brief A LR(1) translator with a LR(1) parser and configurable semantic actions (translations).
 * 
 * This LR(1) translator uses an LR(1) parser to perform translations on SDTS with simple 
 * postfix LR(1) underlying grammars in single pass.
 */
class lr1_translator
{
private:
    parser::lr1* m_parser;                      ///< A pointer to a LR(1) parser.
    std::vector<std::string> m_translations;    ///< 
public:
    /**
     * @brief Parser constructor.
     * 
     * @param[in] parser A reference to an LR(1) parser, preferably configured.
     */
    lr1_translator(parser::lr1& parser)
    : m_parser(&parser)
    { }
public:
    /**
     * @brief Loads translations from a string.
     * 
     * @param[in] translation_str Input string containing translations.
     * 
     * @returns True if rules load correctly.
     */
    bool load_translation_rules(std::string translation_str)
    {
        std::istringstream iss(translation_str);
        std::string translation;

        while (std::getline(iss, translation))
        {
            m_translations.push_back(translation);
        }

        return true;
    }

    /**
     * @brief Parses an input string and stores its translation into another string if successful.
     * 
     * @param[in] in Input string.
     * @param[out] out Output string pointer.
     * 
     * @returns True if translation finishes successfully, false otherwise.
     */
    bool translate(std::string in, std::string* out)
    {
        if (!m_parser)
            return false;

        if (!m_parser->parse_init(in))
        {
            return false;
        }

        lexer::string<std::string> terms;
        std::ostringstream oss;

        while (true)
        {
            terms.m_symbols.clear();
            auto action = m_parser->parse_step(&terms);

            switch (action.type)
            {
                case parser::lr1::ACTION_REJECT:
                    return false;
                    break;
                case parser::lr1::ACTION_ACCEPT:
                    (*out) = oss.str();
                    return true;
                    break;
                case parser::lr1::ACTION_REDUCE:
                    {
                        std::istringstream iss(m_translations[action.arg - 1]);
                        std::string term;

                        int id_i = 0;
                        while (std::getline(iss, term, ' '))
                        {
                            if (term == "$id")
                            {
                                oss << terms.m_symbols[id_i++].value().c_str() << " ";
                            }
                            else
                            {
                                oss << term.c_str() << " ";
                            }
                        }

                    }
                    break;
                default:
                    break;
            }
        }
    }
}; // class translator

} // namespace translator

} // namespace ptlib

#endif