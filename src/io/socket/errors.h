#ifndef ERRORS_H
#define ERRORS_H

struct CannotInitializeContext {};
struct CannotInitializeCertificate {};
struct AcceptError {
    int fd;
    const void *ptr;
};
struct WriteError {
    int fd;
    const void *ptr;
};
struct ConnectionClosedByPeer {
    int fd;
    const void *ptr;
};

#endif // ERRORS_H
