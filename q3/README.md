# Assignment 5: Multi-Threading 
# Report for q3
# M22CS3.301: Operating Systems and Networks
# Question - 3: Internet Routing
# Gowlapalli Rohit
>##### All these commands are tested on Ubuntu Version 20.04.3 LTS (Focal Fossa) 
```
2021101113_assign5
├── q1
│   ├── q1.c
│   └── README
├── q2
│   ├── q2.c
│   └── README
└── q3
    ├── client.cpp
    ├── README
    └── server.cpp
```
---------------------------------------------------------------

* > Commands to be executed

    >* Terminal 1

     ``g++ server.cpp -lpthread -o server``

     ``./server``
    >* Terminal 2

     ``g++ client.cpp -o client ``

     ``./client``
### Threads are used in this problem to simulate nodes as individual servers in the Network ; to solve it , we classify the entities in the problem 
### An entity is an integral part of the problem and have an independent existence. There are single / multiple instances of each of these entities. 
### Here Nodes are entities ; We have multiple instances of entities and we want them to fecilitate concurrency in network communication . Hence , we use threads for their execution.
### All requests to the server (pt/send) are to be executed from Terminal 2
* > Command type
    * > pt
        ```
        dest forw delay 
        1    1      15 
        2    1      30 
        3    3      40

        ```
        ### Example output in Terminal-2
        ### If a node is not reachable from the source ,then forw = none and delay=INF=1000000000
        ### pt prints the routing table of the node 0. The routing table consists of the destination node, delay and the node to which the packet needs to be forwarded to.
    * > send 1 Ping!
        ```
        Data received at node: 1 : Source : 0; Destination : 1; Fowarded_Destination : None; Message : “Ping!"; 
        ```
        ### Example output in Terminal-1
        ### send send sends a message to node 0 which will be sent through the network to the destination node.
        ### Message mechanism starts only if the node is reachable from the source , else an error is displayed
        ### The delay between edges is simulated using sleep and async()
