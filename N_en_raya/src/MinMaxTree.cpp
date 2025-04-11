#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <cmath>
#include <limits>
#include <sstream>
#include <iomanip>
#include <algorithm>

enum class Jugador { X, O, VACIO };

struct Nodo {
    std::vector<Jugador> tablero;
    int movimiento;
    int nivel;
    int evaluacion;
    std::vector<std::unique_ptr<Nodo>> hijos;
    sf::Vector2f posicion;
    bool expanded;
    Nodo* padre;

    Nodo(const std::vector<Jugador>& tab, int mov, int niv)
        : tablero(tab), movimiento(mov), nivel(niv),
          evaluacion(0), posicion(0.f, 0.f), expanded(false), padre(nullptr) {}
};

int getMaxDepth(Nodo* nodo) {
    int maxD = nodo->nivel;
    for (auto& h : nodo->hijos)
        maxD = std::max(maxD, getMaxDepth(h.get()));
    return maxD;
}

void contarPosibilidades(const std::vector<Jugador>& board, int boardSize, int& posX, int& posO) {
    posX = 0;
    posO = 0;
    // Rows
    for (int i = 0; i < boardSize; i++) {
        bool filaX = true, filaO = true;
        for (int j = 0; j < boardSize; j++) {
            Jugador c = board[i * boardSize + j];
            if (c == Jugador::O) filaX = false;
            if (c == Jugador::X) filaO = false;
        }
        if (filaX) posX++;
        if (filaO) posO++;
    }
    // Columns
    for (int j = 0; j < boardSize; j++) {
        bool colX = true, colO = true;
        for (int i = 0; i < boardSize; i++) {
            Jugador c = board[i * boardSize + j];
            if (c == Jugador::O) colX = false;
            if (c == Jugador::X) colO = false;
        }
        if (colX) posX++;
        if (colO) posO++;
    }
    // Main diagonal
    {
        bool diagX = true, diagO = true;
        for (int i = 0; i < boardSize; i++) {
            Jugador c = board[i * boardSize + i];
            if (c == Jugador::O) diagX = false;
            if (c == Jugador::X) diagO = false;
        }
        if (diagX) posX++;
        if (diagO) posO++;
    }
    // Anti-diagonal
    {
        bool diagX = true, diagO = true;
        for (int i = 0; i < boardSize; i++) {
            Jugador c = board[i * boardSize + (boardSize - 1 - i)];
            if (c == Jugador::O) diagX = false;
            if (c == Jugador::X) diagO = false;
        }
        if (diagX) posX++;
        if (diagO) posO++;
    }
}

bool tableroLleno(const std::vector<Jugador>& board) {
    for (auto c : board)
        if (c == Jugador::VACIO)
            return false;
    return true;
}

Jugador checkGanador(const std::vector<Jugador>& board, int boardSize) {
    // Rows
    for (int i = 0; i < boardSize; i++) {
        Jugador c0 = board[i * boardSize];
        if (c0 == Jugador::VACIO) continue;
        bool linea = true;
        for (int j = 1; j < boardSize; j++) {
            if (board[i * boardSize + j] != c0) { linea = false; break; }
        }
        if (linea) return c0;
    }
    // Columns
    for (int j = 0; j < boardSize; j++) {
        Jugador c0 = board[j];
        if (c0 == Jugador::VACIO) continue;
        bool linea = true;
        for (int i = 1; i < boardSize; i++) {
            if (board[i * boardSize + j] != c0) { linea = false; break; }
        }
        if (linea) return c0;
    }
    // Main diagonal
    {
        Jugador c0 = board[0];
        if (c0 != Jugador::VACIO) {
            bool diag = true;
            for (int i = 1; i < boardSize; i++) {
                if (board[i * boardSize + i] != c0) { diag = false; break; }
            }
            if (diag) return c0;
        }
    }
    // Anti-diagonal
    {
        Jugador c0 = board[boardSize - 1];
        if (c0 != Jugador::VACIO) {
            bool diag = true;
            for (int i = 1; i < boardSize; i++) {
                if (board[i * boardSize + (boardSize - 1 - i)] != c0) { diag = false; break; }
            }
            if (diag) return c0;
        }
    }
    return Jugador::VACIO;
}

