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

// -----------------------------------------------------------------------------
// Enumeración para los jugadores
// -----------------------------------------------------------------------------
enum class Jugador { X, O, VACIO };

// -----------------------------------------------------------------------------
// Nodo del árbol de minimax (incluye puntero al padre)
// -----------------------------------------------------------------------------
struct Nodo {
    std::vector<Jugador> tablero;
    int movimiento;       // Índice de la celda modificada (-1 para la raíz)
    int nivel;
    int evaluacion;
    float probX;          // Ya no se mostrará, pero se calcula con minimax
    float probO;
    std::vector<std::unique_ptr<Nodo>> hijos;
    sf::Vector2f posicion;  // Posición para dibujar el nodo
    bool expanded;          // Indica si se muestran sus hijos
    Nodo* padre;            // Puntero al nodo padre (nullptr en la raíz)

    Nodo(const std::vector<Jugador>& tab, int mov, int niv)
        : tablero(tab), movimiento(mov), nivel(niv),
          evaluacion(0), probX(0.f), probO(0.f),
          posicion(0.f, 0.f), expanded(false), padre(nullptr) {}
};

// -----------------------------------------------------------------------------
// Función para obtener la profundidad máxima del árbol (útil para calcular dy)
// -----------------------------------------------------------------------------
int getMaxDepth(Nodo* nodo) {
    int maxD = nodo->nivel;
    for (auto& h : nodo->hijos)
        maxD = std::max(maxD, getMaxDepth(h.get()));
    return maxD;
}

// -----------------------------------------------------------------------------
// Funciones auxiliares para evaluar el tablero y construir el árbol
// -----------------------------------------------------------------------------

