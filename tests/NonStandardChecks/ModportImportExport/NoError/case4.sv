interface axi_lite_if #(
    parameter ADDR_WIDTH = 32,
    parameter DATA_WIDTH = 32
) (input logic aclk, input logic aresetn);

    logic [ADDR_WIDTH-1:0] awaddr;
    logic                  awvalid;
    logic                  awready;
    logic [DATA_WIDTH-1:0] wdata;
    logic                  wvalid;
    logic                  wready;

    task automatic write_addr(input logic [ADDR_WIDTH-1:0] addr);
        @(posedge aclk);
        awaddr  <= addr;
        awvalid <= 1'b1;
        @(posedge aclk iff awready);
        awvalid <= 1'b0;
    endtask

    task automatic write_data(input logic [DATA_WIDTH-1:0] data);
        @(posedge aclk);
        wdata  <= data;
        wvalid <= 1'b1;
        @(posedge aclk iff wready);
        wvalid <= 1'b0;
    endtask

    function automatic logic is_write_done();
        return (awready && awvalid && wready && wvalid);
    endfunction

    modport master (
        input  aclk,          
        input  aresetn,        
        output awaddr,        
        output awvalid,       
        input  awready,       
        output wvalid,       
        input  wready,        
        export write_addr,    
        export write_data,    
        import is_write_done   
    );

    modport slave (
        input  aclk,
        input  aresetn,
        input  awaddr,
        input  awvalid,
        output awready,
        input  wdata,
        input  wvalid,
        output wready,
        import write_addr,    
        import write_data,    
        export is_write_done
    );

endinterface