extern fn new_string(value: string) -> string;
extern fn print_string_raw(value: string) -> void;

fn print_string(value: string) -> void {
    print_string_raw(value);
}