//ДОП: функция с переменным количеством аргументов для очистки памяти всех переданных ей указателей
//ДОП: rmdir при непереданном пути удаляет fileManager::path
#include <iostream>
#include <windows.h>
#include <io.h>
#include <iomanip>
#include <direct.h>
#include <string.h>
#include <errno.h>
#include <crtdbg.h> 
void myHandler(const wchar_t* expression, const wchar_t* function,
    const wchar_t* file, unsigned int line, uintptr_t pReserved) {
    // Здесь можно не делать ничё
}
void SaSremover(char* name) {
    size_t SaSremover = strlen(name) - 1;
    while (name[SaSremover] == '\\' || name[SaSremover] == ' ')
        SaSremover--;
    name[SaSremover + 1] = '\0';
}
#define RL_MAX_PATH MAX_PATH+1
class fileManager {
    static bool heaven(char* path) { // отсекает последнюю папку с пути👍
        const char* result = strrchr(path, '\\');
        if (result == nullptr)
            return false;
        int64_t delta = result - path;
        path[delta] = '\0';
        return true;
    }
    void zErrno(int EC) { // записывает в message переданный код ошибки
#define ENOCOM -1
#define ENODISK -2
#define ENOPATH -3
#define FEXIST -4
#define SUCCESS -5
        switch (EC) {
        case EEXIST:
            strcpy_s(message, 1024, "Папка уже существует\n");
            break;
        case FEXIST:
            strcpy_s(message, 1024, "Файл уже существует\n");
            break;
        case EINVAL:
            strcpy_s(message, 1024, "Запрещённое имя (содержит \\/:*?\"<>| )\n");
            break;
        case ERANGE:
        case ENAMETOOLONG:
            strcpy_s(message, 1024, "Слишком большое имя\n");
            break;
        case EACCES:
            strcpy_s(message, 1024, "Нет прав\n");
            break;
        case EPERM:
            strcpy_s(message, 1024, "Путь не является файлом");
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
        case ENOENT:
            strcpy_s(message, 1024, "Не найден путь\n");
            break;
        case ENOPATH:
            strcpy_s(message, 1024, "Путь не указан\n");
            break;
        case SUCCESS:
            strcpy_s(message, 1024, "Команда успешно выполнена.\n");
            break;
        } //https://learn.microsoft.com/ru-ru/cpp/c-runtime-library/errno-constants?view=msvc-170
    }
    static const int size = 8192; // максимальный размер команды + терминальный ноль
    char message[1024]; // сообщение о результатах выполнения операции в интерфейсе
    char path[RL_MAX_PATH]; //максимальный размер пути файла + /0
    static bool isDir(const char* fullpath) { // проверяет папка или файл, а также имя диска
        if (fullpath[1] != ':') {
            throw std::exception("Only Full Path, MF!");
        }
        DWORD attributes = GetFileAttributesA(fullpath);
        return (attributes == INVALID_FILE_ATTRIBUTES) ? false : (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }
    bool isPathExist(char* name) {//ispathexist не троготь
        if (!isDisk(name)) {
            zErrno(ENODISK);
            return false;
        }
        char checkPath[RL_MAX_PATH]; // для работы strcat_s (строка будет испорчена в итоге)
        strcpy_s(checkPath, RL_MAX_PATH, name);
        char* next_token = NULL;
        name[0] = '\0';
        char checkName[RL_MAX_PATH]{};// res
        char* token = strtok_s(checkPath, "\\", &next_token);
        strcat_s(name, RL_MAX_PATH, token);
        strcat_s(checkName, RL_MAX_PATH, name);
        token = strtok_s(nullptr, "\\", &next_token);
        while (token != nullptr) {
            strcat_s(checkName, RL_MAX_PATH, "\\");
            strcat_s(checkName, RL_MAX_PATH, token);
            const char* realName = getName(checkName);
            if (realName == nullptr) return false;
            strcat_s(name, RL_MAX_PATH, "\\");
            strcat_s(name, RL_MAX_PATH, realName);
            delete[]realName;
            strcpy_s(checkName, RL_MAX_PATH, name);
            token = strtok_s(nullptr, "\\", &next_token);
        }
        return true;
    }
    char* formatInputPath(const char* name) { // при необходимости приводит короткий путь  к стандартному;
        if (name == nullptr || name[0] == '\0') {
            zErrno(ENOPATH);
            return nullptr;
        }
        char* fullpath = new char[RL_MAX_PATH] {};
        //Проверка на полный путь к Директории cd C:\sdfsdfds
        if (name[1] == ':') {
            if (!isDisk(name)) {
                zErrno(ENODISK);
                return nullptr;
            }
        }
        else {
            strcpy_s(fullpath, RL_MAX_PATH, path);
            strcat_s(fullpath, RL_MAX_PATH, "\\");
        }
        errno_t i = strcat_s(fullpath, RL_MAX_PATH, name);
        if (i != 0) {
            zErrno(i);
            return nullptr;
        }
        return fullpath;
    }
    const char* getName(const char* path) {
        char* fullpath = formatInputPath(path);
        if (fullpath == nullptr) return nullptr;
        _finddata_t find;
        long long result = _findfirst(fullpath, &find);
        delete[] fullpath;
        if (result == -1) {
            zErrno(ENOENT);
            return nullptr;
        }
        char* rename = new char[strlen(find.name) + 1];
        strcpy_s(rename, strlen(find.name) + 1, find.name);
        return rename;
    }
    void strcopycat_s(char* dest, size_t buffer, const char* path, const char* concat) {
        strcpy_s(dest, buffer, path);
        strcat_s(dest, buffer, "\\");
        strcat_s(dest, buffer, concat);
    }
public:
    fileManager(bool intrface = false) {
        _set_invalid_parameter_handler(myHandler);
        _CrtSetReportMode(_CRT_ASSERT, 0);
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
            SaSremover(action);
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
            else if (!_strnicmp(action, "a", 1)) {
                size_t index = strspn(action + 1, " ") + 1;
                isPathExist(formatInputPath(action + index));
            } //УБИЙЦА СЛОМАЛ ФУНКЦИЮ И ОСТАЛСЯ НА СВОБОДЕ
            else if (!_strnicmp(action, "rename", 6) || !_strnicmp(action, "move", 4) || !_strnicmp(action, "copyfile", 8)) {
                int off = strcspn(action, " ");
                size_t index = strspn(action + off, " ") + off;
                const char* name = action + index;
                int len = strcspn(name, ",");
                if (name[len] != ',') {
                    zErrno(ENOPATH);
                } // len - место пробела, используется для разъединения name на oldname и newname
                char oldname[RL_MAX_PATH] = {};
                strncpy_s(oldname, RL_MAX_PATH, name, len);
                char newname[RL_MAX_PATH] = {};
                if (oldname[0] == '\0') {
                    zErrno(ENOPATH);
                    continue;
                }
                strncpy_s(newname, RL_MAX_PATH, name + len + 1, RL_MAX_PATH);
                strcpy_s(newname, RL_MAX_PATH, newname + strspn(newname, " "));
                if (off == 4) move(oldname, newname);
                else if (off == 6) reName(oldname, newname);
                else copyFile(oldname, newname);
                //off == 4 ? move(oldname, newname) : reName(oldname, newname);
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
        if (newPath == nullptr || !strcmp(newPath, ".") || !strcmp(newPath, "/"))
            return;
        //Поднимаемся в родительский каталог
        if (!strcmp(newPath, "..")) {
            heaven(path);
            return;
        }
        char* checkPath = formatInputPath(newPath);
        if (checkPath == nullptr) return;
        if (!isPathExist(checkPath)) {
            zErrno(ENOPATH);
            delete[] checkPath;
            return;
        };
        //strcat_s(path, RL_MAX_PATH, "\\");
        //strcat_s(path, RL_MAX_PATH, checkPath);
        strcpy_s(path, RL_MAX_PATH, checkPath);
    }
    const char* const getPath() {
        return path;
    }
    void showDir(const char* path, std::ostream& out = std::cout) {
        _finddata_t find;
        char pathfind[RL_MAX_PATH];
        strcopycat_s(pathfind, RL_MAX_PATH, path, "*.*");
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
        char* fullpath = formatInputPath(d);
        if (fullpath == nullptr)
            return false;
        if (_mkdir(fullpath)) {
            if (errno == ENOENT) {
                char copy[RL_MAX_PATH];
                strcpy_s(copy, RL_MAX_PATH, fullpath);
                heaven(copy);
                createDir(copy);
                createDir(fullpath);
            }
            else {
                zErrno(errno);
                delete[] fullpath;
                return false;
            }
        }
        delete[] fullpath;
        zErrno(SUCCESS);
        return true;
    }
    bool createFile(const char* name) {
        const char* fullpath = formatInputPath(name);
        if (fullpath == nullptr) {
            return false;
        }
        char CurDir[RL_MAX_PATH];
        strcpy_s(CurDir, RL_MAX_PATH, fullpath);
        heaven(CurDir);
        if (!isPathExist(CurDir))
            createDir(CurDir);
        FILE* f;
        fopen_s(&f, fullpath, "r"); // проверяем чтоб не перезаписать существующий файл
        if (f) { 
            zErrno(FEXIST);
            delete[] fullpath;
            fclose(f);
            return false;
        }
        fopen_s(&f, fullpath, "w");
        if (!f) {
            zErrno(errno);
            delete[] fullpath;
            return false;
        }
        fclose(f); // закрываем файл
        delete[] fullpath;
        zErrno(SUCCESS);
        return true;
    }
    bool deleteDir(const char* d) {
        const char* fullpath = formatInputPath(d);
        if (fullpath == nullptr) {
            delete[] fullpath;
            return false;
        }
        _finddata_t find;
        char search[RL_MAX_PATH];
        strcopycat_s(search, RL_MAX_PATH, fullpath, "*.*");// получаем первый элемент файловой системы находящийся в fullpath
        long long h = _findfirst(search, &find);
        if (h == -1) {
            if (errno == ENOENT) zErrno(ENOPATH);
            else if (errno == EINVAL) zErrno(ENOTDIR);
            else zErrno(errno);
            delete[] fullpath;
            return false;
        }
        do {
            if (strcmp(find.name, ".") && strcmp(find.name, "..")) { //пропускаем директории . и ..
                char othsearch[RL_MAX_PATH];
                strcopycat_s(othsearch, RL_MAX_PATH, fullpath, find.name); // получаем обозреваемый путь
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
        zErrno(SUCCESS);
        return true;
    }
    bool deleteFile(const char* name) { //ДЕЛИТФАЙЛ ТАЙНА НЕСУЩЕСТВУЮЩЕГО ПУТИ 
        const char* fullpath = formatInputPath(name);
        if (fullpath == nullptr) {
            return false;
        }
        //отрезать последнее имя, проверить что все каталоги в пути-папки, проверить через логику шоудира существование файла.
        char cutpath[RL_MAX_PATH];
        strcpy_s(cutpath, RL_MAX_PATH, fullpath);
        heaven(cutpath);
        if (!isPathExist(cutpath)) {
            delete[] fullpath;
            return false;
        }
        _finddata_t find;
        char pathfind[RL_MAX_PATH];
        strcopycat_s(pathfind, RL_MAX_PATH, cutpath, "*.*");
        bool isfile;
        long long result = _findfirst(pathfind, &find);
        do {
            strcat_s(cutpath, RL_MAX_PATH, find.name);
            if (!_stricmp(cutpath, fullpath)) {
                //Проверяем Директория или Нет
                find.attrib& _A_SUBDIR ? isfile = false : isfile = true;
                if (!isfile) {
                    zErrno(EPERM);
                    delete[] fullpath;
                    return false;
                }
                break;
            }
            heaven(cutpath);
            strcat_s(cutpath, RL_MAX_PATH, "\\");
        } while (_findnext(result, &find) != -1);
        if (remove(fullpath) == -1) {//надо потестить, нужно ли вызывать isPathExist() или нет?
            zErrno(ENOPATH);
            delete[]fullpath;
            return false;
        }
        delete[]fullpath;
        zErrno(SUCCESS);
        return true;
    }


    bool reName(const char* oldname, const char* newname) {
        const char* fullpathOld = formatInputPath(oldname);
        if (fullpathOld == nullptr) return false;
        const char* fullpathNew = formatInputPath(newname);
        if (fullpathNew == nullptr) return false;
        int result = rename(fullpathOld, fullpathNew);//аналогично потестить!! нужно ли проверять, что путь существует???
        delete[]fullpathNew;
        delete[]fullpathOld;
        if (result != 0) {
            zErrno(errno);
            return false;
        }
        zErrno(SUCCESS);
        return true;
    }

    bool copyFile(const char* oldFile, const char* newFolder) { // 1: несуществующий файл (1 арг)
        //ДЗ! отдебажиьт функцию 1 дальше видно будет что дебажить
        const char* file = formatInputPath(oldFile);
        if (file == nullptr) return false;
        const char* dest = formatInputPath(newFolder);
        if (dest == nullptr) { delete[] file; return false; }
        const char* name = getName(file);
        if (name == nullptr) {
            delete[] file;
            delete[] dest;
        }
        char filecpy[RL_MAX_PATH];
        strcopycat_s(filecpy, RL_MAX_PATH, dest, name);
        if (!createFile(filecpy)) {
            delete[] file;
            delete[] dest;
            delete[] name;
            return false;
        }
        FILE* w;
        FILE* r;
        fopen_s(&w, filecpy, "wb");
        fopen_s(&r, file, "rb");
        if (!r) {
            zErrno(errno);
            delete[] file;
            delete[] dest;
            delete[] name;
            fclose(w);
            return false;
        }
        char buffer[4096];
        size_t bytesRead;
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), r)) > 0) {
            fwrite(buffer, 1, bytesRead, w);
        }
        fclose(w);
        fclose(r);
        zErrno(SUCCESS);
        delete[] file;
        delete[] dest;
        delete[] name;
        return true;
    }
    bool copyDir(const char* oldFolder, const char* newFolder) {
        const char* fullpathOld = formatInputPath(oldFolder);
        if (fullpathOld == nullptr || !isDir(fullpathOld)) return false;
        const char* fullpathNew = formatInputPath(newFolder);
        if (fullpathNew == nullptr || !isDir(fullpathNew)) return false;
        char folderName[RL_MAX_PATH];
        const char* lastName = getName(fullpathOld);
        if (lastName == nullptr) {
            delete[]fullpathOld;
            delete[]fullpathNew;
            return false;
        }
        strcopycat_s(folderName, RL_MAX_PATH, fullpathNew, lastName);
        delete[]lastName;
        if (!createDir(folderName)) {
            delete[]fullpathOld;
            delete[]fullpathNew;
            return false;
        }

        _finddata_t find;
        char search[RL_MAX_PATH];
        strcopycat_s(search, RL_MAX_PATH, fullpathOld, "*.*");// получаем первый элемент файловой системы находящийся в fullpath
        long long h = _findfirst(search, &find);
        if (h == -1) {
            if (errno == ENOENT) zErrno(ENOPATH);
            else if (errno == EINVAL) zErrno(ENOTDIR);
            else zErrno(errno);
            delete[] fullpathOld;
            delete[] fullpathNew;
            return false;
        }
        do {
            if (strcmp(find.name, ".") && strcmp(find.name, "..")) { //пропускаем директории . и ..
                char othsearch[RL_MAX_PATH];
                strcopycat_s(othsearch, RL_MAX_PATH, fullpathOld, find.name);// получаем обозреваемый путь
                if (isDir(othsearch)) {
                    copyDir(othsearch, folderName); // рекурсивно копируем
                }
                else copyFile(othsearch, folderName); // если не директория - копируем не рекурсивно
            }
        } while (_findnext(h, &find) == 0);
        _findclose(h);
        delete[] fullpathOld;
        delete[] fullpathNew;
        zErrno(SUCCESS);
        return true;
    };
    bool move(const char* oldname, const char* moveto) {
        char newpath[RL_MAX_PATH];
        strcopycat_s(newpath, RL_MAX_PATH, moveto, "\0");
        const char* source = formatInputPath(oldname);
        if (source == nullptr) return false;
        const char* lastName = getName(source);
        if (lastName == nullptr) {
            delete[]source;
            return false;
        }
        strcat_s(newpath, RL_MAX_PATH, lastName);
        delete[]lastName;
        if (newpath[0] == '\0') {
            delete[]source;
            return false;
        }
        if (isDir(source)) {
            if (!copyDir(source, formatInputPath(moveto))) {
                zErrno(errno);
                return false;
            }
            deleteDir(source);
            delete[]source;
            return true;
        }
        delete[]source;
        return reName(oldname, newpath);
    }
#undef ENOCOM
#undef ENODISK
#undef ENOPATH
#undef FEXIST
#undef SUCCESS
};
int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    fileManager c(1);

    //c.createFile("C:\\");
    //c.setPath(R"(C:\Users\Saturn\source\repos\Файловый Манагер\Файловый Манагер\Владислв)");

    //if (_stricmp(c.getPath(), R"(C:\Users\Saturn\source\repos\Файловый Манагер\Файловый Манагер\Владислв)"))
    //    std::cout << "Ошибка"; // C:\Users\Saturn\source\repos\Файловый Манагер\Файловый Манагер\Владислв
}