#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <stdexcept>
#include <vector>
#include <string>
#include <map>
#include <set>

namespace CustomDS {

// ==========================================
// 1. TEMPLATE LINKED LIST (Singly Linked List)
// ==========================================
template <typename T>
class LinkedList {
public:
    struct Node {
        T data;
        Node* next;
        Node(const T& val) : data(val), next(nullptr) {}
    };

private:
    Node* head;
    Node* tail;
    int sz;

public:
    LinkedList() : head(nullptr), tail(nullptr), sz(0) {}
    
    ~LinkedList() {
        clear();
    }

    void clear() {
        Node* curr = head;
        while (curr) {
            Node* temp = curr;
            curr = curr->next;
            delete temp;
        }
        head = tail = nullptr;
        sz = 0;
    }

    void append(const T& val) {
        Node* newNode = new Node(val);
        if (!head) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        sz++;
    }

    void prepend(const T& val) {
        Node* newNode = new Node(val);
        if (!head) {
            head = tail = newNode;
        } else {
            newNode->next = head;
            head = newNode;
        }
        sz++;
    }

    bool remove(const T& val) {
        Node* curr = head;
        Node* prev = nullptr;
        while (curr) {
            if (curr->data == val) {
                if (prev) {
                    prev->next = curr->next;
                    if (curr == tail) {
                        tail = prev;
                    }
                } else {
                    head = curr->next;
                    if (!head) tail = nullptr;
                }
                delete curr;
                sz--;
                return true;
            }
            prev = curr;
            curr = curr->next;
        }
        return false;
    }

    int size() const { return sz; }
    bool isEmpty() const { return sz == 0; }

    Node* getHead() const { return head; }
};

// ==========================================
// 2. TEMPLATE QUEUE (FIFO Queue)
// ==========================================
template <typename T>
class Queue {
private:
    typename LinkedList<T>::Node* head;
    typename LinkedList<T>::Node* tail;
    int sz;

public:
    Queue() : head(nullptr), tail(nullptr), sz(0) {}
    
    ~Queue() {
        clear();
    }

    void clear() {
        auto* curr = head;
        while (curr) {
            auto* temp = curr;
            curr = curr->next;
            delete temp;
        }
        head = tail = nullptr;
        sz = 0;
    }

    void enqueue(const T& val) {
        auto* newNode = new typename LinkedList<T>::Node(val);
        if (!head) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        sz++;
    }

    T dequeue() {
        if (isEmpty()) {
            throw std::underflow_error("Queue is empty!");
        }
        auto* temp = head;
        T val = temp->data;
        head = head->next;
        if (!head) {
            tail = nullptr;
        }
        delete temp;
        sz--;
        return val;
    }

    T peek() const {
        if (isEmpty()) {
            throw std::underflow_error("Queue is empty!");
        }
        return head->data;
    }

    int size() const { return sz; }
    bool isEmpty() const { return sz == 0; }
};

// ==========================================
// 3. TEMPLATE STACK (LIFO Stack)
// ==========================================
template <typename T>
class Stack {
private:
    typename LinkedList<T>::Node* head;
    int sz;

public:
    Stack() : head(nullptr), sz(0) {}
    
    ~Stack() {
        clear();
    }

    void clear() {
        auto* curr = head;
        while (curr) {
            auto* temp = curr;
            curr = curr->next;
            delete temp;
        }
        head = nullptr;
        sz = 0;
    }

    void push(const T& val) {
        auto* newNode = new typename LinkedList<T>::Node(val);
        newNode->next = head;
        head = newNode;
        sz++;
    }

    T pop() {
        if (isEmpty()) {
            throw std::underflow_error("Stack is empty!");
        }
        auto* temp = head;
        T val = temp->data;
        head = head->next;
        delete temp;
        sz--;
        return val;
    }

    T peek() const {
        if (isEmpty()) {
            throw std::underflow_error("Stack is empty!");
        }
        return head->data;
    }

    int size() const { return sz; }
    bool isEmpty() const { return sz == 0; }
};

// ==========================================
// 4. TEMPLATE MULTI-WAY TREE (For Virtual FS)
// ==========================================
template <typename T>
class TreeNode {
public:
    T data;
    TreeNode* parent;
    std::vector<TreeNode*> children;

    TreeNode(const T& val, TreeNode* par = nullptr) : data(val), parent(par) {}

    ~TreeNode() {
        for (auto* child : children) {
            delete child;
        }
    }

    void addChild(TreeNode* child) {
        child->parent = this;
        children.push_back(child);
    }

    bool removeChild(TreeNode* child) {
        for (auto it = children.begin(); it != children.end(); ++it) {
            if (*it == child) {
                children.erase(it);
                return true;
            }
        }
        return false;
    }
};

// ==========================================
// 5. RESOURCE ALLOCATION GRAPH (For Deadlock)
// ==========================================
class ResourceAllocationGraph {
public:
    struct Edge {
        std::string from;
        std::string to;
        bool isRequest; // true: Process requests Resource, false: Resource allocated to Process
    };

private:
    std::set<std::string> nodes;
    std::vector<Edge> edges;

    // Helper for DFS cycle detection
    bool dfs(const std::string& u, std::map<std::string, int>& visited, std::vector<std::string>& cycle) const {
        visited[u] = 1; // Visiting
        cycle.push_back(u);

        for (const auto& edge : edges) {
            if (edge.from == u) {
                std::string v = edge.to;
                if (visited[v] == 1) { // Cycle detected
                    cycle.push_back(v);
                    return true;
                } else if (visited[v] == 0) {
                    if (dfs(v, visited, cycle)) {
                        return true;
                    }
                }
            }
        }

        visited[u] = 2; // Visited
        cycle.pop_back();
        return false;
    }

public:
    void clear() {
        nodes.clear();
        edges.clear();
    }

    void addNode(const std::string& name) {
        nodes.insert(name);
    }

    void removeNode(const std::string& name) {
        nodes.erase(name);
        // Remove associated edges
        for (auto it = edges.begin(); it != edges.end(); ) {
            if (it->from == name || it->to == name) {
                it = edges.erase(it);
            } else {
                ++it;
            }
        }
    }

    void addEdge(const std::string& from, const std::string& to, bool isRequest) {
        addNode(from);
        addNode(to);
        edges.push_back({from, to, isRequest});
    }

    void removeEdge(const std::string& from, const std::string& to) {
        for (auto it = edges.begin(); it != edges.end(); ++it) {
            if (it->from == from && it->to == to) {
                edges.erase(it);
                break;
            }
        }
    }

    const std::set<std::string>& getNodes() const { return nodes; }
    const std::vector<Edge>& getEdges() const { return edges; }

    // Cycle detection returns list of nodes in a detected deadlock cycle (empty if safe)
    std::vector<std::string> detectCycle() const {
        std::map<std::string, int> visited; // 0 = unvisited, 1 = visiting, 2 = visited
        for (const auto& node : nodes) {
            visited[node] = 0;
        }

        for (const auto& node : nodes) {
            if (visited[node] == 0) {
                std::vector<std::string> cycle;
                if (dfs(node, visited, cycle)) {
                    // Extract exact cycle path
                    std::vector<std::string> exactCycle;
                    std::string last = cycle.back();
                    bool capture = false;
                    for (const auto& n : cycle) {
                        if (n == last) capture = true;
                        if (capture) exactCycle.push_back(n);
                    }
                    return exactCycle;
                }
            }
        }
        return {};
    }
};

} // namespace CustomDS

#endif // DATA_STRUCTURES_H
