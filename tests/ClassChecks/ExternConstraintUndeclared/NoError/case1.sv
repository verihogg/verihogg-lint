// Properly declared extern constraint
class RandClass;
    rand int value;
    
    extern constraint valid_range;
endclass

constraint RandClass::valid_range {
    value <= 1000;
};

