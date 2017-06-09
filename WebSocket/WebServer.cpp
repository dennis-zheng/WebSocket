#include <iostream>
#include "WebServer.h"
#include "serverManager.h"

WebServer::WebServer(unsigned short port, const std::string & certPem, const std::string & keyPem, const std::string & keyPasswd, ServerManager*	serverManager)
	: _certPem(certPem)
    , _keyPem(keyPem)
    , _keyPasswd(keyPasswd)
    , _listenPort(port)
	, _next_sessionid(1)
	, _serverManager(serverManager)
{
}

WebServer::~WebServer()
{
    _thread.join();
}

void WebServer::threadFunc()
{
	_server.init_asio();

	_server.set_open_handler(bind(&WebServer::on_open, this, ::_1));
	_server.set_close_handler(bind(&WebServer::on_close, this, ::_1));
	_server.set_message_handler(bind(&WebServer::on_message, this, ::_1, ::_2));
#ifdef USE_TLS
	_server.set_tls_init_handler(bind(&WebServer::on_tls_init, this, ::_1));
#endif
	// listen on specified port
	_server.listen(_listenPort);

	// Start the server accept loop
	_server.start_accept();

	// Start the ASIO io_service run loop
	try {
		_server.run();
	}
	catch (const std::exception & e) {
		std::cout << e.what() << std::endl;
	}
}

void WebServer::run()
{
	_thread = thread(bind(&WebServer::threadFunc, this));
}
void WebServer::stop()
{

}

void WebServer::send(void* sessionId, std::string message)
{
	auto it = _connections.begin();
	while (it != _connections.end()) {
		if (it->second == sessionId) {
			_server.send(it->first, message, websocketpp::frame::opcode::text);
			break;
		}
		it++;
	}
}

void WebServer::send(void* sessionIdSrc, void* sessionIdDst, std::string message)
{
	if (sessionIdDst == NULL)
	{
		auto it = _connections.begin();
		while (it != _connections.end()) {
			if (it->second != sessionIdSrc)
				_server.send(it->first, message, websocketpp::frame::opcode::text);
			it++;
		}
	}
	else
	{
		auto it = _connections.begin();
		while (it != _connections.end()) {
			if (it->second == sessionIdDst) {
				_server.send(it->first, message, websocketpp::frame::opcode::text);
				break;
			}
			it++;
		}
	}
}

std::string WebServer::get_password() {
	return _keyPasswd;
}

// 初始化的时候回调一次，以后每连接上一个web，回调一次
context_ptr WebServer::on_tls_init(websocketpp::connection_hdl hdl) {
	std::cout << "on_tls_init called with hdl: " << hdl.lock().get() << std::endl;
	context_ptr ctx(new boost::asio::ssl::context(boost::asio::ssl::context::tlsv1));

	try {
		ctx->set_options(boost::asio::ssl::context::default_workarounds |
			boost::asio::ssl::context::no_sslv2 |
			boost::asio::ssl::context::no_sslv3 |
			boost::asio::ssl::context::single_dh_use);
		ctx->set_password_callback(bind(&WebServer::get_password, this));
		ctx->use_certificate_chain_file(_certPem);
		ctx->use_private_key_file(_keyPem, boost::asio::ssl::context::pem);
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	return ctx;
}

void WebServer::on_open(connection_hdl hdl) {
	{
		lock_guard<mutex> guard(_connection_lock);
		_connections[hdl] = hdl.lock().get();
	}
	_serverManager->onConnect(hdl.lock().get());
}

void WebServer::on_close(connection_hdl hdl) {
	{
		lock_guard<mutex> guard(_connection_lock);
		_connections.erase(hdl);
	}
	_next_sessionid--;
	_serverManager->onClose(hdl.lock().get());
}

void WebServer::on_message(connection_hdl hdl, server::message_ptr msg) {
	auto it = _connections.find(hdl);
	if (it != _connections.end()) {
		_serverManager->onMessage(hdl.lock().get(), msg->get_payload());
	}
}