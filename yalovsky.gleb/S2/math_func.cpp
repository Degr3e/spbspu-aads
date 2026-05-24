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