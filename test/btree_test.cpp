#include "btree.hpp"

#include <climits>
#include <unordered_map>
#include <random>

size_t ITERATION_COUNT = 200000;
size_t SEARCH_TEST_DURATION = 10000;
size_t RANGE_TEST_DURATION = 10000;
size_t DELETION_TEST_DURATION = 1000;
size_t DELETIONS_PER_ROUND = 99;
typedef long long T;

int main() {
#ifdef VERDANT_FLAG_DEBUG
  std::cout << "[DEBUG] Debug enabled" << std::endl;
#endif
  BTree<T> btree;

  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<T> uni(0, INT_MAX);
  std::unordered_map<T, size_t> comparativeStructure;
  std::cout << "[DEBUG] Start fuzzing" << std::endl;

  for (size_t i = 0; i < ITERATION_COUNT; i++) {
    size_t round = i + 1;
    if (round % DELETION_TEST_DURATION == 0) {
      for (size_t i = 0; i < DELETIONS_PER_ROUND; i++) {
        auto item = *comparativeStructure.begin();
        comparativeStructure.erase(item.first);
        btree.remove(item.first);
        assert(btree.validate());
      }
    } else {
      T randomValue = uni(rng);
      btree.insert(randomValue);
      comparativeStructure[randomValue] = round;
    }
    assert(btree.validate());
    if (round % SEARCH_TEST_DURATION == 0) {
      for (auto val: comparativeStructure) {
        if (!btree.search(val.first).unwrappable()) {
          std::cout << "[ERROR] Value " << val.first << " (added in interation " << val.second << ") not found" << std::endl;
          exit(1);
        }
      }
      std::cout << "[DEBUG] Comparative data structure compared successfully" << std::endl;
    }
    if (round % RANGE_TEST_DURATION == 0) {
      auto minVal = btree.getMinValue();
      auto maxVal = btree.getMaxValue();
      if (minVal.unwrappable() && maxVal.unwrappable()) {
        auto result = btree.searchRange(btree.getMinValue().unwrap(), btree.getMaxValue().unwrap()).unwrap();
        assert(result.size() == comparativeStructure.size());
        for (size_t i = 0; i < result.size() - 1; i++) {
          assert(result[i] < result[i + 1]);
        }
      }
      std::cout << "[DEBUG] Range queried successfully" << std::endl;
    }
    std::cout << "[DEBUG] Finished round " << round << std::endl;
  }

  std::cout << "[DEBUG] End fuzzing" << std::endl;
  std::cout << "[DEBUG] Tree height: " << btree.getHeight() << std::endl;
  std::cout << "[DEBUG] Average keys per node: " << btree.getAverageKeyPerNode() << std::endl;
  std::cout << "[DEBUG] Number of nodes: " << btree.countNodes() << std::endl;

  return 0;
}
