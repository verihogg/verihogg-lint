// Nested classes with unique constructors
class Outer;
    class Inner1;
        int x;
        
        function new(int v);
            x = v;
        endfunction
    endclass
    
    class Inner2;
        string name;
        
        function new(string n);
            name = n;
        endfunction
    endclass
endclass
