#include "sysSPIFFS.h"

// 初始化SPIFFS
bool initSPIFFS()
{
    return SPIFFS.begin(true);
}

// 检查文件是否存在
bool checkFileExists(const char *filePath)
{
    return SPIFFS.exists(filePath);
}

// 检查多个文件是否存在
void checkFilesExist(const char **filePaths, int count)
{
    for (int i = 0; i < count; i++)
    {
        if (!SPIFFS.exists(filePaths[i]))
        {
            Serial.printf("File not found: %s\n", filePaths[i]);
        }
        else
        {
            Serial.printf("File found: %s\n", filePaths[i]);
        }
    }
}

// 处理静态文件请求
void handleStaticFile()
{
    String path = globalServer->uri();

    // 确保路径以/开头
    if (!path.startsWith("/"))
    {
        path = "/" + path;
    }

    // 检查文件是否存在
    if (SPIFFS.exists(path))
    {
        File file = SPIFFS.open(path, "r");
        if (file)
        {
            // 根据文件扩展名设置正确的MIME类型
            String contentType = "text/plain";
            if (path.endsWith(".html"))
                contentType = "text/html";
            else if (path.endsWith(".css"))
                contentType = "text/css";
            else if (path.endsWith(".js"))
                contentType = "application/javascript";
            else if (path.endsWith(".png"))
                contentType = "image/png";
            else if (path.endsWith(".jpg"))
                contentType = "image/jpeg";
            else if (path.endsWith(".gif"))
                contentType = "image/gif";
            else if (path.endsWith(".ico"))
                contentType = "image/x-icon";
            else if (path.endsWith(".xml"))
                contentType = "text/xml";
            else if (path.endsWith(".pdf"))
                contentType = "application/pdf";
            else if (path.endsWith(".zip"))
                contentType = "application/zip";

            globalServer->streamFile(file, contentType);
            file.close();
            return;
        }
    }

    // 如果文件不存在或打开失败，返回404
    globalServer->send(404, "text/plain", "File Not Found");
}

// 初始化SPIFFS并检查文件
bool initAndCheckSPIFFS()
{
    // 初始化SPIFFS
    if (!initSPIFFS())
    {
        M5.Display.println("SPIFFS Mount Failed");
        Serial.println("SPIFFS Mount Failed");
        return false;
    }
    else
    {
        M5.Display.println("SPIFFS Mount OK");
        Serial.println("SPIFFS Mount OK");

        // 检查index.html文件是否存在
        if (checkFileExists("/index.html"))
        {
            M5.Display.println("index.html file found");
            Serial.println("index.html file found");
        }
        else
        {
            M5.Display.println("ERROR: index.html file not found");
            Serial.println("ERROR: index.html file not found");
        }
        return true;
    }
}