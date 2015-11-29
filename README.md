#C++ Web Development Framework

The main goal of this project is to create a way of developing complex and highly scalable web applications (and not only) in C++, on Linux.

#Features:

Sendfile support. Transfers files from the filesystem with no userspace copying

High performance (can run on a Raspberry Pi with no problem)

Used as a library. One only has to provide the route/method/callback combinations

HTTP responses can be returned asynchronously via std::future objects. Works great if you don't want other clients to wait for expensive operations

Can be used as a backend for JavaScript frameworks like AngularJS

#Long term goals:

SSL support

MVC support

