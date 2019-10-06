#define genf(...)   gen_result = strf("%s%s", gen_result, strf(__VA_ARGS__))
#define genlnf(...) gen_result = strf("%s\n", gen_result); gen_indentation(); genf(__VA_ARGS__)
#define genln()     gen_result = strf("%s\n", gen_result); gen_indentation()

global_var char *gen_result = "";
global_var int gen_indent   = 0;

internal_proc void
gen_indentation() {
    gen_result = strf("%s%*.s", gen_result, 4 * gen_indent, "         ");
}

internal_proc Val *
val_from_field(Resolved_Expr *expr) {
    Val *result = 0;
    u8 *raw = (u8 *)expr->expr_field.base->val->_ptr + expr->expr_field.offset;

    switch ( expr->type->kind ) {
        case TYPE_STR: {
            result = val_str(*(char **)raw);
        } break;

        case TYPE_FLOAT: {
            result = val_float(*(float *)raw);
        } break;

        case TYPE_INT: {
            result = val_int(*(int *)raw);
        } break;
    }

    return result;
}

internal_proc Val *
exec_expr(Resolved_Expr *expr) {
    Val *result = 0;

    switch (expr->kind) {
        case EXPR_NAME: {
            result = expr->sym->val;
        } break;

        case EXPR_INT: {
            result = expr->val;
        } break;

        case EXPR_FLOAT: {
            result = expr->val;
        } break;

        case EXPR_STR: {
            result = expr->val;
        } break;

        case EXPR_RANGE: {
            result = val_range(expr->expr_range.min, expr->expr_range.max);
        } break;

        case EXPR_UNARY: {
            /* @TODO: vorzeichen */
            result = exec_expr(expr->expr_unary.expr);
        } break;

        case EXPR_FIELD: {
            /* @TODO: mehr als nur eine ebene unterstützen? */
            result = val_from_field(expr);
        } break;

        case EXPR_BINARY: {
            switch ( expr->expr_binary.op ) {
                case T_MUL: {
                    Val calc = *exec_expr(expr->expr_binary.left) * *exec_expr(expr->expr_binary.right);
                    result = val_copy(&calc);
                } break;

                case T_DIV: {
                    Val calc = *exec_expr(expr->expr_binary.left) / *exec_expr(expr->expr_binary.right);
                    result = val_copy(&calc);
                } break;

                case T_PLUS: {
                    Val calc = *exec_expr(expr->expr_binary.left) + *exec_expr(expr->expr_binary.right);
                    result = val_copy(&calc);
                } break;

                case T_MINUS: {
                    Val calc = *exec_expr(expr->expr_binary.left) - *exec_expr(expr->expr_binary.right);
                    result = val_copy(&calc);
                } break;

                case T_LT: {
                    Val calc = *exec_expr(expr->expr_binary.left) < *exec_expr(expr->expr_binary.right);
                    result = val_copy(&calc);
                } break;
            }
        } break;

        case EXPR_IS: {
            Val *var_val = exec_expr(expr->expr_is.expr);
            Type *type = expr->expr_is.test->type;
            assert(type->kind == TYPE_TEST);

            result = type->type_test.callback(var_val, expr->expr_is.args, expr->expr_is.num_args);
        } break;

        default: {
            assert(0);
        } break;
    }

    return result;
}

internal_proc void
exec_stmt(Resolved_Stmt *stmt) {
    switch ( stmt->kind ) {
        case STMT_LIT: {
            genf("%s", stmt->stmt_lit.lit);
        } break;

        case STMT_VAR: {
            char *value = to_char(exec_expr(stmt->stmt_var.expr));

            for ( int i = 0; i < stmt->stmt_var.num_filter; ++i ) {
                Resolved_Filter *filter = stmt->stmt_var.filter[i];
                value = filter->proc(value, filter->args, filter->num_args);
            }

            genf("%s", value);
        } break;

        case STMT_BLOCK: {
            genlnf("<!-- %s -->", stmt->stmt_block.name);
            genln();
            for ( int i = 0; i < stmt->stmt_block.num_stmts; ++i ) {
                exec_stmt(stmt->stmt_block.stmts[i]);
            }
        } break;

        case STMT_SET: {
            stmt->stmt_set.sym->val = exec_expr(stmt->stmt_set.expr);
        } break;

        case STMT_FOR: {
            Val *list = exec_expr(stmt->stmt_for.expr);
            assert(list->kind == VAL_RANGE); /* @TODO: arrays müssen auch funktionieren */

            for ( int i = list->_range[0]; i < list->_range[1]; ++i ) {
                stmt->stmt_for.it->val->_s32 = i;
                for ( int j = 0; j < stmt->stmt_for.num_stmts; ++j ) {
                    exec_stmt(stmt->stmt_for.stmts[j]);
                }
            }
        } break;

        case STMT_IF: {
            Val *val = exec_expr(stmt->stmt_if.expr);

            if ( val->_bool ) {
                for ( int i = 0; i < stmt->stmt_if.num_stmts; ++i ) {
                    exec_stmt(stmt->stmt_if.stmts[i]);
                }
            } else {
                b32 elseif_matched = false;
                for ( int i = 0; i < stmt->stmt_if.num_elseifs; ++i ) {
                    Resolved_Stmt *elseif = stmt->stmt_if.elseifs[i];
                    val = exec_expr(elseif->stmt_if.expr);

                    if ( val->_bool ) {
                        for ( int j = 0; j < elseif->stmt_if.num_stmts; ++j ) {
                            exec_stmt(elseif->stmt_if.stmts[j]);
                        }
                        elseif_matched = true;
                        break;
                    }
                }

                if ( !elseif_matched && stmt->stmt_if.else_stmt ) {
                    Resolved_Stmt *else_stmt = stmt->stmt_if.else_stmt;
                    for ( int i = 0; i < else_stmt->stmt_if.num_stmts; ++i ) {
                        exec_stmt(else_stmt->stmt_if.stmts[i]);
                    }
                }
            }
        } break;

        default: {
            assert(0);
        } break;
    }
}

internal_proc void
exec() {
    for ( int i = 0; i < buf_len(resolved_stmts); ++i ) {
        exec_stmt(resolved_stmts[i]);
    }
}