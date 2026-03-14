// Multiple extends from non-existing classes
package test_pkg;
    class Child1 extends NonExistent1;
        bit flag;
    endclass
    
    class Child2 extends NonExistent2;
        string name;
    endclass
endpackage
