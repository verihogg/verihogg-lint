class Outer;
    class Inner;
        int value;
        // Declare the compute function as extern
        function void compute();
    endclass
endclass

// Define the extern function outside the class
function void Outer::Inner::compute();
    value = 0;
endfunction
