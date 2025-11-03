#ifndef ITEM_H
#define ITEM_H

#include <QMainWindow>
#include <windows.h>

class Item {
public:
    QPixmap pixmap;
    std::string path;
    std::string runnable;
    std::string name;
    Item(std::string name, std::string runnable, std::string path);
    Item();
};

#endif // ITEM_H
