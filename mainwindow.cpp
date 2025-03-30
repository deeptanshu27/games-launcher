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
#include "thread"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    showFullScreen();
    ui->setupUi(this);
    setMouseTracking(true);
    ui->centralwidget->setMouseTracking(true);

    buttonsPos = size().height() + 5;
    ui->widget->move(QPoint(ui->widget->x(), buttonsPos));


    initImages();

    if (pixList.size() <= 4) {
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
}

void MainWindow::paintEvent(QPaintEvent *) {
    QPainter painter(this);

    //painter.drawPixmap(0, 0, pixList[newPos]);

    int y_pos = 0;
    int height = size().height();

    for (int i = 0; i < (int) pixList.size(); i++) {
        int initialPos = (1707/2 - width/2) + (i - currPos) * (width + gap);
        QRect rect(
            initialPos,
            y_pos,
            width,
            height
            );

        QRegion r(rect);
        painter.setClipRegion(r);
        painter.drawPixmap(0, 0, pixList[i]);
    }

    // if (newPos < 3) {
        for (int i = 0; i < 3; i++) {
            int initialPos = (1707/2 - width/2) + (i - currPos - 3) * (width + gap);
            QRect rect(
                initialPos,
                y_pos,
                width,
                height
                );

            QRegion r(rect);
            painter.setClipRegion(r);
            painter.drawPixmap(0, 0, pixList[pixList.size() - 3 + i]);
        }
    // }

    // if (newPos > (int) pixList.size() - 1 - 3) {
        for (int i = 0; i < 3; i++) {
            int initialPos = (1707/2 - width/2) + (i + (pixList.size() - currPos)) * (width + gap);
            QRect rect(
                initialPos,
                y_pos,
                width,
                height
                );

            QRegion r(rect);
            painter.setClipRegion(r);
            painter.drawPixmap(0, 0, pixList[i]);
        }
    // }
}

void MainWindow::runProgram() {
    QProcess::startDetached("cmd", {"/C", runnablesList[newPos]});
    exit(0);
}

void MainWindow::setButtonSelect() {
    playSelected = !playSelected;

    ui->label_5->setText(playSelected ? "<" : "");
    ui->label_4->setText(!playSelected ? "<" : "");
}

void MainWindow::keyPressEvent(QKeyEvent *e) {
    if (currActivity == 1) {
        if (e->key() == Qt::Key_Escape) {
            if (currActivity == 1)
                currActivity = -2;
            else
                currActivity = -1;
        } else if (e->key() == Qt::Key_Return) {
            if (playSelected)
                runProgram();
            else {
                std::string loc = runnablesList[newPos].toStdString();
                loc = loc.substr(0, loc.find("run.bat"));
                QString qLoc = QString::fromStdString(loc);
                QProcess::startDetached("cmd", {"/C", "start", "", qLoc});
                exit(0);
            }
        } else if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) {
            setButtonSelect();
        }
    } else {
        if (e->key() == Qt::Key_Left) {
            newPos++;
        } else if (e->key() == Qt::Key_Right) {
            newPos--;
        } else if (e->key() == Qt::Key_Return) {
            currActivity = 1;
        }
        update();
    }

    if (newPos < 0) {newPos = pixList.size() - 1; jump = true;}
    else if (newPos > (int) pixList.size() - 1) {newPos = 0; jump = true;}
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

                pixList.push_back(QPixmap(QString::fromStdString(image)));
                runnablesList.push_back(QString::fromStdString(runnable));
            }
        }
        std::thread thread_obj(&MainWindow::addFilesToList, this, baseFolder, allPathsInFile);
        thread_obj.detach();
    } else {
        addFilesToList(baseFolder, allPathsInFile);
    }

    jsonFile.close();

    for (int i = 0; i < (int) pixList.size(); i++) {
        pixList[i] =  size().height() > size().width() ? pixList[i].scaledToHeight(size().height(), Qt::SmoothTransformation) : pixList[i].scaledToWidth(size().width(), Qt::SmoothTransformation);
    }
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

    std::ofstream outFile;
    outFile.open("C:\\Users\\Deeptanshu\\Documents\\killme.txt", std::ios_base::app);
    for (auto& s : toAdd) {
        outFile << s << "\n";
    }
    outFile.close();
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
        if (pixList.size() > 4) {
            if (currPos > newPos) {
                currPos = lerp(currPos, newPos, 0.1);
                update();
            }

            if (currPos < newPos) {
                currPos = lerp(currPos, newPos, 0.1);
                update();
            }
        }
    }

    if (currActivity == 1) {
        width = lerp(width, size().width() + 2*gap, 0.1);

        buttonsPos = lerp(buttonsPos, 760, 0.2);
        ui->widget->move(QPoint(ui->widget->x(), buttonsPos));

        update();
    } else if (currActivity == -2) {
        width = lerp(width, originalWidth, 0.1);

        buttonsPos = lerp(buttonsPos, size().height() + 5, 0.2);
        ui->widget->move(QPoint(ui->widget->x(), buttonsPos));
    }
}

float MainWindow::lerp(float start, float end, float t) {
    float val = start + (end - start) * t;
    return val;
}

MainWindow::~MainWindow()
{
    delete ui;
}
