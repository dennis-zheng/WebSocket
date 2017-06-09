#include <string>

class WebServer;
class ServerManager
{
public:
	void onMessage(void* sessionId, const std::string message);

	void onConnect(void* sessionId);

	void onClose(void* sessionId);

	ServerManager();
  	~ServerManager();
	void init();

private:
	WebServer *ws_;
};
