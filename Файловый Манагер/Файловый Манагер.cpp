#include <iostream>
#include <windows.h>
#include <io.h>
#include <iomanip>
#include <direct.h>
#include <string>
#include <vector>
#include <errno.h>
#include <crtdbg.h> 

void myHandler(const wchar_t* expression, const wchar_t* function,
    const wchar_t* file, unsigned int line, uintptr_t pReserved) {
    // Здесь можно не делать ничё
}

// Переписано с std::string
void SaSremover(std::string& name) {
    while (!name.empty() && (name.back() == '\\' || name.back() == ' ')) {
        name.pop_back();
    }
}

class fileManager {
    // Отсекает последнюю папку с пути
    static bool heaven(std::string& path) {
        size_t found = path.find_last_of('\\');
        if (found == std::string::npos)
            return false;
        path.erase(found);
        return true;
    }

    void zErrno(int EC) {
#define ENOCOM -1
#define ENODISK -2
#define ENOPATH -3
#define FEXIST -4
#define SUCCESS -5
#define ERRIDK 0xFFFFFFFFFFFFFFFF
        switch (EC) {
        case EEXIST: message = "Папка уже существует\n"; break;
        case FEXIST: message = "Файл уже существует\n"; break;
        case EINVAL: message = "Запрещённое имя (содержит \\/:*?\"<>| )\n"; break;
        case ERANGE:
        case ENAMETOOLONG: message = "Слишком большое имя\n"; break;
        case EACCES: message = "Нет прав\n"; break;
        case EPERM: message = "Путь не является файлом"; break;
        case ENOTDIR: message = "Путь не является директорией"; break;
        case ENOCOM: message = "Несуществующая команда\n"; break;
        case ENODISK: message = "Несуществующий диск\n"; break;
        case ENOENT: message = "Не найден путь\n"; break;
        case ENOPATH: message = "Путь не указан\n"; break;
        case SUCCESS: message = "Команда успешно выполнена.\n"; break;
        }
    }

    std::string message;
    std::string path;

