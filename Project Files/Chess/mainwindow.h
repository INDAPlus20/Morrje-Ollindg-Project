#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItemGroup>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QPushButton>
#include <QImage>
#include <QDir>
#include <QDebug>
#include <QPushButton>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <QMouseEvent>
#include <QGraphicsEllipseItem>
#include <QInputDialog>
#include <QLineEdit>
#include <QApplication>
#include <QMessageBox>
#include<QtConcurrent>
#include<QFuture>
#include"Logic/Game.hpp"

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    #define nullSquare -1
    int squareSize;
    std::map<char, QImage*> pieceImages;
    std::vector<QGraphicsPixmapItem*> pieces;
    std::vector<QGraphicsEllipseItem*> moveCircles;
    QGraphicsView* view;
    QGraphicsScene* scene;
    QGraphicsRectItem* board[8][8];
    QPushButton* btnMakeAiMove;
    QPushButton* btnSetupGame;
    QPushButton* btnFlip;
    QPushButton* btnGoBack;
    QLineEdit* editSetupGame;
    int prevPress;
    std::pair<int,int> prevRowCol;
    bool white;
    QColor whiteSquare = Qt::white;
    QColor blackSquare = Qt::lightGray;
    QColor markedSquare = Qt::darkGray;

    Game* game;
    LaunchMode mode;


    void generate_board();
    void render_pieces();
    void isGameOver();
    void resetColor();

public:
    MainWindow(LaunchMode = PVEWHITE, QWidget *parent = nullptr);
    ~MainWindow();


public slots:
    void funcMakeAiMove();
    void funcSetupGame();
    void funcFlip();
    void funcGoBack();

protected:
    void mousePressEvent(QMouseEvent *event);


};
#endif // MAINWINDOW_H
