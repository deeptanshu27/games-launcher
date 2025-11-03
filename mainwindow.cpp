#include <mainwindow.h>
#include <ui_mainwindow.h>
#include <qpainter.h>
#include <QKeyEvent>
#include <QTimer>
#include <QDebug>
#include <QGraphicsEffect>
#include <QFontDatabase>
#include <QProcess>
#include <filesystem>
#include <fstream>
#include <QPainterPath>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QtConcurrent/QtConcurrentRun>
#include <windows.h>
#include <iostream>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    showFullScreen();
    setMouseTracking(true);
    ui->centralwidget->setMouseTracking(true);

    std::wstring regSubKey = L"Control Panel\\Desktop";
    std::wstring regValue(L"WallPaper");
    std::wstring valueFromRegistry;
    valueFromRegistry = GetStringValueFromHKLM(regSubKey, regValue);

    QString str(valueFromRegistry);
    QPixmap bkgnd(str);
    bkgnd = bkgnd.scaled(this->size());
    QPalette palette;
    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
    blur->setBlurHints(QGraphicsBlurEffect::PerformanceHint);
    blur->setBlurRadius(8);
    palette.setBrush(QPalette::Window, applyEffectToImage(bkgnd.toImage(), blur));
    this->setPalette(palette);

    buttonsPos = size().height() + 5;
    ui->widget->move(QPoint(ui->widget->x(), buttonsPos));

    initImages();
    QFuture _ = QtConcurrent::run(&MainWindow::AddPixs, this);


    width = (size().width()) / 5 - gap;

    currActivity = 0;
    currPos = 0;
    newPos = gamesVector.size() / 2;

    y_pos  = size().height() * 0.1f;
    height = size().height() * 0.8f;

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::animationFunc);
    timer->setTimerType(Qt::PreciseTimer);
    timer->start(17);
}

void MainWindow::paintEvent(QPaintEvent *) {
    QPainter painter(this);

    int vectorSize = gamesVector.size();
    if (vectorSize >= 5) {
        for (int i = currPos - 1; i < currPos + 5; i++) {
            int initialPos = (i - currPos) * (width + gap);
            int index = (i + vectorSize) % vectorSize;
            PaintGameRect(initialPos, &painter, index, newPos + 2 >= vectorSize ? (newPos + 2) - vectorSize: newPos + 2);
        }
    } else {
        for (int i = 0; i < (int) vectorSize; i++) {
            int initialPos = (5 - vectorSize) * (width + gap)/2 + (i) * (width + gap);
            PaintGameRect(initialPos, &painter, i, newPos);
        }
    }
}

void MainWindow::PaintGameRect(int initialPos, QPainter *painter, int i, int j) {
    QRect rect;
    rect = QRect(
        initialPos,
        y_pos,
        width,
        height
        );

    int move_x = 0;
    if (gamesVector.size() < 5) {
        // ts can prob  be simplified ngl
        move_x = ((5 - gamesVector.size()) * (width + gap)/2 + (i) * (width + gap)) - size().width()/2 + (width + gap)/2;
    }

    QRegion r(rect);
    painter->setClipRegion(r);

    painter->drawPixmap(move_x, 0, gamesVector[i].pixmap);

    if (i != j) {
        QPainterPath path;
        path.addRect(rect);
        QColor translucentColor(0, 0, 0, 128); // semi transparent black
        painter->setBrush(QBrush(translucentColor));
        painter->fillPath(path, painter->brush());
    }
}

void MainWindow::runProgram() {
    int i = newPos + 2 >= gamesVector.size() ? (newPos + 2) - gamesVector.size(): newPos + 2;
    QProcess::startDetached("cmd", {"/C", gamesVector[i].runnable.c_str()});
    exit(0);
}

void MainWindow::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Left) {
        newPos--;
        if (!timer->isActive()) timer->start();
        update();
    } else if (e->key() == Qt::Key_Right) {
        newPos++;
        if (!timer->isActive()) timer->start();
        update();
    } else if (e->key() == Qt::Key_Return) {
        runProgram();
    } else if (e->key() == Qt::Key_Backspace) {
        if (search.size() > 0) search.pop_back();
    }else {
        if (e->text() != "" && e->text().length() == 1) {
            search += e->text().toStdString();
        }
    }

    if (newPos < 0) {newPos = gamesVector.size() - 1; jump = true;}
    else if (newPos > (int) gamesVector.size() - 1) {newPos = 0; jump = true;}
}

void MainWindow::AddPixs() {
    // TODO: load the middle ones first

    for (auto& p : gamesVector) {
        QImage img(p.path.c_str());
        img = size().height() > size().width() ? img.scaledToHeight(size().height(), Qt::FastTransformation) : img.scaledToWidth(size().width(), Qt::SmoothTransformation);
        p.pixmap = QPixmap::fromImage(img);
    }
    completedLoading = true;
}

