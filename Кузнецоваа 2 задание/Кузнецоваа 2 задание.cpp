#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <functional>
#include <sstream>

#include "Resource.h"
#include "File.h"
#include "Directory.h"
#include "Utils.h"

using namespace std;

// Глобальные переменные (для упрощения, но можно обернуть в класс)
unique_ptr<Directory> root;
Resource::AccessLevel currentUserLevel = Resource::AccessLevel::ADMIN; // для теста

// Прототипы функций
void showMenu();
void createResource();
void deleteResource();
void moveResource();
void copyResource();
void search();
void calculateSize();
void globalAudit();
void filterByDate();
void visualizeTree(Resource* node, int depth = 0, const string& prefix = "");
void saveToBinary();
void loadFromBinary();
void exportToCSV();
void sortResources(); // сортировка функторами
void searchByContent(); // дополнительная функция

// Проверка прав доступа
bool checkAccess(Resource* res, const string& operation) {
    // Если уровень текущего пользователя >= уровня ресурса — доступ разрешён
    if (currentUserLevel >= res->getAccessLevel()) return true;
    // Иначе — логируем отказ и выводим сообщение пользователю
    Utils::log("Access denied: " + operation + " on " + res->getName());
    cout << "Access denied! Your level is lower than required.\n";
    return false;
}

int main() {
    setlocale(LC_ALL, "Russian"); // Устанавливаем русскую локаль для вывода
    try {
        // Создаём корневую папку с уровнем доступа ADMIN
        root = make_unique<Directory>("root", Resource::AccessLevel::ADMIN, nullptr);
        Utils::log("Program started");

        int choice;
        do {
            showMenu();            // Показываем меню
            cin >> choice;         // Читаем выбор пользователя
            cin.ignore();          // Очищаем буфер после ввода числа

            switch (choice) {
            case 1: createResource(); break;
            case 2: deleteResource(); break;
            case 3: moveResource(); break;
            case 4: copyResource(); break;
            case 5: search(); break;
            case 6: calculateSize(); break;
            case 7: globalAudit(); break;
            case 8: filterByDate(); break;
            case 9: visualizeTree(root.get()); break;
            case 10: saveToBinary(); break;
            case 11: loadFromBinary(); break;
            case 12: exportToCSV(); break;
            case 13: sortResources(); break;
            case 14: searchByContent(); break;
            case 0: Utils::log("Program ended"); break;
            default: cout << "Invalid choice.\n";
            }
        } while (choice != 0);

    }
    catch (const ArchiveException& e) {
        // Перехватываем наши пользовательские исключения
        cerr << "Error: " << e.what() << endl;
        Utils::log(string("Exception: ") + e.what());
        return 1;
    }
    catch (const exception& e) {
        // Перехватываем все остальные исключения
        cerr << "Unexpected error: " << e.what() << endl;
        Utils::log(string("Unexpected: ") + e.what());
        return 1;
    }
    return 0;
}

void showMenu() {
    cout << "\n=== Виртуальный архив ===\n";
    cout << "1. Создать ресурс (файл/каталог)\n";
    cout << "2. Удалить ресурс\n";
    cout << "3. Переместить ресурс\n";
    cout << "4. Скопировать ресурс\n";
    cout << "5. Искать по маске\n";
    cout << "6. Рассчитать размер каталога\n";
    cout << "7. Глобальный аудит\n";
    cout << "8. Фильтровать по диапазону дат\n";
    cout << "9. Визуализируем дерево\n";
    cout << "10. Сохраняем в двоичный файл\n";
    cout << "11. Загружаем из двоичного файла\n";
    cout << "12. Экспортируем в CSV\n";
    cout << "13. Сортировка дочерних элементов каталога\n";
    cout << "14. Поиск по содержимому (тексту в файлах)\n";
    cout << "0. Выход\n";
    cout << "Выбор: ";
}

