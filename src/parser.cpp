struct Parser {
    Lexer lex;
};

internal_proc b32
valid(Parser *p) {
    return p->lex.input != 0;
}

global_var Stmt **parsed_stmts;

internal_proc Expr * parse_expr(Parser *p);
internal_proc char * parse_name(Parser *p);
internal_proc Stmt * parse_stmt(Parser *p);
internal_proc Stmt * parse_stmt_var(Parser *p);
internal_proc Stmt * parse_stmt_lit(Parser *p);
internal_proc Parsed_Templ * parse_file(char *filename);
internal_proc Stmt * parse_stmt_endmacro(Parser *p);
internal_proc char * parse_str(Parser *p);

global_var char ** keywords;
global_var char *keyword_and;
global_var char *keyword_or;
global_var char *keyword_true;
global_var char *keyword_false;
global_var char *keyword_if;
global_var char *keyword_else;
global_var char *keyword_for;
global_var char *keyword_from;
global_var char *keyword_in;
global_var char *keyword_is;
global_var char *keyword_endfor;
global_var char *keyword_endif;
global_var char *keyword_endblock;
global_var char *keyword_endfilter;
global_var char *keyword_endmacro;
global_var char *keyword_endraw;
global_var char *keyword_extends;
global_var char *keyword_block;
global_var char *keyword_embed;
global_var char *keyword_set;
global_var char *keyword_filter;
global_var char *keyword_include;
global_var char *keyword_macro;
global_var char *keyword_import;
global_var char *keyword_raw;

internal_proc void
init_keywords() {
#define ADD_KEYWORD(K) keyword_##K = intern_str(#K); buf_push(keywords, keyword_##K)

    ADD_KEYWORD(and);
    ADD_KEYWORD(or);
    ADD_KEYWORD(true);
    ADD_KEYWORD(false);
    ADD_KEYWORD(if);
    ADD_KEYWORD(else);
    ADD_KEYWORD(for);
    ADD_KEYWORD(from);
    ADD_KEYWORD(in);
    ADD_KEYWORD(is);
    ADD_KEYWORD(endfor);
    ADD_KEYWORD(endif);
    ADD_KEYWORD(endblock);
    ADD_KEYWORD(endfilter);
    ADD_KEYWORD(endmacro);
    ADD_KEYWORD(endraw);
    ADD_KEYWORD(extends);
    ADD_KEYWORD(block);
    ADD_KEYWORD(embed);
    ADD_KEYWORD(set);
    ADD_KEYWORD(filter);
    ADD_KEYWORD(include);
    ADD_KEYWORD(macro);
    ADD_KEYWORD(import);
    ADD_KEYWORD(raw);

#undef ADD_KEYWORD
}

internal_proc void
init_parser(Parser *p, char *input, char *name) {
    p->lex.input = input;
    p->lex.pos.name = name;
    p->lex.pos.row = 1;
    p->lex.token.pos = p->lex.pos;
    refill(&p->lex);
    init_keywords();
    next_raw_token(&p->lex);
}

internal_proc b32
is_token(Parser *p, Token_Kind kind) {
    return p->lex.token.kind == kind;
}

internal_proc b32
is_valid(Parser *p) {
    b32 result = p->lex.at[0] != 0;

    return result;
}

internal_proc b32
match_token(Parser *p, Token_Kind kind) {
    if ( is_token(p, kind) ) {
        next_token(&p->lex);

        return true;
    }

    return false;
}

internal_proc void
expect_token(Parser *p, Token_Kind kind) {
    if ( is_token(p, kind) ) {
        next_token(&p->lex);
    } else {
        assert(0);
    }
}

internal_proc b32
is_keyword(Parser *p, char *expected_keyword) {
    if ( p->lex.token.name == expected_keyword ) {
        return true;
    }

    return false;
}

internal_proc b32
match_keyword(Parser *p, char *expected_keyword) {
    if ( is_keyword(p, expected_keyword) ) {
        next_token(&p->lex);

        return true;
    }

    return false;
}

internal_proc b32
match_str(Parser *p, char *str) {
    if ( is_token(p, T_NAME) && p->lex.token.literal == intern_str(str) ) {
        next_token(&p->lex);

        return true;
    }

    return false;
}