void generarHijos(Nodo* nodo, Jugador turno, int boardSize) {
    if (checkGanador(nodo->tablero, boardSize) != Jugador::VACIO || tableroLleno(nodo->tablero))
        return;
    int total = boardSize * boardSize;
    for (int i = 0; i < total; i++) {
        if (nodo->tablero[i] == Jugador::VACIO) {
            std::vector<Jugador> nuevoTab = nodo->tablero;
            nuevoTab[i] = turno;
            auto hijo = std::make_unique<Nodo>(nuevoTab, i, nodo->nivel + 1);
            hijo->padre = nodo;
            nodo->hijos.push_back(std::move(hijo));
        }
    }
}

void construirArbol(Nodo* nodo, int G, int boardSize, Jugador turno) {
    if (nodo->nivel >= G) return;
    generarHijos(nodo, turno, boardSize);
    Jugador siguiente = (turno == Jugador::X) ? Jugador::O : Jugador::X;
    for (auto& hijo : nodo->hijos)
        construirArbol(hijo.get(), G, boardSize, siguiente);
}

void evaluarNodo(Nodo* nodo, int boardSize) {
    int posX = 0, posO = 0;
    contarPosibilidades(nodo->tablero, boardSize, posX, posO);
    nodo->evaluacion = posX - posO;
}

int minimax(Nodo* nodo, int profundidad, int boardSize, bool maximizando) {
    // Terminal condition for minimax:
    if (profundidad == 0 || nodo->hijos.empty()) {
        evaluarNodo(nodo, boardSize);
        return nodo->evaluacion;
    }

    if (maximizando) {
        int mejorVal = std::numeric_limits<int>::min();
        for (auto& hijo : nodo->hijos) {
            int val = minimax(hijo.get(), profundidad - 1, boardSize, false);
            mejorVal = std::max(mejorVal, val);
        }
        nodo->evaluacion = mejorVal;
        return mejorVal;
    } else {
        int peorVal = std::numeric_limits<int>::max();
        for (auto& hijo : nodo->hijos) {
            int val = minimax(hijo.get(), profundidad - 1, boardSize, true);
            peorVal = std::min(peorVal, val);
        }
        nodo->evaluacion = peorVal;
        return peorVal;
    }
}

// Structure to hold the count of outcomes from a subtree
struct WinCounts {
    int xWins;
    int oWins;
    int draws;
};

// Recursively gather how many terminal states in the subtree lead to X-win, O-win, or draw.
WinCounts computeStats(Nodo* nodo, int boardSize) {
    Jugador g = checkGanador(nodo->tablero, boardSize);
    if (g == Jugador::X) {
        return {1, 0, 0};
    } else if (g == Jugador::O) {
        return {0, 1, 0};
    } else if (tableroLleno(nodo->tablero)) {
        // Board is full but no winner => draw
        return {0, 0, 1};
    }

    // If no children, treat it as a terminal => draw (edge case)
    if (nodo->hijos.empty()) {
        return {0, 0, 1};
    }

    WinCounts total{0, 0, 0};
    for (auto& h : nodo->hijos) {
        WinCounts childRes = computeStats(h.get(), boardSize);
        total.xWins += childRes.xWins;
        total.oWins += childRes.oWins;
        total.draws += childRes.draws;
    }
    return total;
}

void collapseSubtree(Nodo* nodo) {
    nodo->expanded = false;
    for (auto& hijo : nodo->hijos)
        collapseSubtree(hijo.get());
}

