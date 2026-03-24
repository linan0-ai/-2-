#include "Resource.h"
#include "File.h"
#include "Directory.h"
#include <fstream>
#include <iostream>

//  онструктор базового класса Resource: инициализирует им€, дату создани€ (текуща€),
// уровень доступа и указатель на родител€. ≈сли им€ недопустимо, выбрасывает исключение.
Resource::Resource(const std::string& name, AccessLevel level, Resource* parent)
    : name(name), creationDate(Utils::getCurrentDate()), accessLevel(level), parent(parent) {
    if (!Utils::isValidName(name))                // проверка имени на допустимые символы
        throw InvalidNameException("Invalid name: " + name); // выброс исключени€ при ошибке
}

// ¬озвращает им€ ресурса (константна€ ссылка).
std::string Resource::getName() const { return name; }

// ¬озвращает дату создани€ ресурса.
Utils::Date Resource::getCreationDate() const { return creationDate; }

// ¬озвращает уровень доступа ресурса.
Resource::AccessLevel Resource::getAccessLevel() const { return accessLevel; }

// ¬озвращает указатель на родител€ (может быть nullptr).
Resource* Resource::getParent() const { return parent; }

// ”станавливает нового родител€.
void Resource::setParent(Resource* newParent) { parent = newParent; }

// ”станавливает уровень доступа.
void Resource::setAccessLevel(AccessLevel level) { accessLevel = level; }

// —ериализует общие пол€ (им€, дату, уровень доступа) в бинарный поток.
void Resource::serializeBase(std::ofstream& out) const {
    size_t len = name.size();                     // длина имени
    out.write(reinterpret_cast<const char*>(&len), sizeof(len)); // запись длины
    out.write(name.c_str(), len);                 // запись имени (без завершающего нул€)

    // запись дн€, мес€ца, года даты создани€
    out.write(reinterpret_cast<const char*>(&creationDate.day), sizeof(creationDate.day));
    out.write(reinterpret_cast<const char*>(&creationDate.month), sizeof(creationDate.month));
    out.write(reinterpret_cast<const char*>(&creationDate.year), sizeof(creationDate.year));

    int levelInt = static_cast<int>(accessLevel); // преобразуем enum в int
    out.write(reinterpret_cast<const char*>(&levelInt), sizeof(levelInt)); // запись уровн€ доступа
}

// ƒесериализует общие пол€ из бинарного потока.
void Resource::deserializeBase(std::ifstream& in) {
    size_t len;
    in.read(reinterpret_cast<char*>(&len), sizeof(len)); // читаем длину имени
    name.resize(len);                                   // выдел€ем пам€ть под им€
    in.read(&name[0], len);                             // читаем им€

    // читаем день, мес€ц, год даты создани€
    in.read(reinterpret_cast<char*>(&creationDate.day), sizeof(creationDate.day));
    in.read(reinterpret_cast<char*>(&creationDate.month), sizeof(creationDate.month));
    in.read(reinterpret_cast<char*>(&creationDate.year), sizeof(creationDate.year));

    int levelInt;
    in.read(reinterpret_cast<char*>(&levelInt), sizeof(levelInt)); // читаем уровень доступа
    accessLevel = static_cast<AccessLevel>(levelInt);              // преобразуем обратно в enum
    // parent будет установлен позже (при восстановлении иерархии)
}

// —татический метод: десериализует объект из потока, определ€€ его тип (File/Directory)
// и создава€ соответствующий экземпл€р.
std::unique_ptr<Resource> Resource::deserialize(std::ifstream& in) {
    int typeInt;
    in.read(reinterpret_cast<char*>(&typeInt), sizeof(typeInt)); // читаем идентификатор типа
    Type type = static_cast<Type>(typeInt);                     // преобразуем в enum Type

    std::unique_ptr<Resource> res;                              // умный указатель на ресурс
    if (type == Type::File) {
        // создаЄм временный файл с пустыми параметрами
        res = std::make_unique<File>("", AccessLevel::GUEST, nullptr);
    }
    else {
        // создаЄм временную директорию
        res = std::make_unique<Directory>("", AccessLevel::GUEST, nullptr);
    }
    res->deserializeBase(in);                                   // десериализуем общие пол€
    if (type == Type::File) {
        // десериализуем специфичные пол€ файла
        dynamic_cast<File*>(res.get())->deserializeFile(in);
    }
    else {
        // десериализуем специфичные пол€ директории (детей)
        dynamic_cast<Directory*>(res.get())->deserializeDirectory(in);
    }
    return res;                                                 // возвращаем готовый объект
}