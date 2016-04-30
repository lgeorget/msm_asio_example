#include <iostream>
#include <memory>

#include "echo_server.hpp"

#include <vector>

int main()
{
	// Build a io_service object to manage all asynchronous operations
	boost::asio::io_service ioService;

	// Build an IPv4 server that will listen on port 8888
	ip::tcp::acceptor acceptor(ioService,
				   ip::tcp::endpoint(ip::tcp::v4(), 8888));

	// Build the socket the server will use
	ip::tcp::socket socket(ioService);

	// Finally, tell the server to get ready to listen and accept one
	// client. This is blocking, we won't return until a client is connected
	// on the socket.
	acceptor.accept(socket);

	// Start the Echo service, i.e. start the state machine, with the socket
	// as a parameter. The Echo service can communicate with whoever is on
	// the other side. The "connection" and the "communication" parts are
	// separated to ease the maintainance and evolution of the service.
	EchoServer e(std::ref(socket));
	e.start();

	// Launch the entire event loop, this is blocking. Maybe this goes
	// against the intuition but this needs to be done after the service
	// runs on the socket. Otherwise the io_service would run out of work
	// as soon as it would start and exit immediately.
	//
	// There is no risk of race condition here. Even if an asynchronous
	// operation is done by the Echo service before the io_service object
	// runs, the callback handler execution will simply be delayed because
	// the Boost asio library gives the strong guarantee that only threads
	// calling io_service::run() will run callback handlers.
	ioService.run();

	// We end up at this line when the io_service instance has finished
	// running, which happens only, if its queue of asynchronous actions is
	// clear (so no more listening, no more messages waiting to be delivered,
	// etc.). In this case, we stop the state machine and destroy the Echo
	// service.
	e.stop();

}
