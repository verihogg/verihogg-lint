class Parent;
    rand int x;
endclass

class Child extends Parent;
    constraint undeclared;
endclass

constraint Child::undeclared {
    x inside [1:10];
}
