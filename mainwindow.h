#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <windows.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QTimer *timer;
public slots:
    void animationFunc();
private:
    Ui::MainWindow *ui;
    QPixmap pix;

    const int gap = 10;
    std::vector<QPixmap> pixList;
    std::vector<std::string> pixPathsList;
    std::vector<QString> runnablesList;
    std::vector<std::string> namesList;

    bool playSelected = true;

    int width;

    int originalWidth;

    int oldPos;
    float currPos;
    int newPos;

    int currActivity; // 0 = scrolling, 1 = loading

    bool jump = false;
    bool firstTime = true;
    bool completedLoading = false;

    std::string search;

    int buttonsPos;

    void AddPixs();
    void initImages();
    float lerp(float start, float end, float t);
    void runProgram();
    void setButtonSelect();
    void PaintGameRect(int initial_pos, QPainter *painter, int i, int j, int k = 0);
    bool addToLists(std::string image, std::string runnable, std::string dirname, std::vector<std::string> *toAdd, std::vector<std::string> *allPathsInFile);
    void addFilesToList(std::string baseFolder, std::vector<std::string> allPathsInFile);
    QImage applyEffectToImage(QImage src, QGraphicsEffect *effect, int extent = 0);

    std::wstring GetStringValueFromHKLM(const std::wstring& regSubKey, const std::wstring& regValue);
protected:
    void paintEvent(QPaintEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
};

#endif // MAINWINDOW_H
