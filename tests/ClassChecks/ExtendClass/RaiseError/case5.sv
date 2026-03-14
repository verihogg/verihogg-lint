// Nested inheritance with undefined parent
module top;
    class Parent;
        int value;
    endclass
    
    class Child extends Parent;
        string name;
    endclass
    
    class GrandChild extends GrandParent;
        real data;
    endclass
endmodule
