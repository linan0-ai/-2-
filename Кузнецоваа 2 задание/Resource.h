#pragma once
#include <string>
#include <memory>
#include <vector>
#include "Exception.h"
#include "Utils.h"

// Ѕазовый абстрактный класс дл€ всех ресурсов (файлов и папок).
class Resource {
public:
    // ѕеречисление типов ресурсов: File (файл) или Directory (папка).
    enum class Type { File, Directory };
    // ѕеречисление уровней доступа: GUEST (гость), USER (пользователь), ADMIN (администратор).
    enum class AccessLevel { GUEST, USER, ADMIN };

protected:
    std::string name;               // им€ ресурса
    Utils::Date creationDate;       // дата создани€ ресурса
    AccessLevel accessLevel;        // уровень доступа к ресурсу
    Resource* parent;               // сырой указатель на родител€ (чтобы избежать циклических ссылок с unique_ptr)

public:
    //  онструктор: принимает им€, уровень доступа и указатель на родител€ (по умолчанию nullptr).
    Resource(const std::string& name, AccessLevel level, Resource* parent = nullptr);
    // ¬иртуальный деструктор по умолчанию (дл€ корректного удалени€ производных классов).
    virtual ~Resource() = default;

    // √еттеры и сеттеры
    std::string getName() const;                    // возвращает им€ ресурса
    Utils::Date getCreationDate() const;            // возвращает дату создани€
    AccessLevel getAccessLevel() const;             // возвращает уровень доступа
    Resource* getParent() const;                    // возвращает указатель на родител€
    void setParent(Resource* newParent);            // устанавливает нового родител€
    void setAccessLevel(AccessLevel level);         // устанавливает уровень доступа

    // „исто виртуальные методы (об€зательны дл€ переопределени€ в производных классах)
    virtual Type getType() const = 0;               // возвращает тип ресурса (File/Directory)
    virtual size_t getSize() const = 0;             // возвращает размер (дл€ файла - его размер, дл€ папки - суммарный)
    virtual std::unique_ptr<Resource> clone() const = 0; // создаЄт глубокую копию ресурса

    // —ериализаци€
    virtual void serialize(std::ofstream& out) const = 0; // записывает ресурс в бинарный поток (чисто виртуальный)
    static std::unique_ptr<Resource> deserialize(std::ifstream& in); // статический метод, читает ресурс из потока

protected:
    // ¬спомогательные методы дл€ сериализации/десериализации общих полей
    void serializeBase(std::ofstream& out) const;   // записывает name, creationDate, accessLevel
    void deserializeBase(std::ifstream& in);        // читает name, creationDate, accessLevel
};