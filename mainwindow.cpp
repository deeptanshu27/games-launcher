#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qpainter.h"
#include "QKeyEvent"
#include "QTimer"
#include "QDebug"
#include "QGraphicsEffect"
#include "QFontDatabase"
#include "QProcess"
#include <filesystem>
#include "fstream"
#include <QPainterPath>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
// #include <chrono>
#include <QtConcurrent/QtConcurrentRun>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // auto time_4= std::chrono::system_clock::now();

    showFullScreen();
    ui->setupUi(this);
    setMouseTracking(true);
    ui->centralwidget->setMouseTracking(true);

    QPixmap bkgnd("C:/Users/Deeptanshu/Pictures/dat_game_thingy_background.png");
    bkgnd = bkgnd.scaled(this->size());
    QPalette palette;
    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
    blur->setBlurRadius(8);
    palette.setBrush(QPalette::Window, applyEffectToImage(bkgnd.toImage(), blur));
    this->setPalette(palette);

    QFuture _ = QtConcurrent::run(&MainWindow::AddPixs, this);

    buttonsPos = size().height() + 5;
    ui->widget->move(QPoint(ui->widget->x(), buttonsPos));

    initImages();

    if (pixList.size() <= 4) {
        // if (pixList.size() != 0)
        width = (size().width()) / pixList.size() - gap;
    } else {
        width = (size().width()) / 5 - gap;
    }

    originalWidth = width;

    currActivity = 0;
    currPos = 0;
    newPos = pixList.size() / 2;

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(animationFunc()));
    timer->start(17);

    // auto time_5= std::chrono::system_clock::now();
    // std::chrono::duration<double> elapsed54 = time_5 - time_4;
    // qInfo() << "elapsed from 4 -- 5: " << elapsed54.count() << "s";
}

void MainWindow::paintEvent(QPaintEvent *) {
    QPainter painter(this);

    if (newPos - currPos < 0.0001) {}

    for (int i = 0; i < (int) pixList.size(); i++) {
        int initialPos = (1707/2 - width/2) + (i - currPos) * (width + gap);
        PaintGameRect(initialPos, &painter, i, newPos);
    }

    int max_int = pixList.size() > 6 ? 6 : pixList.size();

    if (currPos < 3) {
        for (int i = 0; i < max_int; i++) {
            int initialPos = (1707/2 - width/2) + (i - currPos - max_int) * (width + gap);
            PaintGameRect(initialPos, &painter, i, i - 1, pixList.size() - max_int);
        }
    }

    if (currPos > 3) {
        for (int i = 0; i < max_int; i++) {
            int initialPos = (1707/2 - width/2) + (i + (pixList.size() - currPos)) * (width + gap);
            PaintGameRect(initialPos, &painter, i, i - 1);
        }
    }
}

void MainWindow::PaintGameRect(int initialPos, QPainter *painter, int i, int j, int k) {
    float y_pos  = size().height() * 0.1f;
    float height = size().height() * 0.8f;

    QRect rect(
        initialPos,
        y_pos,
        width,
        height
        );

    QRegion r(rect);
    painter->setClipRegion(r);
    painter->drawPixmap(0, 0, pixList[k + i]);

    if (i != j) {
        QPainterPath path;
        path.addRect(rect);
        QColor translucentColor(0, 0, 0, 128); // semi transparent black
        painter->setBrush(QBrush(translucentColor));
        painter->fillPath(path, painter->brush());
    }
}

void MainWindow::runProgram() {
    QProcess::startDetached("cmd", {"/C", runnablesList[newPos]});
    exit(0);
}

void MainWindow::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Left) {
        newPos--;
    } else if (e->key() == Qt::Key_Right) {
        newPos++;
    } else if (e->key() == Qt::Key_Return) {
        runProgram();
    }
    update();

    if (newPos < 0) {newPos = pixList.size() - 1; jump = true;}
    else if (newPos > (int) pixList.size() - 1) {newPos = 0; jump = true;}
}

void MainWindow::AddPixs() {
    for (size_t i = 0; i < pixPathsList.size(); i++) {
        QPixmap temp = QPixmap(QString::fromStdString(pixPathsList[i]));
        pixList[i].swap(temp);
        pixList[i] = size().height() > size().width() ? pixList[i].scaledToHeight(size().height(), Qt::SmoothTransformation) : pixList[i].scaledToWidth(size().width(), Qt::SmoothTransformation);
    }
}

