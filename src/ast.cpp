struct Parsed_Templ;
struct Expr;

internal_proc void *
ast_dup(void *src, size_t size) {
    if (size == 0) {
        return NULL;
    }

    void *ptr = xmalloc(size);
    memcpy(ptr, src, size);

    return ptr;
}

#define AST_DUP(x) ast_dup(x, num_##x * sizeof(*x))

struct Arg {
    char *name;
    Expr *expr;
};

internal_proc Arg *
arg_new(char *name, Expr *expr) {
    Arg *result = (Arg *)xcalloc(1, sizeof(Arg));

    result->name = name;
    result->expr = expr;

    return result;
}

struct Filter {
    char*  name;
    Arg **args;
    size_t num_args;
};

internal_proc Filter *
filter(char *name, Arg **args, size_t num_args) {
    Filter *result = (Filter *)xcalloc(1, sizeof(Filter));

    result->name = name;
    result->args = args;
    result->num_args = num_args;

    return result;
}

enum Expr_Kind {
    EXPR_NONE,
    EXPR_PAREN,
    EXPR_NAME,
    EXPR_INT,
    EXPR_FLOAT,
    EXPR_BOOL,
    EXPR_STR,
    EXPR_UNARY,
    EXPR_BINARY,
    EXPR_TERNARY,
    EXPR_FIELD,
    EXPR_INDEX,
    EXPR_RANGE,
    EXPR_CALL,
    EXPR_IS,
    EXPR_NOT,
    EXPR_IN,
    EXPR_IF,
    EXPR_TUPLE,
    EXPR_LIST,
    EXPR_DICT,
};

struct Expr {
    Pos pos;
    Expr_Kind kind;
    Filter **filters;
    size_t num_filters;

    union {
        struct {
            Expr *expr;
        } expr_paren;

        struct {
            char *value;
        } expr_name;

        struct {
            int value;
        } expr_int;

        struct {
            float value;
        } expr_float;

        struct {
            bool value;
        } expr_bool;

        struct {
            char *value;
        } expr_str;

        struct {
            Token_Kind op;
            Expr *expr;
        } expr_unary;

        struct {
            Token_Kind op;
            Expr *left;
            Expr *right;
        } expr_binary;

        struct {
            Expr *left;
            Expr *middle;
            Expr *right;
        } expr_ternary;

        struct {
            Expr *expr;
            char *field;
        } expr_field;

        struct {
            Expr *expr;
            Expr **index;
            size_t num_index;
        } expr_index;

        struct {
            Expr *left;
            Expr *right;
        } expr_range;

        struct {
            Expr *expr;
            Arg **args;
            size_t num_args;
        } expr_call;

        struct {
            Expr *var;
            Expr *test;
            Expr **args;
            size_t num_args;
        } expr_is;

        struct {
            Expr *expr;
        } expr_not;

        struct {
            Expr *expr;
            Expr *set;
        } expr_in;

        struct {
            Expr *cond;
            Expr *else_expr;
        } expr_if;

        struct {
            Expr **exprs;
            size_t num_exprs;
        } expr_tuple;

        struct {
            Expr **expr;
            size_t num_expr;
        } expr_list;

        struct {
            Map *map;
            char **keys;
            size_t num_keys;
        } expr_dict;
    };
};

internal_proc Expr *
expr_new(Expr_Kind kind) {
    Expr *result = (Expr *)xcalloc(1, sizeof(Expr));

    result->kind = kind;

    return result;
}

internal_proc Expr *
expr_paren(Expr *expr) {
    Expr *result = expr_new(EXPR_PAREN);

    result->expr_paren.expr = expr;

    return result;
}

internal_proc Expr *
expr_name(char *value) {
    Expr *result = expr_new(EXPR_NAME);

    result->expr_name.value = value;

    return result;
}

internal_proc Expr *
expr_int(int value) {
    Expr *result = expr_new(EXPR_INT);

    result->expr_int.value = value;

    return result;
}

internal_proc Expr *
expr_float(float value) {
    Expr *result = expr_new(EXPR_FLOAT);

    result->expr_float.value = value;

    return result;
}

internal_proc Expr *
expr_str(char *value) {
    Expr *result = expr_new(EXPR_STR);

    result->expr_str.value = value;

    return result;
}

internal_proc Expr *
expr_bool(bool value) {
    Expr *result = expr_new(EXPR_BOOL);

    result->expr_bool.value = value;

    return result;
}

