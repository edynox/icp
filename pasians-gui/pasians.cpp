#include "pasians.h"
#include "ui_pasians.h"
#include <QSize>
#include <QResizeEvent>
#include <QGraphicsScene>
#include <vector>
#include <iostream>
#include "playwrapper.h"

Pasians::Pasians(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Pasians)
{
    ui->setupUi(this);

    scene = new QGraphicsScene();

    view = ui->graphicsView;

    view->setScene(scene);

    GGame *game = generateGame();
    games.push_back(game);

    QSize s = size();

    Layout layout(s);

    showGame(game, layout);

    view->show();

    view->setMouseTracking(true);
    this->setMouseTracking(true);
    ui->gameBoard->setMouseTracking(true);
}

void Pasians::resizeEvent(QResizeEvent* event)
{
    Layout layout(event->size());

    showGames(layout);

    view->setSceneRect(scene->sceneRect());
    view->centerOn(0, 0);
}

Pasians::~Pasians()
{
    delete ui;
    for (auto &game: games) {
        delete game;
    }
}

void Pasians::redraw()
{
    showGames(activeLayout);
}

/**
 * @brief Pasians::showGames show all games in global layout
 * @param layout global layout
 */
void Pasians::showGames(Layout &layout)
{

    size_t gsize = games.size();

    ui->actionNew->setEnabled(gsize < 4);
    ui->menuGame_1->setEnabled(gsize > 0);
    ui->menuGame_2->setEnabled(gsize > 1);
    ui->menuGame_3->setEnabled(gsize > 2);
    ui->menuGame_4->setEnabled(gsize > 3);

    switch (gsize) {
    case 1:
        showGame(games.front(), layout);
        break;
    case 2:


        break;
    case 3:


        break;
    case 4:

        break;
    default:
        return;
    }
}


/**
 * @brief Pasians::drawCard draw single card
 *
 * Column and row is specified by card.position
 * @param card game card
 * @returns card label reference
 */
PlayLabel *Pasians::drawCard(Card &card, QSize &cardSize)
{
    PlayLabel *lbl = new PlayLabel(&card, cardSize);

    PlayWrapper *item = new PlayWrapper(lbl);

    scene->addItem(item);

    return lbl;
}

/**
 * @brief Pasians::generateGame generate new game
 */
GGame *Pasians::generateGame()
{
    GGame *game = new GGame(this);
    game->setup();
    return game;
}

/**
 * @brief Pasians::showGame print out single game within internal layout
 * @param game particular game
 * @param layout inner layout
 */
