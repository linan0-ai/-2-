#pragma once
#include "Resource.h"
#include <vector>
#include <memory>
#include <functional>

// Класс Directory (каталог) наследует базовый класс Resource.
class Directory : public Resource {
private:
    // Вектор умных указателей на дочерние ресурсы (файлы и подкаталоги).
    // Используется unique_ptr для автоматического управления памятью.
    std::vector<std::unique_ptr<Resource>> children;

public:
    // Конструктор: принимает имя, уровень доступа и указатель на родителя.
    // Родитель по умолчанию nullptr.
    Directory(const std::string& name, AccessLevel level, Resource* parent = nullptr);

    // Возвращает тип ресурса (Directory). Переопределение виртуального метода.
    Type getType() const override { return Type::Directory; }

    // Возвращает общий размер каталога (сумму размеров всех вложенных файлов).
    // Переопределение виртуального метода.
    size_t getSize() const override;

    // Добавляет дочерний ресурс (перемещая умный указатель).
    void addChild(std::unique_ptr<Resource> child);

    // Удаляет дочерний ресурс по сырому указателю.
    void removeChild(Resource* child);

    // Находит дочерний ресурс по имени. Возвращает сырой указатель или nullptr.
    Resource* findChild(const std::string& name) const;

    // Возвращает константную ссылку на вектор детей (только для чтения).
    const std::vector<std::unique_ptr<Resource>>& getChildren() const { return children; }

    // Извлекает дочерний ресурс из вектора и возвращает unique_ptr (используется при перемещении).
    std::unique_ptr<Resource> detachChild(Resource* child);

    // Сортирует детей с помощью переданной функции-компаратора.
    void sortChildren(std::function<bool(const std::unique_ptr<Resource>&, const std::unique_ptr<Resource>&)> cmp);

    // Создаёт глубокую копию каталога (включая всех потомков).
    std::unique_ptr<Resource> clone() const override;

    // Сериализует каталог в бинарный поток (записывает тип, базовые поля, количество детей и каждого ребёнка).
    void serialize(std::ofstream& out) const override;

    // Десериализует дочерние элементы из потока (вызывается после базовой десериализации).
    void deserializeDirectory(std::ifstream& in);

    // Восстанавливает связи parent после загрузки из файла (рекурсивно для всей иерархии).
    void rebuildParentLinks(Resource* newParent);
};