internal_proc Expr *
expr_unary(Token_Kind op, Expr *expr) {
    Expr *result = expr_new(EXPR_UNARY);

    result->expr_unary.op = op;
    result->expr_unary.expr = expr;

    return result;
}

internal_proc Expr *
expr_binary(Token_Kind op, Expr *left, Expr *right) {
    Expr *result = expr_new(EXPR_BINARY);

    result->expr_binary.op = op;
    result->expr_binary.left = left;
    result->expr_binary.right = right;

    return result;
}

internal_proc Expr *
expr_ternary(Expr *left, Expr *middle, Expr *right) {
    Expr *result = expr_new(EXPR_TERNARY);

    result->expr_ternary.left = left;
    result->expr_ternary.middle = middle;
    result->expr_ternary.right = right;

    return result;
}

internal_proc Expr *
expr_field(Expr *expr, char *field) {
    Expr *result = expr_new(EXPR_FIELD);

    result->expr_field.expr = expr;
    result->expr_field.field = field;

    return result;
}

internal_proc Expr *
expr_index(Expr *expr, Expr **index, size_t num_index) {
    Expr *result = expr_new(EXPR_INDEX);

    result->expr_index.expr = expr;
    result->expr_index.index = index;
    result->expr_index.num_index = num_index;

    return result;
}

internal_proc Expr *
expr_range(Expr *left, Expr *right) {
    Expr *result = expr_new(EXPR_RANGE);

    result->expr_range.left = left;
    result->expr_range.right = right;

    return result;
}

internal_proc Expr *
expr_call(Expr *expr, Arg **args, size_t num_args) {
    Expr *result = expr_new(EXPR_CALL);

    result->expr_call.expr = expr;
    result->expr_call.args = (Arg **)AST_DUP(args);
    result->expr_call.num_args = num_args;

    return result;
}

internal_proc Expr *
expr_is(Expr *var, Expr *test, Expr **args = 0, size_t num_args = 0)
{
    Expr *result = expr_new(EXPR_IS);

    result->expr_is.var = var;
    result->expr_is.test = test;
    result->expr_is.args = (Expr **)AST_DUP(args);
    result->expr_is.num_args = num_args;

    return result;
}

internal_proc Expr *
expr_not(Expr *expr) {
    Expr *result = expr_new(EXPR_NOT);

    result->expr_not.expr = expr;

    return result;
}

internal_proc Expr *
expr_in(Expr *expr, Expr *set) {
    Expr *result = expr_new(EXPR_IN);

    result->expr_in.expr = expr;
    result->expr_in.set  = set;

    return result;
}

internal_proc Expr *
expr_if(Expr *cond, Expr *else_expr) {
    Expr *result = expr_new(EXPR_IF);

    result->expr_if.cond = cond;
    result->expr_if.else_expr = else_expr;

    return result;
}

internal_proc Expr *
expr_tuple(Expr **exprs, size_t num_exprs) {
    Expr *result = expr_new(EXPR_TUPLE);

    result->expr_tuple.exprs = (Expr **)AST_DUP(exprs);
    result->expr_tuple.num_exprs = num_exprs;

    return result;
}

internal_proc Expr *
expr_list(Expr **expr, size_t num_expr) {
    Expr *result = expr_new(EXPR_LIST);

    result->expr_list.expr = (Expr **)AST_DUP(expr);
    result->expr_list.num_expr = num_expr;

    return result;
}

internal_proc Expr *
expr_dict(Map *map, char **keys, size_t num_keys) {
    Expr *result = expr_new(EXPR_DICT);

    result->expr_dict.map = map;
    result->expr_dict.keys = (char **)AST_DUP(keys);
    result->expr_dict.num_keys = num_keys;

    return result;
}

struct Param {
    char *name;
    Expr *default_value;
};

internal_proc Param *
param_new(char *name, Expr *default_value) {
    Param *result = (Param *)xcalloc(1, sizeof(Param));

    result->name = name;
    result->default_value = default_value;

    return result;
}

struct Imported_Sym {
    char *name;
    char *alias;
};

internal_proc Imported_Sym *
imported_sym(char *name, char *alias) {
    Imported_Sym *result = (Imported_Sym *)xcalloc(1, sizeof(Imported_Sym));

    result->name = name;
    result->alias = alias;

    return result;
}

