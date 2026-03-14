// Multiple duplicate class declarations in different scopes
package my_pkg;
    class Widget;
        int data;
    endclass : Widget

    class Widget;
        string info;
    endclass : Widget
endpackage

program test;
    class Widget;
        real value;
    endclass : Widget
endprogram
