#pragma once
#include "../webserv.hpp"

void print_error(const std::string &error, int *i);

enum State
{
    STRING,
    QUOTE,
    SYMBOL,
    WHITESPACE,
    COMMENT,
    NEWLINE,
    UNKNOWN
};
typedef struct s_state
{
    State state;
    std::string value;
} t_state;

enum TokenType
{
    VALUE,
    OPEN_BRACKET,
    CLOSE_BRACKET,
    SEMICOLON,
    KEY,
    BLOCK,
    AFTER_BLOCK,
    LOCATION_KEY,
    UNDEFINED,
};
typedef struct s_token
{
    TokenType type;
    std::string value;
} t_token;

class Tokenizer
{
private:
    std::ifstream &file;
    std::vector<std::string> lines;
    bool is_special_char(char c);
    bool is_whitespace(char c);
    void processLines(const std::string &lines, std::vector<t_state> &F_states);
    void processStates(std::vector<t_state> &F_states , int *i);
    void tokenizeStates(std::vector<t_state> &F_states, std::vector<t_token> &tokens, int *x);

public:
    Tokenizer() : file(*(new std::ifstream())) {}
    Tokenizer(std::ifstream &file);
    ~Tokenizer();
    void processFile();
    std::vector<std::string> getLines();
    std::vector<t_token> tokenize(std::string lines);
};
