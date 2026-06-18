import "string";
import "math";
import "std";

class Student {
    studentId: int;
    name: string;
};

fn main() -> void {
    let student: Student = Student(1, "John Doe");

    print_int(student.studentId);
    print_string(student.name);
}