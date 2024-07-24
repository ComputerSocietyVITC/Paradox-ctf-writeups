#pragma once

#include "whyzly.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <iosfwd>
#include <optional>
#include <sstream>
#include <variant>
#include <vector>
using namespace std;

const string whitespace = "     ";

class Generator {
public:
    inline explicit Generator(NodeProg prog) : m_prog(std::move(prog)) {}

    void gen_term(const NodeTerm* term) {
        struct TermVisitor {
            Generator& gen;
            void operator()(const NodeTermIntLit* term_int_lit) const {
                gen.m_output << whitespace << "mov rax, " << term_int_lit->int_lit.value.value() << "\n";
                gen.push("rax");
            }
            void operator()(const NodeTermIdent* term_ident) const {
                auto iterator = find_if(gen.m_vars.cbegin(), gen.m_vars.cend(), [&](const Var& var) {return var.name == term_ident->ident.value.value();});
                if (iterator ==gen.m_vars.cend()) {
                    cerr << RED << "[Token Error] " << "Undeclared identifier: " << term_ident->ident.value.value() << endl;
                    exit(EXIT_FAILURE);
                }
                stringstream offset;
                offset << "QWORD [rsp + " << (gen.m_stack_size - (*iterator).stack_loc - 1) * 8 << "]";
                gen.push(offset.str());
            }
            void operator()(const NodeTermParen* paren) const {
                gen.gen_expr(paren->expr);
            }
        };
        TermVisitor visitor({.gen = *this});
        visit(visitor, term->var);
    }

