class Y;

class X2 {
public:
    void foo(Y* y);
};

class Y {
public:
    void bar() {
        std::cout << "Hello from Y::bar()\n";
    }

    X2 x2;
    void callFoo() {
        x2.foo(this);
    }
};

void X2::foo(Y* y) {
    // Now we can call Y's methods
    y->bar();
}
