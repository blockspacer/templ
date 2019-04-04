global_var Arena instr_arena;

enum Instr_Kind {
    INSTR_NONE,
    INSTR_NOP,
    INSTR_PRINT,
    INSTR_ADD,
    INSTR_SET,
    INSTR_LOOP,
    INSTR_LABEL,
    INSTR_IF,
};

struct Instr {
    Instr_Kind kind;

    union {
        struct {
            char *val;
        } instr_print;

        struct {
            /* @TODO: bedingung speichern */
            Instr **instr;
            size_t num_instr;
        } instr_if;

        struct {
            int *it;
            int min;
            int max;
            Instr **instr;
            size_t num_instr;
        } instr_loop;

        struct {
            char *name;
            Instr **instr;
            size_t num_instr;
        } instr_label;
    };
};

global_var Instr instr_nop = { INSTR_NOP };

global_var Instr **instructions;

internal_proc Instr *
instr_new(Instr_Kind kind) {
    Instr *result = ALLOC_STRUCT(&instr_arena, Instr);

    result->kind = kind;

    return result;
}

internal_proc Instr *
instr_print(char *val) {
    Instr *result = instr_new(INSTR_PRINT);

    result->instr_print.val = val;

    return result;
}

internal_proc Instr *
instr_add() {
    Instr *result = instr_new(INSTR_ADD);

    return result;
}

internal_proc Instr *
instr_set() {
    Instr *result = instr_new(INSTR_SET);

    return result;
}

internal_proc Instr *
instr_if(Instr **instr, size_t num_instr) {
    Instr *result = instr_new(INSTR_IF);

    return result;
}

internal_proc Instr *
instr_loop(int *it, int min, int max, Instr **instr, size_t num_instr) {
    Instr *result = instr_new(INSTR_LOOP);

    result->instr_loop.it  = it;
    result->instr_loop.min = min;
    result->instr_loop.max = max;
    result->instr_loop.instr = instr;
    result->instr_loop.num_instr = num_instr;

    return result;
}

internal_proc Instr *
instr_label(char *name, Instr **instr, size_t num_instr) {
    Instr *result = instr_new(INSTR_LABEL);

    result->instr_label.name = name;
    result->instr_label.instr = instr;
    result->instr_label.num_instr = num_instr;

    return result;
}

internal_proc void
push_instr(Instr *instr) {
    buf_push(instructions, instr);
}

