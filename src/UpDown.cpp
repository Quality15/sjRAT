#include "UpDown.h"

void ReceiveFile(SOCKET s, const std::string& filename)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file)
    {
        printf("[! | UpDown.cpp] Failed to create file: %s\n", filename.c_str());
        return;
    }

    // Получаем размер файла от клиента
    std::streampos fileSize;
    int bytesReceived = recv(s, reinterpret_cast<char*>(&fileSize), sizeof(fileSize), 0);
    if (bytesReceived <= 0)
    {
        printf("[! | UpDown.cpp] Failed to receive file size\n");
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
        int bytesReceived = recv(s, buffer, FILE_BUFF_SIZE, 0);
        if (bytesReceived <= 0)
        {
            printf("[! | UpDown.cpp] Failed to receive file\n");
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
        printf("[* | UpDown.cpp] File received: %s\n", filename.c_str());
    }
    else
    {
        printf("[! | UpDown.cpp] Failed to receive complete file: %s\n", filename.c_str());
        std::remove(filename.c_str()); // Удаляем частично принятый файл
    }
}

void SendFile(SOCKET s, char* filename)
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
    send(s, reinterpret_cast<char*>(&fileSize), sizeof(fileSize), 0);

    // Читаем и отправляем файл по частям
    while (fileSize > 0) {
        // Читаем очередную часть файла
        std::streamsize bytesRead = file.readsome(buffer, FILE_BUFF_SIZE);
        if (bytesRead <= 0) {
            printf("[!] Failed to read file: %s\n", filename);
            break;
        }

        // Отправляем прочитанные данные клиенту
        int bytesSent = send(s, buffer, bytesRead, 0);
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
}