    void gen_bin_expr(const NodeBinExpr* bin_expr) {
        struct BinExprVisitor {
            Generator& gen;
            void operator()(const NodeBinExprSub* sub) const {
                gen.m_output << whitespace << ";; sub op\n";
                gen.gen_expr(sub->rhs);
                gen.gen_expr(sub->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                gen.m_output << whitespace << "sub rax, rbx\n";
                gen.push("rax");
                gen.m_output << whitespace << ";; /sub op\n";
            }
            void operator()(const NodeBinExprAdd* add) const {
                gen.m_output << whitespace << ";; add op\n";
                gen.gen_expr(add->rhs);
                gen.gen_expr(add->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                gen.m_output << whitespace << "add rax, rbx\n";
                gen.push("rax");
                gen.m_output << whitespace << ";; /add op\n";

            }
            void operator()(const NodeBinExprDiv* div) const {
                gen.m_output << whitespace << ";; div op\n";
                gen.gen_expr(div->rhs);
                gen.gen_expr(div->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                // gen.m_output << whitespace << "div rbx\n" <-THIS IS WRONG?!!!
                gen.m_output << whitespace << "div rax, rbx\n";
                gen.push("rax");
                gen.m_output << whitespace << ";; /div op\n";
            }
            void operator()(const NodeBinExprMult* mult) const {
                gen.m_output << whitespace << ";; mult op\n";
                gen.gen_expr(mult->rhs);
                gen.gen_expr(mult->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                gen.m_output << whitespace << "mul rbx\n";
                gen.push("rax");
                gen.m_output << whitespace << ";; /mult op\n";
            }
        };
        BinExprVisitor visitor{.gen = *this};
        visit(visitor, bin_expr->var);
    }

    void gen_expr(const NodeExpr* expr)
    {
        struct ExprVisitor {
            Generator& gen;
            void operator()(const NodeTerm* term) const
            {
                gen.gen_term(term);
            }
            void operator()(const NodeBinExpr* bin_expr) const
            {
                gen.gen_bin_expr(bin_expr);
            }
        };

        ExprVisitor visitor { .gen = *this };
        visit(visitor, expr->var);
    }

    void gen_scope(const NodeScope* scope) {
        begin_scope();
        for (const NodeStmt* stmt : scope->stmts) {
            gen_stmt(stmt);
        }
        end_scope();
    }

    void gen_ifpred(const NodeIfPred* pred, const string& end_label) {
        struct PredVisitor {
            Generator& gen;
            const string& end_label;
            void operator()(const NodeIfPredElif* elif) const {
                gen.m_output << whitespace << ";; elif\n";
                gen.gen_expr(elif->expr);
                gen.pop("rax");
                string label = gen.create_label();
                gen.m_output << whitespace << "test rax, rax\n";
                gen.m_output << whitespace << "jz " << label << "\n";
                gen.gen_scope(elif->scope);
                gen.m_output << whitespace << "jmp " << end_label << "\n";
                if (elif->pred.has_value()) {
                    gen.m_output << label << ":\n";
                    gen.gen_ifpred(elif->pred.value(), end_label);
                }
                gen.m_output << whitespace << ";; /elif\n";
            }
            void operator()(const NodeIfPredElse* else_) const {
                gen.m_output << whitespace << ";; else\n";
                gen.gen_scope(else_->scope);
                gen.m_output << whitespace << ";; /else\n";
            }
        };
        PredVisitor visitor{.gen = *this, .end_label = end_label};
        visit(visitor, pred->var);

    }

    void gen_stmt(const NodeStmt* stmt)
    {
        struct StmtVisitor {
            Generator& gen;
            void operator()(const NodeStmtExit* stmt_exit) const
            {
                gen.m_output << whitespace << ";; exit\n";
                gen.gen_expr(stmt_exit->expr);
                gen.m_output << whitespace << "mov rax, 60\n";
                gen.pop("rdi");
                gen.m_output << whitespace << "syscall\n";
                gen.m_output << whitespace << ";; /exit\n";
            }
            void operator()(const NodeStmtMay* stmt_may) const
            {
                gen.m_output << whitespace << ";; may\n";
                auto iterator = find_if(gen.m_vars.cbegin(), gen.m_vars.cend(), [&](const Var& var) {return var.name == stmt_may->ident.value.value();});
                if (iterator != gen.m_vars.cend()) {
                    cerr << RED << "Identifier already used: " << stmt_may->ident.value.value() << endl;
                    exit(EXIT_FAILURE);
                }
                gen.m_vars.push_back({ .name = stmt_may->ident.value.value(), .stack_loc = gen.m_stack_size });
                gen.gen_expr(stmt_may->expr);
            }
            void operator()(const NodeScope* scope) const {
               gen.gen_scope(scope);
            }
            void operator()(const NodeStmtIf* stmt_if) const
            {
                gen.m_output << whitespace << ";; if\n";
                gen.gen_expr(stmt_if->expr);
                gen.pop("rax");
                const std::string label = gen.create_label();
                gen.m_output << whitespace << "test rax, rax\n";
                gen.m_output << whitespace << "jz " << label << "\n";
                gen.gen_scope(stmt_if->scope);
                if (stmt_if->pred.has_value()) {
                    string end_label;
                    end_label = gen.create_label();
                    gen.m_output << whitespace << "jmp " << end_label << "\n";
                    gen.m_output << label << ":\n";
                    gen.gen_ifpred(stmt_if->pred.value(), end_label);
                    gen.m_output << end_label << ":\n";
                } else {
                    gen.m_output << label << ":\n";
                }
                
                gen.m_output << whitespace << ";; /if\n";
            } 
            void operator()(const NodeStmtAssign* stmt_assign) const {
                    const auto it = find_if(gen.m_vars.cbegin(), gen.m_vars.cend(), [&](const Var& var){return var.name == stmt_assign->ident.value.value();});
                    if (it == gen.m_vars.end()) {
                        cerr << RED << "[Assign Error] " << "Undeclared identifier: " << stmt_assign->ident.value.value() << endl;
                        exit(EXIT_FAILURE);
                    }
                    gen.gen_expr(stmt_assign->expr);
                    gen.pop("rax");
                    gen.m_output << whitespace << "mov [rsp + " << (gen.m_stack_size - it->stack_loc - 1) * 8 << "], rax\n";
            }
            void operator()(const NodeStmtOut* stmt_out) const { 
                static bool data_section_written = false;
                static int print_call_counter = 0;
                ++print_call_counter;

                string convert_loop_label = "convert_loop_" + to_string(print_call_counter);
                string print_loop_label = "print_loop_" + to_string(print_call_counter);
                string end_print_loop_label = "end_print_loop_" + to_string(print_call_counter);

                // Keeping track of current index
                const streampos pos_before = gen.m_output.tellp();
                
                // Go to the top index and insert the new sections
                gen.m_output.seekp(0, std::ios::beg);
                
                // Create a temporary buffer to hold the original content
                std::string original_content = gen.m_output.str();
                
                // Clear the stream and write the new sections
                gen.m_output.str("");
                gen.m_output.clear();

                if (!data_section_written) {
                    gen.m_output << "section .data\n"
                                << " buffer db 20 dup(0)\n"
                                << " newline db 10\n\n";
                    data_section_written = true;
                }

                gen.m_output << original_content;
                
                // Seek back to the original position to continue from where we left off
                gen.m_output.seekp(pos_before + std::streamoff(176), std::ios::beg); // Adjusting the offset manually for the new content
                gen.m_output.clear();

                gen.gen_expr(stmt_out->expr);

                gen.m_output << whitespace << "push rsi           ; save rsi\n"
                 << whitespace << "push rdi           ; save rdi\n"
                 << whitespace << "mov rsi, buffer    ; rsi points to the buffer\n"
                 << whitespace << "mov rcx, 10        ; rcx is the divisor (10 for decimal)\n"
                 << whitespace << "add rsi, 19        ; move rsi to the end of buffer\n"
                 << whitespace << "mov byte [rsi], 0  ; null-terminate the string\n"
                 << whitespace << "dec rsi            ; move to position for last digit\n"
                 << whitespace << convert_loop_label << ":\n"
                 << whitespace << "xor rdx, rdx       ; clear rdx before div\n"
                 << whitespace << "div rcx            ; divide rax by 10, quotient in rax, remainder in rdx\n"
                 << whitespace << "add dl, '0'        ; convert remainder to ASCII\n"
                 << whitespace << "mov [rsi], dl      ; store ASCII character in buffer\n"
                 << whitespace << "dec rsi            ; move to next position in buffer\n"
                 << whitespace << "cmp rax, 0         ; check if quotient is zero\n"
                 << whitespace << "jne " << convert_loop_label << "   ; if not, continue loop\n"
                 << whitespace << "inc rsi            ; adjust rsi to point to the first digit\n"
                 << whitespace << print_loop_label << ":\n"
                 << whitespace << "mov al, [rsi]      ; load character to print\n"
                 << whitespace << "test al, al        ; check for null-terminator\n"
                 << whitespace << "je " << end_print_loop_label << "  ; if null-terminator, end loop\n"
                 << whitespace << "mov rax, 1         ; syscall number for sys_write\n"
                 << whitespace << "mov rdi, 1         ; file descriptor 1 is stdout\n"
                 << whitespace << "mov rdx, 1         ; number of bytes to write\n"
                 << whitespace << "syscall            ; invoke the system call\n"
                 << whitespace << "inc rsi            ; move to the next character\n"
                 << whitespace << "jmp " << print_loop_label << "     ; repeat loop\n"
                 << end_print_loop_label << ":\n"
                 << whitespace << "mov rax, 1         ; syscall number for sys_write\n"
                 << whitespace << "mov rdi, 1         ; file descriptor 1 is stdout\n"
                 << whitespace << "mov rsi, newline   ; address of newline character\n"
                 << whitespace << "mov rdx, 1         ; number of bytes to write\n"
                 << whitespace << "syscall            ; invoke the system call\n"
                 << whitespace << "pop rdi            ; restore rdi\n"
                 << whitespace << "pop rsi            ; restore rsi\n";

                gen.m_output << whitespace << ";; /print\n";
            }

        };

        StmtVisitor visitor { .gen = *this };
        visit(visitor, stmt->var);
    }

    [[nodiscard]] string gen_prog()
    {
        m_output << "section .text\n" << " global _start\n  _start:\n";

        for (const NodeStmt* stmt : m_prog.stmts) {
            gen_stmt(stmt);
        }

        m_output << whitespace << "mov rax, 60\n";
        m_output << whitespace << "mov rdi, 0\n";
        m_output << whitespace << "syscall" << " ;; force-exit -> asm end\n";
        return m_output.str();
    }

private:
    void push(const string& reg)
    {
        m_output << whitespace << "push " << reg << "\n";
        m_stack_size++;
    }

    void pop(const string& reg)
    {
        m_output << whitespace << "pop " << reg << "\n";
        m_stack_size--;
    }

    void begin_scope() {
        m_scopes.push_back(m_vars.size());
    }

    void end_scope() {
        int pop_count = m_vars.size() - m_scopes.back();
        m_output << whitespace << "add rsp, " << pop_count * 8 << "\n";
        m_stack_size -= pop_count;
        for (int i=0;i<pop_count;i++) {
            m_vars.pop_back();
        }
        m_scopes.pop_back();
    }

    string create_label() {
        stringstream lablecount;
        lablecount << "label" << m_label_count++;
        return lablecount.str();
    }

    struct Var {
        string name;
        size_t stack_loc;
    };

    const NodeProg m_prog;
    stringstream m_output;
    size_t m_stack_size = 0;
    vector<Var> m_vars {};
    vector<std::size_t> m_scopes {};
    int m_label_count = 0;
};
