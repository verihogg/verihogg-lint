module incorrect_inside_example;
    logic [3:0] a;
    logic [3:0] x;

    initial begin
        a = 4'b1010;
        x = 4'b1100;

        if (a inside x) begin
            $display("a is inside x");
        end
    end
endmodule