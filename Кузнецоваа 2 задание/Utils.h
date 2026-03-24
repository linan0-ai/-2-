#pragma once
#include <string>
#include <ctime>
#include <regex>
#include <fstream>

// Пространство имён Utils содержит вспомогательные утилитарные функции.
namespace Utils {
    // Журналирование: записывает сообщение в файл history.log с временной меткой.
    void log(const std::string& message);

    // Проверка имени на допустимые символы с использованием регулярного выражения.
    // Запрещены символы: \ / : * ? " < > |
    bool isValidName(const std::string& name);

    // Проверка расширения файла: только латинские буквы и цифры, может быть пустым.
    bool isValidExtension(const std::string& ext);

    // Получить текущее время в виде строки формата "ГГГГ-ММ-ДД ЧЧ:ММ:СС".
    std::string getCurrentTime();

    // Структура для хранения даты (день, месяц, год).
    struct Date {
        int day;      // день (1-31)
        int month;    // месяц (1-12)
        int year;     // год (например, 2025)

        // Оператор "меньше" для сравнения дат.
        bool operator<(const Date& other) const;
        // Оператор "меньше или равно" для сравнения дат.
        bool operator<=(const Date& other) const;
    };

    // Преобразует строку формата "дд.мм.гггг" в структуру Date.
    Date parseDate(const std::string& str);

    // Преобразует структуру Date в строку формата "дд.мм.гггг".
    std::string dateToString(const Date& date);

    // Возвращает текущую дату в виде структуры Date.
    Date getCurrentDate();
}