#pragma once

#include <pwos/common.h>

enum ArgType
{
    INT,
    STR,
    VEC4f,
    VEC2i
};

/**
 * Helper struct that represents a particular flag.
 */
struct Arg
{
    string id;
    ArgType type;
    vector<string> values;

    bool isSet = false;

    /**
     * Default constructor
     */
    Arg();
    
    /**
     * Constructor for Arg with both id and the type
     * 
     * @param id
     * @param type
     */
    Arg(std::string id, ArgType type);

    /**
     * Set the value for the arg (assumes one value, overrwrites values[0])
     * 
     * @param value
     * 
     */
    void setValue(string value);

    /**
     * Push a new value onto the values array (does not assume only one value)
     * 
     * @param value
     */
    void pushValue(string value);
};

/**
 * Helper for parsing command line arguments.
 */
class ArgParse
{
public:
    /**
     * Initialize with a list of options to parse.
     * 
     * @param flagList     the flags to look for when parsing
     */
    ArgParse(vector<Arg> flagList);

    /**
     * Parse the command line arguments.
     * 
     * @param argc
     * @param argv
     * 
     */
    void parse(int argc, char* argv[]);

    /**
     * Return the int corresponding to the flag with id "id".
     * Throws an error if arg corresponding to id is not an int.
     * 
     * @param id
     * @param defaultValue
     * 
     * @returns the int corresponding to id or default value.
     */
    int getInt(string id, int defaultValue);

    /**
     * Return the string corresponding to the flag with id "id".
     * Throws an error if arg corresponding to id is not a string.
     * 
     * @param id
     * @param defaultValue
     * 
     * @returns the string corresponding to id or default value.
     */
    string getStr(string id, string defaultValue);

    /**
     * Return the Vec2i corresponding to the flag with id "id".
     * Throws an error if arg corresponding to id is not a vec2i
     * 
     * @param id
     * @param defaultValue
     * 
     * @returns the vec2i corresponding to id or default value
     */

    Vec2i getVec2i(string id, Vec2i defaultValue);

    /**
     * Return the Vec4f corresponding to the flag with id "id".
     * Throws an error if arg corresponding to id is not a vec4f
     * 
     * @param id
     * @param defaultValue
     * 
     * @returns the vec4f corresponding to id or default value
     */
    Vec4f getVec4f(string id, Vec4f defaultValue);

    /**
     * Returns the string corresponding to the ith non-flagged argument
     * 
     * @param i
     * 
     * @returns the ith main argument (not including the program name in indexing)
     */
    string getMain(int i, string errMsg = "");

private:
    // main args (i.e. no flags, in order they appear)
    vector<string> main;

    // flagged arguments (i.e. --opt or -opt)
    map<string, Arg> flags;
};
