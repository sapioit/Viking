#C++ Web Development Framework v0.8.2

The main goal of this project is to develop a highly scalabe, general purpose HTTP server.

#Features:

High performance

Sendfile support. Transfers files from the filesystem with no userspace copying

Used as a library. One only has to provide the route/method/callback combinations

Can be used for anything - Game servers, Website backends, Application backends

HTTP responses can be returned asynchronously via std::future objects. Works great if you don't want other clients to wait for expensive operations

Works fast on embedded platforms - for hobbyists

#Requirements:

Linux (anything above kernel version 3 should work)

Modern compiler with filesystem support in the standard library. Tested with GCC 5.3.1 and GCC 6, but Clang fails to compile it due to the incomplete filesystem TS implementation.

#Backlog:

Multiple threading within the internals. Request handlers should still be called from the main thread

HTTPS support

Support for chunked transfer

Do not read the request body if it's too big. Instead, give the user the option to do it when he needs it
