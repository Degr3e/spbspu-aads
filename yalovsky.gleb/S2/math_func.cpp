#include "math_func.hpp"

#include <cstdlib>
#include <istream>
#include <limits>
#include <stdexcept>

namespace
{
  const int bitOrPriority = 0;
  const int addSubPriority = 1;
  const int multDivModPriority = 2;

  bool isDelimiter(char symbol)
  {
    return symbol == ' ' || symbol == '\t' || symbol == '\r';
  }
}

bool yalovsky::isOperator(const std::string& token)
{
  return token == "+" || token == "-" || token == "*" || token == "/" || token == "%"
      || token == "|";
}

int yalovsky::getPriority(const std::string& token)
{
  if (token == "|")
  {
    return bitOrPriority;
  }
  if (token == "+" || token == "-")
  {
    return addSubPriority;
  }
  if (token == "*" || token == "/" || token == "%")
  {
    return multDivModPriority;
  }
  throw std::invalid_argument("Input error");
}

yalovsky::lli_t yalovsky::add(const lli_t& lhs, const lli_t& rhs)
{
  const lli_t maxValue = std::numeric_limits< lli_t >::max();
  const lli_t minValue = std::numeric_limits< lli_t >::min();
  if ((rhs > 0 && lhs > maxValue - rhs) || (rhs < 0 && lhs < minValue - rhs))
  {
    throw std::overflow_error("Add overflow");
  }
  return lhs + rhs;
}

yalovsky::lli_t yalovsky::sub(const lli_t& lhs, const lli_t& rhs)
{
  const lli_t maxValue = std::numeric_limits< lli_t >::max();
  const lli_t minValue = std::numeric_limits< lli_t >::min();
  if ((rhs > 0 && lhs < minValue + rhs) || (rhs < 0 && lhs > maxValue + rhs))
  {
    throw std::overflow_error("Sub overflow");
  }
  return lhs - rhs;
}

yalovsky::lli_t yalovsky::mult(const lli_t& lhs, const lli_t& rhs)
{
  const lli_t maxValue = std::numeric_limits< lli_t >::max();
  const lli_t minValue = std::numeric_limits< lli_t >::min();

  if (lhs == 0 || rhs == 0)
  {
    return 0;
  }
  if (lhs == -1)
  {
    if (rhs == minValue)
    {
      throw std::overflow_error("Mult overflow");
    }
    return -rhs;
  }
  if (rhs == -1)
  {
    if (lhs == minValue)
    {
      throw std::overflow_error("Mult overflow");
    }
    return -lhs;
  }
  if (lhs > 0)
  {
    if ((rhs > 0 && lhs > maxValue / rhs) || (rhs < 0 && rhs < minValue / lhs))
    {
      throw std::overflow_error("Mult overflow");
    }
  }
  else
  {
    if ((rhs > 0 && lhs < minValue / rhs) || (rhs < 0 && lhs < maxValue / rhs))
    {
      throw std::overflow_error("Mult overflow");
    }
  }
  return lhs * rhs;
}