internal_proc b32
is_str(Parser *p, char *str) {
    if ( p->lex.token.literal == intern_str(str) ) {
        return true;
    }

    return false;
}

internal_proc void
expect_str(Parser *p, char *str) {
    if ( !match_str(p, str) ) {
        assert(0);
    }
}

internal_proc void
expect_keyword(Parser *p, char *expected_keyword) {
    if ( !match_keyword(p, expected_keyword) ) {
        assert(0);
    }
}

internal_proc Expr *
parse_expr_base(Parser *p) {
    Lexer *lex = &p->lex;
    Expr *result = 0;

    if ( is_token(p, T_INT) ) {
        result = expr_int(lex->token.int_value);
        next_token(lex);
    } else if ( is_token(p, T_FLOAT) ) {
        result = expr_float(lex->token.float_value);
        next_token(lex);
    } else if ( is_token(p, T_STR) ) {
        result = expr_str(lex->token.str_value);
        next_token(lex);
    } else if ( is_token(p, T_NAME) ) {
        if ( match_keyword(p, keyword_true) ) {
            result = expr_bool(true);
        } else if ( match_keyword(p, keyword_false) ) {
            result = expr_bool(false);
        } else {
            result = expr_name(lex->token.name);
            next_token(lex);
        }
    } else if ( match_token(p, T_LBRACE) ) {
        Map *map = (Map *)xcalloc(1, sizeof(Map));
        char **keys = 0;
        char *key = 0;
        char *value = 0;

        do {
            key = parse_str(p);
            expect_token(p, T_COLON);
            value = parse_str(p);

            map_put(map, key, value);
            buf_push(keys, key);
        } while ( match_token(p, T_COMMA) );

        expect_token(p, T_RBRACE);

        result = expr_dict(map, keys, buf_len(keys));

    } else if ( match_token(p, T_LPAREN) ) {
        Expr **exprs = 0;
        buf_push(exprs, parse_expr(p));

        while ( match_token(p, T_COMMA) ) {
            /* @INFO: aus der jinja2 doku geht folgendes hervor:
             *        "If a tuple only has one item, it must be followed by a comma (('1-tuple',))"
             */
            if ( is_token(p, T_RPAREN) ) {
                break;
            }

            buf_push(exprs, parse_expr(p));
        }

        expect_token(p, T_RPAREN);

        size_t num_exprs = buf_len(exprs);
        if ( num_exprs > 1 ) {
            result = expr_tuple(exprs, num_exprs);
        } else {
            result = expr_paren((num_exprs) ? exprs[0] : 0);
        }

        buf_free(exprs);
    }

    return result;
}

internal_proc Expr *
parse_expr_field_or_call_or_index(Parser *p) {
    Expr *left = parse_expr_base(p);

    while ( is_token(p, T_LPAREN) || is_token(p, T_LBRACKET) || is_token(p, T_DOT) ) {
        if ( match_token(p, T_LPAREN) ) {
            Arg **args = 0;

            if ( !is_token(p, T_RPAREN) ) {
                char *name = 0;
                Expr *expr = parse_expr(p);

                if ( match_token(p, T_ASSIGN) ) {
                    assert(expr->kind == EXPR_NAME);
                    name = expr->expr_name.value;
                    expr = parse_expr(p);
                }

                buf_push(args, arg_new(name, expr));

                while ( match_token(p, T_COMMA) ) {
                    name = 0;
                    expr = parse_expr(p);

                    if ( match_token(p, T_ASSIGN) ) {
                        assert(expr->kind == EXPR_NAME);
                        name = expr->expr_name.value;
                        expr = parse_expr(p);
                    }

                    buf_push(args, arg_new(name, expr));
                }
            }

            expect_token(p, T_RPAREN);
            left = expr_call(left, args, buf_len(args));
        } else if ( match_token(p, T_LBRACKET) ) {
            Expr **index = 0;

            buf_push(index, parse_expr(p));
            while ( match_token(p, T_COMMA) ) {
                buf_push(index, parse_expr(p));
            }

            if ( !index ) {
                assert(!"index darf nicht leer sein");
            }

            left = expr_index(left, index, buf_len(index));
            expect_token(p, T_RBRACKET);
        } else if ( match_token(p, T_DOT) ) {
            char *field = p->lex.token.name;
            expect_token(p, T_NAME);
            left = expr_field(left, field);
        }
    }

    return left;
}

