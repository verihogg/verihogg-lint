// Duplicate class in nested scope
class OuterClass;
    int x;
    
    class InnerClass;
        int y;
    endclass
    
    class InnerClass;
        string z;
    endclass
endclass
