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

#include "parser/iparser.h"
#include "common/types.h"
#include "common/utility.h"
#include <iomanip>
#include <map>
#include <vector>
#include <stack>
#include <tuple>


namespace ptlib { namespace parser {

using namespace ptlib::common;

class SLRParser : public IParserImpl
{
public:
    SLRParser() {}
    SLRParser(const Grammar& grammar)
    {
        /*
        std::cout << "GRAMMAR PRODUCTIONS" << std::endl;
        utility::printGrammarProductions(grammar);
        std::cout << std::endl;

        std::cout << "SLR ITEM SETS" << std::endl;
        size_t debugItemSet = 0;
        for (const auto& itemSet : collection)
        {
            std::cout << debugItemSet << std::endl;
            utility::printLRItemSet(grammar, itemSet);
            std::cout << std::endl;
            ++debugItemSet;
        }

        std::cout << "GOTO" << std::endl;
        for (const auto& [gotoParam, nextState] : GOTO)
        {
            std::cout << "{ " << gotoParam.first << ", '" << gotoParam.second->name << "' } = " << nextState << std::endl;
        }
        std::cout << std::endl;

        std::cout << "FIRST" << std::endl;
        for (const auto& firstItem : FIRST)
        {
            std::cout << "{ '" << firstItem.first->name << "' } = { ";
            for (const auto* symbol : firstItem.second)
            {
                std::cout << "'" << symbol->name << "', ";
            }
            std::cout << "}" << std::endl;
        }
        std::cout << std::endl;

        std::cout << "FOLLOW" << std::endl;
        for (const auto& followItem : FOLLOW)
        {
            std::cout << "{ '" << followItem.first->name << "' } = { ";
            for (const auto* symbol : followItem.second)
            {
                std::cout << "'" << symbol->name << "', ";
            }
            std::cout << "}" << std::endl;
        }
        std::cout << std::endl;

        std::cout << "PARSE TABLE" << std::endl;
        size_t curItem = 0;
        for (const auto& [tableKey, parseAction] : PARSE_TABLE)
        {
            if (std::get<0>(tableKey) > curItem)
            {
                curItem = std::get<0>(tableKey);
                std::cout << std::endl;
            }

            std::cout << std::get<0>(tableKey) << ": ";
            std::cout << "{'" << std::get<1>(tableKey)->name << "', ";
            switch (parseAction.actionType)
            {
                case PARSE_ACCEPT:
                    std::cout << "ACCEPT";
                    break;
                case PARSE_SHIFT:
                    std::cout << "s" << parseAction.parameter;
                    break;
                case PARSE_REDUCE:
                    std::cout << "r" << parseAction.parameter;
                    break;
                case PARSE_GOTO:
                    std::cout << "GOTO " << parseAction.parameter;
                    break;
            }

            std::cout << "} ";
        }
        std::cout << std::endl;
        */
    }

    virtual bool Initialize(const common::Grammar& grammar) override
    {
        ParseGotoMap GOTO;
        auto collection = GenerateCanonicalSymbolCollection(grammar, GOTO);

        auto FIRST = GENERATE_FIRST(grammar);
        auto FOLLOW = GENERATE_FOLLOW(grammar, FIRST);

        PARSE_TABLE = GenerateParseTable(grammar, collection, FOLLOW, GOTO);

        return !PARSE_TABLE.empty();
    }

    virtual void ResetParse() override
    {
        while (!PARSE_STACK.empty()) PARSE_STACK.pop();

        PARSE_STACK.push(0);
    }

