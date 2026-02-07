#include <iostream>
#include <windows.h>
#include <io.h>
#include <iomanip>
#include <direct.h>
#include <string.h>
#include <errno.h>
class fileManager {
#define RL_MAX_PATH MAX_PATH+1
    static bool heaven(char* path) { // отсекает последнюю папку с пути👍
        const char* result = strrchr(path, '\\');
        if (result == nullptr) return false;
        int64_t delta = result - path;
        path[delta] = '\0';
        return true;
    }
    void zErrno(int EC) { // записывает в message переданный код ошибки
#define ENOCOM -1
#define ENODISK -2
#define ENOPATH -3
        switch (EC) {
        case EEXIST:
            strcpy_s(message, 1024, "Папка уже существует\n");
            break;
        case EINVAL:
            strcpy_s(message, 1024, "Запрещённое имя (содержит \\/:*?\"<>| )\n");
            break;
        case ENAMETOOLONG:
            strcpy_s(message, 1024, "Слишком большое имя\n");
            break;
        case EACCES:
            strcpy_s(message, 1024, "Нет прав\n");
            break;
        case ENOTDIR:
            strcpy_s(message, 1024, "Путь не является директорией");
            break;
        case ENOCOM:
            strcpy_s(message, 1024, "Несуществующая команда\n");
            break;
        case ENODISK:
            strcpy_s(message, 1024, "Несуществующий диск\n");
            break;
        case ENOPATH:
            strcpy_s(message, 1024, "Не найден путь\n");
            break;
        } //https://learn.microsoft.com/ru-ru/cpp/c-runtime-library/errno-constants?view=msvc-170
    }
    static const int size = 8192; // максимальный размер команды + терминальный ноль
    char message[1024]; // сообщение о результатах выполнения операции в интерфейсе
    char path[RL_MAX_PATH]; //максимальный размер пути файла + /0
    bool isDir(const char* fullpath) { // проверяет папка или файл, а также имя диска
        if (fullpath[1] != ':') {
            throw std::exception("Only Full Path, MF!");
        }
        DWORD attributes = GetFileAttributesA(fullpath);
        return (attributes == INVALID_FILE_ATTRIBUTES) ? false : (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }
    char* formatInputPath(const char* name) { // при необходимости приводит короткий путь  к стандартному;
        char* fullpath;
        if (name == nullptr || name[0] == '\0') {
            zErrno(ENOPATH);
            return nullptr;
        }
        //Проверка на полный путь к Директории
        else if (name[1] == ':') {
            if (!isDisk(name)) {
                zErrno(ENODISK);
                return nullptr;
            }

            //ПРОВЕРКА СУЩЕСТВОВАНИЯ ПАПОК В ПУТИ name
            fullpath = new char[RL_MAX_PATH];
            strcpy_s(fullpath, RL_MAX_PATH, name);
        }
        else {
            fullpath = new char[RL_MAX_PATH];
            strcpy_s(fullpath, RL_MAX_PATH, path);
            strcat_s(fullpath, RL_MAX_PATH, "\\");
            strcat_s(fullpath, RL_MAX_PATH, name);
        }
        return fullpath;
    }
    const char* getName(const char* path) {
        char* fullpath = formatInputPath(path);
        if (!isDir(fullpath)) return nullptr;
        _finddata_t find;
        long long result = _findfirst(path, &find);
        char* rename = new char[strlen(find.name) + 1];
        strcpy_s(rename, strlen(find.name) + 1, find.name);
        return rename;
    }
public:
    fileManager(bool intrface = false) {
        message[0] = '\0';
        //Получаем Путь к текущей Директории
        GetCurrentDirectoryA(sizeof(path), path);
        if (!intrface) return;
        char action[size];
        bool flag = true;
        //Показ содержимого текущей директории
        do {
            showDir();
            std::cout << message << '\n';
            std::cout << path << '>';
            message[0] = '\0';
            //Ввод команды пользователя
            std::cin.getline(action, size);
            //Переход в корневой каталог
            if (!_stricmp(action, "root")) {
                    path[2] = '\0';
            }
            //Проверка на желание пользователя выйти
            else if (!_stricmp(action, "exit")) {
                flag = false;
                system("cls");
            }
            //Команда cd была дана с параметрами
            else if (!_strnicmp(action, "cd", 2)) {
                //Находим индекс параметра
                size_t index = strspn(action + 2, " ") + 2;
                if ((!_stricmp(action, "cd")))
                    continue;
                setPath(action + index);
            }
            else if (!_strnicmp(action, "mkdir", 5)) {
                size_t index = strspn(action + 5, " ") + 5; //1 символ после mkdir
                createDir(action + index);
            }
            else if (!_strnicmp(action, "mkfile", 6)) {
                size_t index = strspn(action + 6, " ") + 6;
                createFile(action + index);
            }
            else if (!_strnicmp(action, "rmdir", 5)) {
                size_t index = strspn(action + 5, " ") + 5;
                deleteDir(action + index);
            }
            else if (!_strnicmp(action, "rmfile", 6)) {
                size_t index = strspn(action + 6, " ") + 6;
                deleteFile(action + index);
            }
            else if (!_strnicmp(action, "rename", 6)) {
                size_t index = strspn(action + 6, " ") + 6;
                const char* name = action + index;
                int len = strcspn(name, " ");
                if (name[len] != ' ') {
                    zErrno(ENOPATH);
                } // len - место пробела, используется для разъединения name на oldname и newname
                char oldname[RL_MAX_PATH] = {};
                strncpy_s(oldname, RL_MAX_PATH, name, len);
                char newname[RL_MAX_PATH] = {};
                strncpy_s(newname, RL_MAX_PATH, name + len + 1, RL_MAX_PATH);
                reName(oldname, newname);
            }
            else {
                zErrno(ENOCOM);
            }
        } while (flag);
    }
    static bool isDisk(const char* fullpath) {
        if (fullpath[1] != ':') {
            throw std::exception("Only Full Path, MF!");
        }
        DWORD drives = GetLogicalDrives();
        DWORD mask = 0x1;
        for (char letter = 'A', disk = toupper(fullpath[0]); letter < disk; letter++) {
            mask <<= 1;
        }
        if (!(drives & mask)) {
            return false;
        }
        return true;
    }
    void setPath(const char* newPath) { //вызов cd в ООПагангамстайл
        if (!strcmp(newPath, ".") || !strcmp(newPath, "/"))
            return;
        //Поднимаемся в родительский каталог
        if (!strcmp(newPath, "..")) {
            heaven(path);
            return;
        }
        char temp[RL_MAX_PATH];
        //Проверка на полный путь к Директории (точнее его отсутствия)
        if (newPath[1] != ':') {
            int c = strcspn(newPath, "\\");
            if (c != strlen(newPath)) {
                char* next_token = NULL;
                char thesame[RL_MAX_PATH];
                strcpy_s(thesame, RL_MAX_PATH, newPath);
                char* token = strtok_s(thesame, "\\", &next_token);
                while (token != nullptr) {
                    setPath(token);
                    token = strtok_s(nullptr, "\\", &next_token);
                }
                return;
            }
            strcpy_s(temp, RL_MAX_PATH, path);
            strcat_s(temp, RL_MAX_PATH, "\\");
            strcat_s(temp, RL_MAX_PATH, newPath);
        }
        // Был дан полный путь
        else {
            strcpy_s(temp, RL_MAX_PATH, newPath);
            GetLongPathNameA(temp, path, RL_MAX_PATH);
            path[0] = toupper(path[0]);
            return;
        }
        //Был дан неполный путь
        const char* name = getName(temp);
        if (name == nullptr) {
            zErrno(ENOPATH);
            delete[] name;
            return;
        }
        strcat_s(path, RL_MAX_PATH, "\\");
        strcat_s(path, RL_MAX_PATH, name);
        delete[] name;
    }
        const char* const getPath() {
        return path;
    }
    void showDir(const char* path, std::ostream& out = std::cout) {
        _finddata_t find;
        char pathfind[RL_MAX_PATH];
        strcpy_s(pathfind, RL_MAX_PATH, path);
        strcat_s(pathfind, RL_MAX_PATH, "\\*.*");
        char info[RL_MAX_PATH];
        long long result = _findfirst(pathfind, &find);
        system("cls");
        do {
            if (strcmp(find.name, ".") && strcmp(find.name, "..")) {
                //Проверяем Директория или Нет
                find.attrib& _A_SUBDIR ? strcpy_s(info, RL_MAX_PATH, " Каталог ") :
                    strcpy_s(info, RL_MAX_PATH, " Файл ");
                out << std::setw(40) << find.name << std::setw(10) << info << std::endl;
            }
        } while (_findnext(result, &find) != -1);
        //Очищаем ресурсы, выделенные под поиск
        _findclose(result);
    }
    void showDir(std::ostream& out = std::cout) {
        showDir(path, out);
    }
    bool createDir(const char* d) {
        const char* fullpath = formatInputPath(d);
        if (fullpath == nullptr) return false;
        if (_mkdir(fullpath)) {
            zErrno(errno);
            if (errno == ENOENT) {
                char copy[RL_MAX_PATH];
                strcpy_s(copy, RL_MAX_PATH, fullpath);
                heaven(copy);
                createDir(copy);
                createDir(fullpath);
            }
            else {
                delete[]fullpath;
                return false;
            }
        }
        delete[]fullpath;
        return true;
    }
    bool createFile(const char* name) {
        const char* fullpath = formatInputPath(name);
        if (fullpath == nullptr) return false;
        FILE* f;
        fopen_s(&f, fullpath, "w");
        if (!f) {
            zErrno(errno);
            return false;
        }
        fclose(f); // закрываем файл
        return true;
    }
    bool deleteDir(const char* d) {
        const char* fullpath = formatInputPath(d);
        if (fullpath == nullptr) return false;
        _finddata_t find;
        char search[RL_MAX_PATH];
        strcpy_s(search, RL_MAX_PATH, fullpath);
        strcat_s(search, RL_MAX_PATH, "\\*.*"); // получаем первый элемент файловой системы находящийся в fullpath
        long long h = _findfirst(search, &find);
        if (h == -1) {
            if (errno == ENOENT) zErrno(ENOPATH);
            else zErrno(errno);
            delete[]fullpath;
            return false;
        }
        do {
            if (strcmp(find.name, ".") && strcmp(find.name, "..")) { //пропускаем директории . и ..
                char othsearch[RL_MAX_PATH];
                strcpy_s(othsearch, RL_MAX_PATH, fullpath);
                strcat_s(othsearch, RL_MAX_PATH, "\\");
                strcat_s(othsearch, RL_MAX_PATH, find.name); // получаем обозреваемый путь
                if (isDir(othsearch)) {
                    deleteDir(othsearch); // рекурсивно удаляем
                }
                else deleteFile(othsearch); // если не директория - удаляем не рекурсивно
            }
        } while (_findnext(h, &find) == 0);
        _findclose(h);
        if (_rmdir(fullpath) == -1) {
            zErrno(errno);
            delete[]fullpath;
            return false;
        }
        delete[]fullpath;
        return true;
    }
    bool deleteFile(const char* name) {
        const char* fullpath = formatInputPath(name);
        if (fullpath == nullptr) return false;
        if (remove(fullpath) == -1) {
            zErrno(errno); //ну тут то чё может быть непонятного
            delete[]fullpath;
            return false;
        }
        delete[]fullpath;
        return true;
    }
    bool reName(const char* oldname, const char* newname) {
        const char* fullpathOld = formatInputPath(oldname);
        if (fullpathOld == nullptr)
            return false;

        const char* fullpathNew = formatInputPath(newname);
        if (fullpathNew == nullptr) {
            delete[]fullpathOld;
            return false;
        }
            int result = rename(fullpathOld, fullpathNew);
        delete[]fullpathNew;
        delete[]fullpathOld;
        if (result != 0) {
            zErrno(errno);
            return false;
        }
        return true;
    }
    const char* gettname(const char* path) {
        return getName(path);
    }
#undef ENOCOM
#undef ENODISK
#undef ENOPATH
};
int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    fileManager c(0);
    c.setPath("D:\\Kladovka");
    //c.createFile("C:\\");
    //c.setPath(R"(C:\Users\Saturn\source\repos\Файловый Манагер\Файловый Манагер\Владислв)");

    //if (_stricmp(c.getPath(), R"(C:\Users\Saturn\source\repos\Файловый Манагер\Файловый Манагер\Владислв)"))
    //    std::cout << "Ошибка"; // C:\Users\Saturn\source\repos\Файловый Манагер\Файловый Манагер\Владислв
}