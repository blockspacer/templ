#include "templ.cpp"

int
main(int argc, char **argv) {
    using namespace templ::api;

    templ_init(MB(100), MB(100), MB(100));

    Templ_Vars vars = templ_vars();

    Templ_Var *address = templ_object("address");
    templ_var_set(address, templ_var("city", "berlin"));
    templ_var_set(address, templ_var("street", "siegerstr. 2"));

    Templ_Var *user = templ_object("user");
    templ_var_set(user, templ_var("name", "noob"));
    templ_var_set(user, templ_var("age", 25));
    templ_var_set(user, address);

    templ_vars_add(&vars, user);

    Templ *templ1 = templ_compile_string("{{ user.name }} in {{ user.address.city }}");
    char *result1 = templ_render(templ1, &vars);

    os_file_write("test1.html", result1, utf8_strlen(result1));
    if ( status_is_error() ) {
        fprintf(stderr, "fehler aufgetreten in der übergebenen zeichenkette: %s\n", status_message());
        assert(0);
        status_reset();
    }

    templ_reset();

    Templ_Var *user2 = templ_object("user2");
    templ_var_set(user2, templ_var("name", "fred"));
    templ_var_set(user2, templ_var("age", 23));
    templ_var_set(user2, address);

    Templ_Var *users = templ_list("users");
    templ_var_add(users, user);
    templ_var_add(users, user2);

    templ_vars_add(&vars, users);

    Templ *templ2 = templ_compile_file(argv[1]);
    char *result2 = templ_render(templ2, &vars);

    os_file_write("test2.html", result2, utf8_str_size(result2));
    if ( status_is_error() ) {
        for ( int i = 0; i < status_num_errors(); ++i ) {
            Status *error = status_error_get(i);
            fprintf(stderr, "%s in %s zeile %lld\n", status_message(error), status_filename(error), status_line(error));
        }

        for ( int i = 0; i < status_num_warnings(); ++i ) {
            Status *warning = status_warning_get(i);
            fprintf(stderr, "%s in %s zeile %lld\n", status_message(warning), status_filename(warning), status_line(warning));
        }

        assert(0);
        status_reset();
    }

    templ_reset();

    Json json = json_parse(R"foo([
        {
            "name": "noob",
            "age" : 25,
            "address": {
                "city": "frankfurt",
                "street": "siegerstr. 2"
            }
        },
        {
            "name": "reinhold",
            "age" : 23,
            "address": {
                "city": "leipzig",
                "street": "mozartstr. 20"
            }
        }
    ])foo");

    Templ *templ3 = templ_compile_string("{{ users[0].name }}: {{ users[0].address.city }} -- {{ users[1].name }}: {{ users[1].address.city }}");

    Templ_Var *json_users = templ_var("users", json);
    Templ_Vars json_vars = templ_vars();

    templ_vars_add(&json_vars, json_users);

    char *result3 = templ_render(templ3, &json_vars);

    return 0;
}
