#include "commsa.h"

CommSA::CommSA()
{
    this->type = "text/plain";
    this->port = 80;
    this->method = "GET";
    this->timeOut = 1000;
}

CommSA::~CommSA()
{

}

std::string CommSA::sendReq(const char* parameters)
{
    return this->sendReq(this->host, this->route, this->method, this->type, this->port, parameters);
}

std::string CommSA::sendReq(const char* host, const char* route, const char* method, const char* type, int port, const char* parameters)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cout << "WSAStartup failed.\n";
        return "";
    }
    SOCKET Socket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    struct hostent *shost;
    shost = gethostbyname(host);
    if (shost == NULL){
        return "CONNECTION_ERROR";
    }
    SOCKADDR_IN SockAddr;
    SockAddr.sin_port = htons(port);
    SockAddr.sin_family = AF_INET;
    SockAddr.sin_addr.s_addr = *((unsigned long*)shost->h_addr);

    // Ajusta o timeout para a conexão
    setsockopt(Socket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&this->timeOut, sizeof(this->timeOut));
    setsockopt(Socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&this->timeOut, sizeof(this->timeOut));
    if(connect(Socket,(SOCKADDR*)(&SockAddr),sizeof(SockAddr)) != 0){
//        std::cout << "Could not connect";
        // Throw exception if it was not possible to connect
        return "CONNECTION_ERROR";
    }
    //Ponteiro da struct está dando erro de permissão, ainda não descobri o motivo.
//    networkInfo *netInfo;
//    for(int i = 0; i< this->getNetworkInfo(netInfo); i++){
//        printf("----------NETWORK----------\n");
//        printf("Ip:         ");
//        std::string aux = netInfo[i].ip;
//        printf("\t%s\n", aux.c_str());
//        printf("SubnetMask: ");
//        aux = netInfo[i].subnetMask;
//        printf("\t%s\n", aux.c_str());
//    }
    std::string req;
    // Check for define method
    if (method) {
        req.append(method);
    } else {
        req.append("GET");
    }

    req.append(" ");
    req.append(route);
    req.append(" HTTP/1.0\n");
    req.append("Host: ");
    req.append(host);
    req.append(" \n");
    //req.append("Connection: close\n");
    req.append("Content-Type: ");
    req.append(type);
    req.append("; charset=utf-8\n\n\n");

    // Check for sent parameters
    if (parameters && parameters != "") {
        req.append(parameters);
    }

//    std::cout << "REQUEST: "  << std::endl << req << std::endl;

    send(Socket, req.c_str(), strlen(req.c_str()),0);
    char buff[10000];
    int nDataLength;

    //Recebe dados
    nDataLength = recv(Socket,buff,10000,0) > 0;
    closesocket(Socket);
    WSACleanup();

    if (nDataLength != SOCKET_ERROR) {
        // Recupera body da requisição
        std::string body = this->getBody(std::string(buff));
        return body;
    } else {
        return "CONNECTION_ERROR";
    }
}

std::string CommSA::getBody(std::string request) const
{
    /*!
     * \brief results
     *
     * Método que encontra o body da requisição
     *
     */

    std::string start ("{");
    std::string end ("}");

    std::size_t found_s = request.find(start);
    //std::cout << "Índice do começo: " << found_s << std::endl;
    std::size_t found_e = request.find(end);
    //std::cout << "Índice do fim: " << found_e << std::endl;
    std::size_t size = (found_e - found_s);

    if (size <= 1) {
        // String vazia. Retorna
//        std::cout << "String vazia" << std::endl;
        return "";
    }

    // Agora retorna a substring
    std::string body_char = request.substr(found_s, (size+1));
    //std::cout << "Valor encontrado: " << body_char << std::endl;
    std::wstring body_str (body_char.begin(), body_char.end());

    // Tenta associar o a string ao struct do map
    JSONValue *value = JSON::Parse(body_str.c_str());
    if (value == NULL) {
//        std::cout << "Valor nulo" << std::endl;
        return "";
    } else {
        JSONObject root = value->AsObject();
        std::wstring ws;
        if (root.find(L"valor") != root.end() && root[L"valor"]->IsString()){
            ws = root[L"valor"]->AsString();
        } else {
            return "";
        }
        //std::wcout << "Valor encontrado: " << ws << std::endl;
        std::string s(ws.begin(), ws.end());
        return s;
    }
}

