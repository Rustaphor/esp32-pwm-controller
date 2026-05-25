#include "AConsole2Cmd.h"

map<const char, string> &AConsole2Cmd::getOpts(map<const char, string> &arglist, int argc, char **argv)
{
    arglist.clear();

    bool chk_param = false;
    map<char, string>::iterator it;

    for (int i = 0; i < argc; ++i) {
        string str{argv[i]};

        // Сканирование флагов
        if (str.starts_with("-")) {
            string str2(str.substr(1));
            for (uint8_t i2 = 0; i2 < str2.length(); i2++) {
                it = arglist.insert({str2[i2],""}).first;
            };

            chk_param = true;
            continue;
        }

        // Проверка флагов на наличие параметров
        if (chk_param) {
            const char K = it->first;
            arglist.erase(it);
            arglist.insert({K,str});

            chk_param = false;
            continue;
        }

        // Вставка команд
        arglist.insert({(char) i, str});
    }
    

    return arglist;
}
