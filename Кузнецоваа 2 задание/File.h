#pragma once
#include "Resource.h"
#include <string>

// Класс File (файл) наследует базовый класс Resource.
class File : public Resource {
private:
    std::string extension;   // расширение файла (например, "txt", "cpp")
    size_t size;             // размер файла в байтах
    std::string content;     // содержимое файла (текст), используется для поиска по содержимому

public:
    // Конструктор: принимает имя, уровень доступа, родителя, расширение, размер и содержимое.
    // Параметры по умолчанию позволяют создавать объект с пустыми значениями.
    File(const std::string& name, AccessLevel level, Resource* parent = nullptr,
        const std::string& ext = "", size_t sz = 0, const std::string& cont = "");

    // Возвращает тип ресурса (File). Переопределение виртуального метода.
    Type getType() const override { return Type::File; }

    // Возвращает размер файла. Переопределение виртуального метода.
    size_t getSize() const override { return size; }

    // Возвращает расширение файла.
    std::string getExtension() const { return extension; }

    // Возвращает содержимое файла (для поиска по тексту).
    std::string getContent() const { return content; }

    // Устанавливает расширение файла (с проверкой допустимых символов).
    void setExtension(const std::string& ext);

    // Устанавливает размер файла.
    void setSize(size_t sz);

    // Устанавливает содержимое файла.
    void setContent(const std::string& cont);

    // Создаёт глубокую копию файла (с новой датой создания).
    std::unique_ptr<Resource> clone() const override;

    // Сериализует файл в бинарный поток: записывает тип, базовые поля,
    // расширение, размер и содержимое.
    void serialize(std::ofstream& out) const override;

    // Десериализует специфичные для файла поля из потока
    // (вызывается после базовой десериализации).
    void deserializeFile(std::ifstream& in);
};