void createResource() {
    string parentPath, name, type;
    // Запрашиваем путь к родительской папке
    cout << "Введите родительский путь (например, root/subdir) или '.' for root: ";
    getline(cin, parentPath);
    Resource* parent = root.get(); // начинаем с корня

    if (parentPath != ".") {
        // Разбиваем путь на части по '/'
        vector<string> parts;
        size_t pos = 0;
        while ((pos = parentPath.find('/')) != string::npos) {
            parts.push_back(parentPath.substr(0, pos));
            parentPath.erase(0, pos + 1);
        }
        parts.push_back(parentPath);
        // Идём по частям пути, спускаясь в дочерние папки
        for (const auto& part : parts) {
            if (part.empty()) continue;
            if (parent->getType() != Resource::Type::Directory) {
                cout << "Неверный путь: это не каталог.\n";
                return;
            }
            Resource* child = dynamic_cast<Directory*>(parent)->findChild(part);
            if (!child) {
                cout << "Путь не найден.\n";
                return;
            }
            parent = child;
        }
    }
    // Проверяем, что родитель — папка
    if (parent->getType() != Resource::Type::Directory) {
        cout << "Родительский каталог не является директорией.\n";
        return;
    }

    // Ввод имени
    cout << "Введите имя: ";
    getline(cin, name);
    if (!Utils::isValidName(name)) {
        cout << "Недопустимое имя.\n";
        return;
    }

    // Ввод типа
    cout << "Тип (файл/каталог): ";
    getline(cin, type);

    if (type == "Файл") {
        // Создание файла
        string ext, content;
        size_t size;
        cout << "Введите добавочный номер (без точки): ";
        getline(cin, ext);
        if (!Utils::isValidExtension(ext)) {
            cout << "Недопустимый добавочный номер.\n";
            return;
        }
        cout << "Введите размер (байт): ";
        cin >> size;
        cin.ignore();
        cout << "Введите содержимое (для поиска): ";
        getline(cin, content);

        auto file = make_unique<File>(name, currentUserLevel, parent, ext, size, content);
        dynamic_cast<Directory*>(parent)->addChild(move(file));
        Utils::log("Созданный файл: " + name);
    }
    else if (type == "dir") {
        // Создание папки
        auto dir = make_unique<Directory>(name, currentUserLevel, parent);
        dynamic_cast<Directory*>(parent)->addChild(move(dir));
        Utils::log("Созданный каталог: " + name);
    }
    else {
        cout << "Недопустимый тип.\n";
    }
}

void deleteResource() {
    string path;
    cout << "Введите путь к ресурсу: ";
    getline(cin, path);
    Resource* res = root.get();
    vector<string> parts;
    size_t pos = 0;
    // Разбиваем путь на компоненты
    while ((pos = path.find('/')) != string::npos) {
        parts.push_back(path.substr(0, pos));
        path.erase(0, pos + 1);
    }
    parts.push_back(path);

    for (size_t i = 0; i < parts.size(); ++i) {
        const auto& part = parts[i];
        if (part.empty()) continue;
        if (res->getType() != Resource::Type::Directory) {
            cout << "Недопустимый путь.\n";
            return;
        }
        Resource* child = dynamic_cast<Directory*>(res)->findChild(part);
        if (!child) {
            cout << "Ресурс не найден.\n";
            return;
        }
        if (i == parts.size() - 1) {
            // Дошли до последнего компонента — это удаляемый ресурс
            if (!checkAccess(child, "удалить")) return;
            if (res->getType() == Resource::Type::Directory) {
                dynamic_cast<Directory*>(res)->removeChild(child);
                Utils::log("Удаленный: " + child->getName());
                cout << "Удаленный.\n";
            }
        }
        else {
            res = child; // идём глубже
        }
    }
}
void moveResource() {
    string srcPath, dstPath;
    cout << "Введите исходный путь: ";
    getline(cin, srcPath);
    cout << "Введите путь к целевому каталогу: ";
    getline(cin, dstPath);

    Resource* src = nullptr;
    Resource* srcParent = nullptr;
    Resource* dst = nullptr;

    // Поиск источника (src)
    Resource* cur = root.get();
    vector<string> parts;
    size_t pos = 0;
    string tmp = srcPath;
    while ((pos = tmp.find('/')) != string::npos) {
        parts.push_back(tmp.substr(0, pos));
        tmp.erase(0, pos + 1);
    }
    parts.push_back(tmp);
    for (size_t i = 0; i < parts.size(); ++i) {
        if (cur->getType() != Resource::Type::Directory) break;
        Resource* child = dynamic_cast<Directory*>(cur)->findChild(parts[i]);
        if (!child) break;
        if (i == parts.size() - 1) {
            src = child;
            srcParent = cur;
        }
        else {
            cur = child;
        }
    }
    if (!src) {
        cout << "Источник не найден.\n";
        return;
    }

    // Поиск назначения (dst)
    cur = root.get();
    parts.clear();
    tmp = dstPath;
    while ((pos = tmp.find('/')) != string::npos) {
        parts.push_back(tmp.substr(0, pos));
        tmp.erase(0, pos + 1);
    }
    parts.push_back(tmp);
    for (size_t i = 0; i < parts.size(); ++i) {
        if (cur->getType() != Resource::Type::Directory) break;
        Resource* child = dynamic_cast<Directory*>(cur)->findChild(parts[i]);
        if (!child) break;
        if (i == parts.size() - 1) {
            dst = child;
        }
        else {
            cur = child;
        }
    }
    if (!dst || dst->getType() != Resource::Type::Directory) {
        cout << "Пункт назначения не найден или это не каталог.\n";
        return;
    }

    // Проверка прав доступа
    if (!checkAccess(src, "переместить") || !checkAccess(dst, "переместить")) return;

    // Извлечение источника из родителя и добавление в назначение
    if (srcParent && srcParent->getType() == Resource::Type::Directory) {
        auto srcDir = dynamic_cast<Directory*>(srcParent);
        auto ptr = srcDir->detachChild(src); // извлекаем unique_ptr
        if (ptr) {
            dynamic_cast<Directory*>(dst)->addChild(std::move(ptr));
            Utils::log("Перемещено " + src->getName() + " в " + dst->getName());
            cout << "Перемещено.\n";
        }
        else {
            cout << "Ошибка отсоединения.\n";
        }
    }
}

