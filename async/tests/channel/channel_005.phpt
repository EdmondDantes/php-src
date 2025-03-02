--TEST--
Channel: two receivers and one producer
--FILE--
<?php

Async\run(function() {
    $channel = new Async\Channel();

    Async\run(function() use($channel) {

        while (($data = $channel->receiveOrNull()) != null) {
            echo "receive by 1: $data\n";
        }
    });

    Async\run(function() use($channel) {
        while (($data = $channel->receiveOrNull()) != null) {
            echo "receive by 2: $data\n";
        }
    });

    for ($i = 0; $i < 5; $i++) {
        $channel->send("event data $i");
    }
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
receive by 1: event data 0
receive by 1: event data 1
receive by 2: event data 2
receive by 1: event data 3
receive by 2: event data 4
end
