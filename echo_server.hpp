#ifndef ECHO_SERVER_H
#define ECHO_SERVER_H

#include <functional>
#include <boost/asio.hpp>
#include <system_error>

// back-end
#include <boost/msm/back/state_machine.hpp>
//front-end
#include <boost/msm/front/state_machine_def.hpp>

// functors
#include <boost/msm/front/functor_row.hpp>
//#include <boost/msm/front/euml/common.hpp>

namespace msm = boost::msm;
namespace mpl = boost::mpl;
namespace ip = boost::asio::ip;
namespace asio = boost::asio;
namespace sys = boost::system;

using namespace msm::front;
using namespace std::placeholders;

namespace
{
	//events
	struct sent
	{
		const sys::error_code& error;
		unsigned int bytesTransferred;

		sent(const sys::error_code& err, unsigned int bytes = 0) :
			error(err),
			bytesTransferred(bytes)
		{}
	};
	struct recvd
	{
		const sys::error_code& error;
		unsigned int bytesTransferred;

		recvd(const sys::error_code& err, unsigned int bytes) :
			error(err),
			bytesTransferred(bytes)
		{}
	};
	struct error
	{};


	struct EchoServer_ : msm::front::state_machine_def<EchoServer_>
	{
		ip::tcp::socket& _sock;
		boost::asio::streambuf _buffer;
		unsigned int _rxErrors = 0;

		EchoServer_(ip::tcp::socket& sock) :
			_sock(sock)
		{}

		template <class Event,class FSM>
		void on_entry(Event const& ,FSM&)
		{
			std::cout << "entering: Server" << std::endl;
		}
		template <class Event,class FSM>
		void on_exit(Event const&,FSM& )
		{
			std::cout << "leaving: Server" << std::endl;
		}

		// FSM states
		struct Idle : public msm::front::state<>
		{
			template <class Event,class FSM>
			void on_entry(Event const&, FSM& fsm)
			{
				asio::async_read_until(fsm._sock, fsm._buffer, '\n',
					[&fsm](const sys::error_code& e,
					       unsigned int bytes) {
						if (e) {
							fsm._rxErrors++;
							std::cerr << "Error on reception: " << e << " " << e.message() << std::endl;
							fsm.process_event(error());
						} else {
							fsm.process_event(recvd(e,bytes));
						}
					});
			}
		};

		struct Parsing : public msm::front::state<>
		{
			template <class Event,class FSM>
			void on_entry(Event const&, FSM& fsm)
			{
				async_write(fsm._sock, fsm._buffer,
					[&fsm](const sys::error_code& e,
					       unsigned int bytes) {
						if (e)
							fsm.process_event(error());
						else
							fsm.process_event(sent(e,bytes));
					}
				);
			}
		};

		struct Sending : public msm::front::state<>
		{
		};

		struct CleaningUp : public msm::front::state<>
		{
			template <class Event,class FSM>
			void on_entry(Event const&, FSM& fsm)
			{
				fsm._sock.close();
			}
		};

		// Initial state
		typedef Idle initial_state;


		struct too_many_errors
		{
			template <class Fsm,class Evt,class SourceState,class TargetState>
			bool operator()(Evt const&, Fsm& fsm, SourceState&,TargetState& )
			{
				return fsm._rxErrors > 2;
			}
		};

		//Transitions
		// none for now, everything is in on_entry() (which may not be fine actually...)

		// Transition table for the server
		struct transition_table : mpl::vector<
			 //    Start     Event         Next         Action                      Guard
			 //  +---------+-------------+------------+---------------------------+----------------------+
			 Row < Idle    , recvd       , Parsing    , none                      , none                 >,
			 Row < Idle    , error       , Idle       , none                      , none                 >,
			 Row < Idle    , error       , CleaningUp , none                      , too_many_errors      >,
			 //  +---------+-------------+---------+---------------------------+----------------------+
			 Row < Parsing , none        , Sending    , none                      , none                 >,
			 //  +---------+-------------+---------+---------------------------+----------------------+
			 Row < Sending , sent        , Idle       , none                      , none                 >,
			 Row < Sending , error       , CleaningUp , none                      , none                 >
			 //  +---------+-------------+---------+---------------------------+----------------------+
		> {};

	};

	typedef msm::back::state_machine<EchoServer_> EchoServer;
}

#endif /* ECHO_SERVER_H */