void copyResource() {
    string srcPath, dstPath;
    cout << "Введите путь к источнику: ";
    getline(cin, srcPath);
    cout << "Введите путь к папке назначения: ";
    getline(cin, dstPath);

    Resource* src = nullptr;
    Resource* dst = nullptr;

    // Поиск источника (аналогично moveResource, но без родителя)
    Resource* cur = root.get();
    vector<string> parts;
    size_t pos = 0;
    string tmp = srcPath;
    while ((pos = tmp.find('/')) != string::npos) {
        parts.push_back(tmp.substr(0, pos));
        tmp.erase(0, pos + 1);
    }
    parts.push_back(tmp);
    for (size_t i = 0; i < parts.size(); ++i) {
        if (cur->getType() != Resource::Type::Directory) break;
        Resource* child = dynamic_cast<Directory*>(cur)->findChild(parts[i]);
        if (!child) break;
        if (i == parts.size() - 1) {
            src = child;
        }
        else {
            cur = child;
        }
    }
    if (!src) {
        cout << "Источник не найден.\n";
        return;
    }

    // Поиск назначения
    cur = root.get();
    parts.clear();
    tmp = dstPath;
    while ((pos = tmp.find('/')) != string::npos) {
        parts.push_back(tmp.substr(0, pos));
        tmp.erase(0, pos + 1);
    }
    parts.push_back(tmp);
    for (size_t i = 0; i < parts.size(); ++i) {
        if (cur->getType() != Resource::Type::Directory) break;
        Resource* child = dynamic_cast<Directory*>(cur)->findChild(parts[i]);
        if (!child) break;
        if (i == parts.size() - 1) {
            dst = child;
        }
        else {
            cur = child;
        }
    }
    if (!dst || dst->getType() != Resource::Type::Directory) {
        cout << "Назначение не найдено или не является папкой.\n";
        return;
    }

    if (!checkAccess(src, "копировать") || !checkAccess(dst, "копировать")) return;

    // Создаём клон и добавляем
    auto clone = src->clone();
    dynamic_cast<Directory*>(dst)->addChild(move(clone));
    Utils::log("Скопировано " + src->getName() + " в " + dst->getName());
    cout << "Скопировано.\n";
}

void search() {
    string nameMask, extMask;
    cout << "Введите маску имени (например, *файл* или оставьте пустым для любого): ";
    getline(cin, nameMask);
    cout << "Введите маску расширения (например, txt или оставьте пустым для любого): ";
    getline(cin, extMask);

    // Рекурсивная функция обхода дерева
    function<void(Resource*)> searchRec = [&](Resource* node) {
        if (node->getType() == Resource::Type::File) {
            File* f = dynamic_cast<File*>(node);
            bool nameMatch = nameMask.empty() || f->getName().find(nameMask) != string::npos;
            bool extMatch = extMask.empty() || f->getExtension() == extMask;
            if (nameMatch && extMatch) {
                cout << "Файл: " << f->getName() << "." << f->getExtension()
                    << " (размер: " << f->getSize() << ")\n";
            }
        }
        else {
            Directory* d = dynamic_cast<Directory*>(node);
            for (const auto& child : d->getChildren()) {
                searchRec(child.get()); // рекурсивный вызов для каждого потомка
            }
        }
        };
    searchRec(root.get());
    Utils::log("Поиск выполнен");
}