internal_proc Expr *
parse_expr_list(Parser *p) {
    if ( match_token(p, T_LBRACKET) ) {
        Expr **expr = 0;

        if ( match_token(p, T_RBRACKET) ) {
            return expr_list(expr, buf_len(expr));
        }

        buf_push(expr, parse_expr(p));
        while ( match_token(p, T_COMMA) ) {
            buf_push(expr, parse_expr(p));
        }

        expect_token(p, T_RBRACKET);

        Expr *result = expr_list(expr, buf_len(expr));
        buf_free(expr);

        return result;
    } else {
        return parse_expr_field_or_call_or_index(p);
    }
}

internal_proc Expr *
parse_expr_unary(Parser *p) {
    if ( is_unary(p->lex.token.kind) ) {
        Token op = eat_token(&p->lex);
        return expr_unary(op.kind, parse_expr_unary(p));
    } else {
        return parse_expr_list(p);
    }
}

internal_proc Expr *
parse_expr_exponent(Parser *p) {
    Expr *left = parse_expr_unary(p);

    while ( match_token(p, T_POT) ) {
        left = expr_binary(T_POT, left, parse_expr_unary(p));
    }

    return left;
}

internal_proc Expr *
parse_expr_mul(Parser *p) {
    Expr *left = parse_expr_exponent(p);

    while ( is_token(p, T_MUL) || is_token(p, T_DIV) || is_token(p, T_DIV_TRUNC) ||
            is_token(p, T_PERCENT) )
    {
        Token op = eat_token(&p->lex);
        left = expr_binary(op.kind, left, parse_expr_exponent(p));
    }

    return left;
}

internal_proc Expr *
parse_expr_add(Parser *p) {
    Expr *left = parse_expr_mul(p);

    while ( is_token(p, T_PLUS) || is_token(p, T_MINUS) ) {
        Token op = eat_token(&p->lex);
        left = expr_binary(op.kind, left, parse_expr_mul(p));
    }

    return left;
}

internal_proc Expr *
parse_expr_range(Parser *p) {
    Expr *left = parse_expr_add(p);

    if ( match_token(p, T_RANGE) ) {
        left = expr_range(left, parse_expr_add(p));
    }

    return left;
}

internal_proc Expr *
parse_expr_is(Parser *p) {
    Expr *left = parse_expr_range(p);

    if ( match_keyword(p, keyword_is) ) {
        Expr *expr = parse_expr_range(p);
        Expr **args = 0;

        while ( !is_keyword(p, keyword_else) && !is_keyword(p, keyword_and) &&
                !is_keyword(p, keyword_or) && !is_token(p, T_CODE_END) )
        {
            buf_push(args, parse_expr_range(p));
        }

        left = expr_is(left, expr, args, buf_len(args));
    }

    return left;
}

internal_proc Expr *
parse_expr_in(Parser *p) {
    Expr *left = parse_expr_is(p);

    if ( match_keyword(p, keyword_in) ) {
        left = expr_in(left, parse_expr_is(p));
    }

    return left;
}

internal_proc Expr *
parse_expr_cmp(Parser *p) {
    Expr *left = parse_expr_in(p);

    while ( is_cmp(p->lex.token.kind) ) {
        Token op = eat_token(&p->lex);
        left = expr_binary(op.kind, left, parse_expr_in(p));
    }

    return left;
}

internal_proc Expr *
parse_expr_and(Parser *p) {
    Expr *left = parse_expr_cmp(p);

    if ( match_keyword(p, keyword_and) ) {
        left = expr_binary(T_AND, left, parse_expr(p));
    }

    return left;
}

internal_proc Expr *
parse_expr_or(Parser *p) {
    Expr *left = parse_expr_and(p);

    if ( match_keyword(p, keyword_or) ) {
        left = expr_binary(T_OR, left, parse_expr(p));
    }

    return left;
}

