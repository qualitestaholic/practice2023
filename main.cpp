#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <filesystem>

namespace fs = std::filesystem;

using namespace std;

size_t writeCallback(char* ptr, size_t size, size_t nmemb, string* data)
{
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

void sendEmailWithAttachments(const string& recipient, const string& subject, const string& body, const string& folderPath)
{
    CURL* curl = curl_easy_init();
    if (!curl)
    {
        cerr << "Failed to initialize libcurl." << endl;
        return;
    }

    string smtpServer = "smtp.gmail.com"; 
    string senderEmail = "gdsher42@gmail.com";
    string senderPassword = "uiucihblmigureno";
    string recipientEmail = recipient;
    string emailSubject = subject;
    string emailBody = body;

    string url = "smtp://" + smtpServer;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    struct curl_slist* recipients = NULL;
    recipients = curl_slist_append(recipients, recipientEmail.c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    string payload = "To: " + recipientEmail + "\r\n"
        + "From: " + senderEmail + "\r\n"
        + "Subject: " + emailSubject + "\r\n\r\n"
        + emailBody + "\r\n";

    curl_mime* mime = curl_mime_init(curl);
    curl_mimepart* part = curl_mime_addpart(mime);
    curl_mime_name(part, "data");
    curl_mime_data(part, payload.c_str(), CURL_ZERO_TERMINATED);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

    vector<string> fileNames;

    for (const auto& entry : fs::directory_iterator(folderPath))
    {
        if (entry.is_regular_file())
        {
            string filePath = entry.path().string();
            string fileName = entry.path().filename().string();
            fileNames.push_back(fileName);

            part = curl_mime_addpart(mime);
            curl_mime_filedata(part, filePath.c_str());
            curl_mime_filename(part, fileName.c_str());
        }
    }

    curl_easy_setopt(curl, CURLOPT_USERNAME, senderEmail.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, senderPassword.c_str());

    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

    string response;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        cerr << "Failed to send email: " << curl_easy_strerror(res) << endl;
    }
    else
    {
        long responseCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        if (responseCode == 250)
        {
            cout << "Email sent successfully." << endl;
        }
        else
        {
            cerr << "Failed to send email. Response code: " << responseCode << endl;
        }
    }

    curl_slist_free_all(recipients);
    curl_mime_free(mime);
    curl_easy_cleanup(curl);
}

int main()
{
    string recipient = "skk-kks11@mail.ru";
    string subject = "Results of calculations";
    string body = "прикрепленные файлы";

    string folderPath = R"(C:\Users\ReachetatiVe\Downloads\test1)";

    sendEmailWithAttachments(recipient, subject, body, folderPath);

    return 0;
}
