#pragma once
#include <stdexcept>
#include <string>

// Базовый класс исключений для всего архива. Наследуется от std::runtime_error.
class ArchiveException : public std::runtime_error {
public:
    // Конструктор: принимает сообщение об ошибке и передаёт его в конструктор базового класса.
    explicit ArchiveException(const std::string& msg) : std::runtime_error(msg) {}
};

// Исключение, выбрасываемое при недопустимом имени ресурса (запрещённые символы).
class InvalidNameException : public ArchiveException {
public:
    // Конструктор: принимает сообщение и передаёт в конструктор ArchiveException.
    explicit InvalidNameException(const std::string& msg) : ArchiveException(msg) {}
};

// Исключение, выбрасываемое при попытке выполнить операцию без достаточных прав доступа.
class AccessDeniedException : public ArchiveException {
public:
    // Конструктор: принимает сообщение и передаёт в конструктор ArchiveException.
    explicit AccessDeniedException(const std::string& msg) : ArchiveException(msg) {}
};

// Исключение, выбрасываемое при ошибках сериализации/десериализации
// (например, неверное магическое число, ошибка открытия файла и т.д.).
class SerializationException : public ArchiveException {
public:
    // Конструктор: принимает сообщение и передаёт в конструктор ArchiveException.
    explicit SerializationException(const std::string& msg) : ArchiveException(msg) {}
};