internal_proc Expr *
parse_expr_ternary(Parser *p) {
    Expr *left = parse_expr_or(p);

    if ( match_token(p, T_QMARK) ) {
        Expr *middle = parse_expr_or(p);
        expect_token(p, T_COLON);
        left = expr_ternary(left, middle, parse_expr_or(p));
    }

    return left;
}

internal_proc Filter **
parse_filter(Parser *p) {
    Filter **result = 0;

    do {
        Expr *call = parse_expr(p);

        Arg **args = 0;
        while ( !is_token(p, T_BAR) && !is_token(p, T_CODE_END) && !is_token(p, T_VAR_END) && !is_token(p, T_COMMA) ) {
            buf_push(args, arg_new(0, parse_expr(p)));
        }

        size_t num_args = buf_len(args);

        if ( call->kind != EXPR_CALL ) {
            call = expr_call(call, args, num_args);
        }

        assert(call->kind == EXPR_CALL);
        assert(call->expr_call.expr->kind == EXPR_NAME);

        buf_push(result, filter(call->expr_call.expr->expr_name.value, call->expr_call.args, call->expr_call.num_args));
    } while ( match_token(p, T_BAR) );

    return result;
}

internal_proc Expr *
parse_expr(Parser *p) {
    Pos pos = p->lex.token.pos;
    Expr *expr = parse_expr_ternary(p);
    expr->pos = pos;

    Filter **filters = 0;
    if ( match_token(p, T_BAR) ) {
        filters = parse_filter(p);
    }

    expr->filters = filters;
    expr->num_filters = buf_len(filters);

    return expr;
}

internal_proc Expr *
parse_expr_if(Parser *p) {
    Expr *cond = parse_expr(p);
    Expr *else_expr = 0;

    if ( match_keyword(p, keyword_else) ) {
        else_expr = parse_expr(p);
    }

    return expr_if(cond, else_expr);
}

internal_proc char *
parse_name(Parser *p) {
    Expr *expr = parse_expr(p);
    assert(expr->kind == EXPR_NAME);

    return expr->expr_name.value;
}

internal_proc char *
parse_str(Parser *p) {
    Expr *expr = parse_expr(p);
    assert(expr->kind == EXPR_STR);

    return expr->expr_str.value;
}

internal_proc Stmt *
parse_stmt_for(Parser *p) {
    Expr *expr = parse_expr(p);
    expect_token(p, T_CODE_END);

    Stmt **stmts = 0;
    Stmt **else_stmts = 0;
    Stmt ***stmts_ptr = &stmts;

    for (;;) {
        if ( is_token(p, T_CODE_BEGIN) ) {
            Stmt *stmt = parse_stmt(p);
            if ( stmt->kind == STMT_ENDFOR ) {
                break;
            } else if ( stmt->kind == STMT_ELSE ) {
                stmts_ptr = &else_stmts;
            } else {
                buf_push(*stmts_ptr, stmt);
            }
        } else {
            buf_push(*stmts_ptr, parse_stmt(p));
        }
    }

    Stmt *result = stmt_for(expr, stmts, buf_len(stmts), else_stmts, buf_len(else_stmts));
    buf_free(stmts);

    return result;
}

internal_proc Stmt *
parse_stmt_if(Parser *p) {
    Expr *cond = parse_expr(p);

    expect_token(p, T_CODE_END);

    Stmt *if_stmt = stmt_if(cond, 0, 0);
    Stmt **stmt_elseifs = 0;
    Stmt *stmt_else = 0;
    Stmt *curr_stmt = if_stmt;

    for (;;) {
        if ( is_token(p, T_CODE_BEGIN) ) {
            Stmt *stmt = parse_stmt(p);
            if ( stmt->kind == STMT_ENDIF ) {
                break;
            } else if ( stmt->kind == STMT_ELSEIF ) {
                buf_push(stmt_elseifs, stmt);
                curr_stmt = stmt;
            } else if ( stmt->kind == STMT_ELSE ) {
                stmt_else = stmt;
                curr_stmt = stmt;
            } else {
                buf_push(curr_stmt->stmt_if.stmts, stmt);
                curr_stmt->stmt_if.num_stmts = buf_len(curr_stmt->stmt_if.stmts);
            }
        } else {
            buf_push(curr_stmt->stmt_if.stmts, parse_stmt(p));
            curr_stmt->stmt_if.num_stmts = buf_len(curr_stmt->stmt_if.stmts);
        }
    }

    if_stmt->stmt_if.elseifs = stmt_elseifs;
    if_stmt->stmt_if.num_elseifs = buf_len(stmt_elseifs);
    if_stmt->stmt_if.else_stmt = stmt_else;

    return if_stmt;
}