void handleTreeClick(Nodo* nodo, const sf::Vector2f& clickPos) {
    float radius = 15.f;
    sf::Vector2f diff = clickPos - nodo->posicion;
    if (std::sqrt(diff.x * diff.x + diff.y * diff.y) <= radius) {
        if (!nodo->expanded && nodo->padre) {
            // Collapse all siblings
            for (auto& sibling : nodo->padre->hijos) {
                if (sibling.get() != nodo)
                    collapseSubtree(sibling.get());
            }
        }
        nodo->expanded = !nodo->expanded;
        return;
    }
    if (nodo->expanded) {
        for (auto& hijo : nodo->hijos)
            handleTreeClick(hijo.get(), clickPos);
    }
}

void asignarPosiciones(Nodo* nodo, float xMin, float xMax, float y, float dy) {
    nodo->posicion = sf::Vector2f((xMin + xMax) * 0.5f, y);
    if (!nodo->expanded || nodo->hijos.empty()) return;
    size_t nVisible = nodo->hijos.size();
    float totalWidth = xMax - xMin;
    float childWidth = totalWidth / nVisible;
    float newY = y + dy + 20;
    for (size_t i = 0; i < nVisible; i++) {
        float xi = xMin + i * childWidth;
        float xf = xi + childWidth;
        asignarPosiciones(nodo->hijos[i].get(), xi, xf, newY, dy);
    }
}

void dibujarArbol(sf::RenderWindow& window, Nodo* nodo, sf::Font& font, int boardSize) {
    sf::CircleShape circ(15.f);
    circ.setOrigin(15.f, 15.f);

    if (nodo->hijos.empty()) {
        Jugador winner = checkGanador(nodo->tablero, boardSize);
        if (winner == Jugador::X) {
            circ.setFillColor(sf::Color::Blue);
        } else if (winner == Jugador::O) {
            circ.setFillColor(sf::Color::Red);
        } else if (tableroLleno(nodo->tablero)) {
            circ.setFillColor(sf::Color::Black);
        } else {
            circ.setFillColor(sf::Color(128, 128, 128));
        }
    } else {
        circ.setFillColor(sf::Color::White);
    }
    circ.setOutlineColor(sf::Color::Black);
    circ.setOutlineThickness(1.f);
    circ.setPosition(nodo->posicion);
    window.draw(circ);

    sf::Text movText("", font, 12);
    movText.setFillColor(sf::Color::Red);
    if (nodo->movimiento == -1) {
        movText.setString("Root");
    } else {
        int fila = nodo->movimiento / boardSize;
        int col = nodo->movimiento % boardSize;
        movText.setString("(" + std::to_string(fila) + "," + std::to_string(col) + ")");
    }
    sf::FloatRect textBounds = movText.getLocalBounds();
    movText.setOrigin(textBounds.width / 2.f, 0.f);
    movText.setPosition(nodo->posicion.x, nodo->posicion.y + 20.f);
    window.draw(movText);

    if (nodo->expanded) {
        for (auto& h : nodo->hijos) {
            sf::Vertex line[2] = {
                sf::Vertex(nodo->posicion, sf::Color::Black),
                sf::Vertex(h->posicion, sf::Color::Black)
            };
            window.draw(line, 2, sf::Lines);
            dibujarArbol(window, h.get(), font, boardSize);
        }
    }
}

