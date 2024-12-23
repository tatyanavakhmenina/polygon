#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <unordered_map>
#include <queue>
#include <stack>
#include <set>
#include <limits>

// Определение структуры узла графа
struct Node {
    double lon, lat; // Долгота и широта узла
    std::vector<std::pair<Node*, double>> neighbors; // Список соседей и расстояний до них
};

// Вычисление географического расстояния (Haversine formula)
double haversine(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371.0; // Радиус Земли в километрах
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;
    lat1 = lat1 * M_PI / 180.0;
    lat2 = lat2 * M_PI / 180.0;

    double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
               std::sin(dLon / 2) * std::sin(dLon / 2) * std::cos(lat1) * std::cos(lat2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    return R * c;
}

// Определение структуры графа
struct Graph {
    std::unordered_map<std::string, Node*> nodes; // Хранилище узлов по ключу "долгота,широта"

    // Получить узел по координатам или создать его, если узел отсутствует
    Node* get_or_create_node(double lon, double lat) {
        std::string key = std::to_string(lon) + "," + std::to_string(lat);
        if (nodes.find(key) == nodes.end()) {
            nodes[key] = new Node{lon, lat};
        }
        return nodes[key];
    }

    // Добавить ребро между двумя узлами с заданным расстоянием
    void add_edge(double lon1, double lat1, double lon2, double lat2, double distance) {
        Node* node1 = get_or_create_node(lon1, lat1);
        Node* node2 = get_or_create_node(lon2, lat2);
        node1->neighbors.push_back({node2, distance});
    }

    // Найти ближайший узел к заданным координатам
    Node* find_closest_node(double lat, double lon) {
        double min_distance = std::numeric_limits<double>::max();
        Node* closest_node = nullptr;

        for (auto& pair : nodes) {
            Node* node = pair.second;
            double distance = haversine(lat, lon, node->lat, node->lon);
            if (distance < min_distance) {
                min_distance = distance;
                closest_node = node;
            }
        }

        return closest_node;
    }

    // Обход в ширину (BFS)
    void bfs(Node* start) {
        std::queue<Node*> q;
        std::set<Node*> visited;

        q.push(start);
        visited.insert(start);

        while (!q.empty()) {
            Node* current = q.front();
            q.pop();

            std::cout << "Посещен узел: (" << current->lat << ", " << current->lon << ")\n";

            for (auto neighbor : current->neighbors) {
                if (visited.find(neighbor.first) == visited.end()) {
                    q.push(neighbor.first);
                    visited.insert(neighbor.first);
                }
            }
        }
    }

    // Обход в глубину (DFS)
    void dfs(Node* start) {
        std::stack<Node*> s;
        std::set<Node*> visited;

        s.push(start);

        while (!s.empty()) {
            Node* current = s.top();
            s.pop();

            if (visited.find(current) == visited.end()) {
                std::cout << "Посещен узел: (" << current->lat << ", " << current->lon << ")\n";
                visited.insert(current);

                for (auto neighbor : current->neighbors) {
                    s.push(neighbor.first);
                }
            }
        }
    }

    // Алгоритм Дейкстры
    void dijkstra(Node* start) {
        std::unordered_map<Node*, double> distances;
        std::set<std::pair<double, Node*>> pq;

        for (auto& pair : nodes) {
            distances[pair.second] = std::numeric_limits<double>::max();
        }
        distances[start] = 0;
        pq.insert({0, start});

        while (!pq.empty()) {
            Node* current = pq.begin()->second;
            pq.erase(pq.begin());

            for (auto neighbor : current->neighbors) {
                double new_distance = distances[current] + neighbor.second;

                if (new_distance < distances[neighbor.first]) {
                    pq.erase({distances[neighbor.first], neighbor.first});
                    distances[neighbor.first] = new_distance;
                    pq.insert({new_distance, neighbor.first});
                }
            }
        }

        for (auto& pair : distances) {
            std::cout << "Расстояние до узла (" << pair.first->lat << ", " << pair.first->lon << ") = " << pair.second << " км\n";
        }
    }

    // Алгоритм A*
    void a_star(Node* start, Node* goal) {
        std::unordered_map<Node*, double> g_cost;
        std::unordered_map<Node*, double> f_cost;
        std::set<std::pair<double, Node*>> pq;

        for (auto& pair : nodes) {
            g_cost[pair.second] = std::numeric_limits<double>::max();
            f_cost[pair.second] = std::numeric_limits<double>::max();
        }

        g_cost[start] = 0;
        f_cost[start] = haversine(start->lat, start->lon, goal->lat, goal->lon);
        pq.insert({f_cost[start], start});

        while (!pq.empty()) {
            Node* current = pq.begin()->second;
            pq.erase(pq.begin());

            if (current == goal) {
                std::cout << "Цель достигнута: (" << goal->lat << ", " << goal->lon << ")\n";
                return;
            }

            for (auto neighbor : current->neighbors) {
                double tentative_g_cost = g_cost[current] + neighbor.second;

                if (tentative_g_cost < g_cost[neighbor.first]) {
                    pq.erase({f_cost[neighbor.first], neighbor.first});
                    g_cost[neighbor.first] = tentative_g_cost;
                    f_cost[neighbor.first] = g_cost[neighbor.first] + haversine(neighbor.first->lat, neighbor.first->lon, goal->lat, goal->lon);
                    pq.insert({f_cost[neighbor.first], neighbor.first});
                }
            }
        }

        std::cout << "Цель недостижима!\n";
    }
};

int main() {
    Graph graph;

    // Чтение файла spb_graph.txt
    std::ifstream file("spb_graph.txt");
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string from, to_part;

        // Разделяем строку на узел "откуда" и его соседей
        std::getline(iss, from, ':');
        double lon1, lat1;
        sscanf(from.c_str(), "%lf,%lf", &lon1, &lat1);

        // Обрабатываем всех соседей узла "откуда"
        while (std::getline(iss, to_part, ';')) {
            if (to_part.empty()) continue;
            double lon2, lat2, distance;
            sscanf(to_part.c_str(), "%lf,%lf,%lf", &lon2, &lat2, &distance);
            graph.add_edge(lon1, lat1, lon2, lat2, distance);
        }
    }

    // Координаты точки А и Б
    double lonA = 30.308108, latA = 59.957238;
    double lonB = 30.443688, latB = 59.914686;

    // Поиск ближайших узлов для точки А
    Node* closest_to_A = graph.find_closest_node(latA, lonA);

    // Поиск ближайших узлов для точки Б
    Node* closest_to_B = graph.find_closest_node(latB, lonB);

    // Вывод ближайшего узла для точки А
    if (closest_to_A) {
        std::cout << "Ближайший узел к точке А: (" << closest_to_A->lat << ", " << closest_to_A->lon << ")\n";
    } else {
        std::cout << "Не найден узел для точки А!\n";
    }

    // Вывод ближайшего узла для точки Б
    if (closest_to_B) {
        std::cout << "Ближайший узел к точке Б: (" << closest_to_B->lat << ", " << closest_to_B->lon << ")\n";
    } else {
        std::cout << "Не найден узел для точки Б!\n";
    }

    // Выполнение алгоритмов
    if (closest_to_A) {
        std::cout << "\nОбход в ширину (BFS) от точки А:\n";
        graph.bfs(closest_to_A);

        std::cout << "\nОбход в глубину (DFS) от точки А:\n";
        graph.dfs(closest_to_A);

        std::cout << "\nКратчайшие пути от точки А (Дейкстра):\n";
        graph.dijkstra(closest_to_A);

        if (closest_to_B) {
            std::cout << "\nАлгоритм A* от точки А до точки Б:\n";
            graph.a_star(closest_to_A, closest_to_B);
        }
    }

    return 0;
}