void calculateSize() {
    string path;
    cout << "Введите путь к папке: ";
    getline(cin, path);
    Resource* dir = root.get();
    // Аналогичный разбор пути
    vector<string> parts;
    size_t pos = 0;
    string tmp = path;
    while ((pos = tmp.find('/')) != string::npos) {
        parts.push_back(tmp.substr(0, pos));
        tmp.erase(0, pos + 1);
    }
    parts.push_back(tmp);
    for (const auto& part : parts) {
        if (part.empty()) continue;
        if (dir->getType() != Resource::Type::Directory) {
            cout << "Некорректный путь.\n";
            return;
        }
        Resource* child = dynamic_cast<Directory*>(dir)->findChild(part);
        if (!child) {
            cout << "Не найдено.\n";
            return;
        }
        dir = child;
    }
    if (dir->getType() != Resource::Type::Directory) {
        cout << "Не является папкой.\n";
        return;
    }
    size_t size = dir->getSize(); // рекурсивная сумма всех файлов внутри
    cout << "Общий размер: " << size << " байт.\n";
    Utils::log("Размер вычислен для " + path);
}

void globalAudit() {
    size_t fileCount = 0, dirCount = 0;
    size_t totalSize = 0;

    // Рекурсивный обход
    function<void(Resource*)> auditRec = [&](Resource* node) {
        if (node->getType() == Resource::Type::File) {
            fileCount++;
            totalSize += node->getSize();
        }
        else {
            dirCount++;
            for (const auto& child : dynamic_cast<Directory*>(node)->getChildren()) {
                auditRec(child.get());
            }
        }
        };
    auditRec(root.get());

    double avgSize = (fileCount > 0) ? static_cast<double>(totalSize) / fileCount : 0;
    cout << "Папок: " << dirCount << "\n";
    cout << "Файлов: " << fileCount << "\n";
    cout << "Общий размер: " << totalSize << " байт\n";
    cout << "Средний размер файла: " << avgSize << " байт\n";
    Utils::log("Аудит выполнен");
}

void filterByDate() {
    string startStr, endStr;
    cout << "Введите начальную дату (дд.мм.гггг): ";
    getline(cin, startStr);
    cout << "Введите конечную дату (дд.мм.гггг): ";
    getline(cin, endStr);
    Utils::Date start = Utils::parseDate(startStr);
    Utils::Date end = Utils::parseDate(endStr);

    // Рекурсивный обход
    function<void(Resource*)> filterRec = [&](Resource* node) {
        if (start <= node->getCreationDate() && node->getCreationDate() <= end) {
            cout << (node->getType() == Resource::Type::File ? "Файл: " : "Папка: ")
                << node->getName() << " (создан: " << Utils::dateToString(node->getCreationDate()) << ")\n";
        }
        if (node->getType() == Resource::Type::Directory) {
            for (const auto& child : dynamic_cast<Directory*>(node)->getChildren()) {
                filterRec(child.get());
            }
        }
        };
    filterRec(root.get());
    Utils::log("Фильтрация по дате выполнена");
}

void visualizeTree(Resource* node, int depth, const string& prefix) {
    if (depth == 0) {
        cout << node->getName() << "\n"; // корень выводим без отступа
    }
    if (node->getType() == Resource::Type::Directory) {
        const auto& children = dynamic_cast<Directory*>(node)->getChildren();
        for (size_t i = 0; i < children.size(); ++i) {
            bool last = (i == children.size() - 1); // последний элемент в списке
            cout << prefix << (last ? "└── " : "├── ");
            cout << children[i]->getName();
            if (children[i]->getType() == Resource::Type::File) {
                cout << "." << dynamic_cast<File*>(children[i].get())->getExtension();
            }
            cout << "\n";
            if (children[i]->getType() == Resource::Type::Directory) {
                // Рекурсивный вызов с увеличенной глубиной и изменённым префиксом
                visualizeTree(children[i].get(), depth + 1, prefix + (last ? "    " : "│   "));
            }
        }
    }
}

void saveToBinary() {
    const uint32_t MAGIC = 0xDEADBEEF; // магическое число для проверки целостности
    ofstream out("archive.dat", ios::binary); // открываем в бинарном режиме
    if (!out) {
        throw SerializationException("Не удалось открыть archive.dat для записи");
    }
    out.write(reinterpret_cast<const char*>(&MAGIC), sizeof(MAGIC)); // пишем магию
    root->serialize(out); // сериализуем всё дерево
    out.close();
    Utils::log("Сохранено в двоичном формате");
    cout << "Сохранено.\n";
}
void loadFromBinary() {
    ifstream in("archive.dat", ios::binary);
    if (!in) {
        throw SerializationException("Не удалось открыть archive.dat для чтения");
    }
    uint32_t magic;
    in.read(reinterpret_cast<char*>(&magic), sizeof(magic)); // читаем магию
    if (magic != 0xDEADBEEF) {
        throw SerializationException("Неверное магическое число, файл повреждён");
    }
    auto newRoot = Resource::deserialize(in); // десериализуем дерево
    if (!newRoot || newRoot->getType() != Resource::Type::Directory) {
        throw SerializationException("Корневой элемент не является папкой");
    }
    root = unique_ptr<Directory>(dynamic_cast<Directory*>(newRoot.release()));
    root->rebuildParentLinks(nullptr); // восстанавливаем связи parent у всех объектов
    Utils::log("Загружено из бинарного файла");
    cout << "Загружено.\n";
}

