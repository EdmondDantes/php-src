--TEST--
Async non-block TCP communication within the same script
--FILE--
<?php

/**
 * This test demonstrates that creating sockets in non-blocking mode does not cause fiber context switching.
 * As expected, PHP function calls immediately return control.
*/

define ('ASYNC_TCP_SERVER', "tcp://127.0.0.1:8964");

Async\async(function () {
    // Fiber acting as the server
    $server = stream_socket_server(ASYNC_TCP_SERVER, $errno, $errstr);
    if (!$server) {
        die("Failed to create server: $errstr ($errno)");
    }

    stream_set_blocking($server, false);

    echo "Server: Listening on ".ASYNC_TCP_SERVER."\n";
    $clientSocket = stream_socket_accept($server);

    if ($clientSocket) {
        echo "Server: Client connected\n";
        $data = fread($clientSocket, 1024);
        echo "Server: Received data\n";

        $response = "Hello from server";
        fwrite($clientSocket, $response);
        echo "Server: Response sent\n";

        fclose($clientSocket);
    }

    fclose($server);
});

Async\async(function () {
    // Fiber acting as the client
    usleep(1); // Ensure the server is ready

    $client = stream_socket_client(ASYNC_TCP_SERVER, $errno, $errstr);

    if (!$client) {
        die("Failed to connect to server: $errstr ($errno)");
    }

    stream_set_blocking($client, false);

    echo "Client: Connected to server\n";
    $message = "Hello from client";
    echo "Client: Message sent\n";
    fwrite($client, $message);

    $response = fread($client, 1024);
    echo "Client: Received response\n";

    fclose($client);
});

echo "Start\n";
Async\launchScheduler();
echo "End\n";

?>
--EXPECT--
Start
Server: Listening on tcp://127.0.0.1:8964
Server: Client connected
Server: Received data
Server: Response sent
Client: Connected to server
Client: Message sent
Client: Received response
End
