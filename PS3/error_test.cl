class A {
  foo(): Int { 42 };
};

class Main inherits IO {
  main(): Object {
    let a: A <- new A in {
      a@A.bar();  -- ❌ ERROR: method 'bar' is not defined in class A
      self;
    }
  };
};
