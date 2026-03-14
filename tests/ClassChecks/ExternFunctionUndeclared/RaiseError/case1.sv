// Outer function not declared extern in class
class MyClass;
    int value;
    function void process();
endclass

function void MyClass::process();
    MyClass.value = 100;
endfunction
