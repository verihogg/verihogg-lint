// Multiple undeclared extern functions
class Handler;
    bit is_active;
    function void Handler::initialize();
    function void Handler::cleanup();
endclass

function void Handler::initialize();
    is_active = 1'b1;
endfunction

function void Handler::cleanup();
    is_active = 1'b0;
endfunction
