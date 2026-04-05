interface axi_if (input logic aclk, input logic aresetn);
    logic [31:0] awaddr;
    logic        awvalid;
    logic        awready;
    logic [31:0] wdata;

    modport master (
        input  aclk, 
        input  aresetn, 
        output awaddr,   
        output awvalid,  
        import awready  
    );

endinterface