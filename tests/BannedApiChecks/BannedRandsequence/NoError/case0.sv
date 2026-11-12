module a;
  int x;
  initial begin
    repeat (8) begin
      x = $urandom_range(2);
      case (x)
        0: x = 1;
        1: x = 2;
        default: x = 0;
      endcase
    end
  end
endmodule
