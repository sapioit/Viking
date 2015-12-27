#C++ Web Development Framework v0.7.8

The main goal of this project is to create a way of developing complex and highly scalable web applications (and not only) in C++, on Linux.

#Features:

High performance

Sendfile support. Transfers files from the filesystem with no userspace copying

Used as a library. One only has to provide the route/method/callback combinations

HTTP responses can be returned asynchronously via std::future objects. Works great if you don't want other clients to wait for expensive operations

Can be used as a backend for JavaScript frameworks like AngularJS

#Requirements:

Modern compiler with filesystem support in the standard library. Tested with GCC 5.3.1 and GCC 6, but Clang fails to compile it due to the incomplete filesystem TS implementation.

#Backlog:

Multiple threading within the internals. Request handlers should still be called from the main thread

HTTPS support

Support for chunked transfer

Do not read the request body if it's too big. Instead, give the user the option to do it when he needs it
