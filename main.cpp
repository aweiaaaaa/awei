#include <iostream>
#include <unordered_map>
#include <set>
#include <mutex>
#include <thread>
#include <vector>
#include <chrono>

// 并查集结构
class UnionFind {
public:
    UnionFind() {}

    int find(int x) {
        if (parent.find(x) == parent.end()) parent[x] = x;
        if (parent[x] != x) parent[x] = find(parent[x]); // 路径压缩
        return parent[x];
    }

    void unionSets(int x, int y) {
        int rootX = find(x);
        int rootY = find(y);
        if (rootX != rootY) {
            parent[rootX] = rootY;
        }
    }

    void displaySets() {
        std::unordered_map<int, std::set<int>> sets;
        for (const auto& p : parent) {
            sets[find(p.first)].insert(p.first);
        }
        
        // 找出所有的父节点
        std::set<int> parents;
        for (const auto& s : sets) {
            parents.insert(s.first);
        }
        
        // 遍历每个子集，找到其父节点并打印出来
        for (const auto& childSet : sets) {
            int childRoot = childSet.first;
            int parentRoot = -1;
            
            // 查找当前子集的父节点
            for (const auto& p : parents) {
                if (p != childRoot && sets[p].count(childRoot)) {
                    parentRoot = p;
                    break;
                }
            }
            
            // 如果找到了父节点，则打印父子关系
            if (parentRoot != -1) {
                std::cout << "{";
                for (auto it = childSet.second.begin(); it != childSet.second.end(); ++it) {
                    std::cout << *it;
                    if (std::next(it) != childSet.second.end()) std::cout << ",";
                }
                std::cout << "}--->{";
                for (auto it = sets[parentRoot].begin(); it != sets[parentRoot].end(); ++it) {
                    std::cout << *it;
                    if (std::next(it) != sets[parentRoot].end()) std::cout << ",";
                }
                std::cout << "}" << std::endl;
            } else {
                // 如果没有找到父节点，则表示这是顶层的集合
                std::cout << "{";
                for (auto it = childSet.second.begin(); it != childSet.second.end(); ++it) {
                    std::cout << *it;
                    if (std::next(it) != childSet.second.end()) std::cout << ",";
                }
                std::cout << "}" << std::endl;
            }
        }
    }

private:
    std::unordered_map<int, int> parent;
};



// 红黑树管理器
class CollectionManagerRBTree {
public:
    CollectionManagerRBTree() : unionFind() {}

    void addSet(const std::vector<int>& newSet) {
        std::lock_guard<std::mutex> lock(mu);
        int firstNonDuplicate = -1;
        for (int elem : newSet) {
            if (rbTree.find(elem) != rbTree.end()) {
                std::cout << "Element " << elem << " 已存在，跳过插入." << std::endl;
                continue;
            }
            if (firstNonDuplicate == -1) {
                firstNonDuplicate = elem;
            }
            rbTree.insert(elem);
            unionFind.unionSets(firstNonDuplicate, elem);
        }
    }

    void display() {
        std::cout << "RBTree 元素: ";
        for (auto val : rbTree) {
            std::cout << val << " ";
        }
        std::cout << "\n并查集的父子关系：" << std::endl;
        unionFind.displaySets();
    }

private:
    UnionFind unionFind;
    std::set<int> rbTree;
    std::mutex mu;
};



// BDD管理器
class CollectionManagerBDD {
public:
    CollectionManagerBDD() : unionFind() {}

    void addSet(const std::vector<int>& newSet) {
        std::lock_guard<std::mutex> lock(mu);
        int firstNonDuplicate = -1;
        for (int elem : newSet) {
            if (bddData.find(elem) != bddData.end()) {
                std::cout << "Element " << elem << " 已存在，跳过插入." << std::endl;
                continue;
            }
            if (firstNonDuplicate == -1) {
                firstNonDuplicate = elem;
            }
            bddData[elem] = true;
            unionFind.unionSets(firstNonDuplicate, elem);
        }
    }

    void display() {
        std::cout << "BDD 元素: ";
        for (const auto& pair : bddData) {
            std::cout << "[" << pair.first << ": " << pair.second << "] ";
        }
        std::cout << "\n并查集的父子关系：" << std::endl;
        unionFind.displaySets();
    }

private:
    UnionFind unionFind;
    std::unordered_map<int, bool> bddData;
    std::mutex mu;
};



// 基准测试
template <typename T>
void benchmark(T& manager, const std::vector<std::vector<int>>& sets, int numThreads, const std::string& label) {
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> threads;
    int chunkSize = sets.size() / numThreads;

    for (int i = 0; i < numThreads; ++i) {
        int startIdx = i * chunkSize;
        int endIdx = (i == numThreads - 1) ? sets.size() : (i + 1) * chunkSize;
        threads.emplace_back([&manager, &sets, startIdx, endIdx] {
            for (int j = startIdx; j < endIdx; ++j) {
                manager.addSet(sets[j]);
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << label << " 时间: " << duration.count() << " 秒" << std::endl;
}

int main() {
    CollectionManagerRBTree managerRBTree;
    CollectionManagerBDD managerBDD;

    std::vector<std::vector<int>> sets = {
        {1, 2, 4, 6, 8, 9},
        {1, 2, 4, 6, 8},
        {1, 2, 6, 8},
        {1, 2},
        {1, 2, 4, 6, 9}
    };

    int numThreads = 4;

    // 基准测试 Union-Find + RBTree
    benchmark(managerRBTree, sets, numThreads, "Union-Find + RBTree");

    // 基准测试 Union-Find + BDD
    benchmark(managerBDD, sets, numThreads, "Union-Find + BDD");

    // 显示结果
    std::cout << "\nUnion-Find + RBTree 结果:" << std::endl;
    managerRBTree.display();

    std::cout << "\nUnion-Find + BDD 结果:" << std::endl;
    managerBDD.display();

    return 0;
}