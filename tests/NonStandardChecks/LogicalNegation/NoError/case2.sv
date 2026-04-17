class Node;
    int data;
    Node next;

    function new(int d);
        data = d;
        next = null;
    endfunction
endclass

module tb;
    Node head;

    initial begin
        head       = new(1);
        head.next  = new(2);

        while (head != null) begin
            $display("data = %0d", head.data);
            head = head.next;
        end
    end
endmodule