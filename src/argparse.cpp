#include <pwos/common.h>
#include <pwos/argparse.h>

Arg::Arg() {};

Arg::Arg(std::string id, ArgType type): id(id), type(type) {};

void Arg::setValue(string value)
{
    this->value = value;
    isSet = true;
}

ArgParse::ArgParse(vector<Arg> flagList)
{
    for (Arg f : flagList)
    {
        flags[f.id] = f;
    }
}

void ArgParse::parse(int argc, char* argv[])
{
    int i = 1;
    while (i < argc)
    {
        string s(argv[i]);

        // check if main or flag
        bool isFlag = false;
        if (s.rfind("--", 0) == 0)
        {
            isFlag = true;
            s = s.substr(2, s.size() - 1);
        }

        if (s.rfind("-", 0) == 0)
        {
            isFlag = true;
            s = s.substr(1, s.size() - 1);
        }

        if (isFlag)
        {
            if  (i + 1 >= argc) return;
            i++;

            if (flags.count(s) > 0)
            {
                flags[s].setValue(string(argv[i]));
            }
        }
        else
        {
            main.push_back(s);
        }

        i++;
    }
};

int ArgParse::getInt(string id, int defaultValue)
{
    THROW_IF(flags.count(id) == 0, "Flag with id " + id + " is not registered with argparse instance.");
    THROW_IF(flags[id].type != ArgType::INT, "Flag with id " + id + " is not type int.");
    return flags[id].isSet ? std::stoi(flags[id].value) : defaultValue;
}

string ArgParse::getStr(string id, string defaultValue)
{
    THROW_IF(flags.count(id) == 0, "Flag with id " + id + " is not registered with argparse instance.");
    THROW_IF(flags[id].type != ArgType::STR, "Flag with id " + id + " is not type string.");
    return flags[id].isSet ? flags[id].value : defaultValue;
}

string ArgParse::getMain(int i)
{
    THROW_IF(i >= main.size(), "No main argument found at index " + std::to_string(i) + " max index is " + std::to_string(main.size()));
    return main[i];
}
