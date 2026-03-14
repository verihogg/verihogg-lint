// Multiple undeclared extern constraints
class TestClass;
    rand bit [31:0] addr;
    rand bit [63:0] data;
    constraint addr_constraint;
    constraint data_constraint;
endclass

constraint TestClass::addr_constraint {
    addr != 0;
}

constraint TestClass::data_constraint {
    data > 1000;
}
