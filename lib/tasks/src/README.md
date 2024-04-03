examples:

```c++
uint32_t intervalId = eventHandler.setTimeout(
    new Callback<>(std::bind(&MyClass::myFunction, &myObject)),
    5000
);```

```c++
uint32_t intervalId = eventHandler.setTimeout(
    new Callback<>(&myFunction),
    5000
);```