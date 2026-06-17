extern fn print_boolean_raw(value: boolean) -> void;
extern fn print_double_raw(value: double) -> void;
extern fn print_int_raw(value: int) -> void;

extern fn print_string_raw(value: string) -> void;

fn print_int(value: int) -> void {
    print_int_raw(value);
}

fn print_double(value: double) -> void {
    print_double_raw(value);
}

fn print_boolean(value: boolean) -> void {
    print_boolean_raw(value);
}

fn print_string(value: string) -> void {
    print_string_raw(value);
}