internal_proc Stmt *
parse_stmt_elseif(Parser *p) {
    Expr *cond = parse_expr(p);
    expect_token(p, T_CODE_END);

    return stmt_elseif(cond, 0, 0);
}

internal_proc Stmt *
parse_stmt_else(Parser *p) {
    expect_token(p, T_CODE_END);

    return stmt_else(0, 0);
}

internal_proc Stmt *
parse_stmt_block(Parser *p) {
    char *name = parse_name(p);
    expect_token(p, T_CODE_END);

    Stmt **stmts = 0;
    for (;;) {
        if ( is_token(p, T_CODE_BEGIN) ) {
            Stmt *stmt = parse_stmt(p);
            if ( stmt->kind == STMT_ENDBLOCK ) {
                break;
            } else {
                buf_push(stmts, stmt);
            }
        } else {
            buf_push(stmts, parse_stmt(p));
        }
    }

    Stmt *result = stmt_block(name, stmts, buf_len(stmts));
    buf_free(stmts);

    return result;
}

internal_proc Stmt *
parse_stmt_extends(Parser *p) {
    char *name = parse_str(p);

    Expr *if_expr = 0;
    if ( match_keyword(p, keyword_if) ) {
        if_expr = parse_expr_if(p);
    }

    expect_token(p, T_CODE_END);

    Parsed_Templ *templ = parse_file(name);
    Parsed_Templ *else_templ = 0;
    if ( if_expr && if_expr->expr_if.else_expr ) {
        Expr *else_expr = if_expr->expr_if.else_expr;
        assert(else_expr->kind == EXPR_STR);

        else_templ = parse_file(else_expr->expr_str.value);
    }

    return stmt_extends(name, templ, else_templ, if_expr);
}

internal_proc Stmt *
parse_stmt_include(Parser *p) {
    Expr *expr = parse_expr(p);

    b32 ignore_missing = false;
    if ( match_str(p, "ignore") && match_str(p, "missing") ) {
        ignore_missing = true;
    }

    b32 with_context = true;
    if ( match_str(p, "with") && match_str(p, "context") ) {
        /* tue nix */
    }

    if ( match_str(p, "without") && match_str(p, "context") ) {
        with_context = false;
    }

    expect_token(p, T_CODE_END);

    Parsed_Templ **templ = 0;
    if ( expr->kind == EXPR_LIST ) {
        for ( int i = 0; i < expr->expr_list.num_expr; ++i ) {
            Expr *name_expr = expr->expr_list.expr[i];
            assert(name_expr->kind == EXPR_STR);

            b32 success = file_exists(name_expr->expr_str.value);
            if ( !success && !ignore_missing ) {
                fatal("konnte datei %s nicht finden", name_expr->expr_str.value);
            }

            if ( success ) {
                buf_push(templ, parse_file(name_expr->expr_str.value));
            }
        }
    } else {
        b32 success = file_exists(expr->expr_str.value);
        if ( !success && !ignore_missing ) {
            fatal("konnte datei %s nicht finden", expr->expr_str.value);
        }

        if ( success ) {
            buf_push(templ, parse_file(expr->expr_str.value));
        }
    }

    return stmt_include(templ, buf_len(templ), ignore_missing, with_context);
}