void exportToCSV() {
    ofstream csv("archive.csv");
    if (!csv) {
        cout << "Не удалось создать CSV-файл.\n";
        return;
    }
    csv << "Путь,Тип,Размер,Дата\n"; // заголовки

    // Рекурсивная функция для обхода и записи
    function<void(Resource*, string)> exportRec = [&](Resource* node, string path) {
        string fullPath = path + "/" + node->getName();
        if (node->getType() == Resource::Type::File) {
            File* f = dynamic_cast<File*>(node);
            csv << fullPath << "." << f->getExtension() << ",Файл,"
                << f->getSize() << "," << Utils::dateToString(node->getCreationDate()) << "\n";
        }
        else {
            csv << fullPath << ",Папка,0," << Utils::dateToString(node->getCreationDate()) << "\n";
            for (const auto& child : dynamic_cast<Directory*>(node)->getChildren()) {
                exportRec(child.get(), fullPath);
            }
        }
        };
    exportRec(root.get(), "");
    csv.close();
    Utils::log("Экспортировано в CSV");
    cout << "Экспортировано.\n";
}

// Сортировка функторами
void sortResources() {
    string path;
    cout << "Введите путь к папке для сортировки: ";
    getline(cin, path);
    Resource* dir = root.get();
    // Разбор пути (аналогично)
    vector<string> parts;
    size_t pos = 0;
    string tmp = path;
    while ((pos = tmp.find('/')) != string::npos) {
        parts.push_back(tmp.substr(0, pos));
        tmp.erase(0, pos + 1);
    }
    parts.push_back(tmp);
    for (const auto& part : parts) {
        if (part.empty()) continue;
        if (dir->getType() != Resource::Type::Directory) {
            cout << "Некорректный путь.\n";
            return;
        }
        Resource* child = dynamic_cast<Directory*>(dir)->findChild(part);
        if (!child) {
            cout << "Не найдено.\n";
            return;
        }
        dir = child;
    }
    if (dir->getType() != Resource::Type::Directory) {
        cout << "Не является папкой.\n";
        return;
    }

    cout << "Сортировать по (1: имени, 2: размеру, 3: дате): ";
    int opt;
    cin >> opt;
    cin.ignore();

    // Получаем ссылку на вектор детей (приводим к неконстантному)
    auto& children = const_cast<vector<unique_ptr<Resource>>&>(dynamic_cast<Directory*>(dir)->getChildren());
    switch (opt) {
    case 1: // по имени
        sort(children.begin(), children.end(),
            [](const unique_ptr<Resource>& a, const unique_ptr<Resource>& b) {
                return a->getName() < b->getName();
            });
        break;
    case 2: // по размеру
        sort(children.begin(), children.end(),
            [](const unique_ptr<Resource>& a, const unique_ptr<Resource>& b) {
                return a->getSize() < b->getSize();
            });
        break;
    case 3: // по дате
        sort(children.begin(), children.end(),
            [](const unique_ptr<Resource>& a, const unique_ptr<Resource>& b) {
                return a->getCreationDate() < b->getCreationDate();
            });
        break;
    default:
        cout << "Неверный вариант.\n";
        return;
    }
    Utils::log("Отсортирована папка " + path);
    cout << "Отсортировано.\n";
}
// Поиск по содержимому
void searchByContent() {
    string query;
    cout << "Введите текст для поиска: ";
    getline(cin, query);
    if (query.empty()) return;

    // Рекурсивный обход
    function<void(Resource*)> searchRec = [&](Resource* node) {
        if (node->getType() == Resource::Type::File) {
            File* f = dynamic_cast<File*>(node);
            if (f->getContent().find(query) != string::npos) {
                cout << "Найдено в файле: " << f->getName() << "." << f->getExtension() << "\n";
            }
        }
        else {
            for (const auto& child : dynamic_cast<Directory*>(node)->getChildren()) {
                searchRec(child.get());
            }
        }
        };
    searchRec(root.get());
    Utils::log("Выполнен поиск по содержимому: " + query);
}