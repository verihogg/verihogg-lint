// Nested class constraint not extern
class OuterClass;
    class Inner;
        rand int id;
        constraint inner_constraint;
    endclass
endclass

constraint OuterClass::Inner::inner_constraint {
    id > 100;
}
