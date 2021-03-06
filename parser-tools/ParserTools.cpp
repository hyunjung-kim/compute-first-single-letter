#include "ParserTools.h"

#include <unordered_map>
#include <algorithm>
#include <set>

namespace parsertools
{
	char ProductionRule::Vn() const
	{
		return vn_;
	}

	std::string ProductionRule::Rhs()
	{
		return rhs_;
	}

	bool ProductionRule::IsValid() const
	{
		return valid_;
	}

	void ProductionRule::Invalidate()
	{
		valid_ = false;
	}

	std::unordered_map<char, std::vector<char>> ParserTools::Compute_FIRST(std::vector<ProductionRule> & rules)
	{
		// Step 1: initialize FIRSTs
		// e.g.) {A -> aA | B | C}  =>  A: [aA, B, C]		
		std::unordered_map<char, std::vector<char>> firsts;

		// Step 1-1: handle single terminal symbol rule
		for (auto & rule : rules)
		{
			if (rule.Rhs().size() == 1 && IsTerminalSymbol(rule.Rhs()[0]))
			{
				firsts.insert({ rule.Vn(), std::vector<char>{rule.Rhs()[0]} });
				rule.Invalidate();
			}
		}

		// Step 2: handle Vn -> aX where a is a terminal symbol
		for (auto & rule : rules)
		{
			if (!rule.IsValid())
				continue;

			std::vector<char> tmpFirsts = {};
			if (rule.Rhs().size() >= 2 && IsTerminalSymbol(rule.Rhs()[0]))
			{
				if (firsts.count(rule.Vn()) > 0)
				{
					tmpFirsts = firsts[rule.Vn()];
				}

				tmpFirsts.push_back(rule.Rhs()[0]);
				firsts[rule.Vn()] = tmpFirsts;
				rule.Invalidate();
			}
			else
			{
				if (IsEpsilon(rule.Rhs()[0]))
				{
					if (firsts.count(rule.Vn()) > 0)
					{
						tmpFirsts = firsts[rule.Vn()];
					}

					tmpFirsts.push_back(rule.Rhs()[0]);
					firsts[rule.Vn()] = tmpFirsts;
					rule.Invalidate();
				}
			}
		}

		// Step 3: handle all remaining rules
		bool modified = false;
		do
		{
			for (auto & rule : rules)
			{
				if (!rule.IsValid())
					continue;

				size_t existingFirstCnt = 0;
				std::vector<char> existingFirst = {};
				if (firsts.count(rule.Vn()) > 0)
				{
					existingFirst = firsts[rule.Vn()];
					existingFirstCnt = existingFirst.size();
				}

				std::vector<char> ringSum = {};
				if (firsts.count(rule.Rhs()[0]) > 0)
				{
					ringSum = firsts[rule.Rhs()[0]];
				}

				for (int i = 1; i < rule.Rhs().size(); ++i)
				{
					std::vector<char> currFirsts = {};
					if (firsts.count(rule.Rhs()[i]) > 0)
					{
						currFirsts = firsts[rule.Rhs()[i]];
					}
					else if (IsTerminalSymbol(rule.Rhs()[i]))
					{
						currFirsts = std::vector<char>{ rule.Rhs()[i] };
					}
					
					ringSum = RingSum(ringSum, currFirsts);
				}

				std::set<char> updatedFirsts;
				for (auto i : ringSum)
				{
					updatedFirsts.insert(i);
				}
				for (auto i : existingFirst)
				{
					updatedFirsts.insert(i);
				}

				existingFirst.clear();
				for (auto i : updatedFirsts)
				{
					existingFirst.push_back(i);
				}

				if (existingFirstCnt != updatedFirsts.size())
				{
					firsts[rule.Vn()] = existingFirst;
					modified = true;
				}
				else
				{
					rule.Invalidate();
					modified = false;
				}
			}
		} while (modified);

		return firsts;
	}

	std::vector<char> ParserTools::RingSum(std::vector<char> & lhs, std::vector<char> & rhs)
	{
		std::vector<char> ringSum;

		auto ret = std::find(lhs.begin(), lhs.end(), '#');
		if (ret != lhs.end())
		{
			lhs.erase(std::remove(lhs.begin(), lhs.end(), '#'), lhs.end());
			ringSum = Union(lhs, rhs);
		}
		else
		{
			ringSum = lhs;
		}

		return std::move(ringSum);
	}

	std::vector<char> ParserTools::Union(std::vector<char> & lhs, std::vector<char> & rhs)
	{
		std::vector<char> unionSum;

		std::sort(lhs.begin(), lhs.end());
		std::sort(rhs.begin(), rhs.end());
		std::set_union(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), std::back_inserter(unionSum));

		return unionSum;
	}

	bool ParserTools::IsEpsilon(char v)
	{
		return v == '#';
	}

	bool ParserTools::IsTerminalSymbol(char v)
	{
		if (v >= 97 && v <= 127) // a - z
		{
			return true;
		}
		else if (v >= 65 && v <= 90) // A - Z
		{
			return false;
		}
		else
		{
			throw std::runtime_error("Symbol must be a letter");
		}
	}
}