internal_proc Stmt *
parse_stmt_filter(Parser *p) {
    Filter **filters = parse_filter(p);
    expect_token(p, T_CODE_END);

    Stmt **stmts = 0;
    for (;;) {
        if ( is_token(p, T_CODE_BEGIN) ) {
            Stmt *stmt = parse_stmt(p);
            if ( stmt->kind == STMT_ENDFILTER ) {
                break;
            } else {
                buf_push(stmts, stmt);
            }
        } else {
            buf_push(stmts, parse_stmt(p));
        }
    }

    Stmt *result = stmt_filter(filters, buf_len(filters), stmts, buf_len(stmts));
    buf_free(filters);
    buf_free(stmts);

    return result;
}

internal_proc Stmt *
parse_stmt_set(Parser *p) {
    char *name = parse_name(p);
    expect_token(p, T_ASSIGN);
    Expr *expr = parse_expr(p);
    expect_token(p, T_CODE_END);

    return stmt_set(name, expr);
}

internal_proc Stmt *
parse_stmt_macro(Parser *p) {
    expect_token(p, T_NAME);
    char *name = p->lex.token.name;
    expect_token(p, T_LPAREN);

    Param **params = 0;
    if ( !match_token(p, T_RPAREN) ) {
        char *param_name = parse_name(p);
        Expr *default_value = 0;
        if ( match_token(p, T_ASSIGN) ) {
            default_value = parse_expr(p);
        }
        buf_push(params, param_new(param_name, default_value));

        while ( match_token(p, T_COMMA) ) {
            param_name = parse_name(p);
            default_value = 0;
            if ( match_token(p, T_ASSIGN) ) {
                default_value = parse_expr(p);
            }
            buf_push(params, param_new(param_name, default_value));
        }

        expect_token(p, T_RPAREN);
    }

    expect_token(p, T_CODE_END);

    Stmt **stmts = 0;
    Stmt *stmt = parse_stmt(p);

    while ( stmt->kind != STMT_ENDMACRO ) {
        buf_push(stmts, stmt);
        stmt = parse_stmt(p);
    }

    return stmt_macro(name, params, buf_len(params), stmts, buf_len(stmts));
}

internal_proc Stmt *
parse_stmt_import(Parser *p) {
    char *filename = parse_str(p);
    expect_str(p, "as");
    char *name = parse_name(p);
    expect_token(p, T_CODE_END);

    return stmt_import(parse_file(filename), name);

}

internal_proc Stmt *
parse_stmt_from_import(Parser *p) {
    char *filename = parse_str(p);
    expect_keyword(p, keyword_import);

    Imported_Sym **syms = 0;
    do {
        char *name = parse_name(p);
        char *alias = 0;

        if ( match_str(p, "as") ) {
            alias = parse_name(p);
        }

        buf_push(syms, imported_sym(name, alias));
    } while ( match_token(p, T_COMMA) );

    expect_token(p, T_CODE_END);

    return stmt_from_import(parse_file(filename), syms, buf_len(syms));
}

internal_proc Stmt *
parse_stmt_raw(Parser *p) {
    char *start = p->lex.input;
    char *end = start;

    expect_token(p, T_CODE_END);

    for (;; ) {
        while ( valid(p) && !match_token(p, T_CODE_BEGIN) ) {
            end = p->lex.input;
            next_token(&p->lex);
        }

        if ( match_keyword(p, keyword_endraw) ) {
            expect_token(p, T_CODE_END);
            break;
        }
    }

    return stmt_raw(intern_str(start, end));
}

internal_proc Stmt *
parse_stmt_endfor(Parser *p) {
    expect_token(p, T_CODE_END);
    return stmt_endfor();
}

internal_proc Stmt *
parse_stmt_endif(Parser *p) {
    expect_token(p, T_CODE_END);
    return stmt_endif();
}

internal_proc Stmt *
parse_stmt_endblock(Parser *p) {
    match_token(p, T_NAME);
    expect_token(p, T_CODE_END);
    return stmt_endblock();
}

internal_proc Stmt *
parse_stmt_endfilter(Parser *p) {
    expect_token(p, T_CODE_END);
    return stmt_endfilter();
}

internal_proc Stmt *
parse_stmt_endmacro(Parser *p) {
    expect_token(p, T_CODE_END);
    return stmt_endmacro();
}

