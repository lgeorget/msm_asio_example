# msm_asio_example

This code shows how to build a simple TCP server whose interaction with a client can be described by a State Machine.
It's build with Boost::MSM for the logic part and Boost::Asio for the communication part. The two work nicely together.

## Compilation

Compile with

  clang++ -std=c++11 -lboost_system main.cpp -o echo_server
  
It works equally well with g++ but clang++ tends to give more helpful error messages, especially with templates.
And Boost is full of templates.

The code needs the library boost_system. Everything else from Boost we use here is headers only.
The code will not compile if you replace boost::system::error_code with std::system::error_code
due to Boost::Asio not supporting std::system::error_code (at least not in Boost 1.56).


## Usage

Run the server with

  ./echo_server
  
and in another console, run a client as

  netcat localhost 8888
  
(telnet will do if you don't have netcat).
The server will open the connection and repeat after you every message you send.
