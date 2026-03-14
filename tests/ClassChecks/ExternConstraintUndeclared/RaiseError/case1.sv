// Outer constraint not declared extern in class
class OuterConstraint;
    rand int value;
    constraint myconstraint;
endclass

constraint OuterConstraint::myconstraint {
    value > 0;
    value < 100;
}
