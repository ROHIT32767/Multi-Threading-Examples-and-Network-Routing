    // while (1)
    // {
    //     client_socket_fd = accept(wel_socket_fd, (struct sockaddr *)&client_addr_obj, &clilen);
    //     if (client_socket_fd < 0)
    //     {
    //         perror("ERROR while accept() system call occurred in SERVER");
    //         exit(-1);
    //     }
    //     string s1 = handle_connection(client_socket_fd);
    //     if (index_of_thread == destination)
    //     {
    //         cout << "Data received at node: " << index_of_thread << ": Source: " << source << "; Destination :" << destination << "; Forwarded_Destination : "
    //              << " None ; Message :"
    //              << "\"" << s1 << "\"" << endl;
    //     }
    //     else
    //     {
    //         current_index++;
    //         int next_index = path[destination][current_index];
    //         int file_descriptor = M[make_pair(index_of_thread, next_index)];
    //         cout << "Data received at node: " << index_of_thread << ": Source: " << source << "; Destination :" << destination << "; Forwarded_Destination : "
    //              << "; Message :"
    //              << "\"" << s1 << "\"" << endl;
    //         send_string_on_socket(file_descriptor, s1);
    //     }
    // }
    // close(wel_socket_fd);