    static bool isDir(const std::string& fullpath) {
        if (fullpath.size() < 2 || fullpath[1] != ':') {
            throw std::exception("Only Full Path, MF!");
        }
        DWORD attributes = GetFileAttributesA(fullpath.c_str());
        return (attributes == INVALID_FILE_ATTRIBUTES) ? false : (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }

    bool isPathExist(std::string& name) {
        if (!isDisk(name)) {
            zErrno(ENODISK);
            return false;
        }
        std::string checkPath = name;
        std::string resultPath;

        // Логика strtok через std::string (сохраняем структуру разбиения)
        size_t start = 0, end = 0;
        bool first = true;

        while ((end = checkPath.find('\\', start)) != std::string::npos || start < checkPath.size()) {
            std::string token = checkPath.substr(start, end - start);
            if (token.empty() && end != std::string::npos) { start = end + 1; continue; }

            if (first) {
                resultPath = token;
                first = false;
            }
            else {
                std::string checkName = resultPath + "\\" + token;
                std::string realName = getName(checkName);
                if (realName.empty()) return false;
                resultPath += "\\" + realName;
            }
            if (end == std::string::npos) break;
            start = end + 1;
        }
        name = resultPath;
        return true;
    }

    std::string formatInputPath(const std::string& name) {
        if (name.empty()) {
            zErrno(ENOPATH);
            return "";
        }
        std::string fullpath;
        if (name.size() >= 2 && name[1] == ':') {
            if (!isDisk(name)) {
                zErrno(ENODISK);
                return "";
            }
            fullpath = name;
        }
        else {
            fullpath = path + "\\" + name;
        }
        return fullpath;
    }

    std::string getName(const std::string& inputPath) {
        std::string fullpath = formatInputPath(inputPath);
        if (fullpath.empty()) return "";
        _finddata_t find;
        intptr_t result = _findfirst(fullpath.c_str(), &find);
        if (result == -1) {
            zErrno(ENOENT);
            return "";
        }
        std::string resName = find.name;
        _findclose(result);
        return resName;
    }

public:
    fileManager(bool intrface = false) {
        _set_invalid_parameter_handler(myHandler);
        _CrtSetReportMode(_CRT_ASSERT, 0);

        char buf[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, buf);
        path = buf;

        if (!intrface) return;
        std::string action;
        bool flag = true;

        do {
            showDir();
            std::cout << message << '\n';
            std::cout << path << '>';
            message.clear();

            std::getline(std::cin, action);
            SaSremover(action);

            if (action == "root" || action == "ROOT") {
                path = path.substr(0, 2);
            }
            else if (action == "exit" || action == "EXIT") {
                flag = false;
                system("cls");
            }
            else if (action.substr(0, 2) == "cd") {
                size_t index = action.find_first_not_of(' ', 2);
                if (index != std::string::npos) setPath(action.substr(index));
            }
            else if (action.substr(0, 5) == "mkdir") {
                size_t index = action.find_first_not_of(' ', 5);
                if (index != std::string::npos) createDir(action.substr(index));
            }
            else if (action.substr(0, 6) == "mkfile") {
                size_t index = action.find_first_not_of(' ', 6);
                if (index != std::string::npos) createFile(action.substr(index));
            }
            else if (action.substr(0, 5) == "rmdir") {
                size_t index = action.find_first_not_of(' ', 5);
                if (index != std::string::npos) deleteDir(action.substr(index));
            }
            else if (action.substr(0, 6) == "rmfile") {
                size_t index = action.find_first_not_of(' ', 6);
                if (index != std::string::npos) deleteFile(action.substr(index));
            }
            else if (action.substr(0, 7) == "getsize") {
                size_t index = action.find_first_not_of(' ', 7);
                if (index != std::string::npos) {
                    unsigned long long count = getSize(action.substr(index));
                    if (count != ERRIDK) message = std::to_string(count) + " байт";
                }
            }
            else if (action.substr(0, 6) == "search") {
                size_t index = action.find_first_not_of(' ', 6);
                if (index != std::string::npos) Search(path, action.substr(index));
            }
            else if (action.substr(0, 6) == "rename" || action.substr(0, 4) == "move" || action.substr(0, 8) == "copyfile") {
                size_t off = action.find(' ');
                size_t index = action.find_first_not_of(' ', off);
                std::string params = action.substr(index);
                size_t comma = params.find(',');

                if (comma == std::string::npos) {
                    zErrno(ENOPATH);
                    continue;
                }
                std::string oldname = params.substr(0, comma);
                std::string newname = params.substr(comma + 1);
                // Trim spaces
                newname.erase(0, newname.find_first_not_of(' '));

                if (off == 4) move(oldname, newname);
                else if (off == 6) reName(oldname, newname);
                else copyFile(oldname, newname);
            }
            else {
                zErrno(ENOCOM);
            }
        } while (flag);
    }

    static bool isDisk(const std::string& fullpath) {
        if (fullpath.size() < 2 || fullpath[1] != ':') return false;
        DWORD drives = GetLogicalDrives();
        int diskIdx = toupper(fullpath[0]) - 'A';
        return (drives & (1 << diskIdx));
    }

    void setPath(const std::string& newPath) {
        if (newPath.empty() || newPath == "." || newPath == "/") return;
        if (newPath == "..") {
            heaven(path);
            return;
        }
        std::string checkPath = formatInputPath(newPath);
        if (checkPath.empty()) return;
        if (!isPathExist(checkPath)) {
            zErrno(ENOPATH);
            return;
        };
        path = checkPath;
    }

    void showDir(const std::string& targetPath, std::ostream& out = std::cout) {
        _finddata_t find;
        std::string pathfind = targetPath + "\\*.*";
        intptr_t result = _findfirst(pathfind.c_str(), &find);
        system("cls");
        if (result != -1) {
            do {
                if (strcmp(find.name, ".") && strcmp(find.name, "..")) {
                    std::string info = (find.attrib & _A_SUBDIR) ? " Каталог " : " Файл ";
                    out << std::setw(40) << find.name << std::setw(10) << info << std::endl;
                }
            } while (_findnext(result, &find) != -1);
            _findclose(result);
        }
    }

    void showDir(std::ostream& out = std::cout) { showDir(path, out); }

    bool createDir(const std::string& d) {
        std::string fullpath = formatInputPath(d);
        if (fullpath.empty()) return false;
        if (_mkdir(fullpath.c_str())) {
            if (errno == ENOENT) {
                std::string copy = fullpath;
                if (heaven(copy)) {
                    createDir(copy);
                    createDir(fullpath);
                }
            }
            else {
                zErrno(errno);
                return false;
            }
        }
        zErrno(SUCCESS);
        return true;
    }

    bool createFile(const std::string& name) {
        std::string fullpath = formatInputPath(name);
        if (fullpath.empty()) return false;
        std::string curDir = fullpath;
        heaven(curDir);
        if (!isPathExist(curDir)) createDir(curDir);

        FILE* f;
        if (fopen_s(&f, fullpath.c_str(), "r") == 0) {
            zErrno(FEXIST);
            fclose(f);
            return false;
        }
        if (fopen_s(&f, fullpath.c_str(), "w") == 0) {
            fclose(f);
            zErrno(SUCCESS);
            return true;
        }
        zErrno(errno);
        return false;
    }

    bool deleteDir(const std::string& d) {
        std::string fullpath = formatInputPath(d);
        if (fullpath.empty()) return false;
        _finddata_t find;
        std::string search = fullpath + "\\*.*";
        intptr_t h = _findfirst(search.c_str(), &find);
        if (h == -1) {
            zErrno(errno == ENOENT ? ENOPATH : (errno == EINVAL ? ENOTDIR : errno));
            return false;
        }
        do {
            if (strcmp(find.name, ".") && strcmp(find.name, "..")) {
                std::string itemPath = fullpath + "\\" + find.name;
                if (isDir(itemPath)) deleteDir(itemPath);
                else deleteFile(itemPath);
            }
        } while (_findnext(h, &find) == 0);
        _findclose(h);
        if (_rmdir(fullpath.c_str()) == -1) {
            zErrno(errno);
            return false;
        }
        zErrno(SUCCESS);
        return true;
    }

    bool deleteFile(const std::string& name) {
        std::string fullpath = formatInputPath(name);
        if (fullpath.empty()) return false;
        std::string cutpath = fullpath;
        heaven(cutpath);
        if (!isPathExist(cutpath)) return false;

        _finddata_t find;
        std::string pathfind = cutpath + "\\*.*";
        intptr_t result = _findfirst(pathfind.c_str(), &find);
        bool found = false;
        if (result != -1) {
            do {
                if (_stricmp((cutpath + "\\" + find.name).c_str(), fullpath.c_str()) == 0) {
                    if (find.attrib & _A_SUBDIR) {
                        zErrno(EPERM);
                        _findclose(result);
                        return false;
                    }
                    found = true;
                    break;
                }
            } while (_findnext(result, &find) != -1);
            _findclose(result);
        }

        if (remove(fullpath.c_str()) == -1) {
            zErrno(ENOPATH);
            return false;
        }
        zErrno(SUCCESS);
        return true;
    }

    bool reName(const std::string& oldname, const std::string& newname) {
        std::string fOld = formatInputPath(oldname);
        std::string fNew = formatInputPath(newname);
        if (fOld.empty() || fNew.empty()) return false;
        if (rename(fOld.c_str(), fNew.c_str()) != 0) {
            zErrno(errno);
            return false;
        }
        zErrno(SUCCESS);
        return true;
    }

    bool copyFile(const std::string& oldFile, const std::string& newFolder) {
        std::string file = formatInputPath(oldFile);
        std::string dest = formatInputPath(newFolder);
        std::string name = getName(file);
        if (file.empty() || dest.empty() || name.empty()) return false;

        std::string filecpy = dest + "\\" + name;
        if (!createFile(filecpy)) return false;

        FILE* w, * r;
        if (fopen_s(&r, file.c_str(), "rb") != 0) { zErrno(errno); return false; }
        if (fopen_s(&w, filecpy.c_str(), "wb") != 0) { fclose(r); return false; }

        char buffer[4096];
        size_t bytesRead;
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), r)) > 0) {
            fwrite(buffer, 1, bytesRead, w);
        }
        fclose(w); fclose(r);
        zErrno(SUCCESS);
        return true;
    }

    bool copyDir(const std::string& oldFolder, const std::string& newFolder) {
        std::string fOld = formatInputPath(oldFolder);
        std::string fNew = formatInputPath(newFolder);
        if (fOld.empty() || !isDir(fOld) || fNew.empty() || !isDir(fNew)) return false;

        std::string lastName = getName(fOld);
        if (lastName.empty()) return false;
        std::string folderName = fNew + "\\" + lastName;
        if (!createDir(folderName)) return false;

        _finddata_t find;
        intptr_t h = _findfirst((fOld + "\\*.*").c_str(), &find);
        if (h != -1) {
            do {
                if (strcmp(find.name, ".") && strcmp(find.name, "..")) {
                    std::string item = fOld + "\\" + find.name;
                    if (isDir(item)) copyDir(item, folderName);
                    else copyFile(item, folderName);
                }
            } while (_findnext(h, &find) == 0);
            _findclose(h);
        }
        zErrno(SUCCESS);
        return true;
    }

    bool move(const std::string& oldname, const std::string& moveto) {
        std::string source = formatInputPath(oldname);
        if (source.empty()) return false;
        std::string lastName = getName(source);
        if (lastName.empty()) return false;

        std::string newpath = formatInputPath(moveto) + "\\" + lastName;

        if (isDir(source)) {
            if (!copyDir(source, formatInputPath(moveto))) return false;
            deleteDir(source);
            return true;
        }
        return reName(oldname, newpath);
    }

    unsigned long long getSize(const std::string& inputPath) {
        std::string fullpath = formatInputPath(inputPath);
        if (fullpath.empty()) return ERRIDK;
        bool isFolder = isDir(fullpath);
        _finddata_t find;
        std::string search = isFolder ? (fullpath + "\\*.*") : fullpath;
        intptr_t h = _findfirst(search.c_str(), &find);
        unsigned long long size = 0;
        if (h == -1) { zErrno(errno); return ERRIDK; }

        if (!isFolder) size = find.size;
        else {
            do {
                if (strcmp(find.name, ".") && strcmp(find.name, "..")) {
                    if (find.attrib & _A_SUBDIR) size += getSize(fullpath + "\\" + find.name);
                    else size += find.size;
                }
            } while (_findnext(h, &find) != -1);
        }
        _findclose(h);
        return size;
    }

    static bool isMaskMatch(const char* pmask, const char* pname) {
        const char* pStar = nullptr, * pNameReset = nullptr;
        while (*pname) {
            if (*pmask == '*') { pStar = pmask++; pNameReset = pname; }
            else if (*pmask == '?' || *pmask == *pname) { pmask++; pname++; }
            else if (pStar) { pmask = pStar + 1; pname = ++pNameReset; }
            else return false;
        }
        while (*pmask == '*') pmask++;
        return *pmask == '\0';
    }

    void Search(const std::string& currentPath, const std::string& mask) {
        _finddata_t find;
        intptr_t h = _findfirst((currentPath + "\\*.*").c_str(), &find);
        if (h == -1) return;
        do {
            if (strcmp(find.name, ".") && strcmp(find.name, "..")) {
                bool isFolder = find.attrib & _A_SUBDIR;
                if (isMaskMatch(mask.c_str(), find.name)) {
                    message += (isFolder ? "Каталог " : "Файл    ");
                    message += currentPath + "\\" + find.name + "\n";
                }
                if (isFolder) Search(currentPath + "\\" + find.name, mask);
            }
        } while (_findnext(h, &find) != -1);
        _findclose(h);
    }
};