enum Stmt_Kind {
    STMT_NONE,
    STMT_FOR,
    STMT_IF,
    STMT_BLOCK,
    STMT_ELSEIF,
    STMT_ELSE,
    STMT_ENDFOR,
    STMT_ENDIF,
    STMT_ENDBLOCK,
    STMT_ENDFILTER,
    STMT_ENDMACRO,
    STMT_VAR,
    STMT_LIT,
    STMT_EXTENDS,
    STMT_SET,
    STMT_FILTER,
    STMT_INCLUDE,
    STMT_MACRO,
    STMT_IMPORT,
    STMT_FROM_IMPORT,
    STMT_RAW,
};

struct Stmt {
    Stmt_Kind kind;

    union {
        struct {
            Expr *expr;
            Stmt **stmts;
            size_t num_stmts;
            Stmt **else_stmts;
            size_t num_else_stmts;
        } stmt_for;

        struct {
            Expr *cond;
            Expr *test;
            Stmt **stmts;
            size_t num_stmts;
            Stmt **elseifs;
            size_t num_elseifs;
            Stmt *else_stmt;
        } stmt_if;

        struct {
            char *name;
            Stmt **stmts;
            size_t num_stmts;
        } stmt_block;

        struct {
            Expr *expr;
            Expr *if_expr;
        } stmt_var;

        struct {
            char *value;
        } stmt_lit;

        struct {
            char *name;
            Parsed_Templ *templ;
            Parsed_Templ *else_templ;
            Expr *if_expr;
        } stmt_extends;

        struct {
            char *name;
            Expr *expr;
        } stmt_set;

        struct {
            Filter **filter;
            size_t num_filter;
            Stmt **stmts;
            size_t num_stmts;
        } stmt_filter;

        struct {
            Parsed_Templ **templ;
            size_t num_templ;
            b32 ignore_missing;
            b32 with_context;
        } stmt_include;

        struct {
            char *name;
            char *alias;
            Param **params;
            size_t num_params;
            Stmt **stmts;
            size_t num_stmts;
        } stmt_macro;

        struct {
            Parsed_Templ *templ;
            char *name;
        } stmt_import;

        struct {
            Parsed_Templ *templ;
            Imported_Sym **syms;
            size_t num_syms;
        } stmt_from_import;

        struct {
            char *value;
        } stmt_raw;
    };
};

internal_proc Stmt *
stmt_new(Stmt_Kind kind) {
    Stmt *result = (Stmt *)xmalloc(sizeof(Stmt));

    result->kind = kind;

    return result;
}

internal_proc Stmt *
stmt_for(Expr *expr, Stmt **stmts, size_t num_stmts,
        Stmt **else_stmts, size_t num_else_stmts)
{
    Stmt *result = stmt_new(STMT_FOR);

    result->stmt_for.expr = expr;
    result->stmt_for.stmts = (Stmt **)AST_DUP(stmts);
    result->stmt_for.num_stmts = num_stmts;
    result->stmt_for.else_stmts = (Stmt **)AST_DUP(else_stmts);
    result->stmt_for.num_else_stmts = num_else_stmts;

    return result;
}

internal_proc Stmt *
stmt_if(Expr *cond, Stmt **stmts, size_t num_stmts) {
    Stmt *result = stmt_new(STMT_IF);

    result->stmt_if.cond = cond;
    result->stmt_if.stmts = (Stmt **)AST_DUP(stmts);
    result->stmt_if.num_stmts = num_stmts;

    return result;
}

internal_proc Stmt *
stmt_elseif(Expr *cond, Stmt **stmts, size_t num_stmts) {
    Stmt *result = stmt_new(STMT_ELSEIF);

    result->stmt_if.cond = cond;
    result->stmt_if.stmts = (Stmt **)AST_DUP(stmts);
    result->stmt_if.num_stmts = num_stmts;

    return result;
}

internal_proc Stmt *
stmt_else(Stmt **stmts, size_t num_stmts) {
    Stmt *result = stmt_new(STMT_ELSE);

    result->stmt_if.cond = 0;
    result->stmt_if.stmts = (Stmt **)AST_DUP(stmts);
    result->stmt_if.num_stmts = num_stmts;

    return result;
}

internal_proc Stmt *
stmt_block(char *name, Stmt **stmts, size_t num_stmts) {
    Stmt *result = stmt_new(STMT_BLOCK);

    result->stmt_block.name = name;
    result->stmt_block.stmts = (Stmt **)AST_DUP(stmts);
    result->stmt_block.num_stmts = num_stmts;

    return result;
}