void MainWindow::initImages() {
    std::string baseFolder = "C:\\Users\\Deeptanshu\\Personal\\Games";

    std::ifstream jsonFile("C:\\Users\\Deeptanshu\\Documents\\killme.txt");
    std::string content;

    std::vector<std::string> allPathsInFile;

    if (!jsonFile.fail()) {
        while (std::getline(jsonFile, content)) {
            if (content == "") {continue;}

            allPathsInFile.push_back(content);

            std::string temp_1 = content.substr(content.find("::") + 2);
            std::string image = content.substr(0, content.find("::"));
            std::string runnable = temp_1.substr(0, content.find("::") - 2);
            std::string name = temp_1.substr(content.find("::"));

            gamesVector.push_back(Item(name, runnable, image));
        }
        jsonFile.close();
    } else {
        jsonFile.close();
        addFilesToList(baseFolder, allPathsInFile);
    }

}

void MainWindow::addFilesToList(std::string baseFolder, std::vector<std::string> allPathsInFile) {
    std::vector<std::string> toAdd;
    std::string image = "";
    std::string runnable = "";
    std::string dirname = "";

    for (auto& c : std::filesystem::directory_iterator(baseFolder)) {
        if (!c.is_directory()) {
            continue;
        }
        for (auto& p : std::filesystem::directory_iterator(c)) {
            if (!p.is_regular_file()) {
                continue;
            }

            std::string path = p.path().string();

            if (path.find("image.png") != std::string::npos || path.find("image.jpg") != std::string::npos) {
                image = path;
            } else if (path.find("run.bat") != std::string::npos) {
                runnable = path;
            }

            dirname = c.path().string().substr(c.path().string().find_last_of("\\") + 1);

            if (addToLists(image, runnable, dirname, &toAdd, &allPathsInFile)) {
                image = "";
                runnable = "";
            }

        }
    }

    addToLists(image, runnable, dirname, &toAdd, &allPathsInFile);

    std::ofstream outFile;
    outFile.open("C:\\Users\\Deeptanshu\\Documents\\killme.txt", std::ios_base::app);
    for (auto& s : toAdd) {
        outFile << s << "\n";
    }
    outFile.close();
}

bool MainWindow::addToLists(std::string image, std::string runnable, std::string dirname, std::vector<std::string> *toAdd, std::vector<std::string> *allPathsInFile) {
    if (image == "" || runnable == "") return false;
    std::string substr = image.substr(0, image.size() - 10);
    if (runnable.find(substr) != std::string::npos) {
        std::string fullName = image + "::" + runnable + "::" + dirname;
        if (std::find(allPathsInFile->begin(), allPathsInFile->end(), fullName) == allPathsInFile->end()) {
            toAdd->push_back(fullName);
            gamesVector.push_back(Item(dirname, runnable, image));
        }
        image = "";
        runnable = "";
        return true;
    }
    return false;
}

void MainWindow::animationFunc() {
    if (firstTime) {
        if (completedLoading)
            firstTime = false;
        update();
    }

    if (gamesVector.size() < 5) {return;}

    if (jump) {
        if (std::abs(currPos - (int) gamesVector.size() - 1) > std::abs(currPos - 0)) {
            currPos = gamesVector.size() + currPos;
        } else {
            currPos = 0 - (gamesVector.size() - currPos);
        }
        jump = false;
        update();
    } else {
        if (std::abs(currPos - newPos) > 0.001f) {
            currPos = lerp(currPos, newPos, 0.1);
            update();
        }

    }
    if (std::abs(currPos - newPos) <= 0.001f) {
        timer->stop();
    }
}

float MainWindow::lerp(float start, float end, float t) {
    float val = start + (end - start) * t;
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

// https://stackoverflow.com/questions/34065/how-to-read-a-value-from-the-windows-registry
std::wstring MainWindow::GetStringValueFromHKLM(const std::wstring& regSubKey, const std::wstring& regValue)
{
    size_t bufferSize = 0xFFF; // If too small, will be resized down below.
    std::wstring valueBuf; // Contiguous buffer since C++11.
    valueBuf.resize(bufferSize);
    auto cbData = static_cast<DWORD>(bufferSize * sizeof(wchar_t));
    auto rc = RegGetValueW(
        HKEY_CURRENT_USER,
        regSubKey.c_str(),
        regValue.c_str(),
        RRF_RT_REG_SZ,
        nullptr,
        static_cast<void*>(valueBuf.data()),
        &cbData
        );
    while (rc == ERROR_MORE_DATA)
    {
        // Get a buffer that is big enough.
        cbData /= sizeof(wchar_t);
        if (cbData > static_cast<DWORD>(bufferSize))
        {
            bufferSize = static_cast<size_t>(cbData);
        }
        else
        {
            bufferSize *= 2;
            cbData = static_cast<DWORD>(bufferSize * sizeof(wchar_t));
        }
        valueBuf.resize(bufferSize);
        rc = RegGetValueW(
            HKEY_LOCAL_MACHINE,
            regSubKey.c_str(),
            regValue.c_str(),
            RRF_RT_REG_SZ,
            nullptr,
            static_cast<void*>(valueBuf.data()),
            &cbData
            );
    }
    if (rc == ERROR_SUCCESS)
    {
        cbData /= sizeof(wchar_t);
        valueBuf.resize(static_cast<size_t>(cbData - 1)); // remove end null character
        return valueBuf;
    }
    else
    {
        throw std::runtime_error("Windows system error code: " + std::to_string(rc));
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