internal_proc Stmt *
parse_stmt(Parser *p) {
    Stmt *result = 0;
    if ( match_token(p, T_CODE_BEGIN) ) {
        if ( match_keyword(p, keyword_for) ) {
            result = parse_stmt_for(p);
        } else if ( match_keyword(p, keyword_if) ) {
            result = parse_stmt_if(p);
        } else if ( match_keyword(p, keyword_endfor) ) {
            result = parse_stmt_endfor(p);
        } else if ( match_keyword(p, keyword_endif) ) {
            result = parse_stmt_endif(p);
        } else if ( match_keyword(p, keyword_endblock) ) {
            result = parse_stmt_endblock(p);
        } else if ( match_keyword(p, keyword_endfilter) ) {
            result = parse_stmt_endfilter(p);
        } else if ( match_keyword(p, keyword_endmacro) ) {
            result = parse_stmt_endmacro(p);
        } else if ( match_keyword(p, keyword_else) ) {
            if ( match_keyword(p, keyword_if) ) {
                result = parse_stmt_elseif(p);
            } else {
                result = parse_stmt_else(p);
            }
        } else if ( match_keyword(p, keyword_block) ) {
            result = parse_stmt_block(p);
        } else if ( match_keyword(p, keyword_extends) ) {
            result = parse_stmt_extends(p);
        } else if ( match_keyword(p, keyword_filter) ) {
            result = parse_stmt_filter(p);
        } else if ( match_keyword(p, keyword_set) ) {
            result = parse_stmt_set(p);
        } else if ( match_keyword(p, keyword_include) ) {
            result = parse_stmt_include(p);
        } else if ( match_keyword(p, keyword_macro) ) {
            result = parse_stmt_macro(p);
        } else if ( match_keyword(p, keyword_import) ) {
            result = parse_stmt_import(p);
        } else if ( match_keyword(p, keyword_from) ) {
            result = parse_stmt_from_import(p);
        } else if ( match_keyword(p, keyword_raw) ) {
            result = parse_stmt_raw(p);
        } else {
            illegal_path();
        }
    } else if ( match_token(p, T_VAR_BEGIN) ) {
        result = parse_stmt_var(p);
    } else {
        result = parse_stmt_lit(p);
    }

    return result;
}

internal_proc Stmt *
parse_stmt_var(Parser *p) {
    Expr *expr = parse_expr(p);

    Expr *if_expr = 0;
    if ( match_keyword(p, keyword_if) ) {
        if_expr = parse_expr_if(p);
    }

    expect_token(p, T_VAR_END);

    return stmt_var(expr, if_expr);
}

internal_proc Stmt *
parse_code(Parser *p) {
    return parse_stmt(p);
}

internal_proc Stmt *
parse_stmt_lit(Parser *p) {
    char *lit = 0;

    buf_printf(lit, "%s", p->lex.token.literal);
    while ( is_valid(p) && is_lit(&p->lex) ) {
        buf_printf(lit, "%c", p->lex.at[0]);
        next(&p->lex);
    }

    Stmt *result = stmt_lit(lit, strlen(lit));
    next_token(&p->lex);

    return result;
}

internal_proc Parsed_Templ *
parse_file(char *filename) {
    char *content = 0;
    Parsed_Templ *templ = (Parsed_Templ *)xcalloc(1, sizeof(Parsed_Templ));

    /* @AUFGABE: nur den dateinamen ohne dateierweiterung übernehmen */
    templ->name = filename;

    if ( file_read(filename, &content) ) {
        Parser parser = {};
        Parser *p = &parser;
        init_parser(p, content, filename);

        while (p->lex.token.kind != T_EOF) {
            if ( is_token(p, T_VAR_BEGIN) || is_token(p, T_CODE_BEGIN) ) {
                Stmt *stmt = parse_code(p);
                buf_push(templ->stmts, stmt);
            } else if ( is_token(p, T_COMMENT) ) {
                next_token(&p->lex);
            } else {
                buf_push(templ->stmts, parse_stmt_lit(p));
            }
        }
    } else {
        fatal("konnte datei %s nicht lesen", filename);
    }

    templ->num_stmts = buf_len(templ->stmts);
    return templ;
}

