class Packet;
    int id;
    function new(int i);
        id = i;
    endfunction
endclass

module tb;
    Packet pkt;

    initial begin
        pkt = null;

        if (!pkt) begin
            $display("Packet is null");
        end

        pkt = new(42);
    end
endmodule