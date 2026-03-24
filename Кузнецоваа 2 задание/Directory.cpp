#include "Directory.h"
#include <algorithm>
#include <fstream>

// Конструктор директории: вызывает конструктор базового класса Resource,
// передавая имя, уровень доступа и родителя. Тело пустое, т.к. инициализация уже выполнена в списке инициализации.
Directory::Directory(const std::string& name, AccessLevel level, Resource* parent)
    : Resource(name, level, parent) {
}

// Возвращает общий размер директории (сумму размеров всех файлов внутри,
// включая вложенные директории, рекурсивно).
size_t Directory::getSize() const {
    size_t total = 0;                      // начальное значение суммы
    for (const auto& child : children) {   // перебираем всех дочерних объектов
        total += child->getSize();         // добавляем размер каждого потомка (рекурсивно)
    }
    return total;                          // возвращаем результат
}

// Добавляет дочерний объект в директорию.
void Directory::addChild(std::unique_ptr<Resource> child) {
    child->setParent(this);                // устанавливаем текущую директорию как родителя для потомка
    children.push_back(std::move(child));  // перемещаем умный указатель в вектор
}

// Удаляет дочерний объект по сырому указателю.
void Directory::removeChild(Resource* child) {
    // Ищем итератор, указывающий на элемент с таким же адресом
    auto it = std::find_if(children.begin(), children.end(),
        [child](const std::unique_ptr<Resource>& ptr) { return ptr.get() == child; });
    if (it != children.end()) {            // если нашли
        children.erase(it);                // удаляем элемент из вектора (автоматически освобождается память)
    }
}

// Находит дочерний объект по имени. Возвращает сырой указатель или nullptr.
Resource* Directory::findChild(const std::string& name) const {
    for (const auto& child : children) {   // перебираем всех детей
        if (child->getName() == name)      // если имя совпадает
            return child.get();            // возвращаем указатель
    }
    return nullptr;                        // не найдено
}

// Извлекает дочерний объект из вектора и возвращает его unique_ptr.
// Используется при перемещении ресурса из одной директории в другую.
std::unique_ptr<Resource> Directory::detachChild(Resource* child) {
    // Ищем элемент по указателю
    auto it = std::find_if(children.begin(), children.end(),
        [child](const std::unique_ptr<Resource>& ptr) {
            return ptr.get() == child;
        });
    if (it != children.end()) {            // если элемент найден
        auto ptr = std::move(*it);         // перемещаем unique_ptr из вектора
        children.erase(it);                // удаляем итератор (элемент уже перемещён)
        return ptr;                        // возвращаем извлечённый указатель
    }
    return nullptr;                        // не найден — возвращаем пустой указатель
}

// Сортирует дочерние элементы с помощью переданной функции-компаратора.
void Directory::sortChildren(std::function<bool(const std::unique_ptr<Resource>&, const std::unique_ptr<Resource>&)> cmp) {
    std::sort(children.begin(), children.end(), cmp); // стандартная сортировка по заданному критерию
}

// Создаёт глубокую копию текущей директории (включая всех потомков).
std::unique_ptr<Resource> Directory::clone() const {
    // Создаём новую директорию с тем же именем и уровнем доступа, родитель временно nullptr
    auto newDir = std::make_unique<Directory>(name, accessLevel, nullptr);
    newDir->creationDate = Utils::getCurrentDate();   // дата создания — текущая (требование ТЗ)
    for (const auto& child : children) {              // перебираем всех детей
        newDir->addChild(child->clone());             // рекурсивно клонируем каждого и добавляем в новую директорию
    }
    return newDir;                                    // возвращаем умный указатель на копию
}

// Сериализует директорию в бинарный поток (ofstream).
void Directory::serialize(std::ofstream& out) const {
    int typeInt = static_cast<int>(Type::Directory); // записываем идентификатор типа (Directory)
    out.write(reinterpret_cast<const char*>(&typeInt), sizeof(typeInt));
    serializeBase(out);                               // записываем базовые поля (имя, дата, уровень доступа)

    size_t childCount = children.size();              // количество детей
    out.write(reinterpret_cast<const char*>(&childCount), sizeof(childCount));
    for (const auto& child : children) {              // рекурсивно сериализуем каждого ребёнка
        child->serialize(out);
    }
}

// Десериализует дочерние элементы директории из бинарного потока (ifstream).
void Directory::deserializeDirectory(std::ifstream& in) {
    size_t childCount;                                // читаем количество детей
    in.read(reinterpret_cast<char*>(&childCount), sizeof(childCount));
    for (size_t i = 0; i < childCount; ++i) {
        auto child = Resource::deserialize(in);       // десериализуем очередного ребёнка (тип определяется автоматически)
        addChild(std::move(child));                   // добавляем его в текущую директорию
    }
}

// Восстанавливает связи parent после десериализации.
// newParent — родитель текущей директории (для корня передаётся nullptr).
void Directory::rebuildParentLinks(Resource* newParent) {
    setParent(newParent);                             // устанавливаем родителя для текущей директории
    for (auto& child : children) {                    // для каждого ребёнка
        if (child->getType() == Type::Directory) {    // если ребёнок — директория
            // рекурсивно вызываем для него этот же метод, передавая this как нового родителя
            dynamic_cast<Directory*>(child.get())->rebuildParentLinks(this);
        }
        else {                                      // иначе (ребёнок — файл)
            child->setParent(this);                   // просто устанавливаем родителя
        }
    }
}