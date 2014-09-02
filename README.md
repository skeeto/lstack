# C11 Lock-free Stack Demo

Article: [C11 Lock-free Stack](http://nullprogram.com/blog/2014/09/02/)

This is a small library that provides a lock-free stack using C11's
new `stdatomic.h` features. It's ABA aware and requires double-wide
CAS.
