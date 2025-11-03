#include <Item.h>

Item::Item(std::string name, std::string runnable, std::string path) {
    this->runnable = runnable;
    this->path = path;
    this->name = name;
}
Item::Item() {}
