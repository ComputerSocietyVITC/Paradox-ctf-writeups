# Problem Overview

readme, file names and other text written in caesar cipher (easy stuff)

**Changes Made**
----------------

### tokenization.hpp

| Line | Before | After |
| --- | --- | --- |
| 53 | tokens.push_back({ .type = TokenType::exit, .line=line_count }); | tokens.push_back({ .type = TokenType::semi, .line=line_count }); |
| 114 | else if (peek().value() == '(') | else if (peek().value() == ')') |
| 118 | else if (peek().value() == ')') | else if (peek().value() == '(') |
| 132 | tokens.push_back({ .type = TokenType::plus, .line=line_count }); | tokens.push_back({ .type = TokenType::div, .line=line_count }); |
| 140 | tokens.push_back({ .type = TokenType::minus, .line=line_count }); | tokens.push_back({ .type = TokenType::plus, .line=line_count }); |
| 144 | tokens.push_back({ .type = TokenType::div, .line=line_count }); | tokens.push_back({ .type = TokenType::minus, .line=line_count }); |
| 155 | consume(); | // ??! consume(); THIS CONSUME IS USELESS; |

### parser.hpp

| Line | Before | After |
| --- | --- | --- |
| 236 | try_consume(TokenType::open_paren, "('") | try_consume(TokenType::open_paren, "')'") |
| 243 | try_consume(TokenType::close_paren, "')'") | try_consume(TokenType::close_paren, "'('") |
| 280 | try_consume(TokenType::semi, "';'") | try_consume(TokenType::semi, "'.'") |
| 300 | try_consume(TokenType::semi, "';'") | try_consume(TokenType::semi, "'.'") |
| 311 | peek().has_value() && peek().value().type == TokenType::may && peek(1).has_value() && peek(1).value().type == TokenType::ident && peek(2).has_value() && peek(2).value().type == TokenType::eq | peek().has_value() && peek().value().type == TokenType::may && peek(1).has_value() && peek(1).value().type == TokenType::ident && peek(3).has_value() && peek(2).value().type == TokenType::semi |
| 323 | try_consume(TokenType::semi, "';'") | try_consume(TokenType::eq, "';'") |
| 393 | if (m_index + offset >= m_tokens.size()) | if (m_index /* ??! + offset */ >= m_tokens.size()) |

### generation.hpp

| Line | Before | After |
| --- | --- | --- |
| 75 | gen.m_output << whitespace << "div rbx\n"; | // gen.m_output << whitespace << "div rbx\n" <-THIS IS WRONG!!!                gen.m_output << whitespace << "div rax, rbx\n"; |

**AFTER FIXING COMPILER**
-------------------------

For now, no operations in `iisac.42` are permutated (they can if they want to it is not related to the challenge)

In the end, they should get exit code 0 with these numbers

#### Get the numbers or variable calls bounded by `()` ONLY

Value of var:
`a` :: 103
`b` :: 0 
`c` :: 2050 
exit(2050) gives `2050 mod 256` == 2 
so exit(c-2) is given

#### var `a` is 100 48 111 109 95 ->from decimal to text it is `d0om_` (/ 102 in a variable is ignored as it is not under `()`)
#### var `b` is (103-1) 49 120 51 100 -> from decimal to text it is `f1x3d`
#### var `c` is (0+103-8) 70 82 ->from decimal to text it is `_FR`

**FINAL FLAG: paradoxCTF{d0om_f1x3d_FR}**
-----------------------------------------
