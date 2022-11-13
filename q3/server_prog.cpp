#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>

/////////////////////////////
// #include <iostream>
#include <assert.h>
#include <tuple>
#include <bits/stdc++.h>
// typedef pair<int, int> ii;
using namespace std;
const int INF = 1000000000;
/////////////////////////////

// Regular bold text
#define BBLK "\e[1;30m"
#define BRED "\e[1;31m"
#define BGRN "\e[1;32m"
#define BYEL "\e[1;33m"
#define BBLU "\e[1;34m"
#define BMAG "\e[1;35m"
#define BCYN "\e[1;36m"
#define ANSI_RESET "\x1b[0m"

typedef long long LL;

#define pb push_back
#define debug(x) cout << #x << " : " << x << endl
#define part cout << "-----------------------------------" << endl;

///////////////////////////////
#define MAX_CLIENTS 4
#define PORT_ARG 8001

const int initial_msg_len = 256;

////////////////////////////////////

const LL buff_sz = 1048576;
///////////////////////////////////////////////////

/******************global variables*************/
vector<vector<pair<int, int>>> adj;
vector<int> parent;
vector<int> dist;
vector<string> argument_string;
int total_vertices = 0;
int total_edges = 0;
vector<vector<int>> path;
int destination = -1;
int source = 0;
int current_index = 0;
int max_index = -1;
map<int, int> port_number_map;
map<int,int> socket_map;
int send_func = 0;
int address = INADDR_ANY;
/***********************************************/
bool isNumber(const string &s)
{
    for (char const &ch : s)
    {
        if (std::isdigit(ch) == 0)
            return false;
    }
    return true;
}
pair<string, int> read_string_from_socket(const int &fd, int bytes)
{
    std::string output;
    output.resize(bytes);
    int bytes_received = read(fd, &output[0], bytes - 1);
  //  debug(bytes_received);
    if (bytes_received <= 0)
    {
        cout << "Failed to read data from socket. \n";
    }
    output[bytes_received] = 0;
    output.resize(bytes_received);
    // debug(output);
    return {output, bytes_received};
}
int send_string_on_socket(int fd, const string &s)
{
    // debug(s.length());
    int bytes_sent = write(fd, s.c_str(), s.length());
    if (bytes_sent < 0)
    {
        cerr << "Failed to SEND DATA via socket.\n";
    }
    return bytes_sent;
}
///////////////////////////////