void dibujarTablero(sf::RenderWindow& window,
                    const std::vector<Jugador>& board,
                    sf::Font& font,
                    float cellSize,
                    float offsetX,
                    float offsetY,
                    double p_X_win,
                    double p_O_win,
                    int boardSize)
{
    sf::RectangleShape line;
    line.setFillColor(sf::Color::Black);

    // Draw vertical grid lines
    for (int col = 1; col < boardSize; col++) {
        line.setSize(sf::Vector2f(2.f, cellSize * boardSize));
        line.setPosition(offsetX + col * cellSize - 1.f, offsetY);
        window.draw(line);
    }

    // Draw horizontal grid lines
    for (int row = 1; row < boardSize; row++) {
        line.setSize(sf::Vector2f(cellSize * boardSize, 2.f));
        line.setPosition(offsetX, offsetY + row * cellSize - 1.f);
        window.draw(line);
    }

    // Draw X and O
    for (int i = 0; i < boardSize * boardSize; i++) {
        int r = i / boardSize;
        int c = i % boardSize;
        float px = offsetX + c * cellSize + cellSize / 2.f;
        float py = offsetY + r * cellSize + cellSize / 2.f;
        if (board[i] != Jugador::VACIO) {
            sf::Text t((board[i] == Jugador::X) ? "X" : "O", font, 48);
            t.setFillColor(sf::Color::Black);
            sf::FloatRect bounds = t.getLocalBounds();
            t.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
            t.setPosition(px, py - 10.f);
            window.draw(t);
        }
    }

    // Probability text
    sf::Text probText("", font, 20);
    probText.setFillColor(sf::Color::Blue);

    std::ostringstream ossProb;
    ossProb << "P(X wins): " << std::fixed << std::setprecision(1)
            << (p_X_win * 100) << "% | P(O wins): "
            << (p_O_win * 100) << "%";
    probText.setString(ossProb.str());
    probText.setPosition(10.f, 10.f);
    window.draw(probText);
}

