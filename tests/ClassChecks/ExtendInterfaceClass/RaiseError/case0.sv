class Class;
    int value;
    function new(int value);
        this.value = value;
    endfunction : new
endclass : Class

// n<> u<56> t<Description> p<57> c<55> l<8:1> el<9:9>
//         n<> u<55> t<Package_item> p<56> c<54> l<8:1> el<9:9>
//             n<> u<54> t<Package_or_generate_item_declaration> p<55> c<53> l<8:1> el<9:9>
//                 n<> u<53> t<Interface_class_declaration> p<54> c<50> l<8:1> el<9:9>
//                     n<> u<50> t<INTERFACE> p<53> s<46> l<8:1> el<8:10>
//                     n<derived_if> u<46> t<StringConst> p<53> s<51> l<8:17> el<8:27>
//                     n<> u<51> t<EXTENDS> p<53> s<49> l<8:28> el<8:35>
//                     n<> u<49> t<Interface_class_type> p<53> c<48> s<52> l<8:36> el<8:41>
//                         n<> u<48> t<Ps_identifier> p<49> c<47> l<8:36> el<8:41>
//                             n<Class> u<47> t<StringConst> p<48> l<8:36> el<8:41>
//                     n<> u<52> t<ENDCLASS> p<53> l<9:1> el<9:9>

interface class derived_if extends Class;
endclass
