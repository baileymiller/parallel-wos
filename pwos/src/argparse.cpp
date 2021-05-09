#include <pwos/common.h>
#include <pwos/argparse.h>

bool isFlag(string s, string &id)
{
    if (s.rfind("--", 0) == 0)
    {
        id = s.substr(2, s.size() - 1);
        return true;
    }

    if (s.rfind("-", 0) == 0)
    {
        id = s.substr(1, s.size() - 1);
        return true;
    }

    return false;
}

Arg::Arg() {};

Arg::Arg(std::string id, ArgType type): id(id), type(type) {};

void Arg::setValue(string value)
{
    this->values[0] = value;
    isSet = true;
}

void Arg::pushValue(string value)
{
    this->values.push_back(value);
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
        string id;
        if (isFlag(s, id))
        {
            if (flags.count(id) > 0)
            {
                ArgType type = flags[id].type;
                int numValues;
                switch (type)
                {
                    case ArgType::INT:
                    case ArgType::STR:
                    case ArgType::FLOAT:
                        numValues = 1;
                        break;
                    case ArgType::VEC4f:
                        numValues = 4;
                        break;
                    case ArgType::VEC2i:
                        numValues = 2;
                        break;
                    default:
                        numValues = 0;
                }

                while (numValues > 0) 
                {
                    if  (i + 1 >= argc) return;
                    i++;
                    numValues--;
                    string value = string(argv[i]);

                    string val;
                    THROW_IF(isFlag(value, val), "Did not provide enough values for flag " + id);
                    flags[id].pushValue(value);
                }
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
    return flags[id].isSet ? std::stoi(flags[id].values[0]) : defaultValue;
}

float ArgParse::getFloat(string id, float defaultValue)
{
    THROW_IF(flags.count(id) == 0, "Flag with id " + id + " is not registered with argparse instance.");
    THROW_IF(flags[id].type != ArgType::FLOAT, "Flag with id " + id + " is not type float.");
    return flags[id].isSet ? std::stof(flags[id].values[0]) : defaultValue;
}

string ArgParse::getStr(string id, string defaultValue)
{
    THROW_IF(flags.count(id) == 0, "Flag with id " + id + " is not registered with argparse instance.");
    THROW_IF(flags[id].type != ArgType::STR, "Flag with id " + id + " is not type string.");
    return flags[id].isSet ? flags[id].values[0] : defaultValue;
}

Vec2i ArgParse::getVec2i(string id, Vec2i defaultValue)
{
    THROW_IF(flags.count(id) == 0, "Flag with id " + id + " is not registered with argparse instance.");
    THROW_IF(flags[id].type != ArgType::VEC2i, "Flag with id " + id + " is not type string.");
    if (!flags[id].isSet) return defaultValue;
    Vec2i v;
    for (int i = 0; i < 2; i ++)
        v[i] = std::stoi(flags[id].values[i]);
    return v;
}

Vec4f ArgParse::getVec4f(string id, Vec4f defaultValue)
{
    THROW_IF(flags.count(id) == 0, "Flag with id " + id + " is not registered with argparse instance.");
    THROW_IF(flags[id].type != ArgType::VEC4f, "Flag with id " + id + " is not type string.");
    if (!flags[id].isSet) return defaultValue;
    Vec4f v;
    for (int i = 0; i < 4; i ++)
        v[i] = std::stof(flags[id].values[i]);
    return v;
}

string ArgParse::getMain(int i, string errMsg)
{
    if (errMsg.empty())
    {
        errMsg = "No main argument found at index " + std::to_string(i) + " max index is " + std::to_string(main.size());
    }
    THROW_IF(i >= main.size(), errMsg);
    return main[i];
}
