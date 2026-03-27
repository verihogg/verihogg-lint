// This will cause a compilation error - circular dependency
package circular_pkg;

    // Class A depends on B
    class A extends B;
    endclass
    
    // Class B depends on A
    class B extends A;
    endclass
    
endpackage
