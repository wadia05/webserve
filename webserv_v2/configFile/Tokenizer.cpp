#include "Tokenizer.hpp"

void print_error(const std::string &error, int *i)
{
    if (*i == 0)
        *i = 1;
    std::cout << RED << "Error: " << error << RESET << std::endl;
}

bool Tokenizer::is_special_char(char c)
{
    return (c == '\n' || c == ';' || c == '{' || c == '}' || c == '"' || c == '\'' || c == '#');
}

bool Tokenizer::is_whitespace(char c)
{
    return (c == 32 || c == 9 || (c >= 11 && c <= 13));
}

Tokenizer::Tokenizer(std::ifstream &file) : file(file)
{
    if (!file.is_open())
    {
        std::cerr << "Error: file not found" << std::endl;
        exit(1);
    }
}

Tokenizer::~Tokenizer()
{
    file.close();
}

void Tokenizer::processFile()
{
    std::string liness;
    std::string line;
    while (std::getline(file, line))
    {
        if (line == "<|||||||||>")
        {
            this->lines.push_back(liness);
            liness.clear();
        }
        else
            liness += line + '\n';
    }
    if (!liness.empty())
        this->lines.push_back(liness);
    if (this->lines.empty())
    {
        std::cerr << "Error: empty file" << std::endl;
        exit(1);
    }
    size_t i = 0;
    for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it)
    {
        i = 0;
        for (std::string::iterator it2 = it->begin(); it2 != it->end(); ++it2)
        {
            if (is_whitespace(*it2) || *it2 == '\n')
                i++;
        }
        if (i == it->size())
        {
            std::cerr << "Error: empty line" << std::endl;
            exit(1);
        }
    }
}

void Tokenizer::processLines(const std::string &lines, std::vector<t_state> &F_states)
{
    std::string::const_iterator it = lines.begin();
    while (it != lines.end())
    {
        t_state state = {UNKNOWN, ""};
        if (is_special_char(*it))
        {
            state.value = *it;
            if (*it == '\n')
                state.state = NEWLINE;
            else if (*it == '#')
                state.state = COMMENT;
            else if (*it == ';' || *it == '{' || *it == '}')
                state.state = SYMBOL;
            else if (*it == '"' || *it == '\'')
                state.state = QUOTE;
            ++it;
        }
        else if (is_whitespace(*it))
        {
            std::string::const_iterator start = it;
            while (it != lines.end() && is_whitespace(*it))
                ++it;
            state.value = std::string(start, it);
            state.state = WHITESPACE;
        }
        else
        {
            std::string::const_iterator start = it;
            while (it != lines.end() && !is_special_char(*it) && !is_whitespace(*it))
                ++it;
            state.value = std::string(start, it);
            state.state = STRING;
        }
        F_states.push_back(state);
    }
}

void Tokenizer::processStates(std::vector<t_state> &F_states, int *i)
{
    char active_quote = '\0';
    std::vector<t_state>::iterator it = F_states.begin();
    while (it != F_states.end())
    {
        int j = 0;
        if (it->state == QUOTE)
        {
            if (active_quote == '\0')
            {
                active_quote = it->value[0];
                it = F_states.erase(it);
            }
            else if (active_quote == it->value[0])
            {
                active_quote = '\0';
                it = F_states.erase(it);
            }
            else
                j = 1;
        }
        else if (active_quote != '\0')
        {
            std::vector<t_state>::iterator start = it;
            std::string value;
            while (it != F_states.end() && !(it->state == QUOTE && it->value[0] == active_quote))
            {
                if (it->state == NEWLINE)
                    it = F_states.erase(it);
                else
                {
                    value += it->value;
                    it = F_states.erase(it);
                }
            }
            if (it != F_states.end() && it->state == QUOTE && it->value[0] == active_quote)
            {
                active_quote = '\0';
                it = F_states.erase(it);
            }
            t_state state = {STRING, value};
            it = F_states.erase(start, it);
            if (value == "{" || value == "}" || value == ";")
                state.state = SYMBOL;
            else
            {
                std::string::const_iterator it = value.begin();
                while (it != value.end() && (is_whitespace(*it) || *it == '\n'))
                    ++it;
                if (it == value.end())
                    state.state = WHITESPACE;
            }
            it = F_states.insert(it, state);
            ++it;
        }
        else
        {
            if (it->state == WHITESPACE)
                it = F_states.erase(it);
            else if (it->state == COMMENT)
            {
                while (it != F_states.end() && it->state != NEWLINE)
                    it = F_states.erase(it);
            }
            else
                ++it;
        }
        if (j == 1)
            it->state = STRING;
    }
    for (std::vector<t_state>::iterator it = F_states.begin(); it != F_states.end(); ++it)
    {
        if (it->state == WHITESPACE)
        {
            F_states.erase(it);
            --it;
        }
        if (it->state == NEWLINE && (it + 1) != F_states.end() && (it + 1)->state == NEWLINE)
        {
            F_states.erase(it);
            --it;
        }
    }
    for (std::vector<t_state>::iterator it = F_states.begin(); it != F_states.end(); ++it)
    {
        if (it->state == SYMBOL && ((it + 1) != F_states.end() && (it + 1)->state != NEWLINE))
        {
            t_state state = {NEWLINE, "\n"};
            it = F_states.insert(it + 1, state);
        }
    }
    for (std::vector<t_state>::iterator it = F_states.begin(); it != F_states.end(); ++it)
    {
        if (it->state == SYMBOL && (it->value == "{" || it->value == ";"))
        {
            if (it != F_states.begin() && (it - 1)->state == NEWLINE && (it + 1) != F_states.end() && (it + 1)->state == NEWLINE)
            {
                F_states.erase(it - 1);
                --it;
            }
        }
    }
    for (std::vector<t_state>::iterator it = F_states.begin(); it != F_states.end(); ++it)
    {
        if (it->state == SYMBOL && it->value == ";")
        {
            if (it != F_states.begin() && ((it - 1)->state == NEWLINE || (it - 1)->state == SYMBOL))
            {
                print_error("Error: unexpected ;", i);
                return;
            }
            if ((it + 1) != F_states.end() && (it + 1)->value == "{")
            {
                print_error("Error: unexpected ;", i);
                return;
            }
        }
    }
    if (active_quote != '\0')
    {
        print_error("Error: unclosed quote", i);
        return;
    }
}

