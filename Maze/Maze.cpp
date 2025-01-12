#include <SFML/Graphics.hpp>
#include <queue>
#include <vector>
#include <limits>
#include <unistd.h>

// Constants for visualization
const int CELL_SIZE = 50;        // Size of each cell in pixels
const int GRID_SIZE = 10;        // 10x10 grid
const int DELAY_MS = 200000;     // Animation delay in microseconds (0.2 seconds)

// Different states a cell can be in
enum CellState {
    EMPTY,          // Unvisited cell
    START,          // Start point
    END,           // End point
    EXPLORING,     // Currently being explored
    IN_QUEUE,      // In priority queue
    VISITED,       // Already explored
    CURRENT_PATH   // Part of current path being shown
};

// Structure to represent each cell in the grid
struct Cell {
    int x, y;                   // Cell coordinates
    float distance;             // Distance from start
    Cell* previous;             // Previous cell in path
    CellState state;           // Current state of cell

    // Constructor
    Cell(int x = 0, int y = 0) :
        x(x),
        y(y),
        distance(std::numeric_limits<float>::infinity()),
        previous(nullptr),
        state(EMPTY) {}
};

class PathFinder {
private:
    sf::RenderWindow window;                    // SFML window
    std::vector<std::vector<Cell>> grid;        // 2D grid of cells
    Cell* startCell;                            // Pointer to start cell
    Cell* endCell;                              // Pointer to end cell

    // Priority queue comparison function
    struct CompareDistance {
        bool operator()(const Cell* a, const Cell* b) {
            return a->distance > b->distance;
        }
    };

    // Movement directions (right, left, down, up, diagonals)
    const std::vector<std::pair<int, int>> directions = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1},
        {1, 1}, {-1, 1}, {1, -1}, {-1, -1}
    };

    // Get color for different cell states
    sf::Color getStateColor(CellState state) {
        switch (state) {
        case START: return sf::Color::Green;
        case END: return sf::Color::Red;
        case EXPLORING: return sf::Color(255, 165, 0);    // Orange
        case IN_QUEUE: return sf::Color(144, 238, 144);   // Light green
        case VISITED: return sf::Color(200, 200, 255);    // Light blue
        case CURRENT_PATH: return sf::Color::Yellow;
        default: return sf::Color::White;
        }
    }

    // Draw the current state of the grid
    void drawGrid() {
        window.clear(sf::Color::White);

        // Create shape for cells
        sf::RectangleShape cellShape(sf::Vector2f(CELL_SIZE - 2, CELL_SIZE - 2));

        // Draw each cell
        for (int y = 0; y < GRID_SIZE; y++) {
            for (int x = 0; x < GRID_SIZE; x++) {
                cellShape.setPosition(x * CELL_SIZE + 1, y * CELL_SIZE + 1);
                cellShape.setFillColor(getStateColor(grid[y][x].state));
                window.draw(cellShape);
            }
        }

        window.display();
        usleep(DELAY_MS); // Add delay for animation
    }

    // Check if coordinates are valid
    bool isValid(int x, int y) {
        return x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE;
    }

    // Visualize current path from start to given cell
    void showPath(Cell* current) {
        // Reset previous path visualization
        for (auto& row : grid) {
            for (auto& cell : row) {
                if (cell.state == CURRENT_PATH) {
                    cell.state = VISITED;
                }
            }
        }

        // Highlight new path
        Cell* pathCell = current;
        while (pathCell != startCell && pathCell != nullptr) {
            if (pathCell != endCell && pathCell->state != START) {
                pathCell->state = CURRENT_PATH;
            }
            pathCell = pathCell->previous;
        }

        drawGrid();
    }

public:
    PathFinder() :
        window(sf::VideoMode(GRID_SIZE* CELL_SIZE, GRID_SIZE* CELL_SIZE),
            "Dijkstra's Algorithm - Path Visualization") {
        // Initialize grid
        grid.resize(GRID_SIZE, std::vector<Cell>(GRID_SIZE));
        for (int y = 0; y < GRID_SIZE; y++) {
            for (int x = 0; x < GRID_SIZE; x++) {
                grid[y][x] = Cell(x, y);
            }
        }

        // Set start and end points
        startCell = &grid[0][0];
        startCell->state = START;
        startCell->distance = 0;

        endCell = &grid[GRID_SIZE - 1][GRID_SIZE - 1];
        endCell->state = END;

        drawGrid();
    }

    void findPath() {
        // Create priority queue for Dijkstra's algorithm
        std::priority_queue<Cell*, std::vector<Cell*>, CompareDistance> pq;

        // Start from the beginning
        pq.push(startCell);

        while (!pq.empty() && window.isOpen()) {
            // Handle window events
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
            }

            // Get cell with smallest distance
            Cell* current = pq.top();
            pq.pop();

            // Skip if already visited
            if (current->state == VISITED) continue;

            // Mark current cell as exploring
            if (current != startCell && current != endCell) {
                current->state = EXPLORING;
                drawGrid();
            }

            // Show current path being considered
            showPath(current);

            // Found the end!
            if (current == endCell) {
                showPath(current);  // Show final path
                break;
            }

            // Check all neighboring cells (including diagonals)
            for (const auto& dir : directions) {
                int newX = current->x + dir.first;
                int newY = current->y + dir.second;

                if (isValid(newX, newY)) {
                    // Calculate new distance (1.4 for diagonal, 1 for orthogonal)
                    float stepCost = (dir.first != 0 && dir.second != 0) ? 1.4f : 1.0f;
                    float newDist = current->distance + stepCost;

                    // If found shorter path, update it
                    if (newDist < grid[newY][newX].distance) {
                        grid[newY][newX].distance = newDist;
                        grid[newY][newX].previous = current;

                        // Mark as in queue
                        if (&grid[newY][newX] != endCell &&
                            grid[newY][newX].state != VISITED) {
                            grid[newY][newX].state = IN_QUEUE;
                        }

                        pq.push(&grid[newY][newX]);
                        drawGrid();
                    }
                }
            }

            // Mark current cell as visited
            if (current != startCell && current != endCell) {
                current->state = VISITED;
                drawGrid();
            }
        }

        // Keep window open until closed
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
            }
        }
    }
};

int main() {
    PathFinder finder;
    finder.findPath();
    return 0;
}