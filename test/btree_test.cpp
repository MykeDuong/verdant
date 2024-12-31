#include "btree.h"
#include "parameters.h"

#include <climits>
#include <unordered_map>
#include <random>

size_t BLOCK_SIZE = 512;
size_t ITERATION_COUNT = 200000;
size_t SEARCH_TEST_DURATION = 10000;
size_t RANGE_TEST_DURATION = 10000;
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
    T randomValue = uni(rng);
    btree.insert(randomValue);
    comparativeStructure[randomValue] = round;
    assert(btree.validate());
    if ((round) % SEARCH_TEST_DURATION == 0) {
      for (auto val: comparativeStructure) {
        if (btree.search(val.first).error != 0) {
          std::cout << "[ERROR] Value " << val.first << " (added in interation " << val.second << ") not found" << std::endl;
          exit(1);
        }
      }
      std::cout << "[DEBUG] Comparative data structure compared successfully" << std::endl;
    }
    if (round % RANGE_TEST_DURATION == 0) {
      auto result = btree.searchRange(btree.getMinValue().unwrap(), btree.getMaxValue().unwrap()).unwrap();
      assert(result.size() == comparativeStructure.size());
      for (size_t i = 0; i < result.size() - 1; i++) {
        assert(result[i] < result[i + 1]);
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
