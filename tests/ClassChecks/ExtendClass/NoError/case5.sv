// Multi-level valid inheritance
class GrandParent;
    bit flag;
endclass

class Parent extends GrandParent;
    int count;
endclass

class Child extends Parent;
    string name;
endclass
