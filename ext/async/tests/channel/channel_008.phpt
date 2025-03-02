--TEST--
Channel: waitUntilWritable / waitUntilReadable + trySend + tryReceive
--FILE--
<?php

Async\run(function() {
    $channel = new Async\Channel();

    Async\run(function() use($channel) {
        while ($channel->waitUntilReadable()) {
            $data = $channel->tryReceive();
            echo "receive: $data\n";
        }
    });

    for ($i = 0; $i < 4; $i++) {
        if(false === $channel->waitUntilWritable()) {
            break;
        }

        echo "send: event data $i\n";
        $data = $channel->trySend("event data $i");
    }
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
send: event data 0
send: event data 1
send: event data 2
send: event data 3
receive: event data 0
receive: event data 1
receive: event data 2
receive: event data 3
end
