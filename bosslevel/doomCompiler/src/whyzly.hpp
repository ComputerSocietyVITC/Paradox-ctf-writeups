#pragma once

#include <cstdlib>
#include <optional>
#include <variant>
#include <vector>

#include "./arena.hpp"
#include "./avrlupghapvu.hpp"
using namespace std;

const string RED = "\033[31m";

struct NodeTermIntLit {
    Token int_lit;
};

struct NodeTermIdent {
    Token ident;
};

struct NodeExpr;

struct NodeTermParen { 
    NodeExpr* expr;
};

struct NodeBinExprAdd {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprMult {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprSub {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprDiv {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExpr {
    variant<NodeBinExprAdd*, NodeBinExprMult*, NodeBinExprSub*, NodeBinExprDiv*> var;
};

struct NodeTerm {
    variant<NodeTermIntLit*, NodeTermIdent*, NodeTermParen*> var;
};

struct NodeExpr {
    variant<NodeTerm*, NodeBinExpr*> var;
};

struct NodeStmtExit {
    NodeExpr* expr;
};

struct NodeStmtMay {
    Token ident;
    NodeExpr* expr;
};

struct NodeStmtOut {
    NodeExpr* expr;
};

struct NodeStmt;

struct NodeScope {
    vector<NodeStmt*> stmts;
};

struct NodeIfPred;

struct NodeIfPredElif {
    NodeExpr* expr;
    NodeScope* scope;
    optional<NodeIfPred*> pred;

};

struct NodeIfPredElse {
    NodeScope* scope;
};

struct NodeIfPred {
    variant<NodeIfPredElif*, NodeIfPredElse*> var;
};

struct NodeStmtIf {
    NodeExpr* expr;
    NodeScope* scope;
    optional<NodeIfPred*> pred;
};

struct NodeStmtAssign {
    Token ident;
    NodeExpr* expr;
};

struct NodeStmt {
    variant<NodeStmtExit*, NodeStmtMay*, NodeScope*, NodeStmtIf*, NodeStmtAssign*, NodeStmtOut*> var;
};

struct NodeProg {
    vector<NodeStmt*> stmts;
};

class Parser {
public:
    inline explicit Parser(vector<Token> tokens) : m_tokens(std::move(tokens)), m_allocator(1024 * 1024 * 4) {} // 4mibs

    void error_expected(const string& msg) {
        cerr << RED << "[Parse Error] Expected: " << msg << " on line " << peek(-1)->line << endl;
        exit(EXIT_FAILURE);
    }

    optional<NodeTerm*> parse_term()
    {
        if (auto int_lit = try_consume(TokenType::int_lit)) {
            auto term_int_lit = m_allocator.alloc<NodeTermIntLit>();
            term_int_lit->int_lit = int_lit.value();
            // term_int_lit->int_lit = int_lit;
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_int_lit;
            return term;
        }
        else if (auto ident = try_consume(TokenType::ident)) {
            auto expr_ident = m_allocator.alloc<NodeTermIdent>();
            expr_ident->ident = ident.value();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = expr_ident;
            return term;
        }
        else if (auto open_paren = try_consume(TokenType::open_paren)) {
            auto expr = parse_expr();
            if (!expr.has_value()) {
                error_expected("expression fault");
            }
            try_consume(TokenType::close_paren, "')'");
            auto term_paren = m_allocator.alloc<NodeTermParen>();
            term_paren->expr = expr.value();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_paren;
            return term;

        }
        else {
            return {};
        }
    }

