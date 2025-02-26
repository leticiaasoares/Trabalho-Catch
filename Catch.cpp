#include "Catch.h"
#include "ui_Catch.h"
#include "Player.h"

#include <QDebug>
#include <QMessageBox>
#include <QActionGroup>
#include <QSignalMapper>

#include <vector>


Catch::Catch(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::Catch),
      m_player(Player::player(Player::Red)) {

    ui->setupUi(this);

    QObject::connect(ui->actionNew, SIGNAL(triggered(bool)), this, SLOT(reset()));
    QObject::connect(ui->actionQuit, SIGNAL(triggered(bool)), qApp, SLOT(quit()));
    QObject::connect(ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(showAbout()));

    QSignalMapper* map = new QSignalMapper(this);
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            QString cellName = QString("cell%1%2").arg(row).arg(col);
            Cell* cell = this->findChild<Cell*>(cellName);
            Q_ASSERT(cell != nullptr);
            Q_ASSERT(cell->row() == row && cell->col() == col);

            m_board[row][col] = cell;

            int id = row * 8 + col;
            map->setMapping(cell, id);
            QObject::connect(cell, SIGNAL(clicked(bool)), map, SLOT(map()));
            QObject::connect(cell, SIGNAL(mouseOver(bool)), this, SLOT(updateSelectables(bool)));
        }
    }
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    QObject::connect(map, SIGNAL(mapped(int)), this, SLOT(play(int)));
#else
    QObject::connect(map, SIGNAL(mappedInt(int)), this, SLOT(play(int)));
#endif

    // When the turn ends, switch the player.
    QObject::connect(this, SIGNAL(turnEnded()), this, SLOT(switchPlayer()));

    this->reset();

    this->adjustSize();
    this->setFixedSize(this->size());
}

Catch::~Catch() {
    delete ui;
}

void Catch::play(int id) {
    Cell* cell = m_board[id / 8][id % 8];
    Cell* cell2 = verificaAdjacente(cell);

    if ((cell == nullptr || !cell->isSelectable()) && (cell2 == nullptr || !cell2->isSelectable()))
        return;

    cell->setState(Cell::Blocked);
    cell2->setState(Cell::Blocked);

    /*aqui começa a logica logica do jogo*/
    /*=============================================================================================================*/

    //percorre a matriz
    for(int row=0 ; row<8 ; row++) {
        for(int col=0 ; col<8 ; col++) {

            //para cada celula da matriz vai fazer uma busca

            if(m_board[row][col]->isEmpty() && !m_board[row][col]->isBlocked()) {

                bool visitado[8][8] = {{false}};

                Cell* fila[64];
                int inicio = 0;
                int final = 0;

                visitado[row][col] = true;
                fila[final] = m_board[row][col];
                final++;

                std::vector<Cell*> celulasParaMarcarBolinha;

                while(inicio != final) {


                    int nrow = fila[inicio]->row();
                    int ncol = fila[inicio]->col();

                    celulasParaMarcarBolinha.push_back(fila[inicio]);
                    inicio++;


                    int deslocX[] = {-1, 1, 0, 0};
                    int deslocY[] = {0, 0, -1, 1};

                    for(int k=0 ; k<4 ; k++) {

                        if( (nrow+deslocX[k]<8 && nrow+deslocX[k]>=0) && (ncol+deslocY[k]<8 && ncol+deslocY[k]>=0)
                                && !(visitado[nrow+deslocX[k]][ncol+deslocY[k]]) &&
                                (m_board[nrow+deslocX[k]][ncol+deslocY[k]]->isEmpty()) &&
                                !(m_board[nrow+deslocX[k]][ncol+deslocY[k]]->isBlocked()) &&
                                !(m_board[nrow+deslocX[k]][ncol+deslocY[k]]->isCaptured()) )
                        {

                            fila[final] = m_board[nrow+deslocX[k]][ncol+deslocY[k]];
                            final++;
                            visitado[nrow+deslocX[k]][ncol+deslocY[k]] = true;

                        }
                    }
                }

                if(celulasParaMarcarBolinha.size() <= 3) {

                    for(int h=0 ; h<(int)celulasParaMarcarBolinha.size() ; h++) {
                        posicionaPlayer(celulasParaMarcarBolinha[h]);
                    }

                    celulasParaMarcarBolinha.clear();

                } else {
                    celulasParaMarcarBolinha.clear();
                }

            }

        }
    }


    /*=============================================================================================================*/


    if (verificaFinal()) showEndGame();
    else
        emit turnEnded();
}

void Catch::switchPlayer() {
    // Switch the player.
    m_player = m_player->other();

    // Finally, update the status bar.
    this->updateStatusBar();
}

