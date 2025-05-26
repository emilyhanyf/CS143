Class C {
	a : Int;
	b : Bool;
	init(x : Int, y : Bool) : C {
           {
		a <- x;
		b <- y;
		self;
           }
	};
};

Class B inherits C {
	init(l : Int, p : Bool) : C {
           {
		a <- l;
		b <- p;
		self;
           }
	};
};

Class D inherits B {
	init(l : Int, p : Bool) : C {
           {
		a <- l;
		b <- p;
		self;
           }
	};
};

Class Main {
	main():C {
	  (new C).init(1,true)
	};
};
