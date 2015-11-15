# CppWeb
HTTP proof-of-concept server written in C++
(under development)

This is a HTTP web server written in C++, just to prove that websites can be written in this too, without sacrificing anything.

Features:
- Sendfile support. Transfers files from with no userspace copying
- Single threaded architecture (soon multi-process)
- High performance (can run on a Raspberry Pi with no problem)
- Used as a library. One only has to provide the routes/methods, and a callback function
- To be used as a backend for JS frameworks like Angular

Goals:
- MVC support
- SSL support


2015-11-15 siege log, running on an i5 processor with 50 concurrent users constantly requesting a 64MB file:

Transactions:		         846 hits
Availability:		      100.00 %
Elapsed time:		       52.73 secs
Data transferred:	    51987.66 MB
Response time:		        2.54 secs
Transaction rate:	       16.04 trans/sec
Throughput:		      985.92 MB/sec
Concurrency:		       40.74
Successful transactions:         846
Failed transactions:	           0
Longest transaction:	        5.27
Shortest transaction:	        1.00

