#ifndef _WEB_SERVER_H_
#define _WEB_SERVER_H_

#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/common/thread.hpp>
#include <map>

//#define  USE_TLS

#ifdef USE_TLS
typedef websocketpp::server<websocketpp::config::asio_tls> server;
#else
typedef websocketpp::server<websocketpp::config::asio> server;
#endif


typedef websocketpp::lib::shared_ptr<boost::asio::ssl::context> context_ptr;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

using websocketpp::lib::thread;
using websocketpp::lib::mutex;
using websocketpp::lib::lock_guard;
using websocketpp::lib::unique_lock;
using websocketpp::lib::condition_variable;


class ServerManager;
class WebServer
{
public:
    WebServer(unsigned short port, const std::string & certPem, const std::string & keyPem, const std::string & keyPasswd, ServerManager*	serverManager);
    ~WebServer();
public:
    void send(void* sessionId, std::string message);
    void send(void* sessionIdSrc, void* sessionIdDst, std::string message);
	void run();
	void stop();

private:
    void threadFunc();

	std::string get_password();
	context_ptr on_tls_init(connection_hdl hdl);
	void on_open(connection_hdl hdl);
	void on_close(connection_hdl hdl);
	void on_message(connection_hdl hdl, server::message_ptr msg);

private:
	typedef std::map<connection_hdl, void*, std::owner_less<connection_hdl>> con_list;

	ServerManager*	_serverManager;
	server _server;

    thread _thread;

	int _next_sessionid;
	con_list _connections;
	mutex _connection_lock;
    
    std::string _certPem;
    std::string _keyPem;
    std::string _keyPasswd;
	unsigned short _listenPort; 
};


#endif // _WEB_SERVER_H_
