#include <iostream>
#include <sstream>
#include <string>

int main()
{
  std::string line;

  while (std::getline(std::cin, line))
  {
    std::istringstream input(line);
    std::string command;
    if (!(input >> command))
    {
      continue;
    }

    std::string extra;
    if (input >> extra)
    {
      std::cout << "<INVALID COMMAND>\n";
    }
    else if (command == "help")
    {
      std::cout << "help\nexit\n";
    }
    else if (command == "exit")
    {
      return 0;
    }
    else
    {
      std::cout << "<INVALID COMMAND>\n";
    }
  }

  if (std::cin.bad() || !std::cout)
  {
    std::cerr << "Input/output error\n";
    return 2;
  }
  return 0;
}