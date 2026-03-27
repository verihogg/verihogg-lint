package circular_inheritance_pkg;

    class Parent extends Child;
        int parent_data;
    endclass
    
    class Child extends Parent;
        int child_data;
    endclass
    
endpackage
