#include "Utils.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace Utils {
    // Запись сообщения в журнал (файл history.log) с добавлением временной метки.
    void log(const std::string& message) {
        std::ofstream logFile("history.log", std::ios::app); // открываем файл для добавления в конец
        if (logFile.is_open()) {                             // если файл успешно открыт
            logFile << getCurrentTime() << " - " << message << std::endl; // пишем время и сообщение
        }
    }

    // Проверка имени на отсутствие запрещённых символов.
    bool isValidName(const std::string& name) {
        static std::regex forbidden(R"([\\/:*?"<>|])"); // регулярное выражение: запрещённые символы
        return !std::regex_search(name, forbidden);     // true, если ни одного запрещённого символа нет
    }

    // Проверка расширения файла: только латинские буквы и цифры, может быть пустым.
    bool isValidExtension(const std::string& ext) {
        if (ext.empty()) return true;                   // пустое расширение разрешено
        static std::regex pattern(R"(^[a-zA-Z0-9]+$)"); // только буквы и цифры от начала до конца
        return std::regex_match(ext, pattern);          // true, если соответствует шаблону
    }

    // Возвращает текущее время в виде строки "ГГГГ-ММ-ДД ЧЧ:ММ:СС".
    std::string getCurrentTime() {
        auto now = std::chrono::system_clock::now();    // текущий момент времени
        std::time_t t = std::chrono::system_clock::to_time_t(now); // преобразование в time_t
        std::tm tm;                                      // структура для разобранного времени
        localtime_s(&tm, &t);                            // безопасное преобразование в локальное время
        std::ostringstream oss;                          // поток для форматирования
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S"); // форматируем как "ГГГГ-ММ-ДД ЧЧ:ММ:СС"
        return oss.str();                                // возвращаем строку
    }

    // Преобразование строки "дд.мм.гггг" в структуру Date.
    Date parseDate(const std::string& str) {
        Date d;                                          // создаём пустую структуру
        sscanf_s(str.c_str(), "%d.%d.%d", &d.day, &d.month, &d.year); // безопасный разбор
        return d;                                        // возвращаем заполненную структуру
    }

    // Преобразование структуры Date в строку формата "дд.мм.гггг".
    std::string dateToString(const Date& date) {
        char buf[11];                                    // буфер для строки (10 символов + 1)
        sprintf_s(buf, "%02d.%02d.%04d", date.day, date.month, date.year); // форматируем
        return std::string(buf);                         // возвращаем как std::string
    }

    // Возвращает текущую дату в виде структуры Date (день, месяц, год).
    Date getCurrentDate() {
        auto now = std::chrono::system_clock::now();    // текущий момент
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm;
        localtime_s(&tm, &t);                           // локальное время
        Date d;
        d.day = tm.tm_mday;                             // день месяца
        d.month = tm.tm_mon + 1;                        // месяц (tm_mon от 0 до 11)
        d.year = tm.tm_year + 1900;                     // год (tm_year от 1900)
        return d;
    }

    // Оператор "меньше" для структуры Date (сравнение дат).
    bool Date::operator<(const Date& other) const {
        if (year != other.year) return year < other.year;      // сначала сравниваем год
        if (month != other.month) return month < other.month;  // затем месяц
        return day < other.day;                                // затем день
    }

    // Оператор "меньше или равно" для структуры Date.
    bool Date::operator<=(const Date& other) const {
        return *this < other || (year == other.year && month == other.month && day == other.day);
        // истина, если дата меньше другой ИЛИ все компоненты равны
    }
}