const wchar_t *CommSA::GetWC(const char *c)
{
    const size_t cSize = strlen(c)+1;
    wchar_t* wc = new wchar_t[cSize];
    mbstowcs (wc, c, cSize);

    return wc;
}

bool CommSA::downloadFile(const char *url, const char *filePath)
{
    std::string request; // HTTP Header //
    LPCSTR filename = filePath;
    char buffer[BUFFERSIZE];
    struct sockaddr_in serveraddr;
    int sock;

    WSADATA wsaData;
    int port = 80;

//    std::cout << "Baixando arquivo: " << url << " para o caminho: " << filePath << std::endl;

    std::string urlAux(url);
    // Remove's http:// part //,

    if(urlAux.find("http://") != -1){
        std::string temp = urlAux;
        urlAux = temp.substr(urlAux.find("http://") + 7);
    }
    // Split host and file location //
    int dm = urlAux.find("/");
    std::string file = urlAux.substr(dm);
    dm = urlAux.find("/");
    std::string shost = urlAux.substr(0, dm);
    // Generate http header //
    request += "GET " + file + " HTTP/1.0\r\n";
    request += "Host: " + shost + "\r\n";
    request += "\r\n";

    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
        return false;
    }

    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        return false;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));

    //Verifica se é possível conexão com a url repassada
    if (!gethostbyaddr(url, strlen(url),AF_INET)){
        //Com a url completa funciona
//        std::cout << "Fail gethostbyaddr: " << url << std::endl;
        return false;
    }

    // ip address of link //
    hostent *record = gethostbyname(shost.c_str());
    in_addr *address = (in_addr * )record->h_addr;
    std::string ipd = inet_ntoa(* address);
    const char *ipaddr = ipd.c_str();

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(ipaddr);
    serveraddr.sin_port = htons(port);

    if (connect(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
//        std::cout << "Erro na conexão!!!" << std::endl;
        return false;
    }

    if (send(sock, request.c_str(), request.length(), 0) != request.length()) {
//        std::cout << "Erro na conexão!!!" << std::endl;
        return false;
    }

    int nRecv, npos;
    nRecv = recv(sock, (char*)&buffer, BUFFERSIZE, 0);
    // getting end of header //
    std::string str_buff = buffer;

    // Checa se existe a possibildiade do arquivo não estar lá
    npos = str_buff.find("404 Not Found");
    if (npos != std::string::npos) {
//        std::cout << "Arquivo não existe no servidor!!!" << std::endl;
        return false;
    }

    npos = str_buff.find("\r\n\r\n");

    // open the file in the beginning //
    HANDLE hFile;
    hFile = CreateFileA(filename,
                        GENERIC_WRITE,
                        FILE_SHARE_WRITE,
                        NULL, CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, 0);
    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);

    // Download file //
    DWORD ss;
    bool result;
    while((nRecv > 0) && (nRecv != INVALID_SOCKET)){
        if(npos > 0){
            char bf[BUFFERSIZE];
            // copy from end position of header //
            memcpy(bf, buffer + (npos + 4), nRecv - (npos + 4));
            result = WriteFile(hFile, bf,nRecv - (npos + 4), &ss, NULL);
        }else{
            // write normally if end not found //
            result = WriteFile(hFile, buffer, nRecv, &ss, NULL);
        }

        // buffer cleanup  //
        ZeroMemory(&buffer, sizeof(buffer));
        nRecv = recv(sock, (char*)&buffer, BUFFERSIZE, 0);
        str_buff = buffer;
        npos = str_buff.find("\r\n\r\n");
    }

    // Close connection and handle //
    CloseHandle(hFile);
    closesocket(sock);
    WSACleanup();

    return true;
}

bool CommSA::log(const char *message)
{
    return this->log(99, "", "", message);
}