void Tokenizer::tokenizeStates(std::vector<t_state> &F_states, std::vector<t_token> &tokens, int *x)
{
    int i = 0;
    int j = 0;
    for (std::vector<t_state>::iterator it = F_states.begin(); it != F_states.end(); ++it)
    {
        t_token token = {UNDEFINED, ""};
        if (it->state == SYMBOL)
        {
            if (it->value == "{")
                token.type = OPEN_BRACKET;
            else if (it->value == "}")
                token.type = CLOSE_BRACKET;
            else if (it->value == ";")
                token.type = SEMICOLON;
            token.value = it->value;
        }
        else if (it->state == STRING)
        {
            std::vector<t_state> tmp;
            for (std::vector<t_state>::iterator it2 = it; it2 != F_states.end(); ++it2)
            {
                if (it2->state == NEWLINE)
                    break;
                if (it2->state == SYMBOL && it2->value == "{")
                    i = 1;
            }
            if (i == 1 && j == 0)
            {
                token.type = BLOCK;
                j = 1;
            }
            else if (it != F_states.begin() && (tokens.back().type == BLOCK || tokens.back().type == AFTER_BLOCK))
                token.type = AFTER_BLOCK;
            else if (it == F_states.begin() || (it - 1)->state == NEWLINE)
                token.type = KEY;
            else
                token.type = VALUE;
            token.value = it->value;
            i = 0;
        }
        else if (it->state == NEWLINE)
        {
            j = 0;
            continue;
        }
        tokens.push_back(token);
    };
    int open_brackets = 0;
    int close_brackets = 0;
    for (std::vector<t_token>::iterator it = tokens.begin(); it != tokens.end(); ++it)
    {
        if (it->type == VALUE && (it + 1) != tokens.end())
        {
            if ((it + 1)->type == VALUE)
            {
                while ((it + 1) != tokens.end() && (it + 1)->type == VALUE)
                    ++it;
                if ((it + 1) == tokens.end() || (it + 1)->type != SEMICOLON)
                {
                    print_error("Error: missing ;", x);
                    return;
                }
            }
            else if ((it + 1)->type != SEMICOLON)
            {
                print_error("Error: missing ;", x);
                return;
            }
        }
        if (it->type == OPEN_BRACKET)
            open_brackets++;
        else if (it->type == CLOSE_BRACKET)
            close_brackets++;
    }
    if (open_brackets != close_brackets)
    {
        print_error("Error: missing }{", x);
        return;
    }
    for (std::vector<t_token>::iterator it = tokens.begin(); it != tokens.end(); ++it)
    {
        if (it->type == SEMICOLON && (it == tokens.begin() || (it - 1)->type != VALUE))
        {
            print_error("Error: unexpected ;", x);
            return;
        }
        else if (it->type == CLOSE_BRACKET && (it == tokens.begin() || ((it - 1)->type != SEMICOLON && (it - 1)->type != CLOSE_BRACKET && (it - 1)->type != OPEN_BRACKET)))
        {
            print_error("Error: unexpected }", x);
            return;
        }
        else if (it->type == OPEN_BRACKET && (it == tokens.begin() || ((it - 1)->type != BLOCK && (it - 1)->type != AFTER_BLOCK)))
        {
            print_error("Error: unexpected {", x);
            return;
        }
    }
    for (std::vector<t_token>::iterator it = tokens.begin(); it != tokens.end(); ++it)
    {
        if (it->type == BLOCK && it->value == "location")
        {
            for (std::vector<t_token>::iterator it2 = it + 1; it2 != tokens.end() && it2->type != CLOSE_BRACKET; ++it2)
            {
                if (it2->type == KEY)
                    it2->type = LOCATION_KEY;
                else if(it2->type == BLOCK && it2->value == "location")
                {
                    print_error("Error: nested location block", x);
                    return;
                }
            }
        }
        if (it->type == KEY || it->type == LOCATION_KEY)
        {
            if ((it + 1) != tokens.end() && (it + 1)->type != VALUE)
            {
                print_error("Error: missing value", x);
                return;
            }
        }
    }
    for (std::vector<t_token>::iterator it = tokens.begin(); it != tokens.end(); ++it)
    {
        if (it->type == SEMICOLON || it->type == OPEN_BRACKET || it->type == CLOSE_BRACKET)
        {
            tokens.erase(it);
            --it;
        }
        if (it->type == AFTER_BLOCK)
        {
            if ((it - 1)->type == AFTER_BLOCK || ((it - 1)->type == BLOCK && (it - 1)->value != "location"))
            {
                print_error("Error: After block", x);
                return;
            }
        }
    }
}

std::vector<std::string> Tokenizer::getLines()
{
    return lines;
}

std::vector<t_token> Tokenizer::tokenize(std::string lines)
{
    std::vector<t_state> F_states;
    processLines(lines, F_states);
    int i = 0;
    processStates(F_states, &i);
    if (i == 1)
        return std::vector<t_token>();
    std::vector<t_token> tokens;
    tokenizeStates(F_states, tokens, &i);
    if (i == 1)
        return std::vector<t_token>();
    return tokens;
}