void Catch::reset() {
    // Reset board.
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            Cell* cell = m_board[row][col];
            cell->reset();
        }
    }

    // Reset the players.
    Player* red = Player::player(Player::Red);
    red->reset();

    Player* blue = Player::player(Player::Blue);
    blue->reset();

    m_player = red;

    // Finally, update the status bar.
    this->updateStatusBar();
}

void Catch::showAbout() {
    QMessageBox::information(this, tr("Sobre"), tr("Catch\n\nMatheus Monteiro Huebra Perdigão - mathuebra@gmail.com\nLetícia de Oliveira Soares - leticiaoliveiras2123@gmail.com"));
}

Cell* Catch::verificaAdjacente(Cell* baseCell) {
    Cell* secondaryCell = nullptr;
    int row = baseCell->row();
    int col = baseCell->col();

    if (m_player->orientation() == Player::Horizontal) {
        if (col < 7) {
            secondaryCell = m_board[row][col+1];
        } else if (col == 7) {
            secondaryCell = m_board[row][col-1];
        }
    } else if (m_player->orientation() == Player::Vertical) {
        if (row < 7) {
            secondaryCell = m_board[row+1][col];
        } else if (row == 7) {
            secondaryCell = m_board[row-1][col];
        }
    }

    return secondaryCell;
}

bool Catch::verificaFinal() {

    Cell* possibleCell;
    Cell* currentCell;
    bool final = true;

    int controlFlag = 0;

    if (!(m_player->orientation() == Player::Horizontal)) {
        for (int row = 0; row <= 7; row ++) {
            for (int col = 0; col < 7; col ++) {
                currentCell = m_board[row][col];
                possibleCell = m_board[row][col+1];

                if (currentCell->isEmpty() && possibleCell->isEmpty()) controlFlag ++;
            }
        }
    } else if (!(m_player->orientation() == Player::Vertical)) {
        for (int col = 0; col <= 7; col ++) {
            for (int row = 0; row < 7; row ++) {
                currentCell = m_board[row][col];
                possibleCell = m_board[row+1][col];

                if (currentCell->isEmpty() && possibleCell->isEmpty()) controlFlag ++;
            }
        }
    }

    if (controlFlag > 0) final = false;
    else if (controlFlag == 0) final = true;

    return final;
}

void Catch::posicionaPlayer(Cell* targetCell) {
    Q_ASSERT(targetCell != nullptr);

    m_player->incrementCount();

    targetCell->setState(Cell::Blocked);
    targetCell->setIcon(m_player->pixmap());
}

void Catch::updateSelectables(bool over) {
    Cell* cell = qobject_cast<Cell*>(QObject::sender());
    Cell* cell2 = verificaAdjacente(cell);

    Q_ASSERT(cell != nullptr);
    Q_ASSERT(cell2 != nullptr);

    if (over) {
        if (cell->isEmpty() && cell2->isEmpty()) {
            cell->setState(Cell::Selectable);
            cell2->setState(Cell::Selectable);
        }
    } else {
        if (cell->isSelectable() && cell2->isSelectable()){
            cell->setState(Cell::Empty);
            cell2->setState(Cell::Empty);
        }
    }
}

void Catch::updateStatusBar() {
    ui->statusbar->showMessage(tr("Vez do %1 (%2 a %3)")
        .arg(m_player->name()).arg(m_player->count()).arg(m_player->other()->count()));
}

void Catch::showEndGame() {

    if (m_player->type() == Player::Red) {
        if (m_player->count() > m_player->other()->count()) {
            QMessageBox::information(nullptr, QString("Fim de jogo!"), QString("O jogador Vermelho venceu!\n\nResultado: %1 a %2").arg(m_player->count()).arg(m_player->other()->count()));
        } else if (m_player->count() == m_player->other()->count()) {
            QMessageBox::information(nullptr, QString("Fim de jogo!"), QString("O jogo empatou!\n\nResultado: %1 a %2").arg(m_player->count()).arg(m_player->other()->count()));
        } else
            QMessageBox::information(nullptr, QString("Fim de jogo!"), QString("O jogador Azul venceu!\n\nResultado: %1 a %2").arg(m_player->other()->count()).arg(m_player->count()));
    } else if (m_player->type() == Player::Blue) {
        if (m_player->count() > m_player->other()->count()) {
            QMessageBox::information(nullptr, QString("Fim de jogo!"), QString("O jogador Azul venceu!\n\nResultado: %1 a %2").arg(m_player->count()).arg(m_player->other()->count()));
        } else if (m_player->count() == m_player->other()->count()) {
            QMessageBox::information(nullptr, QString("Fim de jogo!"), QString("O jogo empatou!\n\nResultado: %1 a %2").arg(m_player->count()).arg(m_player->other()->count()));
        } else
            QMessageBox::information(nullptr, QString("Fim de jogo!"), QString("O jogador Vermelho venceu!\n\nResultado: %1 a %2").arg(m_player->other()->count()).arg(m_player->count()));
    }

    reset();
}