bool CommSA::log(double codigo, const char *user, const char *so, const char *message)
{
    JSONObject root;

    // Add JSON values
    if (user != "") {
        const wchar_t *u = this->GetWC(user);
        root[L"user"] = new JSONValue(u);
    }
    if (so != "") {
        const wchar_t *s = this->GetWC(so);
        root[L"so"] = new JSONValue(s);
    }
    root[L"codigo"] = new JSONValue(codigo);

    // Convert
    const wchar_t *m = this->GetWC(message);
    root[L"message"] = new JSONValue(m);

    // Convert JSON to text
    JSONValue *value = new JSONValue(root);

    // Convert JSON Values
    this->setRoute("/ws/instala/erro");
    this->setMethod("POST");
    this->setType("application/json");
    std::wstring params = value->Stringify();
    std::string par (params.begin(), params.end());

//    std::cout << "Arquivo JSON Enviado: " << std::endl;
//    std::wcout << par.c_str() << std::endl;

    // Send JSON request to Server
    std::string response =  this->sendReq(this->host, this->route, this->method, this->type, this->port, par.c_str());

    if (response == "CONNECTION_ERROR") {
        return false;
    } else {
        return true;
    }
}

int CommSA::getNetworkInfo(networkInfo *arrayNetInfo)
{
    int i;

    /* Variables used by GetIpAddrTable */
    PMIB_IPADDRTABLE pIPAddrTable;
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;
    IN_ADDR IPAddr;

    /* Variables used to return error message */
    LPVOID lpMsgBuf;

    // Before calling AddIPAddress we use GetIpAddrTable to get
    // an adapter to which we can add the IP.
    pIPAddrTable = (MIB_IPADDRTABLE *) MALLOC(sizeof (MIB_IPADDRTABLE));

    if (pIPAddrTable) {
        // Make an initial call to GetIpAddrTable to get the
        // necessary size into the dwSize variable
        if (GetIpAddrTable(pIPAddrTable, &dwSize, 0) ==
            ERROR_INSUFFICIENT_BUFFER) {
            FREE(pIPAddrTable);
            pIPAddrTable = (MIB_IPADDRTABLE *) MALLOC(dwSize);

        }
        if (pIPAddrTable == NULL) {
            printf("Memory allocation failed for GetIpAddrTable\n");
            return -1;
        }
    }
    // Make a second call to GetIpAddrTable to get the
    // actual data we want
    if ( (dwRetVal = GetIpAddrTable( pIPAddrTable, &dwSize, 0 )) != NO_ERROR ) {
        printf("GetIpAddrTable failed with error %d\n", dwRetVal);
        if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),       // Default language
                          (LPTSTR) & lpMsgBuf, 0, NULL)) {
            printf("\tError: %s", lpMsgBuf);
            LocalFree(lpMsgBuf);
        }
        return -1;
    }
    int numEntries = (int) pIPAddrTable->dwNumEntries;
    arrayNetInfo = (struct networkInfo *) malloc(sizeof(struct networkInfo *) * numEntries);
    for (i=0; i < numEntries; i++) {
        if (!(pIPAddrTable->table[i].wType & MIB_IPADDR_DISCONNECTED)){
            struct networkInfo netInfo;
            IPAddr.S_un.S_addr = (u_long) pIPAddrTable->table[i].dwAddr;
            std::string aux = inet_ntoa(IPAddr);
            netInfo.ip = aux.c_str();
//            printf("\tIP Address[%d]:     \t%s\n", i, netInfo.ip);
            IPAddr.S_un.S_addr = (u_long) pIPAddrTable->table[i].dwMask;
            aux = inet_ntoa(IPAddr);
            netInfo.subnetMask = aux.c_str();
//            printf("\tSubnet Mask[%d]:    \t%s\n", i,  netInfo.subnetMask);

            arrayNetInfo[i] = netInfo;
        }
    }

    if (pIPAddrTable) {
        FREE(pIPAddrTable);
        pIPAddrTable = NULL;
    }
    return numEntries;
}

const char *CommSA::getHost() const
{
    return host;
}

void CommSA::setHost(const char *value)
{
    host = value;
}
int CommSA::getPort() const
{
    return port;
}

void CommSA::setPort(int value)
{
    port = value;
}
const char *CommSA::getMethod() const
{
    return method;
}

void CommSA::setMethod(const char *value)
{
    method = value;
}
const char *CommSA::getType() const
{
    return type;
}

void CommSA::setType(const char *value)
{
    type = value;
}
const char *CommSA::getRoute() const
{
    return route;
}

void CommSA::setRoute(const char *value)
{
    route = value;
}
int CommSA::getTimeOut() const
{
    return timeOut;
}

void CommSA::setTimeOut(int value)
{
    timeOut = value;
}
