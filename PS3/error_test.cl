class A {
  attr: Int <- 0;
  attr: String <- "duplicate";  -- ERROR: Multiply defined attribute

  f(x: Int): Int {
    x + 1
  };

  f(x: Int, y: Int): Int {      -- ERROR: Multiply defined method
    x + y
  };
};

class B inherits A {
  f(x: Int): String {           -- ERROR: Return type differs from inherited method
    "wrong"
  };

  g(x: Int): Int {
    x
  };

  g(x: String): Int {           -- ERROR: Multiply defined method
    0
  };

  bad_attr: UndefinedType;      -- ERROR: Attribute has undefined type

  main(): Object {
    {
      let x: Int <- "not an int" in x;             -- ERROR: Let binding type mismatch
      let y: Bool <- 0 in x;                       -- ERROR: Let binding type mismatch
      let z: Ghost <- 5 in z;                      -- ERROR: Let binding has undefined type

      if "not a bool" then x else y fi;            -- ERROR: Predicate of 'if' is not Bool

      while x loop                                 -- ERROR: Predicate of 'while' is not Bool
        x <- x + 1
      pool;

      case x of
        a: Int => a + 1;
        a: String => a;                            -- ERROR: Duplicate branch variable name
        b: Unknown => 5;                           -- ERROR: Unknown branch type
      esac;

      x <- method_not_defined();                   -- ERROR: Dispatch to undefined method

      x <- self@String.unknown_method();           -- ERROR: Static dispatch to unrelated class + method not found

      self <- 5;                                   -- ERROR: Cannot assign to 'self'

      x <- new UndefinedClass;                     -- ERROR: 'new' used on undefined class

      self;
    }
  };
};
