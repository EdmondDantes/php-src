--TEST--
Channel: waitUntilReadable + receiveAsync
--FILE--
<?php

Async\run(function() {
    $channel = new Async\Channel();

    Async\run(function() use($channel) {
        while ($channel->waitUntilReadable()) {
            $data = $channel->receiveAsync();
            echo "receive: $data\n";
        }
    });

    for ($i = 0; $i < 4; $i++) {
        $channel->send("event data $i");
    }
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
receive: event data 0
receive: event data 1
receive: event data 2
receive: event data 3
end
