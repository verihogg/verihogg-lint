// Properly declared extern function
class Calculator;
    int result;
    
    extern function void add(int a, int b);
endclass

function void Calculator::add(int a, int b);
    result = a + b;
endfunction