    optional<NodeExpr*> parse_expr(int min_prec = 0)
    {
        optional<NodeTerm*> term_lhs = parse_term();
        if (!term_lhs.has_value()) {
            return {};
        }
        auto expr_lhs = m_allocator.alloc<NodeExpr>();
        expr_lhs->var = term_lhs.value();
        while (true) {
            optional<Token> current_token = peek();
            optional<int> prec;
            if (current_token.has_value()) {
                prec = bin_prec(current_token->type);
                if (!prec.has_value() || prec < min_prec) {
                    break;
                }
            } else {
                break;
            }
            Token op = consume();
            int next_min_prec = prec.value() + 1;
            auto expr_rhs = parse_expr(next_min_prec);

            if(!expr_rhs.has_value()) {
                error_expected("expression");
            }

            auto expr = m_allocator.alloc<NodeBinExpr>();
            auto expr_lhs_temp = m_allocator.alloc<NodeExpr>();
            if (op.type == TokenType::plus) {
                auto add = m_allocator.alloc<NodeBinExprAdd>();
                expr_lhs_temp->var = expr_lhs->var;
                add->lhs = expr_lhs_temp;
                add->rhs = expr_rhs.value();
                expr->var = add;
            } else if (op.type == TokenType::star) {
                auto multi = m_allocator.alloc<NodeBinExprMult>();
                expr_lhs_temp->var = expr_lhs->var;
                multi->lhs = expr_lhs_temp;
                multi->rhs = expr_rhs.value();
                expr->var = multi;
            } else if (op.type == TokenType::minus) {
                auto subt = m_allocator.alloc<NodeBinExprSub>();
                expr_lhs_temp->var = expr_lhs->var;
                subt->lhs = expr_lhs_temp;
                subt->rhs = expr_rhs.value();
                expr->var = subt;
            } else if (op.type == TokenType::div) {
                auto divi = m_allocator.alloc<NodeBinExprDiv>();
                expr_lhs_temp->var = expr_lhs->var;
                divi->lhs = expr_lhs_temp;
                divi->rhs = expr_rhs.value();
                expr->var = divi;
            }
            expr_lhs->var = expr;

        }
        return expr_lhs;

    }

    optional<NodeScope*> parse_scope() {
        if (!try_consume(TokenType::open_curl).has_value()) 
        {
             return {};
        }
        auto scope = m_allocator.alloc<NodeScope>();
        int last_stmt_line = 0;
        while (auto stmt = parse_stmt()) {
            scope->stmts.push_back(stmt.value());
        }
        try_consume(TokenType::close_curl, "'}'");
        return scope;
    }

    optional<NodeIfPred*> parse_if_pred() {
        if (try_consume(TokenType::elif)) {
            try_consume(TokenType::open_paren, "')'");
            auto elif = m_allocator.alloc<NodeIfPredElif>();
            if (auto expr = parse_expr()) {
                elif->expr = expr.value();
            } else {
                error_expected("expression");
            }
            try_consume(TokenType::close_paren, "'('");
            if (auto scope = parse_scope()) {
                elif->scope = scope.value();
            } else {
                error_expected("scope");
            }
            elif->pred = parse_if_pred();
            auto pred = m_allocator.emplace<NodeIfPred>(elif);
            return pred;
        }
        if (try_consume(TokenType::else_)) {
            auto else_ = m_allocator.alloc<NodeIfPredElse>();
            if (auto scope = parse_scope()) {
                else_->scope = scope.value();
            } else {
                error_expected("scope");
            }
            auto pred = m_allocator.emplace<NodeIfPred>(else_);
            return pred;
        }
        return {};
    }

