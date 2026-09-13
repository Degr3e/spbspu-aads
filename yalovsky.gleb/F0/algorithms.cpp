#include "algorithms.hpp"
#include <limits>
#include <stdexcept>
#include <utility>
#include "avl-map.hpp"

namespace yalovsky
{
  namespace detail
  {
    struct PathState
    {
      Distance distance;
      std::string previous;
      bool reachable;
      bool visited;
      bool overflow;
    };

    using DistanceTable = AvlMap< std::string, PathState >;

    DistanceTable findDistances(const RoadGraph& graph, const std::string& from);
  }
}

yalovsky::detail::DistanceTable
yalovsky::detail::findDistances(const RoadGraph& graph, const std::string& from)
{
  if (!graph.hasCity(from))
  {
    throw std::out_of_range("City not found");
  }

  DistanceTable states;
  graph.getCities().forEach([&states](const std::string& city, const RoadGraph::Roads&)
  {
    states.insert(city, PathState{0, "", false, false, false});
  });
  states.at(from).reachable = true;

  const Distance maximum = std::numeric_limits< Distance >::max();

  while (true)
  {
    const std::string* currentName = nullptr;
    PathState* current = nullptr;

    states.forEach([&currentName, &current](const std::string& city, PathState& state)
    {
      if (!state.reachable || state.visited)
      {
        return;
      }

      const bool finiteIsBetter = current && current->overflow && !state.overflow;
      const bool shorterIsBetter = current && !current->overflow && !state.overflow
          && state.distance < current->distance;

      if (!current || finiteIsBetter || shorterIsBetter)
      {
        currentName = &city;
        current = &state;
      }
    });

    if (!current)
    {
      break;
    }

    current->visited = true;
    graph.getRoads(*currentName).forEach(
        [&states, current, currentName](const std::string& next, const Distance& weight)
        {
          PathState& destination = states.at(next);
          if (destination.visited)
          {
            return;
          }

          const bool overflow = current->overflow || weight > maximum - current->distance;
          const Distance candidate = overflow ? 0 : current->distance + weight;
          const bool finiteIsBetter = destination.overflow && !overflow;
          const bool shorterIsBetter = !destination.overflow && !overflow
              && candidate < destination.distance;

          if (!destination.reachable || finiteIsBetter || shorterIsBetter)
          {
            destination.previous = *currentName;
            destination.distance = candidate;
            destination.reachable = true;
            destination.overflow = overflow;
          }
        });
  }

  return states;
}

yalovsky::PathResult yalovsky::findShortestPath(const RoadGraph& graph,
    const std::string& from, const std::string& to)
{
  if (!graph.hasCity(to))
  {
    throw std::out_of_range("City not found");
  }

  const detail::DistanceTable states = detail::findDistances(graph, from);
  const detail::PathState& destination = states.at(to);

  if (!destination.reachable)
  {
    throw std::logic_error("Path not found");
  }
  if (destination.overflow)
  {
    throw std::overflow_error("Path distance is too large");
  }

  PathResult result{
    std::unique_ptr< std::string[] >(new std::string[graph.getCities().size()]),
    0,
    destination.distance
  };

  std::string current = to;
  while (true)
  {
    result.cities[result.size] = current;
    ++result.size;

    if (current == from)
    {
      break;
    }
    current = states.at(current).previous;
  }

  for (std::size_t index = 0; index < result.size / 2; ++index)
  {
    std::swap(result.cities[index], result.cities[result.size - index - 1]);
  }

  return result;
}

yalovsky::RoadSuggestion yalovsky::suggestRoad(const RoadGraph& graph, Distance weight)
{
  if (weight == 0)
  {
    throw std::invalid_argument("Invalid road weight");
  }

  RoadSuggestion best{"", "", weight, 0};

  graph.getCities().forEach([&graph, &best, weight](const std::string& from,
      const RoadGraph::Roads&)
  {
    const detail::DistanceTable states = detail::findDistances(graph, from);

    states.forEach([&graph, &best, &from, weight](const std::string& to,
        const detail::PathState& state)
    {
      if (!(from < to) || graph.hasRoad(from, to) || !state.reachable)
      {
        return;
      }
      if (state.overflow)
      {
        throw std::overflow_error("Cannot calculate improvement");
      }
      if (state.distance <= weight)
      {
        return;
      }

      const Distance improvement = state.distance - weight;
      if (improvement > best.improvement)
      {
        best.from = from;
        best.to = to;
        best.improvement = improvement;
      }
    });
  });

  if (best.improvement == 0)
  {
    throw std::logic_error("No improving road");
  }

  return best;
}
