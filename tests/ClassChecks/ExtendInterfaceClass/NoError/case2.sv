interface class BaseInterface;
    pure virtual function void base_method();
endclass

interface class DerivedInterface extends BaseInterface;
    pure virtual function void derived_method();
endclass