    virtual ParseStepResult StepParse(const Symbol* token) override
    {
        const LRStateID& topState = PARSE_STACK.top();

        //auto actionIt = PARSE_TABLE.find({ topState, token });

        auto actionIt = std::find_if(PARSE_TABLE.begin(), PARSE_TABLE.end(), [&topState, &token](const std::pair<std::tuple<LRStateID, const Symbol*>, ParseAction>& parseAction)
        {
            /*LRStateID stateId = std::get<0>(parseAction.first);
            const Symbol* nextToken = std::get<1>(parseAction.first);*/
            auto [stateId, nextToken] = parseAction.first;
            return topState == stateId && token->TypeEquals(nextToken);
        });

        // no parsing action for state/symbol == error
        if (actionIt == PARSE_TABLE.end())
        {
            std::cout << "PARSING FAILED!!!" << std::endl;
            return { false, false, false };
        }

        const ParseAction& action = actionIt->second;
        switch (action.actionType)
        {
            case PARSE_ACCEPT:
            {
                std::cout << "PARSING SUCCESSFUL!!!" << std::endl;
                return { true, false, true };
            }
            break;
            case PARSE_SHIFT:
            {
                std::cout << "s" << action.parameter << " - shift " << std::quoted(token->value) << std::endl;
                PARSE_STACK.push(action.parameter);
                return { true, true, false };
            }
            break;
            case PARSE_REDUCE:
            {
                std::cout << "r" << action.parameter << " - reduce " << action.parameter << std::endl;

                for (size_t j = 0; j < action.reduceAmount; ++j)
                {
                    PARSE_STACK.pop();
                }

                const LRStateID& newTopState = PARSE_STACK.top();
                auto gotoIt = PARSE_TABLE.find({ newTopState, action.reduceGotoNonterm });
                if (gotoIt == PARSE_TABLE.end() || gotoIt->second.actionType != PARSE_GOTO)
                {
                    std::cout << "EXPECTED GOTO ACTION!!!" << std::endl;
                    return { false, false, false };
                }
                PARSE_STACK.push(gotoIt->second.parameter);
                return { true, false, false };
            }
            break;
        }

        return { true, false, true };
    }

protected:
    LRItemSet CLOSURE(const Grammar& inGrammar, const LRItemSet& initialItemSet) const
    {
        LRItemSet outItems;
        outItems = initialItemSet;

        bool itemsAdded;
        do
        {
            itemsAdded = false;
            for (const auto& item : outItems)
            {
                Production prod = inGrammar.productions[item.first];

                if (item.second >= prod.second.size())
                {
                    continue;
                }

                const Symbol* symbolAfterDot = prod.second[item.second];
                if (symbolAfterDot->IsNonterminal())
                {
                    for (int i = 0; i < inGrammar.productions.size(); ++i)
                    {
                        LRItem newItem(i, 0);
                        if (inGrammar.productions[i].first->TypeEquals(symbolAfterDot))
                        {
                            bool added = std::find(outItems.begin(), outItems.end(), newItem) == outItems.end();
                            itemsAdded |= added;

                            if (added)
                            {
                                outItems.push_back(newItem);
                            }
                        }
                    }
                }
            }
        } while (itemsAdded);

        return outItems;
    }

    LRItemSet GENERATE_GOTO(const Grammar& inGrammar, const LRItemSet& inItemSet, const Symbol* inSymbol) const
    {
        LRItemSet inItems;

        for (const auto& item : inItemSet)
        {
            const Production& prod = inGrammar.productions[item.first];

            if (item.second >= prod.second.size())
            {
                continue;
            }

            const Symbol* symbolAfterDot = prod.second[item.second];

            if (symbolAfterDot->TypeEquals(inSymbol))
            {
                bool added = std::find(inItems.begin(), inItems.end(), LRItem{item.first, item.second+1}) == inItems.end();
                if (added)
                {
                    inItems.push_back({item.first, item.second + 1});
                }
            }
        }

        return CLOSURE(inGrammar, inItems);
    }

    LRItemSetCollection GenerateCanonicalSymbolCollection(const Grammar& inGrammar, ParseGotoMap& outGoto) const
    {
        LRItemSetCollection out;
        out.push_back(CLOSURE(inGrammar, LRItemSet{ LRItem{0, 0}}));

        bool newSetAddedToCollection;
        do
        {
            newSetAddedToCollection = false;

            size_t outSize = out.size();
            for (size_t i = 0; i < outSize; ++i)
            {
                const LRItemSet& itemSet = out[i];
                for (const Symbol* symbol : inGrammar.symbols)
                {
                    LRItemSet newItemSet = GENERATE_GOTO(inGrammar, itemSet, symbol);
                    if (newItemSet.empty()) continue;

                    auto findIt = std::find(out.begin(), out.end(), newItemSet);
                    bool added = findIt == out.end();
                    newSetAddedToCollection |= added;

                    // save GOTO action for parsing table
                    outGoto.insert(std::make_pair(GOTOKey{i, symbol}, findIt - out.begin()));

                    if (added)
                    {
                        out.push_back(newItemSet);
                    }
                }
            }

        } while (newSetAddedToCollection);

        return out;
    }

    SymbolSetMap GENERATE_FIRST(const Grammar& inGrammar) const
    {
        std::map<const Symbol*, std::set<const Symbol*>> FIRST;
        for (const Symbol* symbol : inGrammar.symbols)
        {
            if (symbol->IsTerminal())
            {
                FIRST[symbol] = { symbol };
            }
            else
            {
                FIRST[symbol] = {};
            }
        }

        bool bAddedNewFirst;
        do
        {
            bAddedNewFirst = false;

            for (const Production& prod : inGrammar.productions)
            {
                const Symbol* symbol = prod.first;

                if (prod.second.size() == 0) continue;

                if (prod.second.size() == 1 && prod.second[0] == inGrammar.emptyTerminal)
                {
                    bAddedNewFirst |= FIRST[symbol].insert(inGrammar.emptyTerminal).second;
                    continue;
                }

                for (const Symbol* tailSymbol : prod.second)
                {
                    const auto& tailSymbolFirstSet = FIRST[tailSymbol];
                    if (tailSymbolFirstSet.count(inGrammar.emptyTerminal)) continue;

                    for (const Symbol* kokod : tailSymbolFirstSet)
                    {
                        bAddedNewFirst |= FIRST[symbol].insert(kokod).second;
                    }

                    break;
                }
            }

        } while (bAddedNewFirst);

        return FIRST;
    }

