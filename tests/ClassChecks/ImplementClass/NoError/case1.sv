// Interface class being properly implemented
interface class Drawable;
    pure virtual function void draw();
endclass

class Rectangle implements Drawable;
    function void draw();
        $display("Drawing Rectangle");
    endfunction
endclass