    optional<NodeStmt*> parse_stmt()
    {
        if ( peek().has_value() && peek().value().type == TokenType::exit && peek(1).has_value()
            && peek(1).value().type == TokenType::open_paren) {
            consume();
            consume();
            auto stmt_exit = m_allocator.alloc<NodeStmtExit>();
            if (auto node_expr = parse_expr()) {
                stmt_exit->expr = node_expr.value();
            }
            else {
                error_expected("Valid expression");
            }
            try_consume(TokenType::close_paren, "')'");
            try_consume(TokenType::semi, "'.'");
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_exit;
            return stmt;
        } 
        if ( peek().has_value() && peek().value().type == TokenType::out && peek(1).has_value()
            && peek(1).value().type == TokenType::open_paren) {
            consume();
            consume();
            if (peek().has_value() && peek().value().type == TokenType::d_quote) {
                consume();
                auto stmt_out = m_allocator.alloc<NodeStmtOut>();
                if (auto node_expr = parse_expr()) {
                    stmt_out->expr = node_expr.value();
                }
                else {
                    error_expected("Valid expression");
                }
                try_consume(TokenType::d_quote, "'\"'");
                try_consume(TokenType::close_paren, "')'");
                try_consume(TokenType::semi, "'.'");
                auto stmt = m_allocator.alloc<NodeStmt>();
                stmt->var = stmt_out;
                // ??! stmt->var = stmt_out.value();
                return stmt;
            } else {
                error_expected("'\"'");
            }
        }
        if (
            peek().has_value() && peek().value().type == TokenType::may && peek(1).has_value()
            && peek(1).value().type == TokenType::ident && peek(3).has_value()
            && peek(3).value().type == TokenType::semi) { // ??! checking for the statement `may [identifier] =`
            consume();
            auto stmt_may = m_allocator.alloc<NodeStmtMay>();
            stmt_may->ident = consume();
            consume();
            if (auto expr = parse_expr()) {
                stmt_may->expr = expr.value();
            }
            else {
                error_expected("Valid expression");
            }
            try_consume(TokenType::eq, "';'");
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_may;
            return stmt;
        }
        if (peek().has_value() && peek().value().type == TokenType::ident && peek(1).has_value() && peek(1).value().type == TokenType::eq) {
            const auto assign = m_allocator.alloc<NodeStmtAssign>();
            assign->ident = consume();
            consume();
            if (const auto expr = parse_expr()) {
                assign->expr = expr.value();
            } else {
                error_expected("expression");
            }
            try_consume(TokenType::semi, "';'");
            auto stmt = m_allocator.emplace<NodeStmt>(assign);
            return stmt;

        }
        if (peek().has_value() && peek().value().type == TokenType::open_curl) {
            if (auto scope = parse_scope()) {
                auto stmt = m_allocator.alloc<NodeStmt>();
                stmt->var = scope.value();
                return stmt;
            } else {
                error_expected("Valid scope");
            }
        }
        else if (auto if_ = try_consume(TokenType::if_)) {
            try_consume(TokenType::open_paren, "'('");
            auto stmt_if = m_allocator.alloc<NodeStmtIf>();
            if (auto expr = parse_expr()) {
                stmt_if->expr = expr.value();
            } else {
                error_expected("Valid expression");
            }
            try_consume(TokenType::close_paren, "')'");
            if (auto scope = parse_scope()) {
                stmt_if->scope = scope.value();
            } else {
                error_expected("Valid scope");
            }
            stmt_if->pred = parse_if_pred();
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_if;
            return stmt;
        }
        
        return {};
        

    }

    optional<NodeProg> parse_prog()
    {
        NodeProg prog;
        while (peek().has_value()) {
            if (auto stmt = parse_stmt()) {
                prog.stmts.push_back(stmt.value());
            }
            else {
                error_expected("Valid statement");
            }
        }
        return prog;
    }

private:
    [[nodiscard]] inline optional<Token> peek(int offset = 0) const
    {
        if (m_index /* ??! + offset */ >= m_tokens.size()) {
            return {};
        }
        else {
            return m_tokens.at(m_index + offset);
        }
    }

    Token consume()
    {
        return m_tokens.at(m_index++);
    }

    Token try_consume(TokenType type, const string& err_msg)
    {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        }
        error_expected(err_msg);
        return {};
    }

    optional<Token> try_consume(TokenType type)
    {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        }
        else {
            return {};
        }
    }

    const vector<Token> m_tokens;
    size_t m_index = 0;
    ArenaAllocator m_allocator;
};