string handle_connection(int client_socket_fd)
{
    // int client_socket_fd = *((int *)client_socket_fd_ptr);
    //####################################################

    int received_num, sent_num;

    /* read message from client */
    int ret_val = 1;
    while (true)
    {
        string cmd;
        tie(cmd, received_num) = read_string_from_socket(client_socket_fd, buff_sz);
        ret_val = received_num;
        // debug(ret_val);
        // printf("Read something\n");
        if (ret_val <= 0)
        {
            // perror("Error read()");
            printf("Server could not read msg sent from client\n");
            goto close_client_socket_ceremony;
        }
        cout << "Client sent : " << cmd << endl;
        if (cmd == "exit")
        {
            cout << "Exit pressed by client" << endl;
            goto close_client_socket_ceremony;
        }
        string msg_to_send_back = cmd;
        if (!send_func)
        {
            argument_string = {};
            stringstream ss(cmd);
            string word;
            while (ss >> word)
            {
                argument_string.push_back(word);
            }
            if (argument_string.size() == 1 && argument_string[0] == "pt")
            {
                int col_width = 10;
                cout << setw(col_width) << "dest";
                cout << setw(col_width) << "forw";
                cout << setw(col_width) << "delay" << endl;
                for (int i = 1; i < total_vertices; i++)
                {
                    if (path[i].size() != 1)
                    {
                        cout << setw(col_width) << i;
                        cout << setw(col_width) << path[i][1];
                        cout << setw(col_width) << dist[i] << endl;
                    }
                    else
                    {
                        cout << setw(col_width) << i;
                        cout << setw(col_width) << "None";
                        cout << setw(col_width) << dist[i] << endl;
                    }
                }
                ////////////////////////////////////////
                // "If the server write a message on the socket and then close it before the client's read. Will the client be able to read the message?"
                // Yes. The client will get the data that was sent before the FIN packet that closes the socket.
                int sent_to_client = send_string_on_socket(client_socket_fd, msg_to_send_back);
                // debug(sent_to_client);
                if (sent_to_client == -1)
                {
                    perror("Error while writing to client. Seems socket has been closed");
                    goto close_client_socket_ceremony;
                }
            }
            else if (argument_string.size() == 3 && argument_string[0] == "send" && isNumber(argument_string[1]) && (stoi(argument_string[1]) < total_vertices) && (stoi(argument_string[1]) >= 0))
            {
                printf("entered send\n");
                destination = stoi(argument_string[1]);
                send_func = 1;
                int start_client = send_string_on_socket(socket_map[0], argument_string[2]);
                current_index++;
                if (start_client == -1)
                {
                    perror("Error while writing to start client.Seems socket has been closed");
                    goto close_client_socket_ceremony;
                }
                printf("entered send\n");
                int sent_to_client = send_string_on_socket(client_socket_fd, msg_to_send_back);
                printf("entered send\n");
                // debug(sent_to_client);
                if (sent_to_client == -1)
                {
                    perror("Error while writing to client. Seems socket has been closed");
                    goto close_client_socket_ceremony;
                }
                send_func = 0;
            }
        }
        else
        {
            int sent_to_client = send_string_on_socket(client_socket_fd, msg_to_send_back);
            if (sent_to_client == -1)
            {
                perror("Error while writing to client. Seems socket has been closed");
                goto close_client_socket_ceremony;
            }
            return msg_to_send_back;
        }
    }
close_client_socket_ceremony:
    printf("socket closed\n");
    close(client_socket_fd);
    printf(BRED "Disconnected from client" ANSI_RESET "\n");
    return NULL;
}
void *thread_func(void *arg)
{
    int wel_socket_fd, client_socket_fd, port_number;
    socklen_t clilen;
    int index_of_thread = *(int *)arg;
    struct sockaddr_in serv_addr_obj, client_addr_obj;
    wel_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    socket_map[index_of_thread] = wel_socket_fd;
    if (wel_socket_fd < 0)
    {
        perror("ERROR creating welcoming socket");
        exit(-1);
    }
    bzero((char *)&serv_addr_obj, sizeof(serv_addr_obj));
    port_number = port_number_map[index_of_thread];
    serv_addr_obj.sin_family = AF_INET;
    serv_addr_obj.sin_addr.s_addr = INADDR_ANY;
    serv_addr_obj.sin_port = htons(port_number); // process specifies port
    if (bind(wel_socket_fd, (struct sockaddr *)&serv_addr_obj, sizeof(serv_addr_obj)) < 0)
    {
        perror("Error on bind on welcome socket: ");
        exit(-1);
    }
    listen(wel_socket_fd, MAX_CLIENTS);
    cout << "Server has started listening on the LISTEN PORT" << endl;
    clilen = sizeof(client_addr_obj);
    while (1)
    {
        client_socket_fd = accept(wel_socket_fd, (struct sockaddr *)&client_addr_obj, &clilen);
        if (client_socket_fd < 0)
        {
            perror("ERROR while accept() system call occurred in SERVER");
            exit(-1);
        }
        printf(BGRN "New client connected from port number %d and IP %s \n" ANSI_RESET, ntohs(client_addr_obj.sin_port), inet_ntoa(client_addr_obj.sin_addr));
        string s = handle_connection(client_socket_fd);
        if (index_of_thread == destination)
        {
            cout << "Data received at node: " << index_of_thread << ": Source: " << source << "; Destination :" << destination << "; Forwarded_Destination : "
                 << " None ; Message :"
                 << "\"" << s << "\"" << endl;
        }
        else
        {
            current_index++;
            int next_index = path[destination][current_index];
            int file_descriptor = socket_map[next_index];
            cout << "Data received at node: " << index_of_thread << ": Source: " << source << "; Destination :" << destination << "; Forwarded_Destination : "
                 << next_index << "; Message :"
                 << "\"" << s << "\"" << endl;
            send_string_on_socket(file_descriptor, s);
        }
    }
    close(wel_socket_fd);
    return 0;
}
void find_path(int current_vertex, vector<int> &parent, vector<int> &path)
{
    if (current_vertex == -1)
    {
        return;
    }
    find_path(parent[current_vertex], parent, path);
    path.push_back(current_vertex);
}
int main(int argc, char *argv[])
{
    int n, m;
    cin >> n >> m;
    total_edges = m;
    total_vertices = n;
    adj.assign(n, vector<pair<int, int>>());
    int current_port_number = 8003;
    for (int i = 0; i < n; i++)
    {
        port_number_map[i] = current_port_number;
        current_port_number++;
    }
    for (int i = 0; i < m; i++)
    {
        int a, b, d;
        cin >> a >> b >> d;
        adj[a].push_back({b, d});
        current_port_number++;
        adj[b].push_back({a, d});
    }
    /***************djikstra*********************/
    dist.assign(n, INF);
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    dist[0] = 0;
    parent.assign(n, -1);
    pq.push(make_pair(0, 0));
    while (!pq.empty())
    {
        pair<int, int> front = pq.top();
        pq.pop();
        int d = front.first, u = front.second;
        if (d > dist[u])
        {
            continue;
        }
        for (int j = 0; j < adj[u].size(); j++)
        {
            pair<int, int> v = adj[u][j];
            if (dist[u] + v.second < dist[v.first])
            {
                dist[v.first] = dist[u] + v.second;
                parent[v.first] = u;
                pq.push(make_pair(dist[v.first], v.first));
            }
        }
    }
    path.assign(n, vector<int>());
    path[0] = {0};
    for (int i = 1; i < n; i++)
    {
        find_path(i, parent, path[i]);
    }
    /********************************************/
    int wel_socket_fd, client_socket_fd, port_number;
    socklen_t clilen;
    struct sockaddr_in serv_addr_obj, client_addr_obj;
    /////////////////////////////////////////////////////////////////////////
    /* create socket */
    /*
    The server program must have a special door—more precisely,
    a special socket—that welcomes some initial contact
    from a client process running on an arbitrary host
    */
    // get welcoming socket
    // get ip,port
    /////////////////////////
    wel_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (wel_socket_fd < 0)
    {
        perror("ERROR creating welcoming socket");
        exit(-1);
    }

    //////////////////////////////////////////////////////////////////////
    /* IP address can be anything (INADDR_ANY) */
    bzero((char *)&serv_addr_obj, sizeof(serv_addr_obj));
    port_number = PORT_ARG;
    serv_addr_obj.sin_family = AF_INET;
    // On the server side I understand that INADDR_ANY will bind the port to all available interfaces,
    serv_addr_obj.sin_addr.s_addr = INADDR_ANY;
    serv_addr_obj.sin_port = htons(port_number); // process specifies port

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    /* bind socket to this port number on this machine */
    /*When a socket is created with socket(2), it exists in a name space
       (address family) but has no address assigned to it.  bind() assigns
       the address specified by addr to the socket referred to by the file
       descriptor wel_sock_fd.  addrlen specifies the size, in bytes, of the
       address structure pointed to by addr.  */

    // CHECK WHY THE CASTING IS REQUIRED
    if (bind(wel_socket_fd, (struct sockaddr *)&serv_addr_obj, sizeof(serv_addr_obj)) < 0)
    {
        perror("Error on bind on welcome socket: main");
        exit(-1);
    }
    //////////////////////////////////////////////////////////////////////////////////////

    /* listen for incoming connection requests */

    listen(wel_socket_fd, MAX_CLIENTS);
    cout << "Server has started listening on the LISTEN PORT" << endl;
    clilen = sizeof(client_addr_obj);
    /**************Creating edge sockets*************************/

    while (1)
    {
        /* accept a new request, create a client_socket_fd */
        /*
        During the three-way handshake, the client process knocks on the welcoming door
of the server process. When the server “hears” the knocking, it creates a new door—
more precisely, a new socket that is dedicated to that particular client.
        */
        // accept is a blocking call
        printf("Waiting for a new client to request for a connection\n");
        client_socket_fd = accept(wel_socket_fd, (struct sockaddr *)&client_addr_obj, &clilen);
        if (client_socket_fd < 0)
        {
            perror("ERROR while accept() system call occurred in SERVER");
            exit(-1);
        }
        printf(BGRN "New client connected from port number %d and IP %s \n" ANSI_RESET, ntohs(client_addr_obj.sin_port), inet_ntoa(client_addr_obj.sin_addr));
        pthread_t node_threads[total_vertices];
        for (int i = 0; i < total_vertices; i++)
        {
            int index_m = i;
            pthread_create(&node_threads[i], NULL, thread_func, &index_m);
            sleep(1);
        }
        handle_connection(client_socket_fd);
        for (int i = 0; i < total_vertices; i++)
        {
            pthread_join(node_threads[i], NULL);
        }
    }

    close(wel_socket_fd);
    return 0;
}
