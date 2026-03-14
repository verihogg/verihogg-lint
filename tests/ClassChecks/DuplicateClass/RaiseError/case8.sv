// Duplicate class with multiple properties
class Person;
    string name;
    int age;
    
    function new(string n, int a);
        name = n;
        age = a;
    endfunction
endclass

class Person;
    bit is_active;
    real score;
    
    function void display();
        $display("Person info");
    endfunction
endclass