int main() {
    int boardSize;
    std::cout << "Enter board size (n for n-in-a-row): ";
    std::cin >> boardSize;

    int G;
    std::cout << "Enter tree depth (G): ";
    std::cin >> G;

    char ini;
    std::cout << "Who starts? (X=machine, O=human): ";
    std::cin >> ini;
    Jugador turno = (ini == 'X' || ini == 'x') ? Jugador::X : Jugador::O;

    sf::RenderWindow treeWindow(sf::VideoMode(1200, 800), "Move Tree");
    sf::RenderWindow boardWindow(sf::VideoMode(600, 600), "N-in-a-Row Board");
    treeWindow.setView(sf::View(sf::FloatRect(0.f, 0.f, 1200.f, 800.f)));

    sf::Font font;
    if (!font.loadFromFile("fonts/DejaVuSans-ExtraLight.ttf")) {
        std::cerr << "Could not load font.\n";
        return 1;
    }

    std::vector<Jugador> board(boardSize * boardSize, Jugador::VACIO);
    bool fin = false;
    bool maquinaGana = false, humanoGana = false;

    // Build initial tree
    std::unique_ptr<Nodo> tree = std::make_unique<Nodo>(board, -1, 0);
    construirArbol(tree.get(), G, boardSize, turno);
    minimax(tree.get(), G, boardSize, (turno == Jugador::X));
    tree->expanded = true;

    while (boardWindow.isOpen() && treeWindow.isOpen()) {
        // Machine's turn (X)
        if (!fin && turno == Jugador::X) {
            // Rebuild tree based on current board
            tree = std::make_unique<Nodo>(board, -1, 0);
            construirArbol(tree.get(), G, boardSize, Jugador::X);
            minimax(tree.get(), G, boardSize, true);

            // Choose best move
            int bestIndex = -1;
            for (auto& h : tree->hijos) {
                if (h->evaluacion == tree->evaluacion) {
                    bestIndex = h->movimiento;
                    break;
                }
            }

            if (bestIndex >= 0 && bestIndex < static_cast<int>(board.size())) {
                board[bestIndex] = Jugador::X;
                turno = Jugador::O;
                // Rebuild tree for O's turn
                tree = std::make_unique<Nodo>(board, -1, 0);
                construirArbol(tree.get(), G, boardSize, Jugador::O);
                minimax(tree.get(), G, boardSize, false);
                tree->expanded = true;
            }
        }

        // Check if game ended
        Jugador g = checkGanador(board, boardSize);
        if (g == Jugador::X) {
            maquinaGana = true;
            fin = true;
        } else if (g == Jugador::O) {
            humanoGana = true;
            fin = true;
        } else if (tableroLleno(board)) {
            fin = true;
        }

        // Handle board window events (human move)
        sf::Event evB;
        while (boardWindow.pollEvent(evB)) {
            if (evB.type == sf::Event::Closed) {
                boardWindow.close();
            }
            else if (evB.type == sf::Event::MouseButtonReleased && turno == Jugador::O && !fin) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(boardWindow);
                float cellSize = static_cast<float>(boardWindow.getSize().x) / boardSize;
                int col = static_cast<int>(mousePos.x / cellSize);
                int row = static_cast<int>(mousePos.y / cellSize);

                if (row >= 0 && row < boardSize && col >= 0 && col < boardSize) {
                    int idx = row * boardSize + col;
                    if (board[idx] == Jugador::VACIO) {
                        board[idx] = Jugador::O;
                        turno = Jugador::X;
                        // Rebuild tree for X's turn
                        tree = std::make_unique<Nodo>(board, -1, 0);
                        construirArbol(tree.get(), G, boardSize, Jugador::X);
                        minimax(tree.get(), G, boardSize, true);
                        tree->expanded = true;
                    }
                }
            }
        }

        // Handle tree window events
        sf::Event evT;
        while (treeWindow.pollEvent(evT)) {
            if (evT.type == sf::Event::Closed) {
                treeWindow.close();
            }
            else if (evT.type == sf::Event::MouseButtonPressed) {
                sf::Vector2f clickPos = treeWindow.mapPixelToCoords(
                    sf::Vector2i(evT.mouseButton.x, evT.mouseButton.y));
                if (tree) handleTreeClick(tree.get(), clickPos);
            }
        }

        // ---- Calculate probabilities by counting terminal nodes in the subtree ----
        double p_X_win = 0.5, p_O_win = 0.5;
        if (tree && !fin) {
            WinCounts wc = computeStats(tree.get(), boardSize);
            int totalLeaves = wc.xWins + wc.oWins + wc.draws;
            if (totalLeaves > 0) {
                p_X_win = double(wc.xWins) / totalLeaves;
                p_O_win = double(wc.oWins) / totalLeaves;
            }
        } else if (fin) {
            if (maquinaGana) {
                p_X_win = 1.0; 
                p_O_win = 0.0;
            } else if (humanoGana) {
                p_X_win = 0.0; 
                p_O_win = 1.0;
            } else {
                // It's a draw
                p_X_win = 0.5;
                p_O_win = 0.5;
            }
        }

        // Draw board
        boardWindow.clear(sf::Color::White);
        if (fin) {
            sf::Text msg("", font, 24);
            msg.setFillColor(sf::Color::Red);
            msg.setString(maquinaGana ? "Winner: Machine (X)" :
                          (humanoGana ? "Winner: Human (O)" : "Draw!"));

            sf::FloatRect b = msg.getLocalBounds();
            msg.setOrigin(b.width / 2.f, b.height / 2.f);
            msg.setPosition(boardWindow.getSize().x / 2.f, 40.f);
            boardWindow.draw(msg);
        }

        float cellSize = static_cast<float>(boardWindow.getSize().x) / boardSize;
        dibujarTablero(boardWindow, board, font, cellSize, 0.f, 0.f,
                       p_X_win, p_O_win, boardSize);
        boardWindow.display();

        // Draw tree
        treeWindow.clear(sf::Color::White);
        if (tree) {
            asignarPosiciones(tree.get(), 50.f, 1150.f, 50.f, 50.f);
            sf::Text header("Tree for " + std::string(turno == Jugador::X ? "X" : "O"), font, 16);
            header.setFillColor(sf::Color::Black);
            header.setPosition(10.f, 10.f);
            treeWindow.draw(header);

            dibujarArbol(treeWindow, tree.get(), font, boardSize);
        }
        treeWindow.display();
    }
    return 0;
}