### djikstras algorithm is used to find the shortest distances of nodes from node 0
```cpp
void find_path(int current_vertex, vector<int> &parent, vector<int> &path)
{
    if (current_vertex == -1)
    {
        return;
    }
    find_path(parent[current_vertex], parent, path);
    path.push_back(current_vertex);
}
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
```
### path stores the shortest path from node 0 to the respective nodes as a sequence of node indexes
### dist stores distances from node 0 to respective nodes initialised to INF
### port_number_map assigns a unique valid port-number to each node server
### adj is a adjacency list storing edges 
```cpp
    total_edges = m;
    total_vertices = n;
    adj.assign(n, vector<pair<int, int>>());
    int current_port_number = 8002;
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
        adj[b].push_back({a, d});
    }
```
### Socket API lets 2 processes in different machines to communicate with each other over the TCP/IP network stack
### Socket is a door a process/thread needs to use if it needs to communicate with other processes
### TCP sockets guarantee congestion control and reliable delivery
### Regarding the send command ,the parent thread in main function of server is allocated port number as PORT_ARG which recieves message from the client ,Now this parent thread sends the recieved message to Node 0 and Node 0 sends message to nodes(in orderly fashion) corresponding to the path related to reach destination 
### Each node is given a unique socket_id through which it listens to messages sent to it and is also allocated a unique port number > 8001
### Each node sends message to socket corresponding to the forwarded node and the forwarded node reads recieved message from this socket and continues message communication
### pthreads[total_vertices] is declared
``` c
pthread_t node_threads[total_vertices];
    for (int i = 0; i < total_vertices; i++)
    {
        int index_m = i;
        pthread_create(&node_threads[i], NULL, thread_func, &index_m);
        sleep(1);
    }
```
### thread_func() handles message communication from an arbitrary node to correspinding node and correspinding node to forwarded node
```c
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
    while (1)
    {
        listen(wel_socket_fd, MAX_CLIENTS);
        printf(BCYN);
        cout << "Server"
             << "Node " << index_of_thread << " has started listening on the LISTEN PORT" << endl;
        printf(ANSI_RESET);
        clilen = sizeof(client_addr_obj);
        client_socket_fd = accept(wel_socket_fd, (struct sockaddr *)&client_addr_obj, &clilen);
        if (client_socket_fd < 0)
        {
            perror("ERROR while accept() system call occurred in SERVER");
            exit(-1);
        }
        printf(BGRN "New client connected from port number %d and IP %s \n" ANSI_RESET, ntohs(client_addr_obj.sin_port), inet_ntoa(client_addr_obj.sin_addr));
        string s = handle_thread_connection(client_socket_fd);
        if (index_of_thread == destination)
        {
            printf(BYEL);
            cout << "Data received at node: " << index_of_thread << ": Source: " << source << "; Destination :" << destination << "; Forwarded_Destination : "
                 << " None ; Message :"
                 << "\"" << s << "\"" << endl;
            printf(ANSI_RESET);
            current_index = 0;
        }
        else
        {
            printf(BMAG);
            current_index++;
            int next_index = path[destination][current_index];
            int file_descriptor = socket_map[next_index];
            cout << "Data received at node: " << index_of_thread << ": Source: " << source << "; Destination :" << destination << "; Forwarded_Destination : "
                 << next_index << "; Message :"
                 << "\"" << s << "\"" << endl;
            printf(ANSI_RESET);
            struct sockaddr_in server_obj;
            int socket_fd = get_socket_fd(&server_obj, port_number_map[next_index]);
            // cout << "delay to send from " << index_of_thread << " to " << next_index << " is " << adj[index_of_thread]
            int delay_index = 0;
            for (int i = 0; i < adj[index_of_thread].size(); i++)
            {
                if (adj[index_of_thread][i].first == next_index)
                {
                    delay_index = i;
                    break;
                }
            }
            send_string_on_socket(socket_fd, s, adj[index_of_thread][delay_index].second);
            async(launch::async, send_string_on_socket, socket_fd, s, adj[index_of_thread][delay_index].second);
        }
    }
    close(wel_socket_fd);
    return 0;
}
```
### global declaration socket_map fecilitates the usage of sockets from remote threads in the network
### handle_connection parses input string recieved from client and sends appropriate messages to nodes incase of send command and sends routing table to client incase of pt command and returns an ack-message to client incase of arbitrary message
```c
void handle_connection(int client_socket_fd)
{
    int received_num, sent_num;
    int ret_val = 1;
    while (true)
    {
        string cmd;
        tie(cmd, received_num) = read_string_from_socket(client_socket_fd, buff_sz);
        ret_val = received_num;
        if (ret_val <= 0)
        {
            printf("Server could not read msg sent from client\n");
            goto close_client_socket_ceremony;
        }
        printf(BRED);
        cout << "Client sent : " << cmd << endl;
        printf(ANSI_RESET);
        if (cmd == "exit")
        {
            cout << "Exit pressed by client" << endl;
            goto close_client_socket_ceremony;
        }
        string msg_to_send_back = "Ack :" + cmd;
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
            string title_string = "\ndest";
            title_string += '\t';
            title_string += "forw";
            title_string += '\t';
            title_string += "delay\n";
            string buffer[total_vertices];
            for (int i = 1; i < total_vertices; i++)
            {
                if (path[i].size() != 0)
                {
                    buffer[i] = to_string(i) + '\t' + to_string(path[i][1]) + '\t' + to_string(dist[i]) + '\n';
                    title_string += buffer[i];
                }
                else
                {
                    buffer[i] = to_string(i) + '\t' + "None" + '\t' + to_string(dist[i]) + '\n';
                    title_string += buffer[i];
                }
            }
            int sent_to_client = send_string_on_socket(client_socket_fd, title_string, 0);
        }
        else if (argument_string.size() == 3 && argument_string[0] == "send" && isNumber(argument_string[1]) && (stoi(argument_string[1]) < total_vertices) && (stoi(argument_string[1]) >= 0))
        {
            destination = stoi(argument_string[1]);
            struct sockaddr_in server_obj;
            int socket_fd = get_socket_fd(&server_obj, port_number_map[0]);
            int start_client = send_string_on_socket(socket_fd, argument_string[2], 0);
            if (start_client == -1)
            {
                perror("Error while writing to start client.Seems socket has been closed");
                goto close_client_socket_ceremony;
            }
        }
        int sent_to_client = send_string_on_socket(client_socket_fd, msg_to_send_back, 0);
        if (sent_to_client == -1)
        {
            perror("Error while writing to client. Seems socket has been closed");
            goto close_client_socket_ceremony;
        }
        return;
    }
close_client_socket_ceremony:
    close(client_socket_fd);
    printf(BRED "Disconnected from client" ANSI_RESET "\n");
    // return NULL;
}
```
### Since global declaration of destination variable creates problems when using 2 different destined send commands ,it is passed as paramter
## Handling server failures
### A connected socket should be immediately closed when any Read, Write, or Disconnect operation error is detected. Socket errors usually indicate a problem with the underlying connection (or possibly the network itself), and the socket should be considered unstable and be closed.
### Closing a socket almost never raises an exception. Only the “fatal” exceptions (OutOfMemory, StackOverflow, ThreadAbort, and possibly others in future CLR versions) can ever be raised from Socket.Close. This makes it safe to call without requiring a try/catchall.
### Bind and Connect failures are not uncommon. Depending on the application, one may either inform the user and exit, or retry at a later time (see below).
### Listen or Shutdown failures are extremely rare but still possible. These failures may indicate a shortage of OS resources. For the Listen operation, consider notifying the user and then exiting; alternatively, close the listening socket and retry at a later time (see below). For the Shutdown operation, close the socket.
### Accept operations may also fail (though this may be surprising to some). In this case, the server should simply continue accepting new connections. This may be caused by a client socket program unexpectedly exiting.

