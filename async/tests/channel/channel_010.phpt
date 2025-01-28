--TEST--
Channel: getUsed()
--FILE--
<?php

Async\run(function() {
    $channel = new Async\Channel();

    Async\run(function() use($channel) {
        while ($channel->waitUntilReadable()) {
            $data = $channel->tryReceive();
            echo "receive: $data\n";
            echo "channel used out: " . $channel->getUsed() . "\n";
        }
    });

    for ($i = 0; $i < 4; $i++) {
        if(false === $channel->waitUntilWritable()) {
            break;
        }

        echo "send: event data $i\n";
        $data = $channel->trySend("event data $i");
        echo "channel used in: " . $channel->getUsed() . "\n";
    }
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
send: event data 0
channel used in: 1
send: event data 1
channel used in: 2
send: event data 2
channel used in: 3
send: event data 3
channel used in: 4
receive: event data 0
channel used out: 3
receive: event data 1
channel used out: 2
receive: event data 2
channel used out: 1
receive: event data 3
channel used out: 0
end
