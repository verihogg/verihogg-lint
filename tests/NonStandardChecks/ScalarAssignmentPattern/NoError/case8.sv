module test8;
    logic [7:0] b;
    initial begin
        b = '{8'hFF}; // ✅ Корректно
    end
endmodule