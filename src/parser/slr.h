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
#include <map>
#include <vector>
#include <stack>
#include <tuple>


namespace ptlib { namespace parser {

using namespace ptlib::common;

struct ParseStepResult
{
    bool actionSuccess;
    bool requireNextToken;
    bool parseSuccess;
};

typedef std::map<const Symbol*, std::set<const Symbol*>> SymbolSetMap;

enum ParseActionType
{
    PARSE_SHIFT,
    PARSE_REDUCE,
    PARSE_GOTO,
    PARSE_ACCEPT
};

struct ParseAction
{
    ParseActionType actionType;
    size_t parameter;

    bool operator==(const ParseAction& other)
    {
        return actionType == other.actionType && parameter == other.parameter;
    }
};

typedef std::map<std::tuple<LRStateID, const Symbol*>, ParseAction> ParseTableMap;

typedef std::pair<LRStateID, const Symbol*> GOTOKey;
typedef std::map<GOTOKey, size_t> ParseGotoMap;

class SLRParser
{
public:
    SLRParser() {}
    SLRParser(const Grammar& grammar);

    void ResetParse();
    // <successful parse action, require next symbol, complete parsing success>
    ParseStepResult StepParse(const Grammar& grammar, const Symbol* token);

protected:
    LRItemSet CLOSURE(const Grammar& inGrammar, const LRItemSet& initialItemSet) const;
    LRItemSet GENERATE_GOTO(const Grammar& inGrammar, const LRItemSet& inItemSet, const Symbol* inSymbol) const;
    LRItemSetCollection GenerateCanonicalSymbolCollection(const Grammar& inGrammar, ParseGotoMap& outGoto) const;

    SymbolSetMap GENERATE_FIRST(const Grammar& inGrammar) const;
    SymbolSetMap GENERATE_FOLLOW(const Grammar& inGrammar, const SymbolSetMap& FIRST) const;

    ParseTableMap GenerateParseTable(const Grammar& inGrammar, const LRItemSetCollection& inCanonicalCollection, const SymbolSetMap& FOLLOW, const ParseGotoMap& GOTO) const;

protected:
    ParseTableMap PARSE_TABLE;
    std::stack<LRStateID> PARSE_STACK;
}; // class SLRParser

} } // namespace ptlib::parser