void MainWindow::initImages() {
    std::string baseFolder = "C:\\Users\\Deeptanshu\\Personal\\Games";

    std::ifstream jsonFile("C:\\Users\\Deeptanshu\\Documents\\killme.txt");
    std::string content;

    std::vector<std::string> allPathsInFile;

    if (!jsonFile.fail()) {
        while (std::getline(jsonFile, content)) {
            if (content != "") {
                allPathsInFile.push_back(content);

                std::string image = content.substr(0, content.find("::"));
                std::string runnable = content.substr(content.find("::") + 2);

                pixPathsList.push_back(image);
                // pixList.push_back(QPixmap(QString::fromStdString(image)));
                pixList.push_back(QPixmap());
                runnablesList.push_back(QString::fromStdString(runnable));
            }
        }
    } else {
        addFilesToList(baseFolder, allPathsInFile);
    }

    jsonFile.close();
}

void MainWindow::addFilesToList(std::string baseFolder, std::vector<std::string> allPathsInFile) {
    std::vector<std::string> toAdd;
    std::string image = "";
    std::string runnable = "";

    for (auto& p : std::filesystem::recursive_directory_iterator(baseFolder)) {

        if (p.is_regular_file()) {

            std::string path = p.path().string();

            if (path.find("image.png") != std::string::npos || path.find("image.jpg") != std::string::npos) {
                image = path;
            } else if (path.find("run.bat") != std::string::npos) {
                runnable = path;
            }

        }

        addToLists(image, runnable, std::ref(toAdd), std::ref(allPathsInFile));
    }
    addToLists(image, runnable, std::ref(toAdd), std::ref(allPathsInFile));

    std::ofstream outFile;
    outFile.open("C:\\Users\\Deeptanshu\\Documents\\killme.txt", std::ios_base::app);
    for (auto& s : toAdd) {
        outFile << s << "\n";
    }
    outFile.close();
}

void MainWindow::addToLists(std::string image, std::string runnable, std::vector<std::string> toAdd, std::vector<std::string> allPathsInFile) {
    if (image != "" && runnable != "") {
        std::string substr = image.substr(0, image.size() - 10);
        if (runnable.find(substr) != std::string::npos) {
            std::string fullName = image + "::" + runnable;
            if (std::find(allPathsInFile.begin(), allPathsInFile.end(), fullName) == allPathsInFile.end()) {
                toAdd.push_back(fullName);
                QPixmap tmpMap(QString::fromStdString(image));
                tmpMap =  size().height() > size().width() ? tmpMap.scaledToHeight(size().height(), Qt::SmoothTransformation) : tmpMap.scaledToWidth(size().width(), Qt::SmoothTransformation);
                pixList.push_back(tmpMap);
                runnablesList.push_back(QString::fromStdString(runnable));
            }
            image = "";
            runnable = "";
        }
    }
}

void MainWindow::animationFunc() {
    if (jump) {
        if (std::abs(currPos - (int) pixList.size() - 1) > std::abs(currPos - 0)) {
            currPos = pixList.size() + currPos;
        } else {
            currPos = 0 - (pixList.size() - currPos);
        }
        jump = false;
        update();
    } else {
        if (pixList.size() > 4 && std::abs(currPos - newPos) > 0.001f) {
            currPos = lerp2(currPos, newPos, 0.1);
            update();
        }
    }
}

float MainWindow::lerp(float start, float end, float t) {
    float val = start + (end - start) * t;
    return val;
}

float MainWindow::lerp2(float start, float end, float t) {
    float val = start + (std::abs(end - start) > 1 ? (1 * std::abs(end - start)/(end - start)) : (end - start)) * t;
    return val;
}

QImage MainWindow::applyEffectToImage(QImage src, QGraphicsEffect *effect, int extent)
{
    if(src.isNull()) return QImage();   //No need to do anything else!
    if(!effect) return src;             //No need to do anything else!
    QGraphicsScene scene;
    QGraphicsPixmapItem item;
    item.setPixmap(QPixmap::fromImage(src));
    item.setGraphicsEffect(effect);
    scene.addItem(&item);
    QImage res(src.size()+QSize(extent*2, extent*2), QImage::Format_ARGB32);
    res.fill(Qt::transparent);
    QPainter ptr(&res);
    scene.render(&ptr, QRectF(), QRectF( -extent, -extent, src.width()+extent*2, src.height()+extent*2 ) );
    return res;
}

MainWindow::~MainWindow()
{
    delete ui;
}