void Pasians::showGame(GGame *game, Layout &layout)
{
    PlayLabel *w;
    QSize &cardSize = layout.cardSize;

    if (game->initialized()) { // allready initialized

        int z = 0;
        for (auto &card: game->pickPile.cards) {
            z++;
            w = static_cast<PlayLabel *>(card.parent);
            w->setSize(cardSize);
            w->setPos(layout.pick);
            w->setZ(z);
        }

        w = static_cast<PlayLabel *>(game->pickPile.placeHolder());
        w->setPos(layout.pick);
        w->setSize(cardSize);

        z = 0;
        for (auto &card: game->dropPile.cards) {
            z++;
            w = static_cast<PlayLabel *>(card.parent);
            w->setSize(cardSize);
            w->setPos(layout.drop);
            w->setZ(z);
        }

        w = static_cast<PlayLabel *>(game->dropPile.placeHolder());
        w->setPos(layout.drop);
        w->setSize(cardSize);

        QPoint botPos = layout.bot;
        for (auto &pile: game->bottomPiles) {
            z = 0;

            w = static_cast<PlayLabel *>(pile.placeHolder());
            w->setPos(botPos);
            w->setSize(cardSize);

            for (auto &card: pile.cards) {
                z++;
                w = static_cast<PlayLabel *>(card.parent);
                w->setSize(cardSize);
                w->setPos(botPos);
                w->setZ(z);
                botPos.setY(botPos.y() + layout.cardHeight / cardOffsetMod);
            }

            botPos.setX(botPos.x() + layout.cardWidth + layout.wspace);
            botPos.setY(layout.bot.y());
        }

        QPoint topPos = layout.top;
        for (auto &pile: game->topPiles) {
            z = 0;

            w = static_cast<PlayLabel *>(pile.placeHolder());
            w->setPos(topPos);
            w->setSize(cardSize);

            for (auto &card: pile.cards) {
                z++;
                w = static_cast<PlayLabel *>(card.parent);
                w->setSize(cardSize);
                w->setPos(topPos);
                w->setZ(z);
            }

            topPos.setX(topPos.x() + layout.cardWidth + layout.wspace);
        }
    } else { // game initialization

        for (auto &card: game->pickPile.cards) {
            w = drawCard(card, cardSize);
            w->setPos(layout.pick);
            w->setContext(&game->pickPile, game);
        }

        for (auto &card: game->dropPile.cards) {
            w = drawCard(card, cardSize);
            w->setPos(layout.drop);
            w->setContext(&game->dropPile, game);
        }

        QPoint botPos = layout.bot;
        for (auto &pile: game->bottomPiles) {
            for (auto &card: pile.cards) {
                w = drawCard(card, cardSize);
                w->setPos(botPos);
                w->setContext(&pile, game);
            }
            botPos.setX(botPos.x() + layout.cardWidth + layout.wspace);
        }

        QPoint topPos = layout.top;
        for (auto &pile: game->topPiles) {
            for (auto &card: pile.cards) {
                w = drawCard(card, cardSize);
                w->setPos(botPos);
                w->setContext(&pile, game);
            }
            topPos.setX(topPos.x() + layout.cardWidth + layout.wspace);
        }

        game->setupPlaceHolders(layout, scene);

        view->setSceneRect(scene->sceneRect());            
        game->start();
    }
    activeLayout = layout;
}

void Pasians::finalizeGame(GGame *game)
{
    if (!game->initialized()) {
        redraw();
        return;
    }

    PlayLabel *l = static_cast<PlayLabel *>(game->pickPile.placeHolder());
    scene->removeItem(l->wrapper());
    for (auto &card: game->pickPile.cards) {
        l = static_cast<PlayLabel *>(card.parent);
        scene->removeItem(l->wrapper());
    }

    l = static_cast<PlayLabel *>(game->dropPile.placeHolder());
    scene->removeItem(l->wrapper());
    for (auto &card: game->dropPile.cards) {
        l = static_cast<PlayLabel *>(card.parent);
        scene->removeItem(l->wrapper());
    }

    for (auto &pile: game->topPiles) {
        l = static_cast<PlayLabel *>(pile.placeHolder());
        scene->removeItem(l->wrapper());
        for (auto &card: pile.cards) {
            l = static_cast<PlayLabel *>(card.parent);
            scene->removeItem(l->wrapper());
        }
    }

    for (auto &pile: game->bottomPiles) {
        l = static_cast<PlayLabel *>(pile.placeHolder());
        scene->removeItem(l->wrapper());
        for (auto &card: pile.cards) {
            l = static_cast<PlayLabel *>(card.parent);
            scene->removeItem(l->wrapper());
        }
    }

    redraw();
}

void Pasians::on_actionNew_triggered()
{
    GGame *game = generateGame();
    games.push_back(game);

    redraw();
}

void Pasians::on_actionQuit_4_triggered()
{
    const auto &it = games.begin() + 3;
    GGame *game = *it;
    games.erase(it);
    finalizeGame(game);
}

void Pasians::on_actionQuit_3_triggered()
{
    const auto &it = games.begin() + 2;
    GGame *game = *it;
    games.erase(it);
    finalizeGame(game);;
}

void Pasians::on_actionQuit_2_triggered()
{
    const auto &it = games.begin() + 1;
    GGame *game = *it;
    games.erase(it);
    finalizeGame(game);
}

void Pasians::on_actionQuit_triggered()
{
    const auto &it = games.begin();
    GGame *game = *it;
    games.erase(it);
    finalizeGame(game);
}
