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

        if (pkt == null) begin
            $display("Packet is null");
        end

        pkt = new(42);

        if (pkt != null) begin
            $display("Packet id = %0d", pkt.id);
        end
    end
endmodule