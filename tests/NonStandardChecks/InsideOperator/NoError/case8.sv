class C;
  rand int a;
  int b = 1, c = 2;

  constraint c1 {
    a inside {b, c};
  }
endclass