    SymbolSetMap GENERATE_FOLLOW(const Grammar& inGrammar, const SymbolSetMap& FIRST) const
    {
        std::map<const Symbol*, std::set<const Symbol*>> FOLLOW;
        FOLLOW.insert({ inGrammar.startNonterminal, { inGrammar.eofTerminal } });

        for (const Symbol* symbol : inGrammar.symbols)
        {
            if (symbol->IsNonterminal() && symbol != inGrammar.startNonterminal)
            {
                FOLLOW[symbol] = {};
            }
        }

        bool bAddedNewFollow;
        do 
        {
            bAddedNewFollow = false;

            for (const Production& prod : inGrammar.productions)
            {
                if (prod.second.size() == 0) continue;

                for (size_t i = 0; i < prod.second.size(); ++i)
                {
                    const Symbol* tailSymbol = prod.second[i];

                    if (tailSymbol->IsTerminal()) continue;

                    auto firstIt = FIRST.find(prod.second[i]); // TODO sanity check
                    if (i == prod.second.size() - 1 || firstIt->second.count(inGrammar.emptyTerminal))
                    {
                        for (const Symbol* kokodSymbol : FOLLOW[prod.first])
                        {
                            bAddedNewFollow |= FOLLOW[tailSymbol].insert(kokodSymbol).second;
                        }

                        continue;
                    }

                    firstIt = FIRST.find(prod.second[i + 1]); // TODO sanity check
                    for (const Symbol* kokodSymbol : firstIt->second)
                    {
                        bAddedNewFollow |= FOLLOW[tailSymbol].insert(kokodSymbol).second;
                    }
                }
            }

        } while (bAddedNewFollow);

        return FOLLOW;
    }

    inline bool TryAddParseTableAction(ParseTableMap& PARSE_TABLE, std::tuple<LRStateID, const common::Symbol*> parseKey, const ParseAction& newParseAction) const
    {
        auto parseTableIt = PARSE_TABLE.find(parseKey);
        if (parseTableIt != PARSE_TABLE.end()) return false;

        PARSE_TABLE[parseKey] = newParseAction;

        return true;
    }

    ParseTableMap GenerateParseTable(const Grammar& inGrammar, const LRItemSetCollection& inCanonicalCollection, const SymbolSetMap& FOLLOW, const ParseGotoMap& GOTO) const
    {
        ParseTableMap PARSE_TABLE;
        ParseTableMap empty;

        for (size_t i = 0; i < inCanonicalCollection.size(); ++i)
        {
            const LRItemSet& itemSet = inCanonicalCollection[i];

            for (const auto& item : itemSet)
            {
                const size_t prodId = item.first;
                const Production& prod = inGrammar.productions[prodId];
                const int prodDot = item.second;

                if (prodDot == prod.second.size())
                {
                    // [S' -> S.] means ACTION[i, EOF] = ACCEPT
                    if (prod.first == inGrammar.augmentedStartNonterminal)
                    {
                        if (!TryAddParseTableAction(PARSE_TABLE, {i, inGrammar.eofTerminal }, { .actionType = PARSE_ACCEPT })) return empty;
                        continue;
                    }

                    // [A -> w.] means ACTION[i, a] = REDUCE A -> a for all a in FOLLOW(A)
                    auto followIt = FOLLOW.find(prod.first); // TODO sanity check
                    for (const Symbol* followTerminal : followIt->second)
                    {
                        if (!TryAddParseTableAction(PARSE_TABLE, { i, followTerminal }, 
                            { 
                                .actionType = PARSE_REDUCE, 
                                .parameter = prodId, 
                                .reduceAmount = prod.second.size(), 
                                .reduceGotoNonterm = prod.first 
                            })) return empty;
                    }

                    continue;
                }

                // [A -> w.ay] means ACTION[i, a] = SHIFT j for GOTO(i, a) = j, a is a terminal
                const Symbol* nextSymbol = prod.second[prodDot];
                if (!nextSymbol->IsTerminal()) continue;

                auto gotoIt = GOTO.find({ i, nextSymbol }); // TODO sanity check

                if (!TryAddParseTableAction(PARSE_TABLE, { i, nextSymbol }, { .actionType = PARSE_SHIFT, .parameter = gotoIt->second })) return empty;
            }

            for (const Symbol* nterm : inGrammar.symbols)
            {
                if (!nterm->IsNonterminal()) continue;

                auto findIt = GOTO.find({i, nterm});
                if (findIt == GOTO.end()) continue;

                if (!TryAddParseTableAction(PARSE_TABLE, { i, nterm }, { .actionType = PARSE_GOTO, .parameter = findIt->second })) return empty;
            }
        }

        return PARSE_TABLE;
    }

protected:
    ParseTableMap PARSE_TABLE;
    std::stack<LRStateID> PARSE_STACK;
}; // class SLRParser

} } // namespace ptlib::parser