// Cuenta las "posibilidades" (líneas ganadoras potenciales) para cada jugador.
void contarPosibilidades(const std::vector<Jugador>& board, int boardSize, int& posX, int& posO) {
    posX = 0;
    posO = 0;
    // Filas
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
    // Columnas
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
    // Diagonal principal
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
    // Diagonal secundaria
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

// Función auxiliar que verifica el ganador en un estado dado.
Jugador checkGanador(const std::vector<Jugador>& board, int boardSize) {
    // Revisar filas
    for (int i = 0; i < boardSize; i++) {
        Jugador c0 = board[i * boardSize];
        if (c0 == Jugador::VACIO) continue;
        bool linea = true;
        for (int j = 1; j < boardSize; j++) {
            if (board[i * boardSize + j] != c0) { linea = false; break; }
        }
        if (linea)
            return c0;
    }
    // Revisar columnas
    for (int j = 0; j < boardSize; j++) {
        Jugador c0 = board[j];
        if (c0 == Jugador::VACIO) continue;
        bool linea = true;
        for (int i = 1; i < boardSize; i++) {
            if (board[i * boardSize + j] != c0) { linea = false; break; }
        }
        if (linea)
            return c0;
    }
    // Diagonal principal
    {
        Jugador c0 = board[0];
        if (c0 != Jugador::VACIO) {
            bool diag = true;
            for (int i = 1; i < boardSize; i++) {
                if (board[i * boardSize + i] != c0) { diag = false; break; }
            }
            if (diag)
                return c0;
        }
    }
    // Diagonal secundaria
    {
        Jugador c0 = board[boardSize - 1];
        if (c0 != Jugador::VACIO) {
            bool diag = true;
            for (int i = 1; i < boardSize; i++) {
                if (board[i * boardSize + (boardSize - 1 - i)] != c0) { diag = false; break; }
            }
            if (diag)
                return c0;
        }
    }
    return Jugador::VACIO;
}

// Solo se generan hijos si el tablero del nodo NO tiene ganador.
void generarHijos(Nodo* nodo, Jugador turno, int boardSize) {
    // Si en el estado actual ya hay ganador, no se generan más nodos.
    if (checkGanador(nodo->tablero, boardSize) != Jugador::VACIO)
        return;
    // Si el tablero está lleno, tampoco hay movimientos válidos.
    if (tableroLleno(nodo->tablero))
        return;

    int total = boardSize * boardSize;
    for (int i = 0; i < total; i++) {
        // Solo se crea un nodo para la posición vacía.
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
    if (nodo->nivel >= G)
        return;
    generarHijos(nodo, turno, boardSize);
    Jugador siguiente = (turno == Jugador::X) ? Jugador::O : Jugador::X;
    for (auto& hijo : nodo->hijos)
        construirArbol(hijo.get(), G, boardSize, siguiente);
}

void evaluarNodo(Nodo* nodo, int boardSize) {
    int posX = 0, posO = 0;
    contarPosibilidades(nodo->tablero, boardSize, posX, posO);
    // Aquí se utiliza MinMax: eval = posX - posO.
    nodo->evaluacion = posX - posO;
    int total = posX + posO;
    if (total > 0) {
        nodo->probX = static_cast<float>(posX) / total;
        nodo->probO = static_cast<float>(posO) / total;
    } else {
        nodo->probX = 0.f;
        nodo->probO = 0.f;
    }
}

int minimax(Nodo* nodo, int profundidad, int boardSize, bool maximizando) {
    if (profundidad == 0 || nodo->hijos.empty()) {
        evaluarNodo(nodo, boardSize);
        return nodo->evaluacion;
    }
    if (maximizando) {
        int mejorVal = std::numeric_limits<int>::min();
        for (auto& hijo : nodo->hijos) {
            int val = minimax(hijo.get(), profundidad - 1, boardSize, false);
            if (val > mejorVal)
                mejorVal = val;
        }
        nodo->evaluacion = mejorVal;
        return mejorVal;
    } else {
        int peorVal = std::numeric_limits<int>::max();
        for (auto& hijo : nodo->hijos) {
            int val = minimax(hijo.get(), profundidad - 1, boardSize, true);
            if (val < peorVal)
                peorVal = val;
        }
        nodo->evaluacion = peorVal;
        return peorVal;
    }
}

// -----------------------------------------------------------------------------
// Colapsa recursivamente un subárbol (cerrando la rama).
// -----------------------------------------------------------------------------
void collapseSubtree(Nodo* nodo) {
    nodo->expanded = false;
    for (auto& hijo : nodo->hijos)
        collapseSubtree(hijo.get());
}

// -----------------------------------------------------------------------------
// Maneja el clic en un nodo: alterna su estado y, al expandir, colapsa los hermanos.
// -----------------------------------------------------------------------------
void handleTreeClick(Nodo* nodo, const sf::Vector2f& clickPos) {
    float radius = 20.f;
    sf::Vector2f diff = clickPos - nodo->posicion;
    if (std::sqrt(diff.x * diff.x + diff.y * diff.y) <= radius) {
        if (!nodo->expanded && nodo->padre) {
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

// -----------------------------------------------------------------------------
// Asigna posiciones a los nodos para su visualización.
// Se aumenta fuertemente el gap horizontal entre hermanos a partir del nivel 2
// y se reduce el gap vertical (dy) para poder ver mayor profundidad.
// -----------------------------------------------------------------------------
void asignarPosiciones(Nodo* nodo, float xMin, float xMax, float y, float dy) {
    nodo->posicion = sf::Vector2f((xMin + xMax) * 0.5f, y);
    if (!nodo->expanded || nodo->hijos.empty())
        return;
    
    size_t nVisible = nodo->hijos.size();
    float gap;
    if (nodo->nivel < 1)
        gap = 1.f;            // Primer nivel (raíz): gap pequeño
    else if (nodo->nivel == 1)
        gap = 600.f;           // Primer nivel de hijos: un buen separador
    else 
        gap = 1100.f;           // Niveles profundos: espacio aún mayor entre hermanos
    
    float totalGap = gap * (nVisible - 1);
    float effectiveWidth = (xMax - xMin) - totalGap;
    float childWidth = effectiveWidth / nVisible;
    float newY = y + dy+20;  // dy vertical reducido para ver más niveles (por ejemplo, 50 px)
    for (size_t i = 0; i < nVisible; i++) {
        float xi = xMin + i * (childWidth + gap);
        float xf = xi + childWidth;
        asignarPosiciones(nodo->hijos[i].get(), xi, xf, newY, dy);
    }
}

// -----------------------------------------------------------------------------
// Dibuja el árbol interactivo: cada nodo mostrará la jugada (celda) que representa.
// Se omite la información de probabilidad ya que los árboles son solo para visualizar los posibles movimientos.
// -----------------------------------------------------------------------------
void dibujarArbol(sf::RenderWindow& window, Nodo* nodo, sf::Font& font, int boardSize) {
    // Dibuja el nodo (círculo)
    sf::CircleShape circ(20.f);
    circ.setOrigin(20.f, 20.f);
    circ.setFillColor(sf::Color(200, 200, 250));
    circ.setOutlineColor(sf::Color::Black);
    circ.setOutlineThickness(1.f);
    circ.setPosition(nodo->posicion);
    window.draw(circ);

    // Mostrar la jugada: si es la raíz, se muestra "Root"; para los demás, se muestra la celda.
    sf::Text movText("", font, 14);
    movText.setFillColor(sf::Color::Red);
    if (nodo->movimiento == -1) {
        movText.setString("Root");
    } else {
        int fila = nodo->movimiento / boardSize;
        int col = nodo->movimiento % boardSize;
        movText.setString("Cel: (" + std::to_string(fila) + "," + std::to_string(col) + ")");
    }
    sf::FloatRect textBounds = movText.getLocalBounds();
    movText.setOrigin(textBounds.width / 2.f, 0.f);
    movText.setPosition(nodo->posicion.x, nodo->posicion.y + 25.f);
    window.draw(movText);

    // Si el nodo ya representa un movimiento ganador, opcionalmente se puede resaltar.
    if (nodo->movimiento != -1) {
        Jugador win = checkGanador(nodo->tablero, boardSize);
        if (win != Jugador::VACIO) {
            sf::Text winText("", font, 14);
            winText.setFillColor(sf::Color::Blue);
            winText.setString("WIN " + std::string((win == Jugador::X) ? "X" : "O"));
            sf::FloatRect wb = winText.getLocalBounds();
            winText.setOrigin(wb.width / 2.f, wb.height / 2.f);
            winText.setPosition(nodo->posicion.x, nodo->posicion.y - 25.f);
            window.draw(winText);
        }
    }

    // Dibujar las ramas (líneas) y recorrer hijos si el nodo está expandido.
    if (nodo->expanded) {
        for (auto& h : nodo->hijos) {
            sf::Vertex line[2];
            line[0].position = nodo->posicion;
            line[0].color = sf::Color::Black;
            line[1].position = h->posicion;
            line[1].color = sf::Color::Black;
            window.draw(line, 2, sf::Lines);
            dibujarArbol(window, h.get(), font, boardSize);
        }
    }
}

// -----------------------------------------------------------------------------
// Dibuja el tablero de n en raya: líneas, fichas, etc.
// -----------------------------------------------------------------------------
void dibujarTablero(sf::RenderWindow& window, const std::vector<Jugador>& board,
                    sf::Font& font, float cellSize, float offsetX, float offsetY,
                    float probX, float probO, int boardSize) {
    sf::RectangleShape line;
    line.setFillColor(sf::Color::Black);
    // Líneas verticales
    for (int col = 1; col < boardSize; col++) {
        line.setSize(sf::Vector2f(2.f, cellSize * boardSize));
        line.setPosition(offsetX + col * cellSize - 1.f, offsetY);
        window.draw(line);
    }
    // Líneas horizontales
    for (int row = 1; row < boardSize; row++) {
        line.setSize(sf::Vector2f(cellSize * boardSize, 2.f));
        line.setPosition(offsetX, offsetY + row * cellSize - 1.f);
        window.draw(line);
    }
    // Dibujar fichas (X y O)
    for (int i = 0; i < boardSize * boardSize; i++) {
        int r = i / boardSize;
        int c = i % boardSize;
        float px = offsetX + c * cellSize + cellSize / 2.f;
        float py = offsetY + r * cellSize + cellSize / 2.f;
        if (board[i] != Jugador::VACIO) {
            sf::Text t("", font, 48);
            t.setFillColor(sf::Color::Black);
            t.setString((board[i] == Jugador::X) ? "X" : "O");
            sf::FloatRect bounds = t.getLocalBounds();
            t.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
            t.setPosition(px, py - 10.f);
            window.draw(t);
        }
    }
    // Dibujar el texto de probabilidades (opcional, aunque ya no se mostrarán en los nodos del árbol)
    sf::Text probText("", font, 20);
    probText.setFillColor(sf::Color::Blue);
    std::ostringstream ossProb;
    ossProb << "Prob X: " << std::fixed << std::setprecision(1) << (probX * 100)
            << "% | Prob O: " << std::fixed << std::setprecision(1) << (probO * 100) << "%";
    probText.setString(ossProb.str());
    probText.setPosition(offsetX, 5.f);
    window.draw(probText);
}

// -----------------------------------------------------------------------------
// Función principal
// -----------------------------------------------------------------------------
int main() {
    int boardSize;
    std::cout << "Ingrese el tamaño del tablero (n en raya): ";
    std::cin >> boardSize;

    int G;
    std::cout << "Ingrese la profundidad del árbol (G): ";
    std::cin >> G;

    char ini;
    std::cout << "¿Quién inicia? (X=máquina, O=humano): ";
    std::cin >> ini;
    Jugador turno = (ini == 'X' || ini == 'x') ? Jugador::X : Jugador::O;

    // Abrir primero las ventanas de los árboles y luego la del tablero.
    sf::RenderWindow treeWindowX(sf::VideoMode(800, 600), "Árbol Minimax - X");
    sf::RenderWindow treeWindowO(sf::VideoMode(800, 600), "Árbol Minimax - O");
    sf::RenderWindow boardWindow(sf::VideoMode(600, 600), "N en Raya - Board");

    // Fijar la vista en los árboles para que mapPixelToCoords opere en un rango fijo.
    treeWindowX.setView(sf::View(sf::FloatRect(0.f, 0.f, 800.f, 600.f)));
    treeWindowO.setView(sf::View(sf::FloatRect(0.f, 0.f, 800.f, 600.f)));

    sf::Font font;
    if (!font.loadFromFile("fonts/DejaVuSans-ExtraLight.ttf")) {
        std::cerr << "No se pudo cargar la fuente.\n";
        return 1;
    }

    std::vector<Jugador> board(boardSize * boardSize, Jugador::VACIO);
    bool fin = false;
    bool maquinaGana = false, humanoGana = false;
    float probX = 0.f, probO = 0.f;

    std::vector<Jugador> prevBoard = board;
    std::unique_ptr<Nodo> treeX;
    std::unique_ptr<Nodo> treeO;

    while (boardWindow.isOpen() && treeWindowX.isOpen() && treeWindowO.isOpen()) {
        // Movimiento automático de la máquina (X)
        if (!fin && turno == Jugador::X) {
            treeX = std::make_unique<Nodo>(board, -1, 0);
            construirArbol(treeX.get(), G, boardSize, Jugador::X);
            minimax(treeX.get(), G, boardSize, true);
            int bestIndex = -1;
            for (auto& h : treeX->hijos) {
                if (h->evaluacion == treeX->evaluacion) {
                    bestIndex = h->movimiento;
                    break;
                }
            }
            if (bestIndex >= 0 && bestIndex < static_cast<int>(board.size()))
                board[bestIndex] = Jugador::X;
            turno = Jugador::O;
            prevBoard = board;
        }

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

        // Procesar eventos en la ventana del tablero (jugador O)
        sf::Event evB;
        while (boardWindow.pollEvent(evB)) {
            if (evB.type == sf::Event::Closed)
                boardWindow.close();
            else if (evB.type == sf::Event::MouseButtonReleased && turno == Jugador::O && !fin) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(boardWindow);
                int mouseX = mousePos.x;
                int mouseY = mousePos.y;
                float cellSize = static_cast<float>(boardWindow.getSize().x) / boardSize;
                float offsetX = 0.f, offsetY = 0.f;
                int col = static_cast<int>((mouseX - offsetX) / cellSize);
                int row = static_cast<int>((mouseY - offsetY) / cellSize);
                if (row >= 0 && row < boardSize && col >= 0 && col < boardSize) {
                    int idx = row * boardSize + col;
                    if (board[idx] == Jugador::VACIO) {
                        board[idx] = Jugador::O;
                        turno = Jugador::X;
                    }
                }
            }
        }

        // Procesar eventos en la ventana del árbol X
        sf::Event evX;
        while (treeWindowX.pollEvent(evX)) {
            if (evX.type == sf::Event::Closed)
                treeWindowX.close();
            else if (evX.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i pixelPos(evX.mouseButton.x, evX.mouseButton.y);
                sf::Vector2f clickPos = treeWindowX.mapPixelToCoords(pixelPos);
                if (treeX)
                    handleTreeClick(treeX.get(), clickPos);
            }
        }

        // Procesar eventos en la ventana del árbol O
        sf::Event evO;
        while (treeWindowO.pollEvent(evO)) {
            if (evO.type == sf::Event::Closed)
                treeWindowO.close();
            else if (evO.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i pixelPos(evO.mouseButton.x, evO.mouseButton.y);
                sf::Vector2f clickPos = treeWindowO.mapPixelToCoords(pixelPos);
                if (treeO)
                    handleTreeClick(treeO.get(), clickPos);
            }
        }

        // Si el tablero cambió, reconstruir los árboles
        if (board != prevBoard) {
            treeX = std::make_unique<Nodo>(board, -1, 0);
            construirArbol(treeX.get(), G, boardSize, Jugador::X);
            minimax(treeX.get(), G, boardSize, true);
            treeX->expanded = true;

            treeO = std::make_unique<Nodo>(board, -1, 0);
            construirArbol(treeO.get(), G, boardSize, Jugador::O);
            minimax(treeO.get(), G, boardSize, false);
            treeO->expanded = true;

            prevBoard = board;
        }

        {
            Nodo tmp(board, -1, 0);
            evaluarNodo(&tmp, boardSize);
            probX = tmp.probX;
            probO = tmp.probO;
        }

        // Dibujar el tablero
        boardWindow.clear(sf::Color::White);
        if (fin) {
            sf::Text msg("", font, 24);
            msg.setFillColor(sf::Color::Red);
            if (maquinaGana)
                msg.setString("GANADOR: Máquina (X)");
            else if (humanoGana)
                msg.setString("GANADOR: Humano (O)");
            else
                msg.setString("¡EMPATE!");
            sf::FloatRect b = msg.getLocalBounds();
            msg.setOrigin(b.width / 2.f, b.height / 2.f);
            msg.setPosition(boardWindow.getSize().x / 2.f, 40.f);
            boardWindow.draw(msg);
        }
        float cellSize = static_cast<float>(boardWindow.getSize().x) / boardSize;
        dibujarTablero(boardWindow, board, font, cellSize, 0.f, 0.f, probX, probO, boardSize);
        boardWindow.display();

        // Establecer dy vertical fijo para los árboles (por ejemplo, 50 px) para ver más niveles.
        float dyX = 50.f;
        float dyO = 50.f;

        // Dibujar el árbol X
        treeWindowX.clear(sf::Color::White);
        if (treeX) {
            // Se usa un rango horizontal amplio (50 a 850) y se asigna la separación con dyX.
            asignarPosiciones(treeX.get(), 50.f, 850.f, 50.f, dyX);
            sf::Text headerX("Árbol de X, Eval=" + std::to_string(treeX->evaluacion), font, 16);
            headerX.setFillColor(sf::Color::Black);
            headerX.setPosition(10.f, 10.f);
            treeWindowX.draw(headerX);
            dibujarArbol(treeWindowX, treeX.get(), font, boardSize);
        }
        treeWindowX.display();

        // Dibujar el árbol O
        treeWindowO.clear(sf::Color::White);
        if (treeO) {
            asignarPosiciones(treeO.get(), 50.f, 850.f, 50.f, dyO);
            sf::Text headerO("Árbol de O, Eval=" + std::to_string(treeO->evaluacion), font, 16);
            headerO.setFillColor(sf::Color::Black);
            headerO.setPosition(10.f, 10.f);
            treeWindowO.draw(headerO);
            dibujarArbol(treeWindowO, treeO.get(), font, boardSize);
        }
        treeWindowO.display();
    }
    return 0;
}