internal_proc Stmt *
stmt_endfor() {
    Stmt *result = stmt_new(STMT_ENDFOR);

    return result;
}

internal_proc Stmt *
stmt_endif() {
    Stmt *result = stmt_new(STMT_ENDIF);

    return result;
}

internal_proc Stmt *
stmt_endblock() {
    Stmt *result = stmt_new(STMT_ENDBLOCK);

    return result;
}

internal_proc Stmt *
stmt_endfilter() {
    Stmt *result = stmt_new(STMT_ENDFILTER);

    return result;
}

internal_proc Stmt *
stmt_endmacro() {
    Stmt *result = stmt_new(STMT_ENDMACRO);

    return result;
}

internal_proc Stmt *
stmt_var(Expr *expr, Expr *if_expr) {
    Stmt *result = stmt_new(STMT_VAR);

    result->stmt_var.expr = expr;
    result->stmt_var.if_expr = if_expr;

    return result;
}

internal_proc Stmt *
stmt_lit(char *value, size_t len) {
    Stmt *result = stmt_new(STMT_LIT);

    result->stmt_lit.value = value;
    result->stmt_lit.value[len] = 0;

    return result;
}

internal_proc Stmt *
stmt_extends(char *name, Parsed_Templ *templ, Parsed_Templ *else_templ, Expr *if_expr) {
    Stmt *result = stmt_new(STMT_EXTENDS);

    result->stmt_extends.name       = name;
    result->stmt_extends.templ      = templ;
    result->stmt_extends.else_templ = else_templ;
    result->stmt_extends.if_expr    = if_expr;

    return result;
}

internal_proc Stmt *
stmt_set(char *name, Expr *expr) {
    Stmt *result = stmt_new(STMT_SET);

    result->stmt_set.name = name;
    result->stmt_set.expr = expr;

    return result;
}

internal_proc Stmt *
stmt_filter(Filter **filter, size_t num_filter, Stmt **stmts, size_t num_stmts) {
    Stmt *result = stmt_new(STMT_FILTER);

    result->stmt_filter.filter = (Filter **)AST_DUP(filter);
    result->stmt_filter.num_filter = num_filter;
    result->stmt_filter.stmts = (Stmt **)AST_DUP(stmts);
    result->stmt_filter.num_stmts = num_stmts;

    return result;
}

internal_proc Stmt *
stmt_include(Parsed_Templ **templ, size_t num_templ, b32 ignore_missing,
        b32 with_context)
{
    Stmt *result = stmt_new(STMT_INCLUDE);

    result->stmt_include.templ     = (Parsed_Templ **)AST_DUP(templ);
    result->stmt_include.num_templ = num_templ;
    result->stmt_include.ignore_missing = ignore_missing;
    result->stmt_include.with_context = with_context;

    return result;
}

internal_proc Stmt *
stmt_macro(char *name, Param **params, size_t num_params, Stmt **stmts,
        size_t num_stmts)
{
    Stmt *result = stmt_new(STMT_MACRO);

    result->stmt_macro.name = name;
    result->stmt_macro.alias = 0;
    result->stmt_macro.params = params;
    result->stmt_macro.num_params = num_params;
    result->stmt_macro.stmts = (Stmt **)AST_DUP(stmts);
    result->stmt_macro.num_stmts = num_stmts;

    return result;
}

internal_proc Stmt *
stmt_import(Parsed_Templ *templ, char *name) {
    Stmt *result = stmt_new(STMT_IMPORT);

    result->stmt_import.templ = templ;
    result->stmt_import.name = name;

    return result;
}

internal_proc Stmt *
stmt_from_import(Parsed_Templ *templ, Imported_Sym **syms, size_t num_syms) {
    Stmt *result = stmt_new(STMT_FROM_IMPORT);

    result->stmt_from_import.templ = templ;
    result->stmt_from_import.syms = (Imported_Sym **)AST_DUP(syms);
    result->stmt_from_import.num_syms = num_syms;

    return result;
}

internal_proc Stmt *
stmt_raw(char *value) {
    Stmt *result = stmt_new(STMT_RAW);

    result->stmt_raw.value = value;

    return result;
}

struct Parsed_Templ {
    char *name;
    Parsed_Templ *parent;
    Stmt **stmts;
    size_t num_stmts;
};

