internal_proc Instr * eval_item(Item *item);

internal_proc char *
eval_var(Item *item) {
    assert(item->kind == ITEM_VAR);

    // Val val = extract_val(item->item_var.expr);
    for ( int i = 0; i < item->item_var.num_filter; ++i ) {
        Var_Filter filter = item->item_var.filter[i];
    }

    return "";
}

internal_proc Instr *
eval_stmt(Stmt *stmt) {
    switch ( stmt->kind ) {
        case STMT_LIT: {
            return eval_item(stmt->stmt_lit.item);
        } break;

        case STMT_VAR: {
            return instr_print(eval_var(stmt->stmt_var.item));
        } break;

        case STMT_IF: {
            Operand *op = fetch_resolved_expr(stmt->stmt_if.cond);

            Instr **instr = 0;
            for ( int i = 0; i < stmt->stmt_if.num_stmts; ++i ) {
                buf_push(instr, eval_stmt(stmt->stmt_if.stmts[i]));
            }

            return instr_if(instr, buf_len(instr));
        } break;

        case STMT_EXTENDS: {
            return &instr_nop;
        } break;

        case STMT_SET: {
            Sym *sym = sym_get(stmt->stmt_set.name);
            return instr_set();
        } break;

        case STMT_BLOCK: {
            Instr **instr = 0;
            for ( int i = 0; i < stmt->stmt_block.num_stmts; ++i ) {
                buf_push(instr, eval_stmt(stmt->stmt_block.stmts[i]));
            }

            return instr_label(stmt->stmt_block.name, instr, buf_len(instr));
        } break;

        case STMT_FILTER: {
            return &instr_nop;
        } break;

        case STMT_FOR: {
            Operand *op = fetch_resolved_expr(stmt->stmt_for.cond);
            assert(op->val.kind == VAL_RANGE);

            int min = op->val._range[0];
            int max = op->val._range[1];

            Instr **instr = 0;
            for ( int i = 0; i < stmt->stmt_for.num_stmts; ++i ) {
                buf_push(instr, eval_stmt(stmt->stmt_for.stmts[i]));
            }

            return instr_loop(min, max, instr, buf_len(instr));
        } break;

        default: {
            assert(0);
            return 0;
        } break;
    }
}

internal_proc Instr *
eval_item(Item *item) {
    switch ( item->kind ) {
        case ITEM_LIT: {
            return instr_print(item->item_lit.lit);
        } break;

        case ITEM_VAR: {
            return instr_print(eval_var(item));
        } break;

        case ITEM_CODE: {
            return eval_stmt(item->item_code.stmt);
        } break;

        default: {
            assert(0);
            return 0;
        } break;
    }
}

internal_proc void
eval(Doc *doc) {
    for ( int i = 0; i < doc->num_items; ++i ) {
        push_instr(eval_item(doc->items[i]));
    }
}
