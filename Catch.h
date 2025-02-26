#ifndef CATCH_H
#define CATCH_H

#include <QMainWindow>

#include "Cell.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class Catch;
}
QT_END_NAMESPACE

class Player;

class Catch : public QMainWindow {
    Q_OBJECT

public:
    Catch(QWidget *parent = nullptr);
    virtual ~Catch();

signals:
    void turnEnded();

private:
    Ui::Catch *ui;
    Player* m_player;
    Cell* m_board[8][8];

private slots:
    void play(int id);
    void switchPlayer();
    void reset();

    void showAbout();

    Cell* verificaAdjacente(Cell* baseCell);

    void posicionaPlayer(Cell* targetCell);

    bool verificaFinal();

    void updateSelectables(bool over);
    void updateStatusBar();
    void showEndGame();

};

#endif // CATCH_H
