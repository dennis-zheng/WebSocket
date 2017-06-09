
#include <stdio.h>
#include <vector>
#include "serverManager.h"
#include "WebServer.h"

void ServerManager::onMessage(void* sessionId, const std::string message)
{
	ws_->send(sessionId, message);
}

void ServerManager::onConnect(void* sessionId)
{
	printf("onConnect: sessionId=%p\n", sessionId);
}

void ServerManager::onClose(void* sessionId)
{
	printf("onClose: sessionId=%p\n", sessionId);
}

ServerManager::ServerManager()
	: ws_(NULL)
{
#ifdef  USE_TLS
	ws_ = new WebServer(11011, "config/server.crt", "config/server.key", "", this);
#else
	ws_ = new WebServer(11011, "", "", "", this);
#endif

}

ServerManager::~ServerManager()
{
	if (ws_)
	{
		delete ws_;
		ws_ = NULL;
	}
}

void ServerManager::init()
{
	ws_->run();
}
