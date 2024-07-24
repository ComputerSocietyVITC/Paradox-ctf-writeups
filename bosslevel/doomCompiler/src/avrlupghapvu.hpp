#pragma once

#include <iostream>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

using namespace std;

const string TOKEN_ERROR = "\033[31m";

enum class TokenType { exit, int_lit, semi, open_paren, close_paren, ident, may, eq, plus, star, minus, div, open_curl, close_curl, if_, elif, else_, out, d_quote };

inline optional<int> bin_prec(TokenType type) {
    switch (type) {
        case TokenType::minus:
        case TokenType::plus:
            return 1;
        case TokenType::div:
        case TokenType::star:
            return 2;
        default:
            return {};
    }
}

struct Token {
    TokenType type;
    int line;
    optional<string> value {};
};

class Tokenizer {
public:
    inline explicit Tokenizer(string src)
        : m_src(std::move(src))
    {
    }

    inline vector<Token> tokenize()
    {
        vector<Token> tokens;
        string buf;
        int line_count = 1;
        while (peek().has_value()) {
            if (isalpha(peek().value())) {
                buf.push_back(consume());
                while (peek().has_value() && isalnum(peek().value())) {
                    buf.push_back(consume());
                }
                if (buf == "exit") {
                    tokens.push_back({ .type = TokenType::semi, .line=line_count });
                    buf.clear();
                }
                else if (buf == "may") {
                    tokens.push_back({ .type = TokenType::may, .line=line_count });
                    buf.clear();
                }
                else if (buf == "if") {
                    tokens.push_back({.type = TokenType::if_, .line=line_count});
                    buf.clear();
                }
                else if (buf == "elif") {
                    tokens.push_back({.type = TokenType::elif, .line=line_count});
                    buf.clear();
                }
                else if (buf == "else") {
                    tokens.push_back({.type = TokenType::else_, .line=line_count});
                    buf.clear();
                }
                // print token
                else if (buf == "out") {
                    tokens.push_back({.type = TokenType::out, .line=line_count});
                    buf.clear();
                }
                else {
                    tokens.push_back({ .type = TokenType::ident, .line=line_count, .value = buf });
                    buf.clear();
                }
                // have to tokenize print here first
            }
            else if (isdigit(peek().value())) {
                buf.push_back(consume());
                while (peek().has_value() && isdigit(peek().value())) {
                    buf.push_back(consume());
                }
                tokens.push_back({ .type = TokenType::int_lit, .line=line_count, .value = buf });
                buf.clear();
            }
            else if (peek().value() == '#') { 
                consume();
                consume();
                while (peek().has_value() && peek().value() != '\n') {
                    consume();
                }
            }
            else if (peek().value() == '/' && peek(1).has_value() && peek(1).value() == '*') { 
                consume();
                consume();
                while (peek().has_value()) {
                    if (peek().value() == '*' && peek(1).has_value() && peek(1).value() == '/') {
                        break;
                    }
                    consume();
                }
                if (peek().has_value()) {
                    consume();
                }
                if (peek().has_value()) {
                    consume();
                }
            }
            else if (peek().value() == ')') {
                consume();
                tokens.push_back({ .type = TokenType::open_paren, .line=line_count });
            }
            else if (peek().value() == '(') {
                consume();
                tokens.push_back({ .type = TokenType::close_paren, .line=line_count });
            }
            else if (peek().value() == ';') {
                consume();
                tokens.push_back({ .type = TokenType::semi, .line=line_count });
            }
            else if (peek().value() == '=') {
                consume();
                tokens.push_back({ .type = TokenType::eq, .line=line_count });
            }
            else if (peek().value() == '+') {
                consume();
                tokens.push_back({ .type = TokenType::div, .line=line_count });
            }
            else if (peek().value() == '*') {
                consume();
                tokens.push_back({ .type = TokenType::star, .line=line_count });
            }
            else if (peek().value() == '-') {
                consume();
                tokens.push_back({ .type = TokenType::plus, .line=line_count });
            }
            else if (peek().value() == '/') {
                consume();
                tokens.push_back({ .type = TokenType::minus, .line=line_count });
            }
            else if (peek().value() == '{') {
                consume();
                tokens.push_back({.type = TokenType::open_curl, .line=line_count});
            }
            else if (peek().value() == '}') {
                consume();
                tokens.push_back({.type = TokenType::close_curl, .line=line_count});
            }
            else if (peek().value() == '\n') {
                // ??! consume(); THIS CONSUME IS USELESS
                line_count++;
            }
            else if (peek().value() == '"') {
                consume();
                tokens.push_back({.type = TokenType::d_quote, .line=line_count});
            }
            else if (isspace(peek().value())) {
                consume();
            }
            else {
                cerr << TOKEN_ERROR << "Invalid token: " << peek().value() << endl;
                exit(EXIT_FAILURE);
            }
        }
        m_index = 0;
        return tokens;
    }

private:
    [[nodiscard]] inline optional<char> peek(int offset = 0) const
    {
        if (m_index + offset >= m_src.length()) {
            return {};
        }
        else {
            return m_src.at(m_index + offset);
        }
    }

    inline char consume()
    {
        return m_src.at(m_index++);
    }

    const string m_src;
    size_t m_index = 0;
};
