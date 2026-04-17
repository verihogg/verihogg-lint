class Inner;
    int value;
endclass

class Outer;
    Inner inner_obj;
endclass

module tb;
    Outer o;

    initial begin
        o = new();
        o.inner_obj = null;

        if (o.inner_obj == null) begin
            $display("inner_obj is null");
        end
    end
endmodule